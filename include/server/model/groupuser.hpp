#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

//群组用户，多了一个role角色信息（创建者还是成员），从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(string role) {this->grouprole = role;}

    string getRole() {return this->grouprole;}

private:
    string grouprole;
};

#endif