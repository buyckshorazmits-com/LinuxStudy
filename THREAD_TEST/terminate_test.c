#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<string.h>

//定义结果结构体类型
 typedef struct Result
 {
  char *p;
  int len;
 }Result;
 /**
 * 红玫瑰
 * 
 * void *argv: 指针，可以传递的参数  这里用一个字符表示她的代号
 * return: void* 结果结构体指针 返回结局
 */
void *red_thread(void *argv)
{
   Result*result=malloc(sizeof(Result));
   //强制类型转换  argv 是void *必须先转成具体的指针类型 这里转成char*
   char code=*((char*)argv);

   char *ans=malloc(101);
   while(1)
   {
    fgets(ans,100,stdin);
    if(ans[0]==code)
    {
      //接收到了对应的信息
      free(ans);
      printf("红玫瑰离开了\n");
      char *redAns=strdup("红玫瑰独自去了纽约\n");
      result->p=redAns;
      result->len=strlen(redAns);
      //线程结束 输出返回值
      pthread_exit((void*)result);
    }else{
       printf("红玫瑰还在等你\n");
    }
   
   }
}
/**
 * 白玫瑰
* 
 * void *argv: 指针，可以传递的参数  这里用一个字符表示她的代号
 * return: void* 结果结构体指针 返回结局
 */

void *white_thread(void *argv)
{                                                
  Result*result=malloc(sizeof(Result));
  char code=*((char *)argv);
   // 存放回信
  char *ans=malloc(101);
  while(1)
  {
     //接收到了对应的信息
    fgets(ans,100,stdin);
    if(ans[0]==code)
    {
      free(ans);
      printf("白玫瑰离开了\n");
      char *whiteAns=strdup("白玫瑰独自去了伦敦\n");
      result->p=whiteAns;
      result->len=strlen(whiteAns);
      //结束线程  输出返回值
      pthread_exit((void *)result);

    }else
    {
      printf("白玫瑰还在等你\n");
    }

  }
}
int main()
{
  pthread_t pid_red;
  pthread_t pid_white;
  
  char red_code='r';
  char white_code='w';

  Result *red_result=NULL;
  Result *white_result=NULL;
  //创建红玫瑰线程
  pthread_create(&pid_red,NULL,red_thread,&red_code);
  //创建白玫瑰线程
  pthread_create(&pid_white,NULL,white_thread,&white_code);
  //等待红玫瑰结果
  pthread_join(pid_red,(void**)&red_result);

  printf("红玫瑰故事结局:%s\n",red_result->p);

  //释放内存
  free(red_result->p);
  free(red_result);

  //获取求素数结果
  pthread_join(pid_white,(void **)&white_result);
  printf("白玫瑰故事结局:%s \n",white_result->p);
  //释放内存
  free(white_result->p);
  free(white_result);
  
  return 0;
}