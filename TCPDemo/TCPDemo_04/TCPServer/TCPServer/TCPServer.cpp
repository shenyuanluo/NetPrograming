//
//  TCPServer.cpp
//  TCPServer
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include "TCPServer.hpp"
#include "TransProtocol.h"


#define MAGIC "SYL "
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
    
    int opt = 1;
    // 让端口释放后立即就可以被再次使用。
    setsockopt(serverSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 设置 服务端 地址
    serAddrLen = sizeof(serverAddr);
    memset(&serverAddr, 0, serAddrLen);                     // 清空结构体
    serverAddr.sin_family      = AF_INET;                   // 协议族 类型
    serverAddr.sin_port        = htons(port);               // 服务端 端口
    serverAddr.sin_addr.s_addr = inet_addr(ipStr.c_str());  // 服务端 IP 地址
    
    maxFd = serverSockfd;               // 初始化 maxFd
    FD_ZERO(&masterSet);                // 情况 masterSet
    FD_SET(serverSockfd, &masterSet);   // 添加监听 fd
}


TCPServer::~TCPServer()
{
    for (int sockfd = 0; sockfd <= maxFd; sockfd++)
    {
        if (FD_ISSET(sockfd, &masterSet))
        {
            std::cout << "关闭（sockfd = " << sockfd << "） ！" << std::endl;
            close(sockfd);  // 关闭 sockfd
        }
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
    pthread_t threadId;
    // 创建心跳检测线程
    int ret = pthread_create(&threadId, NULL, CheckHeartBeat, (void*)this);
    if (0 != ret)
    {
        std::cout << "创建 心跳检测 线程失败！" << std::endl;
        return Ret_failed;
    }
    while (1)
    {
        FD_ZERO(&workingSet);   // 每次循环清空 working set
        memcpy(&workingSet, &masterSet, sizeof(masterSet));
        
        timeout.tv_sec  = 30;
        timeout.tv_usec = 0;
        int nums = select(maxFd + 1, &workingSet, NULL, NULL, &timeout);
        if (0 > nums)   // 出错
        {
            std::cout << "select 出错！" << std::endl;
            return Ret_error;
        }
        else if (0 == nums) // 超时，可以通过心跳检测判断是否断开
        {
            continue;
        }
        else    // 有新内容可读写
        {
            if (FD_ISSET(serverSockfd, &workingSet))    // 有新客户端连接请求
            {
                AcceptConn();
            }
            else    // 客户端有消息更新
            {
                RecvMsg();
            }
        }
    }
    return Ret_success;
}


RetValue TCPServer::AcceptConn()
{
    // 客户端 地址
    struct sockaddr_in clientAddr;
    socklen_t cliAddrLen = sizeof(clientAddr);
    
    // 阻塞式等待客户端连接
    std::cout << "等待客户端连接。。。" << std::endl;
    int clientSockfd = accept(serverSockfd, (struct sockaddr*)&clientAddr, &cliAddrLen);
    if (0 > clientSockfd)
    {
        std::cout << "服务器接受客户端连接失败！" << std::endl;
        return Ret_failed;
    }
    std::string ipStr = inet_ntoa(clientAddr.sin_addr);
    std::cout << "服务器接受客户端（sockfd = " << clientSockfd << " ，ip = " << ipStr << "）连接成功！" << std::endl;
    
    // 添加 客户端 sockfd
    sockfdMap.insert(make_pair(clientSockfd, make_pair(ipStr, 0)));
    
    // 将新建的连接 clientSockfd 加入 masterSet
    FD_SET(clientSockfd, &masterSet);
    
    // 更新 max sockfd
    if (clientSockfd > maxFd)
    {
        maxFd = clientSockfd;
    }
    return Ret_success;
}


#pragma mark -- 格式化发送数据
int TCPServer::RespDataFormat(char *buf, unsigned int bufLen, unsigned int cmd, unsigned int status, const char userId[])
{
    if(NULL == buf)
    {
        return -1;
    }
    size_t uIdLen = (NULL == userId ? 0 : strlen(userId));
    if(bufLen < (sizeof(MsgResp) + uIdLen))
    {
        return -2;
    }
    MsgResp *msgResp = (MsgResp *)buf;
    memcpy(&msgResp->magic, MAGIC, 4);
    msgResp->cmd = htonl(cmd);
    msgResp->status = htonl(status);
    if(userId != NULL)
    {
        memcpy(msgResp->userId, userId, uIdLen);
    }
    return (int)(uIdLen + sizeof(MsgResp));
}


#pragma mark -- 数据解析
int TCPServer::ReqDataParse(char *buf, unsigned int msgLen, unsigned int *cmd, char *userId)
{
    if(NULL == buf)
    {
        return -1;
    }
    if(msgLen < (sizeof(MsgReq)))
    {
        return -2;
    }
    
    MsgReq *msgReq = (MsgReq *)buf;
    
    if (0 != memcmp(MAGIC, &msgReq->magic, 4))
    {
        return -3;
    }
    
    if(NULL != cmd)
    {
        *cmd = ntohl(msgReq->cmd);
    }
    
    if(NULL != userId && msgLen > sizeof(MsgReq))
    {
        memcpy(userId, msgReq->userId, msgLen - sizeof(MsgReq));
    }
    return 0;
}


RetValue TCPServer::RecvMsg()
{
    char recvBuf[BUFF_SIZE];
    char sendBuf[BUFF_SIZE];
    
    ssize_t recvLen = 0;
    ssize_t sendMsgLen = 0;
    
    for(int sockfd = 0; sockfd <= maxFd; sockfd++)
    {
        if(FD_ISSET(sockfd, &workingSet))   // 有数据可读写
        {
            bool closeConn = false;         // 标记当前连接是否断开了
            
            memset(recvBuf, 0, BUFF_SIZE);
            memset(sendBuf, 0, BUFF_SIZE);
            
            recvLen = recv(sockfd, &recvBuf, BUFF_SIZE, 0); // 接收消息
            if (0 > recvLen)    // 出错
            {
                std::cout << "接收消息出错（sockfd = " << sockfd << "）！" << std::endl;
                closeConn = true;
            }
            else if (0 == recvLen) // 连接已断开
            {
                std::cout << "接收消息失败，（sockfd = " << sockfd << "）客户端连接已断开！" << std::endl;
                closeConn = true;
            }
            else
            {
                MsgHead *recvMsgH = (MsgHead*)recvBuf;
                if(Msg_heart == recvMsgH->msgType)    // 心跳消息
                {
                    sockfdMap[sockfd].second = 0;   // 每次收到心跳包，count重置为0，标识连接中
                    
                    std::cout << "接收客户端（sockfd = " << sockfd << "）的心跳数据：" << recvMsgH->data << std::endl;
                }
                else    // 其他消息
                {
                    unsigned int reqCmd = 0;
                    unsigned int respCmd = 0;
                    char userId[512];
                    memset(userId, 0, 512);
                    
                    MsgHead *sendMsgH = (MsgHead *)sendBuf;
                    
                    if (0 == ReqDataParse(recvMsgH->data, recvMsgH->msgLen, &reqCmd, userId))
                    {
                        std::cout << "接收到客户端（sockfd = " << sockfd << ")请求：cmd:" << reqCmd << "\tuserId:" << userId << std::endl;
                        // 休眠 3 秒 模拟相关操作
                        sleep(3);
                        
                        respCmd = reqCmd + 1;
                        
                        switch ((CmdType)reqCmd)
                        {
                            case Cmd_reqStatus:
                            {
                               sendMsgLen = RespDataFormat(sendMsgH->data, BUFF_SIZE - sizeof(MsgHead), respCmd, Status_suspended, userId);
                            }
                                break;
                                
                            case Cmd_reqWakeUp:
                            {
                                sendMsgLen = RespDataFormat(sendMsgH->data, BUFF_SIZE - sizeof(MsgHead), respCmd, Status_waking, userId);
                            }
                                break;
                                
                            case Cmd_reqSuspend:
                            {
                                sendMsgLen = RespDataFormat(sendMsgH->data, BUFF_SIZE - sizeof(MsgHead), respCmd, Status_suspending, userId);
                            }
                                break;
                                
                            default:
                                break;
                        }
                        sendMsgH->msgLen = (unsigned int)sendMsgLen;
                    }
                    else
                    {
                        std::cout << "接收客户端（sockfd = " << sockfd << "）的数据：" << recvMsgH->data << std::endl;
//
                        sendMsgLen = recvLen - sizeof(MsgHead);
                        sendMsgH->msgLen = (unsigned int)sendMsgLen;
                        memcpy(sendMsgH->data, recvMsgH->data, sendMsgLen);
                    }
                    
                    sendMsgH->msgType = Msg_other;
                    
                    ssize_t sendLen = send(sockfd, sendBuf, sendMsgLen + sizeof(MsgHead), 0);
                    
                    if (0 > sendLen)    // 出错
                    {
                        std::cout << "发送消息出错！" << std::endl;
                        closeConn = true;
                    }
                    else if (0 == sendLen)  // 连接已断开
                    {
                        std::cout << "发送消息失败，连接已断开！" << std::endl;
                        closeConn = true;
                    }
                    else    // 发送成功！
                    {
                        if (0 == strcmp(recvMsgH->data, "quit"))   // 遇到 “quit” 则退出
                        {
                            std::cout << "客户端（sockfd = " << sockfd << "）已主动关闭" << std::endl;
                            closeConn = true;
                        }
                    }
                }
                
            }
            
            if(true == closeConn)
            {
                std::cout << "关闭客户端（sockfd = " << sockfd << "）" << std::endl;
                close(sockfd);                      // 关闭套接字
                FD_CLR(sockfd, &masterSet);         // 从集合中清除相应 sockfd
                sockfdMap.erase(sockfd);            // 从map中移除该记录
                
                if(sockfd == maxFd)                 // 需要更新 maxFd;
                {
                    while(false == FD_ISSET(maxFd, &masterSet))
                    {
                        maxFd--;
                    }
                }
            }
        }
    }
    return Ret_success;
}


#pragma mark -- 心跳检测 线程
void* CheckHeartBeat(void* arg)
{
    std::cout << "心跳检测 线程启动！" << std::endl;
    TCPServer* server = (TCPServer*)arg;
    while(1)
    {
        std::cout << "当前已连接客户端数量：" << server->sockfdMap.size() << std::endl;
        
        std::map<int, std::pair<std::string, int>>::iterator it = server->sockfdMap.begin();
        for( ; it != server->sockfdMap.end(); ) // 循环检测所有客户端连接 sockfd 心跳
        {
            if(CHECK_HB_COUNT <= it->second.second)   // （规定的时间内）没有收到心跳包，判定客户端掉线
            {
                int sockfd = it->first;
                std::string ipStr = it->second.first;
                
                std::cout << "客户端（sockfd = " << sockfd << "ipStr = " << ipStr << "）已经掉线" << std::endl;

                close(sockfd);                      // 关闭该连接
                FD_CLR(sockfd, &server->masterSet); // 从集合中清除相应 sockfd
                if(sockfd == server->maxFd)         // 需要更新 maxFd;
                {
                    // 更新 maxFd
                    while(false == FD_ISSET(server->maxFd, &server->masterSet))
                    {
                        server->maxFd--;
                    }
                }
                
                server->sockfdMap.erase(it++);      // 从map中移除该记录
            }
            else if(CHECK_HB_COUNT > it->second.second
                    && 0 <= it->second.second)
            {
                it->second.second += 1;
                it++;
            }
            else
            {
                it++;
            }
        }
        sleep(HB_INTERVAL);   // 定时检查
    }
}

