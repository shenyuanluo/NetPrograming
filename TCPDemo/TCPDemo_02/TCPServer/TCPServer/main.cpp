//
//  main.cpp
//  TCPServer
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include <iostream>
#include "TCPServer.hpp"


#define DEMO_PORT 5578
#define DEMO_IP "127.0.0.1"
#define MAX_LISTEN_QUEUE 10

int main()
{
    TCPServer server(DEMO_IP, DEMO_PORT);
    if (Ret_success != server.BindAddr())
    {
        exit(1);
    }
    if (Ret_success != server.Listen(MAX_LISTEN_QUEUE))
    {
        exit(1);
    }
    if (Ret_success != server.Run())
    {
        exit(1);
    }
    
    return 0;
}
