#pragma once

#include <string>

// User表的ORM类
class User 
{
public:
    User(int id = -1, std::string name = "", 
        std::string password = "", std:: string state = "offline")
    {
        this->id_ = id;
        this->name_ = name;
        this->password_ = password;
        this->state_ = state;
    }

    void setId(int id) { this->id_ = id; }
    void setName(std::string name) { this->name_ = name; }
    void setPassword(std::string password) { this->password_ = password; }
    void setState(std::string state) { this->state_ = state; }

    int getId() { return this->id_; }
    std::string getName() { return this->name_; }
    std::string getPassword() { return this->password_; }
    std::string getState() { return this->state_; }

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};