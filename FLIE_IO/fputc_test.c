#include <stdio.h>

int main(int argc, char const *argv[])
{
  /* 打开文件
    char *__restrict __filename: 字符串表示要打开文件的路径和名称
    char *__restrict __modes: 字符串表示访问模式
        (1)"r": 只读模式 没有文件打开失败
        (2)"w": 只写模式 存在文件写入会清空文件,不存在文件则创建新文件
        (3)"a": 只追加写模式 不会覆盖原有内容 新内容写到末尾，如果文件不存在则创建
        (4)"r+": 读写模式 文件必须存在 写入是从头一个一个覆盖
        (5)"w+": 读写模式 可读取,写入同样会清空文件内容，不存在则创建新文件
        (6)"a+": 读写追加模式 可读取,写入从文件末尾开始，如果文件不存在则创建
    return: FILE * 结构体指针 表示一个文件
    FILE *fopen (const char *__restrict __filename,
            const char *__restrict __modes)
    */
  char *filename ="io.text";
  FILE *ioFile=fopen(filename,"a+");
  
  if(ioFile==NULL)
  {
   printf("文件打开失败\n");
  }
  else
  {
    printf("文件打开成功\n");
  }

   /*
    写入文件一个字符 读写权限在fopen函数
    int __c: 写入的char按照AICII值写入 可提前声明一个char
    FILE *__stream: 要写入的文件,写在哪里取决于访问模式
    return: 成功返回char的值 失败返回EOF
    int fputc (int __c, FILE *__stream)
    */
   int putCR=fputc(97,ioFile);

   if(putCR==EOF)
   {
    printf("写入字符失败\n");
   }
   else
   {
    printf("写入字符成功\n");
   }
  return 0;
  /*
    FILE *__stream: 需要关闭的文件
    return: 成功返回0 失败返回EOF(负数) 通常失败会造成系统崩溃
    int fclose (FILE *__stream)
    */
  int result=fclose(ioFile);  
  if(result!=0)
  {
    printf("文件关闭失败\n");
  }
  return 0;
}
