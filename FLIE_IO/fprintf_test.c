#include<stdio.h>
int main()
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
  char *filename="io.text";
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
* FILE *__restrict __stream:需要打开的文件
* const char *__restrict __fmt：带格式化的长字符字符串
* fprintf (FILE *__restrict __stream, const char *__restrict __fmt, ...)
*  ...可变参数：填入格式化的长字符串
* return:成功返回写入的字符的个数  不包含换行符  失败返回EOF
*/
  char *name="qwrrty";
  int fprintS=fprintf(ioFile,"哎呀，那边窗户透了什么光？\n那是东方,而你则是太阳！\n升起吧，骄阳，让骄傲的月亮嫉妒！\n\t\t %s",name);

  if(fprintS==EOF)
  {
    printf("文件写入失败\n");
  }
  else
  {
    printf("文件%d写入成功\n",fprintS);
  }


  int result=fclose(ioFile);
  if(result==EOF)
  {
    printf("关闭文件失败\n");
    
  }
  else if(result ==0)
  {
    printf("关闭文件成功\n");
  }
  return 0;
}