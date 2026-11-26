# Linux I/O 多路复用：select、poll、epoll

## 一、什么是 I/O 多路复用？

### 1.1 基本概念

I/O 多路复用（I/O Multiplexing）是 Linux 网络编程中一种重要的 I/O 处理机制。

它允许**一个进程或线程同时监视多个文件描述符（File Descriptor，简称 fd）**，当某个文件描述符发生可读、可写等就绪事件时，通知应用程序进行相应的 I/O 操作。

简单来说：

假设一个 TCP Server 同时与三个客户端建立连接：

```text
TCP Server
    |
    |--- fd = 4：客户端 A
    |
    |--- fd = 5：客户端 B
    |
    |--- fd = 6：客户端 C
```

传统多进程方式需要为每个客户端创建一个子进程，或者使用多线程方式为每个客户端创建一个线程。

而 I/O 多路复用可以让一个线程同时监视这三个 Socket：

- 客户端 A 有数据，处理 A。
- 客户端 B 有数据，处理 B。
- 客户端 C 有数据，处理 C。

不需要为每个客户端单独创建一个进程或线程。

### 1.2 为什么需要 I/O 多路复用？

传统多进程 TCP Server：

```c
while(1)
{
    int client_fd = accept(sockfd, ...);

    pid_t pid = fork();

    if(pid > 0)
    {
        close(client_fd);
    }
    else if(pid == 0)
    {
        close(sockfd);

        // 子进程负责当前客户端通信
        recv(client_fd, ...);
        send(client_fd, ...);
    }
}
```

这种方式的特点：

- 一个客户端连接对应一个服务子进程。
- 多个客户端需要创建多个进程。
- 进程创建、调度和切换会产生一定开销。

I/O 多路复用则可以采用：

```text
一个 TCP Server 进程
        |
        v
  I/O 多路复用机制
        |
   _____|_____
   |    |    |
   v    v    v
  A     B    C
```

一个线程监视多个客户端 Socket，哪个 Socket 就绪，就处理哪个 Socket。

### 1.3 什么是 I/O 就绪？

I/O 多路复用监视的是文件描述符是否具备执行相应 I/O 操作的条件。

常见情况：

| 事件 | 含义 |
|---|---|
| 可读 | 有数据可读，或者对端关闭发送方向等可读条件出现 |
| 可写 | 当前可以进行写操作，发送缓冲区通常有可用空间 |
| 异常 | 出现特定的错误或异常条件 |

注意：

**I/O 多路复用只负责监视和通知，不会自动完成 recv()、send() 或 accept()。**

应用程序仍然需要根据返回的就绪事件，主动调用相应函数。

---

## 二、三种 I/O 多路复用机制

Linux 网络编程中常见的三种 I/O 多路复用机制：

1. select()
2. poll()
3. epoll()

三者的核心区别是管理文件描述符和返回就绪事件的方式不同。

---

## 三、select()

### 3.1 基本原理

select() 是一种早期的 I/O 多路复用机制。

它通过文件描述符集合（fd_set）管理需要监视的文件描述符。

例如：

```text
监听 Socket：fd = 3
客户端 A：fd = 4
客户端 B：fd = 5
客户端 C：fd = 6
```

可以把这些文件描述符加入 fd_set 集合，然后调用 select() 等待事件。

当客户端 B 发送数据时，select() 返回，告诉程序 fd=5 已经处于可读状态。

### 3.2 select() 函数原型

```c
#include <sys/select.h>

int select(
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    struct timeval *timeout
);
```

参数说明：

| 参数 | 含义 |
|---|---|
| nfds | 最大文件描述符编号 + 1 |
| readfds | 监视可读事件的文件描述符集合 |
| writefds | 监视可写事件的文件描述符集合 |
| exceptfds | 监视特定异常条件的文件描述符集合 |
| timeout | 设置等待超时时间 |

#### （1）nfds

表示需要检查的文件描述符范围上限，不包含该上限。

例如：

```text
sockfd = 3
client_fd_A = 4
client_fd_B = 5
client_fd_C = 6
```

最大文件描述符为 6。

因此：

```c
select(7, &readfds, NULL, NULL, NULL);
```

即：

```c
nfds = maxfd + 1;
```

