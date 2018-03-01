//
//  TCPClient.hpp
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#ifndef TCPClient_hpp
#define TCPClient_hpp

#include <iostream>     // cout
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr
#include <unistd.h>     // close


#define BUFF_SIZE 4096
#define HB_INTERVAL 10                  // 心跳时间间隔（单位：秒）


/** 返回值 枚举 */
typedef enum __retValue {
    Ret_error               = -2,       // 出错
    Ret_failed              = -1,       // 失败
    Ret_success             = 0,        // 成功
}RetValue;

/** 消息类型 枚举 */
typedef enum __msgType {
    Msg_heart               = 0,        // 心跳包
    Msg_other               = 1,        // 其他数据
}MsgType;

/** 消息头 结构体 */
typedef struct __msgHead {
    MsgType msgType;                    // 消息类型
    unsigned int msgLen;                // 消息长度
    char data[0];                       // 消息实体
}MsgHead;


void* SendHeartBeat(void *arg);         /** 发送心跳 */
void* SendMsg(void *arg);               /** 发送消息 */
void* RecvMsg(void *arg);               /** 接收消息 */


class TCPClient
{
private:
    struct sockaddr_in serverAddr;      // 服务端地址
    socklen_t serAddrLen;               // 地址长度
    int clientSockfd;                   // 客户端 socket
    
    /**
     发送心跳
     
     @param arg 线程参数（TCPClient实例指针）
     */
    friend void* SendHeartBeat(void* arg);
    
    /**
     发送消息
     
     @param arg 线程参数（TCPClient实例指针）
     */
    friend void* SendMsg(void* arg);
    
    /**
     接收消息
     
     @param arg 线程参数（TCPClient实例指针）
     */
    friend void* RecvMsg(void* arg);
    
    
public:
    TCPClient(std::string ipStr, unsigned int port);
    ~TCPClient();
    
    /**
     连接 服务端

     @return 参见‘RetValue’
     */
    RetValue Connect();
    
    /**
     运行 客户端

     @return 参见‘RetValue’
     */
    RetValue Run();
    
};

#endif /* TCPClient_hpp */
