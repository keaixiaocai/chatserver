#ifndef DB_H
#define DB_H

#include "mysql/mysql.h"
#include <muduo/base/Logging.h> //muduo的日志 
#include <string>
#include <iostream>

using namespace std;

// 数据库操作类
class MySQL
{
public:
	// 初始化数据库连接
	MySQL();

	// 释放数据库连接资源
	~MySQL();

	// 连接数据库
	bool connect();

	// 更新操作 insert、delete、update
	bool update(string sql);

	// 查询操作 select
	MYSQL_RES* query(string sql);

    //获取连接
    MYSQL* getConnection();

private:
	MYSQL* _conn; // 表示和MySQL Server的一条连接
};

#endif // !DB_H