注意：这里不是文件描述符的数量，而是最大文件描述符编号加 1。

#### （2）readfds

监视文件描述符是否可读。

例如：

```c
FD_SET(client_fd, &readfds);
```

表示将 client_fd 加入可读监视集合。

#### （3）writefds

监视文件描述符是否可写。

例如：

```c
FD_SET(sockfd, &writefds);
```

表示监视该 Socket 是否可以进行写操作。

#### （4）exceptfds

监视特定异常条件，例如 TCP 带外数据等。

#### （5）timeout

通过 struct timeval 设置超时时间。

```c
struct timeval timeout;

timeout.tv_sec = 5;
timeout.tv_usec = 0;
```

表示最多等待 5 秒。

| timeout | 含义 |
|---|---|
| NULL | 一直阻塞，直到事件发生或被信号中断 |
| 5 秒 | 最多等待 5 秒 |
| {0, 0} | 立即返回，不等待 |

### 3.3 fd_set 文件描述符集合

fd_set 可以理解为一个文件描述符登记表，用于保存需要监视的文件描述符。

```c
fd_set readfds;
```

select() 提供四个常用宏：

| 宏 | 作用 |
|---|---|
| FD_ZERO() | 清空文件描述符集合 |
| FD_SET() | 将文件描述符加入集合 |
| FD_CLR() | 将文件描述符从集合中删除 |
| FD_ISSET() | 判断文件描述符是否在集合中 |

例如：

```c
fd_set readfds;

// 清空集合
FD_ZERO(&readfds);

// 加入需要监视的文件描述符
FD_SET(4, &readfds);
FD_SET(5, &readfds);
FD_SET(6, &readfds);

// 判断 fd=5 是否就绪
if(FD_ISSET(5, &readfds))
{
    recv(5, buf, sizeof(buf), 0);
}
```

### 3.4 master_set 和 read_set

在实际 TCP Server 中，通常使用两个集合管理文件描述符。

```c
fd_set master_set;
fd_set read_set;
```

**master_set：长期保存所有需要监视的文件描述符。**

例如：

```text
master_set = {3, 4, 5, 6}
```

**read_set：每次调用 select() 时使用的临时集合。**

因为 select() 会修改传入的文件描述符集合，返回后只保留已经就绪的文件描述符。

因此每轮调用之前，需要重新复制：

```c
read_set = master_set;
```

例如：

调用 select() 前：

```text
master_set = {3, 4, 5, 6}
read_set   = {3, 4, 5, 6}
```

假设只有 fd=5 和 fd=6 可读。

调用 select() 后：

```text
master_set = {3, 4, 5, 6}
read_set   = {5, 6}
```

master_set 不变，read_set 只保留本轮就绪的文件描述符。

下一轮再执行：

```c
read_set = master_set;
```

恢复完整监视集合。

### 3.5 select() 工作流程

```text
创建监听 Socket
       |
       v
    bind()
       |
       v
    listen()
       |
       v
初始化 master_set
       |
       v
FD_SET(sockfd, &master_set)
       |
       v
进入 while 循环
       |
       v
read_set = master_set
       |
       v
调用 select()
       |
       v
检查哪些 fd 就绪
       |
       +----------------------+
       |                      |
       v                      v
监听 Socket 就绪          客户端 Socket 就绪
       |                      |
       v                      v
    accept()                recv()
       |                      |
       v                      v
新客户端加入集合           处理客户端数据
       |                      |
       +----------+-----------+
                  |
                  v
               下一轮
```

### 3.6 select() 的缺点

1. 文件描述符集合存在固定大小限制。在常见 Linux glibc 环境中，FD_SETSIZE 通常为 1024。
2. 每次调用 select() 都需要重新准备文件描述符集合。
3. 内核需要检查相应范围内的文件描述符。
4. 返回后，应用程序还需要遍历集合，判断哪些文件描述符已经就绪。

因此，select() 更适合连接规模较小的场景。

---

## 四、poll()

### 4.1 基本原理

poll() 与 select() 的主要功能相同，都是监视多个文件描述符的 I/O 就绪状态。

但它不再使用 fd_set 位图集合，而是采用 **pollfd 结构体数组** 管理文件描述符。

