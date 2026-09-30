/*
 * 验证:PTHREAD_CANCEL_DISABLE 期间的取消请求到底去哪儿了?
 *
 * A 阶段: DISABLE 状态下被 cancel -> 线程照常跑完, join 拿到的是"正常返回值"
 * B 阶段: DISABLE + 中途把类型改成 ASYNCHRONOUS -> 仍然不被取消(类型只在
 *          ENABLE 状态才起作用); 一旦重新 ENABLE, 挂起的请求立刻生效
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* 用一个能认出来的非 NULL 返回值, 避免和 NULL(0) 混淆 */
#define NORMAL_RET ((void *)0x1234)

static void *task_a(void *arg)
{
  /* 先关掉取消响应, 再干活, 保证主线程的 cancel 一定落在 DISABLE 状态里 */
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  printf("[A] cancel disabled (type still DEFERRED)\n");
  fflush(stdout);

  sleep(1); /* 主线程在 sleep 期间调用 pthread_cancel */

  printf("[A] 1s later: still alive, cancel request is only PENDING\n");
  fflush(stdout);

  /* 正常返回: 挂起的请求不会在这里"补刀" */
  return NORMAL_RET;
}

static void *task_b(void *arg)
{
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  printf("[B] cancel disabled\n");
  fflush(stdout);

  /* 在 DISABLE 状态下改类型: 毫无作用, 请求依然只是挂起 */
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  printf("[B] type changed to ASYNCHRONOUS (but state is DISABLE)\n");
  fflush(stdout);

  sleep(1); /* 主线程在 sleep 期间调用 pthread_cancel */

  printf("[B] 1s later: still alive\n");
  fflush(stdout);

  /* 重新打开取消响应: 挂起的请求此刻才被"接受" */
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  printf("[B] !!! you should NOT see this line !!!\n");
  fflush(stdout);
  return NORMAL_RET;
}

int main(void)
{
  pthread_t tid;
  void *res;

  printf("===== A: DISABLE + DEFERRED, 正常返回 =====\n");
  pthread_create(&tid, NULL, task_a, NULL);
  usleep(200000); /* 确保子线程已进入 DISABLE 状态 */
  pthread_cancel(tid);
  pthread_join(tid, &res);
  printf("[main] res = %p  %s\n\n", res,
         res == PTHREAD_CANCELED ? "PTHREAD_CANCELED" : "正常返回值");

  printf("===== B: DISABLE + ASYNC, 之后重新 ENABLE =====\n");
  pthread_create(&tid, NULL, task_b, NULL);
  usleep(200000);
  pthread_cancel(tid);
  pthread_join(tid, &res);
  printf("[main] res = %p  %s\n", res,
         res == PTHREAD_CANCELED ? "PTHREAD_CANCELED" : "正常返回值");

  return 0;
}
