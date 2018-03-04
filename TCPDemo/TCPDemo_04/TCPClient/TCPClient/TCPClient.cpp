//
//  TCPClient.cpp
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include "TCPClient.hpp"
#include <pthread.h>
#include <sstream>
#include "TransProtocol.h"

#define MAGIC "SYL "
#define USER_ID "ShenYuanLuo"


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


RetValue TCPClient::Connect()
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
    pthread_t heartBeatP;
    pthread_t sendP;
    pthread_t recvP;
    
    // 创建发送心跳线程
    int ret = pthread_create(&heartBeatP, NULL, SendHeartBeat, (void*)this);
    if(0 != ret)
    {
        std::cout << "创建发送心跳线程失败！" << std::endl;
        return  Ret_failed;
    }
    
    // 创建发送消息线程
    ret = pthread_create(&sendP, NULL, SendMsg, (void*)this);
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
    if (0 != pthread_join(heartBeatP, &result))
    {
        perror("等待 发送心跳 线程失败");
        return Ret_failed;
    }
    if (0 != pthread_join(sendP, &result))
    {
        perror("等待 发送消息 线程失败");
        return Ret_failed;
    }
    if (0 != pthread_join(recvP, &result))
    {
        perror("等待 接收消息 线程失败");
        return Ret_failed;
    }
    return Ret_success;
}



#pragma mark -- 格式化发送数据
int TCPClient::SendDataFormat(char *buf, unsigned int bufLen, unsigned int cmd, const char userId[])
{
    if(NULL == buf)
    {
        return -1;
    }
    size_t uIdLen = (NULL == userId ? 0 : strlen(userId));
    if(bufLen < (sizeof(MsgReq) + uIdLen))
    {
        return -2;
    }
    memset(buf, 0, bufLen);
    MsgReq *msgReq = (MsgReq *)buf;
    memcpy(&msgReq->magic, MAGIC, 4);
    msgReq->cmd = htonl(cmd);
    if(userId != NULL)
    {
        memcpy(msgReq->userId, userId, uIdLen);
    }
    return (int)(uIdLen + sizeof(MsgReq));
}


#pragma mark -- 数据解析
int TCPClient::RecvDataParse(char *buf, unsigned int msgLen, unsigned int *cmd, unsigned int *status, char *userId)
{
    if(NULL == buf)
    {
        return -1;
    }
    if(msgLen < (sizeof(MsgResp)))
    {
        return -2;
    }
    
    MsgResp *resp = (MsgResp *)buf;
    
    if (0 != memcmp(MAGIC, &resp->magic, 4))
    {
        return -3;
    }

    if(NULL != cmd)
    {
        *cmd = ntohl(resp->cmd);
    }
    
    if(NULL != status)
    {
        *status = ntohl(resp->status);
    }
    
    if(NULL != userId && msgLen > sizeof(MsgResp))
    {
        memcpy(userId, resp->userId, msgLen - sizeof(MsgResp));
    }
    return 0;
}



#pragma mark -- 心跳发送
void* SendHeartBeat(void* arg)
{
    std::cout << "发送心跳线程启动.\n";
    TCPClient* client = (TCPClient*)arg;
    MsgHead *head     = (MsgHead *)malloc(BUFF_SIZE);
    static int count  = 0;
    std::string msgStr;
    size_t msgLen;
    while(1)
    {
        count++;
        memset(head, 0, BUFF_SIZE);
        std::stringstream msgSS;
        msgSS << "客户端心跳："<< count;
        msgStr = msgSS.str();
        msgLen = msgStr.length();
        
        head->msgType = Msg_heart;
        head->msgLen  = (unsigned int)msgLen;
        memcpy(head->data, msgStr.c_str(), msgLen);

        ssize_t sendLen = send(client->clientSockfd, head, sizeof(head) + msgLen, 0);
        if (0 > sendLen)    // 出错
        {
            std::cout << "发送心跳出错！" << std::endl;
            free(head);
            exit(1);
        }
        else if (0 == sendLen)  // 连接已断开
        {
            std::cout << "连接已断开，发送心跳失败！" << std::endl;
            free(head);
            exit(1);
        }
        else    // 发送成功
        {
//            std::cout << "发送心跳成功！" << std::endl;
        }
        sleep(HB_INTERVAL);
    }
    
    return NULL;
}


