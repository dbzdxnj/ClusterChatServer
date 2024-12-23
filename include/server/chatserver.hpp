#pragma once

#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>

class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop* loop, 
                const InetAddress &listenAddr,
                const std::string nameArg);
    // 启动服务
    void start();

private:
    // 连接建立断开的回调函数
    void onConnection(const TcpConnectionPtr&);

    // 读写事件的回调函数
    void onMessage(const TcpConnectionPtr&,
                    Buffer*,
                    Timestamp);

    TcpServer server_;  // muduo库中的server对象
    EventLoop *loop_;   // 指向事件循环对象的指针
};
