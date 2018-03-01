//
//  TCPClient.cpp
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include "TCPClient.hpp"


TCPClient::TCPClient(std::string ipStr, unsigned int port)
{
    if (true == ipStr.empty()
        || 0 >= ipStr.length())
    {
        std::cout << "Ip 地址不能为空！" << std::endl;
        return;
    }
    // 创建 客户端 socket
    clientSockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > clientSockfd)
    {
        std::cout << "创建 socket 失败！" << std::endl;
        return;
    }
    std::cout << "创建 socket 成功！" << std::endl;
    
    // 设置 服务端 地址
    serAddrLen = sizeof(serverAddr);
    memset(&serverAddr, 0, serAddrLen);                     // 清空结构体
    serverAddr.sin_family      = AF_INET;                   // 协议族 类型
    serverAddr.sin_port        = htons(port);               // 服务端 端口
    serverAddr.sin_addr.s_addr = inet_addr(ipStr.c_str());  // 服务端 IP 地址
}


TCPClient::~TCPClient()
{
    if (0 < clientSockfd)
    {
        std::cout << "关闭客户端 sockfd ！" << std::endl;
        close(clientSockfd);    // 关闭客户端 sockfd
    }
}


RetValue TCPClient::ConnServer()
{
    std::cout << "开始连接 服务端。。。" << std::endl;
    // 连接 服务端
    int ret = connect(clientSockfd, (struct sockaddr*)&serverAddr, serAddrLen);
    if (0 != ret)
    {
        std::cout << "连接 服务端 失败！" << std::endl;
        return Ret_failed;
    }
    std::cout << "连接 服务端 成功！" << std::endl;
    return Ret_success;
}


RetValue TCPClient::Run()
{
    // 发送消息
    std::string msg = "Hello server, I'm client !";
    ssize_t sendLen = send(clientSockfd, msg.c_str(), msg.length(), 0);
    if (0 > sendLen)    // 出错
    {
        std::cout << "发送消息出错！" << std::endl;
        return Ret_error;
    }
    else if (0 == sendLen)  // 连接已断开
    {
        std::cout << "发送消息失败，连接已断开！" << std::endl;
        return Ret_failed;
    }
    else    // 发送成功
    {
        std::cout << "发送消息成功！" << std::endl;
    }
    
    // 接收消息
    char recvBuf[BUFF_SIZE];
    ssize_t recvLen;
    memset(recvBuf, 0, BUFF_SIZE);
    recvLen = recv(clientSockfd, recvBuf, BUFF_SIZE, 0);
    if (0 > recvLen)    // 出错
    {
        std::cout << "接收消息出错！" << std::endl;
        return Ret_error;
    }
    else if (0 == recvLen)  // 连接已断开
    {
        std::cout << "接收消息失败，连接已断开！" << std::endl;
        return Ret_failed;
    }
    else    // 接收成功
    {
        recvBuf[recvLen] = '\0';
        std::cout << "接收到服务端消息：" << recvBuf << std::endl;
    }
    sleep(3);
    return Ret_success;
}
