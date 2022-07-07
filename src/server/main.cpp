#include "chatserver.hpp"
#include <iostream>
#include <signal.h>

using namespace std;

//处理服务器ctrl+c结束后，进行重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset(); //调用重置 
    exit(0);
}

int main(int argc,char *argv[])
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    //解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    int port = atoi(argv[2]);

    signal(SIGINT,resetHandler);

    EventLoop loop;  //相当于像是创建了epoll
    InetAddress addr(ip,port); //IP地址，端口号
    ChatServer server(&loop,addr,"ChatServer");

    server.start();  //listenfd通过 epoll_ctl 添加到 epoll 
    loop.loop();    //相当于epoll_wait，以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}