#pragma mark -- 发送消息
void* SendMsg(void *arg)
{
    std::cout << "发送消息线程启动！" << std::endl;
    TCPClient *client = (TCPClient *)arg;
    
    char sendBuf[BUFF_SIZE];
    MsgHead *reqMsgH = (MsgHead *)sendBuf;
    unsigned int reqCmd;
    size_t headLen = sizeof(MsgHead);
    while(1)
    {
        std::cout << "请输入操作：" << std::endl;
        std::cout << "【0】.退出\t" << "【1】.用户状态\t" << "【3】.唤醒用户\t" << "【5】.挂起用户" << std::endl;
        
        memset(reqMsgH, 0, BUFF_SIZE);
        
        std::cin >> reqCmd;
        
        reqMsgH->msgType = Msg_other;
        
        if (0 == reqCmd)
        {
            std::string msgStr = "quit";
            reqMsgH->msgLen = (unsigned int)msgStr.length();
            memcpy(reqMsgH->data, msgStr.c_str(), reqMsgH->msgLen);
        }
        else
        {
            reqMsgH->msgLen  = client->SendDataFormat(reqMsgH->data, (unsigned int)(BUFF_SIZE - headLen), reqCmd, USER_ID);
        }
        
        ssize_t sendLen = send(client->clientSockfd, reqMsgH, reqMsgH->msgLen + headLen, 0);
        
        if (0 > sendLen)    // 出错
        {
            std::cout << "发送消息出错！" << std::endl;
            exit(1);
        }
        else if (0 == sendLen)  // 连接已断开
        {
            std::cout << "发送消息失败，连接已断开！" << std::endl;
            exit(1);
        }
        else    // 发送成功！
        {
            std::cout << "成功发送字节数：" << sendLen << std::endl;
        }
    }
    return NULL;
}


#pragma mark -- 接收消息
void* RecvMsg(void *arg)
{
    std::cout << "接收消息线程启动！" << std::endl;
    TCPClient *client = (TCPClient *)arg;
    char recvBuf[BUFF_SIZE];
    ssize_t recvLen;
    
    unsigned int cmd = 0;
    unsigned int status = 0;
    char userId[64];
    
    while (1)
    {
        memset(recvBuf, 0, BUFF_SIZE);
        recvLen = recv(client->clientSockfd, recvBuf, BUFF_SIZE, 0);
        if (0 > recvLen)    // 出错
        {
            std::cout << "接收消息出错！" << std::endl;
            exit(1);
        }
        else if (0 == recvLen)  // 连接已断开
        {
            std::cout << "接收消息失败，连接已断开！" << std::endl;
            exit(1);
        }
        else    // 接收成功
        {
            MsgHead *msgHead = (MsgHead*)recvBuf;
            if(Msg_heart == msgHead->msgType)    // 心跳消息
            {
                
            }
            else
            {
                memset(userId, 0, 64);
                
                if (0 == client->RecvDataParse(msgHead->data, msgHead->msgLen, &cmd, &status, userId))
                {
                    std::cout << "接收到服务端消息：cmd:" << cmd << "\tstatus:" << status << "\tuserId:" << userId << std::endl;
                }
                else
                {
                    std::cout << "接收到服务端消息：" << msgHead->data << std::endl;
                    if (0 == strcmp(msgHead->data, "quit"))   // 遇到 “quit” 则退出
                    {
                        close(client->clientSockfd);
                        exit(1);
                    }
                }
            }
        }
    }
    return NULL;
}

