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


#define BUFF_SIZE 4096


/** 返回值 枚举 */
typedef enum __retValue {
    Ret_error               = -2,       // 出错
    Ret_failed              = -1,       // 失败
    Ret_success             = 0,        // 成功
}RetValue;


class TCPServer
{
private:
    struct sockaddr_in serverAddr;      // 服务端地址
    socklen_t serAddrLen;               // 地址长度
    int serverSockfd;                   // 服务端 socket
    
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
