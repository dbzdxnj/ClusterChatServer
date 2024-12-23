#pragma once

#include "user.hpp"

// 群组用户，多了一个role角色，从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(std::string role)
    {
        this->role_ = role;
    }

    std::string getRole()
    {
        return this->role_;
    }

private:
    std::string role_;
};