这是 poll() 与 select() 最直观的区别。

### 4.2 pollfd 结构体

```c
#include <poll.h>

struct pollfd
{
    int fd;          // 文件描述符
    short events;    // 希望监视的事件
    short revents;   // 实际发生的事件
};
```

三个成员：

| 成员 | 含义 |
|---|---|
| fd | 需要监视的文件描述符 |
| events | 希望监视什么事件 |
| revents | 实际发生了什么事件 |

例如：

```c
struct pollfd fds[4];
```

假设需要监视：

```text
fd=3：监听 Socket
fd=4：客户端 A
fd=5：客户端 B
fd=6：客户端 C
```

可以将它们分别放入数组。

### 4.3 events 和 revents 的区别

这是 poll() 最重要的知识点之一。

**events：程序员设置，表示希望监视的事件。**

例如：

```c
fds[0].fd = sockfd;
fds[0].events = POLLIN;
```

表示希望监视 sockfd 是否可读。

**revents：poll() 返回时由系统设置，表示实际发生的事件。**

例如：

客户端 B 和 C 同时发送数据。

poll() 返回后：

| fd | events | revents |
|---|---|---|
| 3 | POLLIN | 0 |
| 4 | POLLIN | 0 |
| 5 | POLLIN | POLLIN |
| 6 | POLLIN | POLLIN |

可以看到：

- events 不变，仍然表示希望监视什么。
- revents 表示实际发生了什么。

通过下面的代码判断：

```c
for(int i = 0; i < 4; i++)
{
    if(fds[i].revents & POLLIN)
    {
        // 当前文件描述符已经可读
    }
}
```

这里的 `&` 是按位与运算，用于检查 revents 中是否包含 POLLIN 标志。

### 4.4 poll() 函数原型

```c
int poll(
    struct pollfd *fds,
    nfds_t nfds,
    int timeout
);
```

参数说明：

| 参数 | 含义 |
|---|---|
| fds | 需要监视的 pollfd 数组 |
| nfds | 数组中 pollfd 元素的数量 |
| timeout | 等待超时时间，单位为毫秒 |

timeout 常见取值：

| timeout | 含义 |
|---|---|
| -1 | 一直等待 |
| 5000 | 最多等待 5 秒 |
| 0 | 立即返回 |

注意与 select() 的区别：

- select()：传入最大文件描述符 + 1。
- poll()：传入需要监视的数组元素数量。

### 4.5 poll() 常见事件

| 事件 | 含义 |
|---|---|
| POLLIN | 有数据可读或出现可读条件 |
| POLLOUT | 可以进行写操作 |
| POLLERR | 出现错误条件 |
| POLLHUP | 对端挂断连接 |
| POLLNVAL | 文件描述符无效 |

其中 POLLERR、POLLHUP、POLLNVAL 等事件，即使没有主动设置在 events 中，也可能出现在 revents 中。

### 4.6 poll() 工作流程

```text
创建监听 Socket
       |
       v
    bind()
       |
       v
    listen()
       |
       v
创建 pollfd 数组
       |
       v
添加监听 Socket
       |
       v
调用 poll()
       |
       v
检查每个元素的 revents
       |
       +----------------------+
       |                      |
       v                      v
监听 Socket 就绪          客户端 Socket 就绪
       |                      |
       v                      v
    accept()                recv()
       |                      |
       v                      v
新客户端加入数组           处理客户端数据
       |                      |
       +----------+-----------+
                  |
                  v
               下一轮
```

### 4.7 poll() 的优缺点

优点：

1. 使用结构体数组管理文件描述符，编程方式比较直观。
2. 不受 select() 的 FD_SETSIZE 位图大小限制。
3. events 和 revents 分离，便于管理监视事件和返回事件。

缺点：

1. 每次调用 poll() 都需要传入文件描述符数组。
2. 内核需要检查数组中的文件描述符。
3. 应用程序通常需要遍历数组，寻找就绪的文件描述符。

因此，当连接数量非常大时，poll() 仍然存在扫描成本。

---

## 五、epoll()

### 5.1 基本原理

epoll() 是 Linux 特有的 I/O 多路复用机制。

它主要针对大量并发连接的场景进行设计。

