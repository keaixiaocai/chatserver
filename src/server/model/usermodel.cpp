#include "usermodel.hpp"

//User表的增加方法
bool UserModel::insert(User &user)
{
    //1.组装sql语句
    char sql[1024] = {};

    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",
    user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    MySQL mysql;//定义一个mysql对象

    if(mysql.connect())//连接成功了 
    {
        if(mysql.update(sql))//更新这个sql语句传进去 
        {
              //获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    else
    {
        return false;
    }
}

//根据用户号码查询用户信息
User UserModel::query(int id)
{
     //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = '%d'",id);

    MySQL mysql;//定义一个mysql对象

    if(mysql.connect())//连接成功了 
    {
        MYSQL_RES * res = mysql.query(sql); //调用mysql数据库的查询 
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res); //获取行，用主键查的，查一行
            if(row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);  //释放资源 
                return user;
            }
        }
    }

    return User();
}

bool UserModel::updateState(User user)
{
     //1.组装sql语句
    int id = user.getId();
    string state = user.getState();
    char sql[1024] = {0};
    sprintf(sql,"update user set state = '%s' where id = '%d'",state.c_str(),id);

    MySQL mysql;//定义一个mysql对象

    if(mysql.connect())//连接成功了 
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
     //1.组装sql语句
    string sql = "update user set state = 'offline';";

    MySQL mysql; //定义mysql对象 

    if(mysql.connect()) //连接成功
    {
        mysql.update(sql); //更新
    }
}