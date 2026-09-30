#include<stdio.h>
int main()
{
  /*
    FILE *__restrict __stream: 读取的文件
    char *__restrict __format: 读取的匹配表达式
    ...: 变长参数列表 用于接收匹配的数据
    return: 成功返回参数的个数  失败返回0 报错或结束返回EOF
    int fscanf (FILE *__restrict __stream,  const char *__restrict __format, ...)
    */

  FILE *userFile=fopen("user.text","r");

   if(userFile==NULL)
  {
    printf("文件打开失败,不能打开不存在的文件\n");
    
  }
  else
    {
      printf("文件打开成功\n");

    }
  char name[50];
  int age;
  char wife[50];
  int scanR;
    while ((scanR = fscanf(userFile, "%49s %d %49s", name, &age, wife)) == 3)
    {
      printf("%s 在%d 岁爱上了%s\n", name, age, wife);
    }
  int result=fclose(userFile);
  if(result !=0)
  {
    printf("文件关闭失败\n");
    return 1;
  }
  return 0;
}