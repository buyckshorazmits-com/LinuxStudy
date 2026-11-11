# Linux 进程间通信（IPC）——Unix Domain Socket

## 一、Unix Domain Socket 是什么？

### 1.1 什么是 IPC？

IPC（Inter-Process Communication），即进程间通信。

在 Linux 系统中，每个进程都有自己独立的虚拟地址空间，一个进程不能直接访问另一个进程的普通变量。

例如：

- 进程 A：负责采集传感器数据。
- 进程 B：负责处理传感器数据。

如果进程 A 想把采集的数据发送给进程 B，就需要使用进程间通信机制（IPC）。

Linux 常见的 IPC 方式：

| IPC 方式 | 主要作用 |
|---|---|
| 无名管道（pipe） | 通常用于具有亲缘关系的进程通信 |
| 有名管道（FIFO） | 可以用于无亲缘关系的进程通信 |
| 消息队列 | 通过消息的形式交换数据 |
| 共享内存 | 多个进程访问同一块共享内存 |
| 信号量 | 实现进程间同步与互斥 |
| Unix Domain Socket | 通过套接字实现本机进程间通信 |

### 1.2 Unix Domain Socket 的概念

Unix Domain Socket（Unix 域套接字）是一种用于同一台主机上不同进程之间通信的机制。

它最初源于网络 Socket 编程，后来被广泛用于本地进程间通信。

与 TCP、UDP 不同，Unix Domain Socket 不需要通过 IP 地址和端口号寻找通信对象，而是可以通过本地套接字地址进行通信。

例如：

TCP 客户端访问服务器：

```text
127.0.0.1:6666
```

Unix Domain Socket 客户端访问服务器：

```text
/tmp/server.sock
```

两种方式的主要区别：

- TCP：通过 IP 地址和端口号定位通信对象。
- Unix Domain Socket：通过本地套接字地址定位通信对象。

Unix Domain Socket 不需要经过 TCP/IP 网络协议栈，而是通过 Linux 内核提供的本地套接字机制传输数据。

因此，在本机进程间通信时，通常具有较低的通信开销。

**注意：Unix Domain Socket 不是 TCP、UDP 之外的另一种网络传输层协议，而是 Linux/POSIX 提供的一种本地 IPC 机制。**

---

## 二、Unix Domain Socket 地址结构体

### 2.1 sockaddr_un 结构体

之前学习 TCP、UDP 时，使用的是：

```c
struct sockaddr_in server_addr;
```

其中：

- `sin_family`：地址族。
- `sin_addr`：IP 地址。
- `sin_port`：端口号。

而 Unix Domain Socket 使用：

```c
struct sockaddr_un
{
    sa_family_t sun_family;
    char sun_path[108];
};
```

需要包含：

```c
#include <sys/socket.h>
#include <sys/un.h>
```

两个成员的含义：

| 成员 | 作用 |
|---|---|
| sun_family | 指定地址族，使用 AF_UNIX |
| sun_path | 保存本地套接字地址 |

例如：

```c
struct sockaddr_un server_addr;

memset(&server_addr, 0, sizeof(server_addr));

server_addr.sun_family = AF_UNIX;

strcpy(server_addr.sun_path, "/tmp/server.sock");
```

其中：

```c
server_addr.sun_family = AF_UNIX;
```

表示使用 Unix Domain Socket。

```c
strcpy(server_addr.sun_path, "/tmp/server.sock");
```

表示指定本地套接字地址。

### 2.2 sockaddr_in 与 sockaddr_un 对比

| 对比 | IPv4 Socket | Unix Domain Socket |
|---|---|---|
| 地址结构体 | sockaddr_in | sockaddr_un |
| 地址族 | AF_INET | AF_UNIX |
| 地址成员 | sin_addr | sun_path |
| 端口号 | sin_port | 不需要 |
| 地址形式 | IP + 端口 | 本地套接字地址 |

可以简单理解为：

- `sockaddr_in`：通过 IP 地址和端口号找到通信对象。
- `sockaddr_un`：通过本地套接字地址找到通信对象。

### 2.3 sun_path 的长度

Linux 中常见的定义：

```c
char sun_path[108];
```

表示路径字段最多占用 108 字节。

注意是字节，不是字符。

编写程序时，应检查路径长度，避免超出数组范围。

对于普通路径名套接字，可以使用下面的方式计算实际地址长度：

```c
socklen_t addr_len =
    offsetof(struct sockaddr_un, sun_path)
    + strlen(server_addr.sun_path) + 1;
```