与 select()、poll() 不同，epoll 可以将需要监视的文件描述符注册到内核中，由内核维护监视关系。

epoll 有两个重要概念：

**1. 兴趣列表（Interest List）**

保存应用程序希望监视的文件描述符和对应事件。

例如：

```text
epoll_interest_list

fd=3：监听 Socket
fd=4：客户端 A
fd=5：客户端 B
fd=6：客户端 C
```

**2. 就绪列表（Ready List）**

保存已经发生就绪事件的文件描述符。

例如：

客户端 B 和 C 同时发送数据：

```text
epoll_ready_list

fd=5：可读
fd=6：可读
```

当应用程序调用 epoll_wait() 时，就可以获取已经就绪的事件。

注意：兴趣列表和就绪列表是理解 epoll 的概念模型，不代表需要由应用程序自己创建两个数组。

### 5.2 epoll 的三个核心函数

#### （1）epoll_create1()

创建一个 epoll 实例，返回一个文件描述符。

```c
int epfd = epoll_create1(0);
```

这里的 epfd 是 epoll 实例的文件描述符。

注意：它不是监听 Socket，也不是客户端 Socket，而是用于管理 epoll 实例的句柄。

#### （2）epoll_ctl()

向 epoll 实例添加、修改或删除需要监视的文件描述符。

函数原型：

```c
int epoll_ctl(
    int epfd,
    int op,
    int fd,
    struct epoll_event *event
);
```

参数说明：

| 参数 | 含义 |
|---|---|
| epfd | epoll 实例的文件描述符 |
| op | 操作类型 |
| fd | 需要管理的目标文件描述符 |
| event | 需要监视的事件及用户数据 |

三种常用操作：

| 操作 | 含义 |
|---|---|
| EPOLL_CTL_ADD | 添加文件描述符 |
| EPOLL_CTL_MOD | 修改监视事件 |
| EPOLL_CTL_DEL | 删除文件描述符 |

例如：

```c
struct epoll_event ev;

ev.events = EPOLLIN;
ev.data.fd = client_fd;

epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
```

表示将 client_fd 加入 epoll 实例，监视它的可读事件。

#### （3）epoll_wait()

等待并获取已经就绪的事件。

函数原型：

```c
int epoll_wait(
    int epfd,
    struct epoll_event *events,
    int maxevents,
    int timeout
);
```

参数说明：

| 参数 | 含义 |
|---|---|
| epfd | epoll 实例句柄 |
| events | 保存返回的就绪事件 |
| maxevents | 本次最多返回多少个事件 |
| timeout | 等待超时时间，单位为毫秒 |

返回值：

- 大于 0：本次获取到的就绪事件数量。
- 等于 0：等待超时。
- 等于 -1：发生错误，需要检查 errno。

### 5.3 epoll_event 结构体

```c
struct epoll_event
{
    uint32_t events;
    epoll_data_t data;
};
```

其中：

- events：监视或返回的事件类型。
- data：用户数据，可以保存文件描述符或用户自定义数据。

例如：

```c
struct epoll_event ev;

ev.events = EPOLLIN;
ev.data.fd = client_fd;
```

表示监视 client_fd 的可读事件，并保存对应的文件描述符。

常见事件：

| 事件 | 含义 |
|---|---|
| EPOLLIN | 可读 |
| EPOLLOUT | 可写 |
| EPOLLERR | 错误条件 |
| EPOLLHUP | 挂断 |
| EPOLLRDHUP | 对端关闭连接或关闭写方向 |

### 5.4 epoll() 完整工作流程

假设：

```text
监听 Socket：fd=3

客户端 A：fd=4
客户端 B：fd=5
客户端 C：fd=6
```

**第一步：创建 epoll 实例。**

```c
int epfd = epoll_create1(0);
```

**第二步：注册监听 Socket。**

```c
struct epoll_event ev;

ev.events = EPOLLIN;
ev.data.fd = sockfd;

epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
```

**第三步：接受客户端连接。**

```c
int client_fd = accept(sockfd, ...);
```

然后将新客户端加入 epoll：

```c
ev.events = EPOLLIN;
ev.data.fd = client_fd;

epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
```

**第四步：等待事件。**

```c
struct epoll_event events[100];

int n = epoll_wait(epfd, events, 100, -1);
```

