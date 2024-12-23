#pragma once

#include "groupuser.hpp"

#include <vector>
#include <iostream>

class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
    {
        this->id_ = id;
        this->name_ = name;
        this->desc_ = desc;
    }

    void setId(int id) { this->id_ = id; }
    void setName(std::string name) { this->name_ = name; }
    void setDesc(std::string desc) { this->desc_ = desc; }

    int getId() { return this->id_; }
    std::string getName() { return this->name_; }
    std::string getDesc() { return this->desc_; }
    std::vector<GroupUser> &getUsers() { return this->users; }

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users;
};