需要包含：

```c
#include <stddef.h>
```

---

## 三、Unix Domain Socket 的三种地址类型

Unix Domain Socket 主要有三种地址形式：

1. 路径名套接字（Pathname Socket）
2. 未命名套接字（Unnamed Socket）
3. 抽象套接字（Abstract Socket）

### 3.1 路径名套接字

这是最常见的 Unix Domain Socket 使用方式。

例如：

```text
/tmp/server.sock
```

服务器通过 `bind()` 将 Socket 绑定到这个路径。

```c
server_addr.sun_family = AF_UNIX;

strcpy(server_addr.sun_path, "/tmp/server.sock");

bind(sockfd,
     (struct sockaddr *)&server_addr,
     addr_len);
```

绑定成功后，Linux 文件系统中通常会出现一个 Socket 类型的文件。

可以通过：

```bash
ls -l /tmp/server.sock
```

查看。

例如：

```text
srwxr-xr-x 1 user user 0 server.sock
```

最前面的 `s` 表示这是一个 Socket 类型的文件。

注意：

这个文件不是用来存储通信数据的普通文件，而是用于标识和访问本地套接字的文件系统对象。真正的数据传输仍然由 Linux 内核管理。

**为什么需要 unlink()？**

如果服务器退出后没有删除套接字路径，下一次启动时再次执行 `bind()`，可能出现：

```text
Address already in use
```

因此，服务器通常需要清理自己创建的旧套接字路径。

```c
unlink("/tmp/server.sock");
```

服务器退出时也可以执行：

```c
unlink("/tmp/server.sock");
```

注意：不能不加判断地删除其他正在运行的服务所使用的 Socket 路径。

### 3.2 未命名套接字

未命名套接字是通过 `socketpair()` 创建的一对已经相互连接的 Socket 端点。

它不需要：

- IP 地址；
- 端口号；
- 文件系统路径。

创建方式：

```c
int sv[2];

socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
```

执行成功后，系统会返回两个文件描述符：

```c
sv[0];
sv[1];
```

这两个文件描述符分别代表一对已经连接的 Socket 端点，可以双向通信。

例如：

```c
// 通过 sv[0] 发送数据
write(sv[0], "Hello", 5);

// 通过 sv[1] 接收数据
read(sv[1], buf, sizeof(buf));
```

反过来也可以：

```c
// 通过 sv[1] 发送数据
write(sv[1], "World", 5);

// 通过 sv[0] 接收数据
read(sv[0], buf, sizeof(buf));
```

#### 为什么经常与 fork() 一起使用？

因为 `fork()` 创建子进程后，子进程会继承父进程已经打开的文件描述符。

因此，可以先使用 `socketpair()` 创建一对 Socket，再通过 `fork()` 创建子进程。

父子进程分别使用其中一个 Socket 端点，实现双向通信。

通信过程：

```text
第一步：父进程创建 Socket 对

socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

第二步：创建子进程

fork();

第三步：父子进程分别关闭不使用的端点

父进程：
    close(sv[1]);
    使用 sv[0];

子进程：
    close(sv[0]);
    使用 sv[1];

第四步：父子进程通过 Socket 交换数据
```

示意图：

```text
       父进程                         子进程
    ┌──────────┐                  ┌──────────┐
    │          │                  │          │
    │  sv[0]   │◄────────────────►│  sv[1]   │
    │          │                  │          │
    └──────────┘                  └──────────┘
                  双向通信
```

注意：

父子进程中的文件描述符虽然数值相同，但它们引用的是同一对内核 Socket 端点，并不是重新创建了新的通信通道。

父进程关闭 `sv[1]`，子进程关闭 `sv[0]`，是为了避免不必要的文件描述符引用，保证连接关闭行为正确。

**重点：**

`socketpair()` 已经创建了相互连接的两个套接字，因此不需要再调用：

```c
bind();
listen();
accept();
connect();
```

### 3.3 抽象套接字

抽象套接字是 Linux 特有的一种地址形式。

普通路径名套接字：

```c
strcpy(server_addr.sun_path, "/tmp/server.sock");
```

抽象套接字：

```c
server_addr.sun_path[0] = '\0';

strcpy(server_addr.sun_path + 1, "my_server");
```

第一个字节设置为 `\0`，表示使用 Linux 抽象命名空间。

它的特点：

