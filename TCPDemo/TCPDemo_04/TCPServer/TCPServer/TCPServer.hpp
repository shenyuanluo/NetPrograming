//
//  TCPServer.hpp
//  TCPServer
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#ifndef TCPServer_hpp
#define TCPServer_hpp

#include <iostream>     // cout
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr
#include <unistd.h>     // close
#include <map>          // map


#define BUFF_SIZE 4096                  // 缓存大小
#define HB_INTERVAL 3                   // 心跳时间间隔（单位：秒）
#define CHECK_HB_COUNT 5                // 无法检测到心跳最大次数


/** 返回值 枚举 */
typedef enum __retValue {
    Ret_error               = -2,       // 出错
    Ret_failed              = -1,       // 失败
    Ret_success             = 0,        // 成功
}RetValue;


void* CheckHeartBeat(void* arg);        // 心跳检测


class TCPServer
{
private:
    struct sockaddr_in serverAddr;      // 服务端地址
    socklen_t serAddrLen;               // 地址长度
    int serverSockfd;                   // 服务端 socket
    struct timeval timeout;             // 超时时间
    int maxFd;                          // 最大 sockfd
    fd_set masterSet;                   // 所有 sockfd 集合，包括服务端 sockfd 和所有已连接 sockfd
    fd_set workingSet;                  // 工作集合
    std::map<int, std::pair<std::string, int>> sockfdMap;    // 记录连接的客户端 : <sockfd, <ip, checkCount>>
    
    /**
     接受客户端连接
     
     @return 参见‘RetValue’
     */
    RetValue AcceptConn();
    
    /**
     接收客户端消息
     
     @return 参见‘RetValue’
     */
    RetValue RecvMsg();
    
    /**
     格式化发送数据
     
     @param buf 数据缓冲区
     @param bufLen 缓冲区长度
     @param cmd 命令类型
     @param status 用户状态
     @param userId 用户 ID
     @return > 0：格式化成功（需要发送数据大小）；否则，失败
     */
    int RespDataFormat(char *buf, unsigned int bufLen, unsigned int cmd, unsigned int status, const char userId[]);
    
    /**
     解析接收的数据
     
     @param buf 数据缓冲区
     @param msgLen 消息长度
     @param cmd 命令类型
     @param userId 用户 ID
     @return 解析是否成功；0：成功，否则失败
     */
    int ReqDataParse(char *buf, unsigned int msgLen, unsigned int *cmd, char *userId);
    
    /**
     心跳检测
     
     @param arg 线程参数（TCPServer实例指针）
     */
    friend void* CheckHeartBeat(void* arg);
    
public:
    TCPServer(std::string ipStr, unsigned int port);
    ~TCPServer();
    
    /**
     绑定 服务端 地址

     @return 参见‘RetValue’
     */
    RetValue BindAddr();
    
    /**
     监听端口连接请求

     @param queueNum 最大等待处理连接队列数
     @return 参见‘RetValue’
     */
    RetValue Listen(unsigned int queueNum);
    
    /**
     运行服务端

     @return 参见‘RetValue’
     */
    RetValue Run();
    
    
};

#endif /* TCPServer_hpp */
