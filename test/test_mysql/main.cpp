#include <iostream>
#include <string>
using namespace std;
#include "Connection.h"

int main()
{
	Connection conn;

	string sql = "insert into employee1(sid,name,sex,salary) values(4,'蔡伟','male',11111)";

	conn.connect("127.0.0.1", 3306, "root", "123456", "myb2");

	conn.update(sql);

	return 0;
}