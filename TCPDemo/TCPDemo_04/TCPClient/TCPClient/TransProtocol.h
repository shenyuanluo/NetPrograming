//
//  TransProtocol.h
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/  All rights reserved.
//

#ifndef TransProtocol_h
#define TransProtocol_h


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


#endif /* TransProtocol_h */
