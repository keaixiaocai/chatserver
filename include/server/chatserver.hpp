#ifndef CHATSERVER_H
#define CHATSERVER_H

/*
muduo网络库给用户提供了两个主要的类
TcpServer ： 用于编写服务器程序的
TcpClient ： 用于编写客户端程序的

epoll + 线程池
好处：能够把网络I/O的代码和业务代码区分开
                        用户的连接和断开       用户的可读写事件
*/

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写时间的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>//事件循环 
#include <iostream>
#include <string>
#include <functional> //绑定器 
#include "json.hpp"
#include "chatservice.hpp"

//muduo的名字空间作用域
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders; //参数占位符
using json = nlohmann::json;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+Port
               const string &nameArg);         // 服务器的名字

    void start();//开启事件循环 


private:

    //专门处理：用户的连接创建和断开 epoll listenfd accept
    //如果有新用户的连接或者断开，muduo库就会调用这个函数
    void onConnection(const TcpConnectionPtr &conn);

    //专门处理：用户的读写事件，muduo库去调用这个函数 
    void onMessage(const TcpConnectionPtr &conn, // 连接，通过这个连接可以读写数据 
                   Buffer *buffer,               // 缓冲区，提高数据收发的性能
                   Timestamp time) ;              // 接收到数据的时间信息

    TcpServer _server; // 第一步
    EventLoop *_loop;  //第二步相当于 epoll 事件循环的指针，有事件发生，loop上报 
};


#endif 