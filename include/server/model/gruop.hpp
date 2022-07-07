#ifndef GROUP_H
#define GROUP_H

#include <iostream>
#include "groupuser.hpp"
#include <vector>

using namespace std;

//Allgroup表的ORM类
class Group
{
public:
    Group(int id = -1,string name = "",string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id) {this->id = id; }
    void setName(string name) {this->name = name;}
    void setDesc(string desc) {this->desc = desc;}

    int getId() {return this->id;}
    string getName() {return this->name;}
    string getDesc() {return this->desc;}   

    vector<GroupUser> &getUsers() {return this->users;}
protected:
    int id;
    string name;
    string desc;   //组的功能描述
    vector<GroupUser> users;  //组的成员
};

#endif // !GROUP_H
