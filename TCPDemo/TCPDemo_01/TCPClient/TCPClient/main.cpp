//
//  main.cpp
//  TCPClient
//
//  Created by shenyuanluo on 2018/2/12.
//  Copyright © 2018年 http://blog.shenyuanluo.com/ All rights reserved.
//

#include <iostream>
#include "TCPClient.hpp"


#define DEMO_PORT 5578
#define DEMO_IP "127.0.0.1"


int main()
{
    TCPClient client(DEMO_IP, DEMO_PORT);
    if (Ret_success != client.ConnServer())
    {
        exit(1);
    }
    if (Ret_success != client.Run())
    {
        exit(1);
    }
    
    return 0;
}
