# Makefile内容通常有以下3部分组成
# <目标名称>:<前置依赖>
# \t<需要执行的命令>

#定义变量objects
objects := hello.o\
	main.o

				
# 放在第一个的是默认目标
# 目标是编译出main文件 依赖hello.o和main.o文件
# 编译的命令是gcc hello.o main.o -o mian
main: $(objects)
	gcc $(objects) -o main

#目标是main.o 依赖main.c 和hello.h
#编译命令是gcc -c main.c
#main.o: main.c hello.h
#	gcc -c main.c 
main.o:  hello.h

#目标是hello.o 依赖hello.c 和hello.h
#编译命令是gcc -c hello.c
#hello.o: hello.c hello.h
#	gcc -c hello.c 

hello.o: hello.h

clean:
	-rm main $(objects)