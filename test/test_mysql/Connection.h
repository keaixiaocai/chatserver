#pragma once

/*
数据库操作代码、增删该查代码实现
*/
#include <iostream>
#include <mysql/mysql.h>
#include <string>
using namespace std;
#include "public.h"
// 数据库操作类
class Connection
{
public:
	// 初始化数据库连接
	Connection();

	// 释放数据库连接资源
	~Connection();

	// 连接数据库
	bool connect(string ip,
		unsigned short port,
		string user,
		string password,
		string dbname);

	// 更新操作 insert、delete、update
	bool update(string sql);

	// 查询操作 select
	MYSQL_RES* query(string sql);

private:
	MYSQL* _conn; // 表示和MySQL Server的一条连接
};