#include "offlinemsgmodel.hpp"

//存储用户的离线消息
void OfflineMsgModel::insert(int userid,string msg)
{
    //1.组装sql语句
    char sql[1024] = {};

    sprintf(sql,"insert into offlinemessage(userid,message) values('%d','%s')",
    userid,msg.c_str());

    MySQL mysql;//定义一个mysql对象

    if(mysql.connect()) //连接成功了 
    {
        mysql.update(sql);//更新这个sql语句传进去
    }
}

//删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    //1.组装sql语句
    char sql[1024] = {};

    sprintf(sql,"delete from offlinemessage where userid='%d'",userid);

    MySQL mysql;//定义一个mysql对象
    
    if(mysql.connect()) //连接成功了 
    {
        mysql.update(sql);//更新这个sql语句传进去
    }
}

//查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    //1.组装sql语句
    char sql[1024] = {};

    sprintf(sql,"select message from offlinemessage where userid='%d'",userid);

    MySQL mysql;//定义一个mysql对象
    vector<string> vec;

    if(mysql.connect()) //连接成功了 
    {
        MYSQL_RES * res = mysql.query(sql);//调用mysql数据库的查询

        if(res != nullptr)
        {
            //把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row ; //获取行，用主键查的，查一行
            
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]); //加入数组中
            }
            mysql_free_result(res); //释放资源
            return vec;
        }
    }

    return vec;
}