#include "chatserver.hpp"

ChatServer::ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+Port
               const string &nameArg)
            : _server(loop,listenAddr,nameArg),_loop(loop)
{
    // 给服务器注册用户连接的创建和断开回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

    // 给服务器注册用户读写事件回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置服务器端的线程数量 1个I/O线程（监听新用户的连接事件）， 3个worker线程
    //不设置的话，就1个线程而已，要处理连接又要处理业务 
    _server.setThreadNum(4);//设置4个线程，1个I/O线程，3个worker线程 
}

void ChatServer::start()
{
    _server.start();
}


void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if (!conn->connected()) 
    {
        ChatService::instance()->clientCloseException(conn); //客户端异常关闭 
        conn->shutdown();//相当于这些close(fd)
        //_loop->quit();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, 
                   Buffer *buffer,              
                   Timestamp time)
{
    string buf = buffer->retrieveAllAsString();//收到的数据放到这个字符串中 
    
    //数据的反序列化
    json js = json::parse(buf);

        //达到的目的：完全解耦网络模块的代码和业务模块的代码
    //通过js["msgid"] 获取=》业务handler处理器（在业务模块事先绑定好的）=》conn  js  time传给你 
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());//转成整型 
    //回调消息绑定好的事件处理器，来执行相应的业务处理，一个ID一个操作 
    msgHandler(conn, js, time);
}