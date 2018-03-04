# 网络编程之 socket 实战


> 远程唤醒用户实战列子。

## 回顾
> 在[上一例子](https://github.com/shenyuanluo/NetPrograming/tree/master/TCPDemo/TCPDemo_03)已经了解了 TCP 客户端与服务端的保活技术：**心跳机制**。
> 
> 现在通过模拟客户端请求服务端对远端用户的一些操作进行实战演练。

本例子由客户端给服务器发送相关请求数据包，基本流程思路：

1. 设计相关的通信协议（数据结构）；
2. 服务端根据客户端的不同请求命令，执行响应的操作予以回复客户端。


# 相关通信协议
``` C++
/** 命令类型 枚举 */
typedef enum __cmdType {
    Cmd_reqStatus           = 0x01,     // 请求用户状态
    Cmd_respStatus          = 0x02,     // 回复用户状态
    Cmd_reqWakeUp           = 0x03,     // 请求唤醒用户
    Cmd_respWakeUp          = 0x04,     // 回复唤醒用户
    Cmd_reqSuspend          = 0x05,     // 请求挂起用户
    Cmd_respSuspend         = 0x06,     // 回复挂起用户
}CmdType;


/** 用户状态 枚举 */
typedef enum __userStatus {
    Status_unknow           = 0,        // 未知状态
    Status_online           = 1,        // 在线状态
    Status_waking           = 2,        // 正在唤醒
    Status_suspending       = 3,        // 正在挂起
    Status_suspended        = 4,        // 挂起状态
    Status_offline          = 5,        // 离线状态
}UserStatus;


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


/** 请求消息体 结构体 */
typedef struct __msgReq {
    unsigned int            magic;      // 魔术符
    unsigned int            cmd;        // 命令类型
    unsigned char           userId[0];  // 用户id号
}MsgReq;


/** 响应消息体 结构体 */
typedef struct __msgResp {
    unsigned int            magic;      // 魔术符
    unsigned int            cmd;        // 命令类型
    unsigned int            status;     // 用户状态
    unsigned char           userId[0];  // 用户id号
}MsgResp;
```