- 不需要在文件系统中创建 Socket 文件。
- 不需要通过 unlink() 删除文件。
- 名称由 Linux 内核管理。
- 不受普通文件系统路径权限的直接控制。

抽象套接字主要用于 Linux 系统，不应假设所有 Unix 系统都支持。

### 3.4 三种地址类型总结

| 类型 | 是否有文件系统路径 | 主要用途 |
|---|---|---|
| 路径名套接字 | 有 | 本地客户端与服务器通信 |
| 未命名套接字 | 无 | 父子进程通信 |
| 抽象套接字 | 无 | Linux 本地服务通信 |

对于初学者，建议优先掌握路径名套接字和未命名套接字。

---

## 四、Unix Domain Socket 的三种通信类型

创建 Socket 时，需要指定通信类型。

Unix Domain Socket 常见的类型有：

- SOCK_STREAM
- SOCK_DGRAM
- SOCK_SEQPACKET

### 4.1 SOCK_STREAM：流式套接字

创建方式：

```c
socket(AF_UNIX, SOCK_STREAM, 0);
```

它与 TCP 的通信模型非常相似。

主要特点：

- 面向连接。
- 支持双向通信。
- 可靠、有序地传输字节流。
- 不保留应用层消息边界。

服务器通信流程：

```c
socket();
bind();
listen();
accept();
recv();
send();
close();
```

客户端通信流程：

```c
socket();
connect();
send();
recv();
close();
```

注意：

SOCK_STREAM 是字节流通信。

例如客户端连续执行：

```c
send(sockfd, "Hello", 5, 0);

send(sockfd, "World", 5, 0);
```

服务器不一定分别接收到 Hello 和 World。

一次 recv() 可能接收到 HelloWorld，也可能只接收到其中一部分。

因此，TCP 中的粘包、拆包问题在 SOCK_STREAM 中同样存在。

### 4.2 SOCK_DGRAM：数据报套接字

创建方式：

```c
socket(AF_UNIX, SOCK_DGRAM, 0);
```

它与 UDP 的通信模型类似。

主要特点：

- 无连接式数据报通信。
- 保留数据报边界。
- 通常使用 sendto()、recvfrom()。
- 不需要 listen() 和 accept()。

服务器：

```c
socket(AF_UNIX, SOCK_DGRAM, 0);

bind(sockfd, ...);

recvfrom(sockfd, ...);

sendto(sockfd, ...);
```

客户端：

```c
socket(AF_UNIX, SOCK_DGRAM, 0);

sendto(sockfd, ...);

recvfrom(sockfd, ...);
```

注意：

Unix Domain Datagram 不经过 IP 网络，与 UDP 并不完全相同。

Linux 本地 Unix 数据报具有可靠传递和有序交付的实现特性，但不能因此认为对端应用程序一定已经处理了数据。

### 4.3 SOCK_SEQPACKET：有序分组套接字

创建方式：

```c
socket(AF_UNIX, SOCK_SEQPACKET, 0);
```

它具有以下特点：

- 面向连接。
- 支持双向通信。
- 保留消息边界。
- 保证有序交付。

可以理解为：

既具有类似 TCP 的连接管理，又能保留独立消息的边界。

不过，不同系统对该类型的支持情况需要确认。

### 4.4 三种通信类型对比

| 类型 | 是否面向连接 | 是否保留消息边界 |
|---|---|---|
| SOCK_STREAM | 是 | 否 |
| SOCK_DGRAM | 否 | 是 |
| SOCK_SEQPACKET | 是 | 是 |

---

## 五、Unix Domain Socket 的客户端与服务器通信流程

这里重点学习最常见的：

```c
AF_UNIX + SOCK_STREAM
```

### 5.1 服务器端通信流程

服务器通常执行以下操作：

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
accept()
   ↓
recv()
   ↓
send()
   ↓
close()
   ↓
unlink()
```

每个函数的作用：

| 函数 | 作用 |
|---|---|
| socket() | 创建 Unix Domain Socket |
| bind() | 将 Socket 绑定到本地路径 |
| listen() | 进入监听状态 |
| accept() | 接受客户端连接 |
| recv() | 接收客户端数据 |
| send() | 向客户端发送数据 |
| close() | 关闭套接字 |
| unlink() | 删除路径名套接字文件 |

### 5.2 客户端通信流程

```text
socket()
   ↓
配置 sockaddr_un
   ↓
connect()
   ↓
send()
   ↓
recv()
   ↓
