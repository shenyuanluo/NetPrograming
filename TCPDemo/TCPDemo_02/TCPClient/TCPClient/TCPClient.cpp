//
//  TCPClient.cpp
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include "TCPClient.hpp"
#include <pthread.h>


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
    pthread_t sendP;
    pthread_t recvP;
    
    // 创建发送消息线程
    int ret = pthread_create(&sendP, NULL, SendMsg, (void*)this);
    if (0 != ret)
    {
        std::cout << "创建发送消息线程失败！" << std::endl;
        return  Ret_failed;
    }
    // 创建接收消息线程
    ret = pthread_create(&recvP, NULL, RecvMsg, (void*)this);
    if (0 != ret)
    {
        std::cout << "创建接收消息线程失败！" << std::endl;
        return  Ret_failed;
    }
    // 等待线程结束
    void *result;
    if (0 != pthread_join(sendP, &result))
    {
        perror("等待 发送消息 线程失败");
        exit(1);
    }
    if (0 != pthread_join(recvP, &result))
    {
        perror("等待 接收消息 线程失败");
        exit(1);
    }
    return Ret_success;
}


void* SendMsg(void *arg)
{
    std::cout << "发送消息线程启动！" << std::endl;
    TCPClient *client = (TCPClient *)arg;
    std::string sendMsg;
    while(1)
    {
        std::cin >> sendMsg;
        
        ssize_t sendLen = send(client->clientSockfd, sendMsg.c_str(), sendMsg.length(), 0);
        if (0 > sendLen)    // 出错
        {
            std::cout << "发送消息出错！" << std::endl;
            break;
        }
        else if (0 == sendLen)  // 连接已断开
        {
            std::cout << "发送消息失败，连接已断开！" << std::endl;
            break;
        }
        else    // 发送成功！
        {
            if (0 == strcmp(sendMsg.c_str(), "quit"))
            {
                break;
            }
//            std::cout << "发送消息成功！" << std::endl;
        }
    }
    return NULL;
}


void* RecvMsg(void *arg)
{
    std::cout << "接收消息线程启动！" << std::endl;
    TCPClient *client = (TCPClient *)arg;
    char recvBuf[BUFF_SIZE];
    ssize_t recvLen;
    while (1)
    {
        memset(recvBuf, 0, BUFF_SIZE);
        recvLen = recv(client->clientSockfd, recvBuf, BUFF_SIZE, 0);
        if (0 > recvLen)    // 出错
        {
            std::cout << "接收消息出错！" << std::endl;
            break;
        }
        else if (0 == recvLen)  // 连接已断开
        {
            std::cout << "接收消息失败，连接已断开！" << std::endl;
            break;
        }
        else    // 接收成功
        {
            recvBuf[recvLen] = '\0';
            std::cout << "接收到服务端消息：" << recvBuf << std::endl;
            if (0 == strcmp(recvBuf, "quit"))   // 遇到 “quit” 则退出
            {
                break;
            }
        }
    }
    return NULL;
}
