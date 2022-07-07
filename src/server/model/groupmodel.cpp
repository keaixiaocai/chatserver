#include "groupmodel.hpp"

//创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};

    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",
    group.getName().c_str(),group.getDesc().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    else
    {
        return false;
    }
}

//加入群组
void GroupModel::addGroup(int userid,int groupid,string role)
{
    char sql[1024] = {0};

    sprintf(sql,"insert into groupuser(groupid,userid,grouprole)\
    values('%d','%d','%s')",groupid,userid,role.c_str());

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

 //查询用户所在群组信息  在客户端呈现 
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
    1. 先根据userid在groupuser表中查询出该用户所属的群组信息
    2. 在根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，
    查出用户的详细信息
    */
    char sql[1024] = {0};

    sprintf(sql,"select a.id,a.groupname,a.groupdesc from allgroup a inner\
    join groupuser b on a.id = b.groupid where b.userid = '%d'",userid);
    //把指定用户的所在的群组信息全部描述出来
    MySQL mysql;

    vector<Group> groupVec;

    if(mysql.connect())
    {
        MYSQL_RES * res = mysql.query(sql);

        if(res != nullptr)
        {
            MYSQL_ROW row;
            //查出userid所有的群组的信息
            while((row = mysql_fetch_row(res)) != nullptr) 
            {
                Group group;

                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);

                groupVec.push_back(group);  
            }
            mysql_free_result(res); //释放资源 
        }
    }

    //查询群组的用户信息
    for(Group &group : groupVec)
    {
        char sql[1024] = {0};

        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from user \
        a inner join groupuser b on a.id = b.userid where b.groupid \
        = '%d'",group.getId());

        MySQL mysql;

        if(mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);

            if(res != nullptr)
            {
                MYSQL_ROW row;

                while((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }

                mysql_free_result(res); //释放资源
            }
        }        
    }
    return groupVec;//这个东西存着用户的所有群组和所有群组里的用户信息 
}

//根据指定的groupid查询群组用户id列表
//除userid自己，主要用户群聊业务给群组其它成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};

    sprintf(sql, "select userid from groupuser \
    where groupid = %d and userid != %d", groupid, userid);

    MySQL mysql;
    vector<int> idVec;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);

        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }

    return idVec;
}