close()
```

客户端不需要调用 bind()，通常由操作系统自动分配本地套接字地址。

### 5.3 与 TCP 通信流程对比

TCP 服务器：

```c
socket(AF_INET, SOCK_STREAM, 0);

bind();

listen();

accept();

recv();

send();
```

Unix Domain Socket 服务器：

```c
socket(AF_UNIX, SOCK_STREAM, 0);

bind();

listen();

accept();

recv();

send();
```

可以发现，两者的主要区别是：

```c
AF_INET
```

变成：

```c
AF_UNIX
```

同时，服务器地址结构体由：

```c
struct sockaddr_in
```

变成：

```c
struct sockaddr_un
```

整体通信流程非常相似。

---

## 六、Unix Domain Socket 文件权限

路径名套接字会在文件系统中创建一个 Socket 类型的文件。

例如：

```text
/tmp/server.sock
```

可以使用：

```bash
ls -l /tmp/server.sock
```

查看其权限。

例如：

```text
srwxr-x--- 1 root root 0 server.sock
```

最前面的 `s` 表示 Socket 文件类型。

Linux 中需要注意：

1. 创建套接字文件，需要对所在目录具有相应权限。
2. 连接路径名流套接字时，需要通过相关权限检查。
3. 向路径名数据报套接字发送数据时，也会进行相关权限检查。
4. 可以使用 chmod() 修改套接字文件权限。
5. 不建议实际工程中直接设置 777 权限。

可以使用：

```bash
chmod 660 /tmp/server.sock
```

修改权限。

注意：不同 Unix 系统的具体权限检查行为可能存在差异。

---

## 七、Unix Domain Socket 与 TCP、UDP 的区别

| 对比项目 | TCP | UDP | Unix Domain Socket |
|---|---|---|---|
| 主要用途 | 网络通信 | 网络通信 | 本机进程间通信 |
| 地址族 | AF_INET | AF_INET | AF_UNIX |
| 地址形式 | IP + 端口 | IP + 端口 | 本地套接字地址 |
| 是否经过 IP 网络层 | 是 | 是 | 否 |
| 是否需要建立连接 | 是 | 否 | 取决于套接字类型 |
| 数据形式 | 字节流 | 数据报 | 字节流或数据报等 |
| 是否保证可靠、有序交付 | 是 | 否 | 取决于套接字类型及系统实现 |
| listen() | 服务器需要 | 不需要 | 流式服务器需要 |
| accept() | 服务器需要 | 不需要 | 流式服务器需要 |
| 常见发送函数 | send() | sendto() | send()/sendto() |
| 常见接收函数 | recv() | recvfrom() | recv()/recvfrom() |

### 为什么 Unix Domain Socket 通常效率较高？

因为本机通信不需要经过 IP 网络层和 TCP/UDP 网络传输流程，减少了部分协议处理开销。

但是，它仍然需要 Linux 内核管理套接字、缓冲区和数据传递，并不意味着完全没有数据复制和内核开销。

---

## 八、socketpair() 实现父子进程通信

这是本节非常重要的实践内容。

### 8.1 创建 Socket 对

```c
int sv[2];

socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
```

成功后得到两个已经相互连接的文件描述符：

```c
sv[0];
sv[1];
```

它们可以双向通信。

### 8.2 为什么与 fork() 一起使用？

因为 fork() 创建子进程后，子进程可以继承父进程的文件描述符。

所以父子进程可以分别使用两个 Socket 端点。

### 8.3 示例代码

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
    int sv[2];

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
    {
        perror("socketpair");
        return -1;
    }

    pid_t pid = fork();

    if(pid < 0)
    {
        perror("fork");
        close(sv[0]);
        close(sv[1]);
        return -1;
    }

    if(pid > 0)
    {
        // 父进程
        close(sv[1]);

        const char msg[] = "Hello Child";

        write(sv[0], msg, sizeof(msg)-1);

        char buf[100] = {0};

        ssize_t count = read(sv[0], buf, sizeof(buf)-1);

        if(count > 0)
        {
            buf[count] = '\0';
            printf("父进程收到：%s\n", buf);
        }

        close(sv[0]);

        wait(NULL);
    }
    else
    {
        // 子进程
        close(sv[0]);

        char buf[100] = {0};

        ssize_t count = read(sv[1], buf, sizeof(buf)-1);

        if(count > 0)
        {
            buf[count] = '\0';
            printf("子进程收到：%s\n", buf);
        }

        const char reply[] = "Hello Parent";

        write(sv[1], reply, sizeof(reply)-1);

        close(sv[1]);
    }

    return 0;
}
```

