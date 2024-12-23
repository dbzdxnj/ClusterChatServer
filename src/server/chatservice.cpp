#include "chatservice.hpp"
#include "public.hpp"
#include "friendmodel.hpp"

#include <mymuduo/logger.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>

using namespace std::placeholders;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的回调操作
ChatService::ChatService()
{
    // 绑定登录事件的回调
    msgHandlerMap_.insert({LOGIN_MSG, std::bind(
            &ChatService::login, this, _1, _2, _3)});
    // 绑定注册事件的回调
    msgHandlerMap_.insert({REGISTER_MSG, std::bind(
            &ChatService::regist, this, _1, _2, _3)});
    // 绑定一对一聊天事件的回调
    msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(
            &ChatService::oneChat, this, _1, _2, _3)});
    // 绑定添加好友事件的回调
    msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(
            &ChatService::addFriend, this, _1, _2, _3)});
    // 绑定添加群组事件的回调
    msgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(
            &ChatService::addGroup, this, _1, _2, _3)});
    // 绑定创建群组事件的回调
    msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(
            &ChatService::createGroup, this, _1, _2, _3)});
    // 绑定群聊消息群组事件的回调
    msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(
            &ChatService::groupChat, this, _1, _2, _3)});
    // 绑定群聊消息群组事件的回调
    msgHandlerMap_.insert({LOGINOUT_MSG, std::bind(
            &ChatService::loginout, this, _1, _2, _3)});

    // 连接redis服务器
    if (redis_.connect())
    {
        // 设置上报消息的回调
        redis_.init_notify_handler(std::bind(
            &ChatService::handleRedisSubscribeMessage, this, _1, _2
        ));
    }
    
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    { 
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time) {
            LOG_ERROR("msgid : %d can not find handler !", msgid);
        };
    }
    else
    {
        return it->second;
    }
}
// 处理登录业务 id pwd 
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"];
    std::string password = js["password"];

    User user = userModel_.query(id);
    if (user.getId() == id && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "user is onlining";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息 保证connMutex_线程安全
            {
                std::lock_guard<std::mutex> lock(connMutex_);
                userConnMap_.insert({user.getId(), conn});
            }

            // 登录成功后向redis订阅id
            redis_.subscribe(id);

            // 登录成功，更新用户状态信息 offline -> online
            user.setState("online");
            userModel_.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            std::vector<std::string> vec = offlineMsgModel_.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取完该用户的离线消息后，把该用户的所有离线消息都删除掉
                offlineMsgModel_.remove(id);
            }

            // 查询该用户的好友信息并返回
            std::vector<User> friendVec = friendModel_.query(id);
            if (!friendVec.empty())
            {
                std::vector<std::string> vec2;
                for (auto &fri : friendVec)
                {
                    json js;
                    js["id"] = fri.getId();
                    js["name"] = fri.getName();
                    js["state"] = fri.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询该用户的群组信息并返回
            std::vector<Group> groupUserVec = groupModel_.queryGroups(id);
            if (!groupUserVec.empty())
            {
                std::vector<std::string> groupV;
                for (auto &group : groupUserVec)
                {
                    json js;
                    js["id"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();
                    std::vector<std::string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json userjs;
                        userjs["id"] = user.getId();
                        userjs["name"] = user.getName();
                        userjs["state"] = user.getState();
                        userjs["role"] = user.getRole();
                        userV.push_back(userjs.dump());
                    }
                    js["users"] = userV;
                    groupV.push_back(js.dump());
                }
                response["groups"] = groupV;
            }
            
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败:用户不存在 / 用户密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "user's id or password is wrong";
        conn->send(response.dump());
    }
}

// 处理注册业务 name password
void ChatService::regist(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = userModel_.insert(user);
    if (state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                // 从map表删除用户的连接信息
                userConnMap_.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于下线，从redis中取消订阅
    redis_.unsubscribe(user.getId());

    //更新用户状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toid = js["toid"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            // toid用户在线，转发消息，服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线
    User user = userModel_.query(toid);
    if (user.getState() == "online")
    {
        redis_.publish(toid, js.dump());
        return;
    }

    // toid用户不在线，存储离线消息
    offlineMsgModel_.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    friendModel_.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid  = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    // 存储创建的群组信息
    Group group(-1, name, desc);
    if (groupModel_.createGroup(group))
    {
        groupModel_.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid  = js["id"].get<int>();
    int groupid  = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid  = js["id"].get<int>();
    int groupid  = js["groupid"].get<int>();
    std::vector<int> userIdVec = groupModel_.queryGroupsUsers(userid, groupid);
    
    std::lock_guard<std::mutex> lock(connMutex_);
    for (int id : userIdVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            // 用户在线 直接转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else
            {
                // 用户不在线，存储离线消息
                offlineMsgModel_.insert(id, js.dump());
            }
        }
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
        {
            userConnMap_.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    redis_.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    userModel_.updateState(user);
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把online状态的用户，设置为offline
    userModel_.resetState();
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, std::string msg)
{
    std::lock_guard<std::mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel_.insert(userid, msg);
}