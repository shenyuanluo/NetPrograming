# 网络编程之 `socket` 相关函数

> 在网络编程中，**socket** 是核心内容，如果无法理解 socket 到底是什么，那么在做网络编程开发时，将会困难无比。在开始一个最简单的 TCP 例子之前，最好先了解一下什么是 socket，以及常用的与 socket 编程相关的函数。

## 什么是 socket？
**socket**是什么？在[维基](https://zh.wikipedia.org/wiki/%E7%B6%B2%E8%B7%AF%E6%8F%92%E5%BA%A7)是这样说的：在计算机科学中，网络套接字（英语：Network socket），又译网络套接字、网络接口、网络插槽，是电脑网络中进程间数据流的端点。

在 Unix 中，`一切都是文件`，socket 也不例外，也是一个特殊的 “文件描述符”；所以对 socket 的也有类似文件的操作，如：`read`、`write`、`close` 等。

# socket 相关函数

## socket
1. **头文件：**`#include<sys/socket.h>`
2. **功能：**建立一个协议族为 `domain`、协议类型为 `type`、协议编号为 `protocol` 的 **socket** 文件描述符。（当创建成功后，socket 在名称空间(网络地址族)中存在，但没有任何地址给它赋值。）在服务器端，`socket()`返回的套接字用于监听 (`listen`) 和接受 (`accept`) 客户端的连接请求，这个套接字不能用于与客户端之间发送和接收数据；在客户端则是用于与服务端之间发送和接收数据。
3. **原型：**

	``` C
	int socket(int domain, int type, int protocol);
	```
4. **参数：**
	- **domain：**用于设置网络通信的域，该函数根据这个参数选择通信协议的族。

		| 名称 						| 含义                    |
		| -------------------- | ----------------------- | 
		| `PF_UNIX`，`PF_LOCAL` | 本地通信                 |
		| `AF_INET`，`PF_INET`  | IPv4 Internet 协议       |
		| `PF_INET6`            | IPv6 Internet 协议       |
		| `PF_IPX `			 	| IPX-Novell 协议           |
		| `PF_NETLINK` 			| 内核用户界面设备           |
		| `PF_X25` 				| ITU-T X25 / ISO-8208 协议 |
		| `PF_AX25` 				| Amateur radio AX.25     |
		| `PF_ATMPVC` 			| 原始 ATM PVC 访问           |
		| `	PF_APPLETALK` 		| Appletalk               |
		| `PF_PACKET` 			| 底层包访问                |
		
	- **type：**用于设置套接字通信的类型。

		| 名称 						| 含义                    |
		| -------------------- | ----------------------- | 
		| `SOCK_STREAM `       | TCP 连接，提供序列化的、可靠的、双向连接的字节流。 |
		| `SOCK_DGRAM `        | 支持 UDP 连接（无连接状态的消息）。 |
		| `SOCK_SEQPACKET `    | 序列化包，提供一个序列化的、可靠的、双向的基本连接的数据传输通道，数据长度定常。每次调用读系统数据需要将全部数据读出。 |
		| `SOCK_RAW `      	   | RAW 类型，提供原始网络协议访问。 |
		| `SOCK_RDM ` 		   | 提供可靠的数据报文，不过可能数据会有乱序。 |
		| `SOCK_PACKET ` 		| 这是一个专用类型，不能在通用程序中使用。 |
	
	- **protocol：**用于制定某个协议的特定类型，即 **type** 类型中的某个类型。（通常某协议中只有一种特定类型，这样 protocol 参数仅能设置为 **0**；但是有些协议有多种特定的类型，就需要设置这个参数来选择特定的类型。）
		- **SOCK_STREAM** 类型的套接字表示一个双向的字节流，与管道类似。流式的套接字在进行数据收发之前必须已经连接，连接使用 `connect()` 函数进行。一旦连接，可以使用 `read()` 或者 `write()` 函数进行数据的传输。流式通信方式保证数据不会丢失或者重复接收；当数据在一段时间内仍然没有接受完毕，可以将这个连接人为关闭。
		- **SOCK_DGRAM** 和 **SOCK_RAW** 这个两种类型的套接字可以使用函数 `sendto()` 来发送数据到指定的 IP 地址；使用 `recvfrom()` 函数接收指定 IP 地址的数据。
		- **SOCK_PACKET**是一种专用的数据包，它直接从设备驱动接受数据。
5. **返回值：**
	- **> 0：**创建成功，返回一个标识这个 `socket` 的文件描述符。
	- **-1：** 创建失败；错误原因可以通过**error**获得。
		
		| 值 						| 含义                    |
		| -------------------- | ----------------------- | 
		| `EACCES `  			   | 没有权限建立制定的 domain 的 type 的 socket |
		| `EAFNOSUPPORT `  		| 不支持所给的地址类型 |
		| `EINVAL `            | 不支持此协议或者协议不可用 |
		| `EMFILE `			 	| 进程文件表溢出 |
		| `ENFILE `  			   | 已经达到系统允许打开的文件数量，打开文件过多 |
		| `ENOBUFS/ENOMEM `  	| 内存不足 |
		| `EPROTONOSUPPORT `  | 制定的协议 type 在 domain 中不存在 |


## bind（服务端）
1. **头文件：**`#include<sys/socket.h>`
2. **功能：**把用 **addr** 指定的地址赋值给用文件描述符代表的 sockfd；**addrlen** 指定了以 addr 所指向的地址结构体的字节长度。（该操作常称为**“给socket命名”**。）
3. **原型：**

	``` C
	int bind(int sockfd, const struct sockaddr *addr, socklen_t *addrlen);
	```
4. **参数：**
	- **sockfd：**需要绑定地址的 socket。
	- **addr：**需要连接的地址；一般设置时使用 `struct sockaddr_in`，之后在使用到的函数参数中强制转换为 `struct sockaddr *` 。
	- **addrlen：**地址参数长度。
5. **返回值：**
	- **0：**成功。
	- **-1：**出错，错误原因可以通过 **error** 获得。
	
	| 值 						| 含义                    |
	| -------------------- | ----------------------- | 
	| `EACCESS` 			   | 地址空间受保护，用户不具有超级用户的权限。 |
	| `EADDRINUSE `        | 给定的地址正被使用。|


## listen（服务端）
1. **头文件：**`#include<sys/socket.h>`
2. **功能：**该函数使主动连接 socket 变为被动连接，使得一个进程可以接受其它进程的请求，从而成为一个服务器进程。（一般在调用 `bind` 之后和调用 `accept` 之前调用。）
3. **原型：**

	``` C
	int listen(int sockfd, int backlog);
	```
4. **参数：**
	- **sockfd：**需要监听连接请求的 socket。
	- **backlog:**连接请求队列数（一般小于：30）。
5. **返回值：**
	- **0：**成功。
	- **-1：**失败，错误原因可以通过 **error** 获得。


## accept（服务端）
1. **头文件：**`#include<sys/socket.h>`
2. **功能：**接受客户端连接请求（阻塞式）。
3. **原型：**

	``` C
	int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	```
4. **参数：**
	- **sockf：**监听等待连接的 socket。
	- **addr：**客户端 地址。
	- **addrlen：**地址长度。
5. **返回值：**
	- **> 0：**成功，返回一个新连接的客户端 (`socket`) 的文件描述符。
	- **-1：** 失败；错误原因可以通过**error**获得。

## connect（客户端）
1. **头文件：**`#include<sys/socket.h>`****
2. **功能：**完成面向连接的协议的连接过程，对于TCP来说就是那个 **三次握手** 过程。
3. **原型：**

	``` C
	int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
	```
4. **参数：**
	- **sockfd：**需要连接到服务端的 socket。
	- **addr：**服务端 地址。
	- **addrlen：**地址长度。
5. **返回值：**
	- **0：**成功。
	- **-1：**失败，错误原因可以通过 **error** 获得。

	| 值 						| 含义                    |
	| -------------------- | ----------------------- | 
	| `EACCES`，`EPERM` | 用户试图在 socket 广播标志没有设置的情况下连接广播地址或由于防火墙策略导致连接失败。|
	| `EADDRINUSE` | 本地地址处于使用状态。|
	| `EAFNOSUPPORT` | 参数serv_add中的地址非合法地址。|
	| `EAGAIN` | 没有足够空闲的本地端口。|
	| `EALREADY` | 套接字为非阻塞 socket ，并且原来的连接请求还未完成。|
	| `EBADF` | 非法的文件描述符。|
	| `ECONNREFUSED` | 远程地址并没有处于监听状态。|
	| `EFAULT` | 指向 socket 结构体的地址非法。|
	| `EINPROGRESS` | socket 为非阻 socket，且连接请求没有立即完成。|
	| `EINTR` | 系统调用的执行由于捕获中断而中止。|
	| `EISCONN` | 已经连接到该 socket。|
	| `ENETUNREACH` | 网络不可到达。|
	| `ENOTSOCK` | 文件描述符不与 socket 相关。|
	| `ETIMEDOUT` | 连接超时。|


## recv
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t recv(int sock, void *buf, size_t len, int flags);
	```
3. **功能：**接收数据（一般用于面向连接：TCP）。
4. **参数说明：**
	- **sock：** 将要接收数据的 socket。
	- **buf：** 存放数据的缓冲区。
	- **len：** 缓冲区大小。
	- **flags：** 一般设置为 0。
5. **返回值：**
	- **> 0：**成功，实际接收的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。


## send
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t send(int sock, const void *buf, size_t len, int flags); 
	```
3. **功能：**发送数据（一般用于面向连接：TCP）。
4. **参数说明：**
	- **sock：** 将要发送数据的 socket。
	- **buf：** 发送数据的缓冲区。
	- **len：** 发送数据的大小。
	- **flags：** 一般设置为 0。
5. **返回值：**
	- **> 0：**成功，实际发送的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。


## recvfrom
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t recvfrom(int sock, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
	```
3. **功能：**从指定地址接收数据（一般用于无连接：UDP）。
4. **参数说明：**
	- **sock：** 将要接收数据的 socket。
	- **buf：** 存放数据的缓冲区。
	- **len：** 缓冲区大小。
	- **flags：** 一般设置为 0。
	- **from：** 指定的来源地址。
	- **fromlen：** 来源地址长度。
5. **返回值：**
	- **> 0：**成功，实际接收的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。


## sendto
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t sendto (int sock, const void *buf, size_t len, unsigned int flags, const struct sockaddr *to, int tolen);
	```
3. **功能：**发送数据到指定地址（一般用于无连接：UDP）。
4. **参数说明：**
	- **sock：** 将要发送数据的 socket。
	- **buf：** 发送数据的缓冲区。
	- **len：** 发送数据的大小。
	- **flags：** 一般设置为 0。
	- **to：** 指定的目的地址。
	- **tolen：** 目的地址长度。 
5. **返回值：**
	- **> 0：**成功，实际发送的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。


## recvmsg
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t recvmsg(int sock, struct msghdr *msg, int flags);
	```
3. **功能：**从指定地址接收数据（一般用于无连接：UDP）。
4. **参数说明：**
	- **sock：** 将要接收数据的 socket。
	- **msg：** 数据结构体。
	
		``` C
		struct msghdr {
		    void          *msg_name;            /* protocol address */
		    socklen_t     msg_namelen;          /* sieze of protocol address */
		    struct iovec  *msg_iov;             /* scatter/gather array */
		    int           msg_iovlen;           /* # elements in msg_iov */
		    void          *msg_control;         /* ancillary data ( cmsghdr struct) */
		    socklen_t     msg_conntrollen;      /* length of ancillary data */
		    int           msg_flags;            /* flags returned by recvmsg() */
		}
		
		struct iovec {
		    void    *iov_base;                  /* starting address of buffer */
		    size_t  iov_len;                    /* size of buffer */
		}
		```
	- **flags：**一般设置为 0。
5. **返回值：**
	- **> 0：**成功，实际接收的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。


## sendmsg
1. **头文件：**`#include <sys/socket.h>`
2. **原型：**

	``` C
	ssize_t sendmsg(int sock, const struct msghdr *msg, int flags);
	```
3. **功能：**发送数据到指定地址（一般用于无连接：UDP）。
4. **参数说明：**
	- **sock：** 将要发送数据的 socket。
	- **msg：**数据结构体。
	- **flags：**一般设置为 0。
5. **返回值：**
	- **> 0：**成功，实际发送的字节数。
	- **= 0：**连接断开。
	- **< 0：**错误，错误原因可以通过 **error** 获得。
