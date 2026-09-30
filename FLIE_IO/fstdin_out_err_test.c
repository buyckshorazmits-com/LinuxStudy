#include<stdio.h>
#include<stdlib.h>
int main(int argc, char const *argv[])
{
  //malloc 动态分配内存  也可以用char ch[100]接受数据
  char *ch=malloc(100);
  // char ch1[100];

  /*
  stdin :标准输入FILE 标准输入,默认接键盘* 
  */
  fgets(ch,100,stdin);//fgets(目的地, 容量, 来源)

  printf("你好：%s",ch);
  /*
    stdout: 标准输出FILE * 写入这个文件流会将数据输出到控制台
    printf底层就是使用的这个注释说明了 printf 底层就是往 stdout 这个文件流写数据,所以屏幕上能看到。


    */
  fputs(ch, stdout);
  /*
    stderr: 错误输出FILE * 标准错误,默认也接屏幕，一般用于输出错误日志
    */
  fputs(ch,stderr);
  return 0;
}