假设客户端 B 和 C 同时发送数据：

```text
n = 2

events[0].data.fd = 5
events[1].data.fd = 6
```

说明本次有两个就绪事件。

**第五步：处理就绪事件。**

```c
for(int i = 0; i < n; i++)
{
    int fd = events[i].data.fd;

    if(fd == sockfd)
    {
        // 监听 Socket 就绪
        // accept() 接受新连接
    }
    else
    {
        // 客户端 Socket 就绪
        // recv() 接收数据
    }
}
```

当客户端断开连接时，需要：

```c
epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);

close(client_fd);
```

先从 epoll 实例中删除监视对象，再关闭对应 Socket。

### 5.5 epoll() 工作流程图

```text
创建监听 Socket
       |
       v
    bind()
       |
       v
    listen()
       |
       v
epoll_create1()
       |
       v
epoll_ctl(ADD, listenfd)
       |
       v
进入 while 循环
       |
       v
epoll_wait()
       |
       v
获取就绪事件
       |
       +----------------------+
       |                      |
       v                      v
监听 Socket 就绪          客户端 Socket 就绪
       |                      |
       v                      v
    accept()                recv()
       |                      |
       v                      v
epoll_ctl(ADD)             处理客户端数据
新客户端注册                  |
       |                      v
       |                    send()
       |                      |
       +----------+-----------+
                  |
                  v
               下一轮
```

### 5.6 epoll 的优点

1. 不受 select() 的 FD_SETSIZE 位图大小限制。
2. 不需要每次等待时重新传入完整的监视文件描述符集合。
3. 可以直接获取就绪事件列表。
4. 在大量并发连接、活跃连接比例较低的场景中通常具有更好的扩展性。

注意：epoll 并不是任何情况下都比 select()、poll() 快。实际性能还与连接数量、活跃比例、事件处理方式等因素有关。

---

## 六、epoll 的两种工作模式：LT 和 ET

epoll 有两种重要的触发模式：

1. LT：水平触发（Level Triggered）。
2. ET：边沿触发（Edge Triggered）。

### 6.1 LT：水平触发

LT 是 epoll 的默认工作模式。

特点：

**只要文件描述符仍然处于就绪状态，epoll 就可以持续通知应用程序。**

例如：

客户端发送了 100 字节数据。

服务端第一次只读取 50 字节：

```c
recv(fd, buf, 50, 0);
```

此时接收缓冲区还剩余 50 字节。

因为 Socket 仍然处于可读状态，所以再次调用 epoll_wait() 时，仍然可以收到可读通知。

工作过程：

```text
客户端发送 100 字节
        |
        v
接收缓冲区有 100 字节
        |
        v
第一次 recv() 读取 50 字节
        |
        v
还剩余 50 字节
        |
        v
再次 epoll_wait()
        |
        v
再次通知可读
        |
        v
读取剩余 50 字节
```

LT 模式下可以分批读取，不要求一次性读完所有数据。

### 6.2 ET：边沿触发

ET 是边沿触发模式。

特点：

**主要在文件描述符状态发生变化时通知应用程序。**

例如：

客户端发送 100 字节。

服务端第一次只读取 50 字节，剩下 50 字节仍在接收缓冲区。

此时不能依赖下一次 epoll_wait() 再次通知读取剩余数据。

因此通常需要循环读取，直到数据全部读取完毕。

ET 模式通常需要配合非阻塞 I/O。

```c
while(1)
{
    int count = recv(fd, buf, sizeof(buf), 0);

    if(count > 0)
    {
        // 处理接收到的数据
    }
    else if(count < 0 && errno == EAGAIN)
    {
        // 当前没有更多数据可读取
        break;
    }
    else if(count < 0 && errno == EINTR)
    {
        // 被信号中断，继续尝试
        continue;
    }
    else
    {
        // 对端关闭或发生其他错误
        break;
    }
}
```

实际工程中还需要区分 recv() 返回 0、其他错误以及连接关闭情况。

### 6.3 LT 和 ET 对比

