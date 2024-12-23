#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <iostream>
#include <string>

class ChatServer {
public:
    ChatServer(EventLoop* loop, 
            const InetAddress& listenAddr,
            const std::string& nameArg) : 
        server_(loop, listenAddr, nameArg),
        loop_(loop)
    {
        // 给服务器注册用户连接和断开回调
        server_.setConnectionCallback(std::bind(
                &ChatServer::onConnection, this, std::placeholders::_1));

        // 给服务器注册用户读写事件回调
        server_.setMessageCallback(std::bind(
                &ChatServer::onMessage, this, std::placeholders::_1, 
                std::placeholders::_2, std::placeholders::_3
        ));

        server_.setThreadNum(4);
    }

    void start() {
        server_.start();
    }

private:

    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << " -> " << 
                conn->localAddress().toIpPort() << "Online" << std::endl;
        }
        else
        {
            std::cout << conn->peerAddress().toIpPort() << " -> " << 
                conn->localAddress().toIpPort() << "Offline" << std::endl;
                conn->shutdown();
                loop_->quit();
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        std::string buffer = buf->retrieveAllAsString();
        std::cout << "recv data : " << buffer << "time : " << time.toString() << std::endl;
        conn->send(buffer);
    }

    TcpServer server_;
    EventLoop *loop_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(6000, "127.0.0.1");
    ChatServer server(&loop, addr, "chatServer");

    server.start();
    loop.loop();
}