编译：

```bash
gcc unix_socketpair.c -o unix_socketpair
```

运行：

```bash
./unix_socketpair
```

预期输出：

```text
子进程收到：Hello Child
父进程收到：Hello Parent
```

注意：

这是一份用于理解通信关系的简单示例。实际工程中还需要处理 read()/write() 的部分读写、错误返回以及连接关闭等情况。

### 8.4 为什么要分别 close()？

父进程：

```c
close(sv[1]);
```

子进程：

```c
close(sv[0]);
```

因为双方只需要使用其中一个端点。

关闭不使用的文件描述符，可以避免多余的内核资源引用影响连接生命周期管理。

---

## 九、Unix Domain Socket 的实际应用

Unix Domain Socket 常用于 Linux 本机的多进程应用。

例如嵌入式 Linux 设备：

```text
┌──────────────────────┐
│  进程 A：设备采集程序  │
│  UART / SPI / I2C     │
└──────────┬───────────┘
           │
           │ Unix Domain Socket
           │
┌──────────▼───────────┐
│  进程 B：数据处理程序  │
│  数据解析、滤波、计算  │
└──────────┬───────────┘
           │
           │ Unix Domain Socket
           │
┌──────────▼───────────┐
│  进程 C：显示控制程序  │
│  数据显示、用户控制    │
└──────────────────────┘
```

例如：

- 进程 A 通过 UART 读取传感器数据。
- 进程 B 接收数据并进行解析、滤波。
- 进程 C 接收处理结果并显示。

这些进程都运行在同一台 Linux 设备上，可以通过 Unix Domain Socket 交换数据。

### 9.1 文件描述符传递

Unix Domain Socket 还有一个比较实用的高级功能：

文件描述符传递（File Descriptor Passing）。

通过：

```c
sendmsg();
recvmsg();
```

以及：

```c
SCM_RIGHTS
```

可以将一个进程打开的文件描述符传递给另一个进程。

例如：

1. 进程 A 打开一个设备文件。
2. 进程 A 通过 Unix Domain Socket 将文件描述符发送给进程 B。
3. 进程 B 获得该文件描述符，可以访问对应的内核对象。

这在 Linux 服务管理、设备管理和多进程应用中比较有用。

---

## 十、核心知识总结

学习完 Unix Domain Socket，需要重点掌握以下内容：

| 知识点 | 需要掌握的内容 |
|---|---|
| IPC | 了解进程间通信的意义 |
| AF_UNIX | 知道 Unix Domain Socket 的地址族 |
| sockaddr_un | 掌握 sun_family 和 sun_path |
| 路径名套接字 | 理解 bind()、connect() 和 unlink() |
| 未命名套接字 | 掌握 socketpair() |
| 抽象套接字 | 了解 Linux 特有的抽象命名空间 |
| SOCK_STREAM | 理解本地字节流通信 |
| SOCK_DGRAM | 理解本地数据报通信 |
| SOCK_SEQPACKET | 了解有序分组通信 |
| fork() | 理解父子进程如何继承 Socket 文件描述符 |
| 文件权限 | 理解路径名套接字的权限管理 |

### 最后记住三个核心结论

**1. Unix Domain Socket 是什么？**

它是 Linux 中用于本机进程间通信的一种 Socket 机制，不需要 IP 地址和端口号。

**2. 路径名套接字和未命名套接字有什么区别？**

- 路径名套接字：通过文件系统路径定位通信对象，适合本地客户端—服务器通信。
- 未命名套接字：通过 socketpair() 直接创建两个相互连接的端点，适合父子进程通信。

**3. Unix Domain Socket 与 TCP 有什么联系？**

两者都可以使用 SOCK_STREAM 实现面向连接的字节流通信，很多 Socket API 的使用方式也相似。但 Unix Domain Socket 用于本机 IPC，不经过 IP 网络协议栈。

---

## 十一、学习验收

建议通过以下三个小任务检查自己是否真正掌握：

- [ ] 任务一：使用 AF_UNIX + SOCK_STREAM 编写本地客户端和服务器，实现双向通信。
- [ ] 任务二：使用 socketpair() + fork() 实现父子进程双向通信。
- [ ] 任务三：比较 pipe() 和 socketpair()，理解两种 IPC 机制的区别。

完成这三个任务后，就基本掌握了 Unix Domain Socket 的基础应用。

