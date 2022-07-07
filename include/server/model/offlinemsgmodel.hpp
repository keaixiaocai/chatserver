#ifndef OFFLINEMSGEMODEL_H
#define OFFLINEMSGEMODEL_H

#include <iostream>
#include <vector>
#include "db.hpp"

using namespace std;

//提供离线消息表的操作接口方法
class OfflineMsgModel
{
public:
    //存储用户的离线消息
    void insert(int userid,string msg);
    //删除用户的离线消息
    void remove(int userid);
    //查询用户的离线消息
    vector<string> query(int userid);
};

#endif // !OFFLINEMSGEMODEL_H