| 对比项目 | LT | ET |
|---|---|---|
| 中文名称 | 水平触发 | 边沿触发 |
| 默认模式 | 是 | 否 |
| 通知方式 | 只要仍然就绪就可以持续通知 | 主要在状态变化时通知 |
| 是否必须使用非阻塞 I/O | 不必须 | 工程上应使用 |
| 数据读取 | 可以分批读取 | 通常需要循环读取到 EAGAIN |
| 编程难度 | 较低 | 较高 |

建议初学 epoll 时先使用 LT 模式，熟悉整个服务器流程后，再学习 ET。

---

## 七、select、poll、epoll 三者对比

| 对比项目 | select | poll | epoll |
|---|---|---|---|
| 管理方式 | fd_set 位图 | pollfd 数组 | 内核事件机制 |
| 固定 FD_SETSIZE 限制 | 有 | 无 | 无 |
| 每次调用传入完整监视集合 | 是 | 是 | 否 |
| 返回就绪事件 | 修改 fd_set | 修改 revents | 返回就绪事件数组 |
| 是否需要遍历检查 | 需要 | 需要 | 通常只需遍历就绪事件 |
| 内核持久维护监视关系 | 否 | 否 | 是 |
| 适用场景 | 少量连接 | 中小规模连接 | 大量并发连接 |

### 7.1 举例理解三者的性能差异

假设 TCP Server 同时管理 10000 个客户端，但只有 10 个客户端正在发送数据。

**select：**

需要检查监视范围内的文件描述符，并在返回后检查就绪集合。

**poll：**

需要扫描传入的文件描述符数组，并检查 revents。

**epoll：**

通过 epoll_ctl() 预先注册监视对象，epoll_wait() 返回就绪事件，不需要每次遍历全部注册对象。

因此，在大量连接且活跃连接较少的场景中，epoll 通常更有优势。

---

## 八、三种 I/O 多路复用在 TCP Server 中的应用

结合之前学习的多进程 TCP Server，可以看到三种架构的区别。

### 8.1 多进程 TCP Server

```text
TCP Server 主进程
       |
       +--- fork() ---> 客户端 A 子进程
       |
       +--- fork() ---> 客户端 B 子进程
       |
       +--- fork() ---> 客户端 C 子进程
```

每个客户端由独立的子进程处理。

### 8.2 select / poll / epoll TCP Server

```text
一个 TCP Server 进程
          |
          v
  I/O 多路复用机制
          |
    +-----+-----+
    |     |     |
    v     v     v
   A      B     C
```

一个执行流监视多个客户端，根据就绪事件调用 accept()、recv()、send()。

### 8.3 工程中的组合使用

实际工程中还可以采用：

```text
          TCP Server
               |
               v
        epoll 事件循环
               |
       获取就绪 Socket
               |
               v
          任务分发
               |
       +-------+-------+
       |       |       |
       v       v       v
     工作线程 工作线程 工作线程
       |       |       |
       +-------+-------+
               |
               v
          处理业务数据
```

这种方式就是 I/O 多路复用与线程池结合。

I/O 线程负责监视和处理网络事件，工作线程负责执行相对耗时的业务任务。

---

## 九、学习总结

### 9.1 必须掌握的核心知识

| 知识点 | 必须理解的内容 |
|---|---|
| I/O 多路复用 | 一个执行流监视多个文件描述符 |
| select | fd_set、FD_ZERO、FD_SET、FD_CLR、FD_ISSET |
| master_set | 长期保存需要监视的文件描述符 |
| read_set | 每次交给 select 的工作集合 |
| poll | pollfd 结构体数组 |
| events | 希望监视的事件 |
| revents | 实际发生的事件 |
| epoll_create1 | 创建 epoll 实例 |
| epoll_ctl | 添加、修改、删除监视对象 |
| epoll_wait | 等待并获取就绪事件 |
| LT | 只要仍然就绪就可以持续通知 |
| ET | 主要在状态变化时通知，通常配合非阻塞 I/O |

### 9.2 三者最核心的区别

记住下面三句话：

1. **select：** 使用 fd_set 集合监视文件描述符，每次调用都需要重新准备集合。
2. **poll：** 使用 pollfd 数组管理文件描述符，通过 events 和 revents 区分监视事件与实际事件。
3. **epoll：** 通过 epoll_ctl() 将监视对象注册到内核，通过 epoll_wait() 获取就绪事件。

