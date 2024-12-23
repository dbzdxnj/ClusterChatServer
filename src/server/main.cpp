#include "chatserver.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <signal.h>

// 处理服务器ctrl + C 结束后重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

// 主函数，启动函数
int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(6000, "127.0.0.1");
    ChatServer server(&loop, addr, "ChatServer");

    // 启动服务器
    server.start();
    // 开启事件循环
    loop.loop();

    return 0;
}