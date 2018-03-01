//
//  TCPServer.cpp
//  TCPServer
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include "TCPServer.hpp"


#define DEFAULT_QUEUE_NUM 5


TCPServer::TCPServer(std::string ipStr, unsigned int port)
{
    if (true == ipStr.empty()
        || 0 >= ipStr.length())
    {
        std::cout << "Ip 地址不能为空！" << std::endl;
        return;
    }
    // 创建 服务端 socket
    serverSockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > serverSockfd)
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


TCPServer::~TCPServer()
{
    if (0 < serverSockfd)
    {
        std::cout << "关闭服务端 sockfd ！" << std::endl;
        close(serverSockfd);    // 关闭服务端 sockfd
    }
}


RetValue TCPServer::BindAddr()
{
    // 绑定 IP 地址和端口号
    int ret = bind(serverSockfd, (struct sockaddr*)&serverAddr, serAddrLen);
    if (0 != ret)
    {
        std::cout << "绑定 IP 地址和端口失败！" << std::endl;
        return Ret_failed;
    }
    std::cout << "绑定 IP 地址和端口成功！" << std::endl;
    return Ret_success;
}


RetValue TCPServer::Listen(unsigned int queueNum)
{
    // 监听端口（使 Socket 变为被动连接，等待客户端 Socket 连接）
    int ret = listen(serverSockfd, 0 >= queueNum ? DEFAULT_QUEUE_NUM : queueNum);
    if (0 != ret)
    {
        std::cout << "监听端口失败！" << std::endl;
        return Ret_failed;
    }
    std::cout << "监听端口成功！" << std::endl;
    return Ret_success;
}


RetValue TCPServer::Run()
{
    // 客户端 地址
    struct sockaddr_in clientAddr;
    socklen_t cliAddrLen = sizeof(clientAddr);
    
    while (1)   // 循环等待客户端连接
    {
        // 阻塞式等待客户端连接
        std::cout << "等待客户端连接。。。" << std::endl;
        int clientSockfd = accept(serverSockfd, (struct sockaddr*)&clientAddr, &cliAddrLen);
        if (0 > clientSockfd)
        {
            std::cout << "服务器接受客户端连接失败！" << std::endl;
            continue;
        }
        std::cout << "服务器接受客户端（sockfd = " << clientSockfd << "）连接成功！" << std::endl;
        
        pthread_t commP;
        // 创建 与 客户通信线程
        int ret = pthread_create(&commP, NULL, HandleClient, (void*)&clientSockfd);
        if (0 != ret)
        {
            std::cout << "创建客户端（sockfd = " << clientSockfd << "）消息处理线程失败！" << std::endl;
            close(clientSockfd);
            continue;
        }
        std::cout << "创建客户端（sockfd = " << clientSockfd << "）消息处理线程成功！" << std::endl;
    }
    
    return Ret_success;
}


void* HandleClient(void* arg)
{
    int sockfd = *(int*)arg;
    while (1)   // 循环处理客户端消息
    {
        // 接收消息
        char recvBuf[BUFF_SIZE];
        memset(recvBuf, 0, BUFF_SIZE);
        ssize_t recvLen = recv(sockfd, recvBuf, BUFF_SIZE, 0);
        if (0 > recvLen)    // 出错
        {
            std::cout << "接收消息出错（sockfd = " << sockfd << "）！" << std::endl;
            close(sockfd);
            break;
        }
        else if (0 == recvLen)  // 连接已断开
        {
            std::cout << "接收消息失败，（sockfd = " << sockfd << "）客户端连接已断开！" << std::endl;
            close(sockfd);
            break;
        }
        else    // 接收成功
        {
            recvBuf[recvLen] = '\0';
            std::cout << "接收到客户端（sockfd = " << sockfd << "）消息：" << recvBuf << std::endl;
        }
        
        // 发送消息（这里直接回复客户端发来的数据）
        ssize_t sendLen = send(sockfd, recvBuf, recvLen, 0);
        if (0 > sendLen)    // 出错
        {
            std::cout << "发送消息出错（sockfd = " << sockfd << "）！" << std::endl;
            close(sockfd);
            break;
        }
        else if (0 == sendLen)  // 连接已断开
        {
            std::cout << "发送消息失败，（sockfd = " << sockfd << "）客户端连接已断开！" << std::endl;
            close(sockfd);
            break;
        }
        else    // 发送成功
        {
//                std::cout << "发送消息成功！" << std::endl;
            if (0 == strcmp(recvBuf, "quit"))
            {
                std::cout << "客户端（sockfd = " << sockfd << "）已主动关闭连接！" << std::endl;
                break;
            }
        }
    }
    std::cout << "关闭客户端 (sockfd = " << sockfd << ")" << std::endl;
    close(sockfd);
    
    return NULL;
}

