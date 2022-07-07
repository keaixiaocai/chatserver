#include "json.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "user.hpp"
#include "gruop.hpp"
#include "groupuser.hpp"
#include "public.hpp"

using namespace std;
using json = nlohmann::json;

// 记录当前登录用户的信息
User g_currentUser;
// 记录当前用户朋友的信息
vector<User> g_currentFriendList; 
// 记录当前用户群组的信息
vector<Group> g_currentGroupList;

//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();

// 显示登录用户的基本信息
void showCurrentUserData();

//接收线程 控制台应用程序，接收用户的手动输入，用户不输入cin就阻塞住，所以要2个线程 
void readTaskHandler(int clientfd);

//显示主菜单
void mainMenu(int clientfd);

//控制主菜单页面程序
bool isMainMenuRunning = false;

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接受线程
int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        cerr << "command invalid example : ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    //创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM , 0);
    if(-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //填写client需要连接的server信息ip+port
    struct sockaddr_in serv;
    memset(&serv,0,sizeof(sockaddr_in));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = inet_addr(ip);

    // client和server进行连接
    if(-1 == connect(clientfd,(const sockaddr *)&serv,sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    //main线程用于接受用户输入，负责发送数据
    for(;;)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); //读掉缓冲区残留的回车

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char password[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get();  //读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(password,50); //读掉缓冲区残留的回车

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = password;
            string request = js.dump();

            if(-1 == send(clientfd,request.c_str(),strlen(request.c_str())+1,0))
            {
                cerr << "send login msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                if(-1 == recv(clientfd,buffer,1024,0))
                {
                    cerr << "recv login msg error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);

                    if(0 != responsejs["errno"].get<int>()) //登录失败
                    {
                        cerr << responsejs["errmsg"] << endl;
                    }
                    else //登录成功
                    {
                        cout << responsejs["errmsg"] << endl;

                        // 记录当前用户的id和name
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);

                        // 记录当前用户的好友列表信息
                        if(responsejs.contains("friends"))
                        {
                            g_currentFriendList.clear();
                            vector<string> vec = responsejs["friends"];
                            for(string &str:vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);

                                g_currentFriendList.push_back(user);
                            }  
                        }

                        if(responsejs.contains("groups"))
                        {
                            g_currentGroupList.clear();
                            vector<string> vec1 = responsejs["groups"];
                            for(string &str1:vec1)
                            {
                                json grpjs = json::parse(str1);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["groupname"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for(string &str2:vec2)
                                {
                                    json js = json::parse(str2);
                                    GroupUser user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);

                                    group.getUsers().push_back(user);
                                }
                                g_currentGroupList.push_back(group);
                            }
                        }

                        // 显示登录用户的基本信息
                        showCurrentUserData();

                        if(responsejs.contains("offlinemsg"))
                        {
                            vector<string> vec = responsejs["offlinemsg"];
                            for(string &str: vec)
                            {
                                json js = json::parse(str);
                                if(ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    // time + [id] + name + " said: " + xxx
                                    cout << js["time"] << " [ " << js["id"] << " ] " << js["name"] 
                                    << " said: " << js["msg"] << endl;
                                }
                                else if(GROUP_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    // time + [id] + name + " said: " + xxx
                                    cout << "群消息[" << js["groupid"] << "]:" << js["time"] 
                                    << " [ " << js["id"] << " ]" << js["name"] << " said: " << js["msg"] 
                                    << endl;
                                    continue;             
                                }
                            }                            
                        }

                        isMainMenuRunning = true;
                        //登陆成功，启动接受线程负责接受数据
                        static int readsignal = 0;
                        if(readsignal == 0)
                        {
                            std::thread readTask(readTaskHandler,clientfd);
                            readTask.detach();
                            readsignal++;
                        }

                        // 进入聊天主菜单页面
                        mainMenu(clientfd);                        
                    }
                }   
            }
        }
        cin.get();
        system("clear");
        break;
        case 2:
        {
            char name[50] = {0};
            char password[50] = {0};
            cout << "name:" ;
            cin.getline(name,50); //读掉缓冲区残留的回车
            cout << "password:";
            cin.getline(password,50); //读掉缓冲区残留的回车

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = password;
            string request = js.dump();

            if(-1 == send(clientfd,request.c_str(),strlen(request.c_str())+1,0))
            {
                cerr << "send reg msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                int len = recv(clientfd,buffer,1024,0);
                if(-1 == len)
                {
                    cerr << "recv reg response error" << endl;
                }
                else 
                {
                    json responsejs = json::parse(buffer);
                    if(0 != responsejs["errno"].get<int>())
                    {
                        cerr << name << " is already exist , register error !" << endl;
                    }
                    else //注册成功
                    {
                        cerr << "register success , userid is " 
                        << responsejs["id"].get<int>() << 
                        " , dot not forget it !" << endl;
                    }
                }
            }     
        }
        cin.get();
        system("clear");
        break;
        case 3: //quit业务
            close(clientfd);
            exit(-1);
            break;            
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
}

// 显示登录用户的基本信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" 
    << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if(!g_currentFriendList.empty()) //如果好友列表不为空
    {
        int i = 1;
        for(User &user:g_currentFriendList)
        {
            cout << "第" << i << "个好友信息:" << user.getId() 
            << " " << user.getName() << " " << user.getState() << endl;
            i++;
        }
    }
    cout << "----------------------group list---------------------" << endl;
    if(!g_currentGroupList.empty())//群组信息不为空，才打印出来
    {
        for(Group &group:g_currentGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " 
            << group.getDesc() << endl;

            int i = 1;
            for(GroupUser &user:group.getUsers())
            {
                cout << "第" << i << "个人信息:" << user.getId() 
                << " " << user.getName() << " " << user.getState() << 
                " " << user.getRole() << endl;
                i++;
            }
        }
    }
}

// 接受线程
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024 , 0);
        if(-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }

        // 接受ChatServer转发的数据，反序列化生产json数据对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if(ONE_CHAT_MSG == msgtype)
        {
            // time + [id] + name + " said: " + xxx
            cout << js["time"] << " [ " << js["id"] << " ] " << js["name"] 
            << " said: " << js["msg"] << endl;
            continue; 
        }
        else if(GROUP_CHAT_MSG == msgtype)
        {
            // time + [id] + name + " said: " + xxx
            cout << "群消息[" << js["groupid"] << "]:" << js["time"] 
            << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"] 
            << endl;
            continue;             
        }
    }
}

// "help" command handler
void help(int fd = 0 , string str = " ");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
//"addgroup" command handler
void addgroup(int, string);
//"groupchat" command handler
void groupchat(int, string);
//"groupchat" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string,string> commandMap = {
    {"help","显示所有支持的命令,格式help"},
    {"chat","一对一聊天,格式chat:friendid:message"},
    {"addfriend","添加好友,格式addfriend:friendid"},
    {"creategroup","创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}
};

// 注册系统支持的客户端命令处理
unordered_map<string,function<void(int,string)>> commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};

//"help" command handler
void help(int, string)
{
    cout << "show command list >>>" << endl;
    for(auto &it:commandMap)
    {
        cout << it.first << ":" << it.second << endl;
    }
    cout << endl;
}

//主聊天页面程序，先显示一下系统支持的命令 
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(':');
        if(-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx);
        }

        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr << "invalid command" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调,mainMenu对修改封闭,添加新功能
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
    }
}

//"chat" command handler
void chat(int clientfd,string str)
{
    int idx = str.find(':');
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi((str.substr(0,idx)).c_str());
    string message = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();
    if(-1 == send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0))
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

//"addfriend" command handler
void addfriend(int clientfd,string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

//"creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd,string str)
{
    int idx = str.find(':');
    if(-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;        
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    if(-1 == send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0))
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

//"addgroup" command handler
void addgroup(int clientfd,string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    if(-1 == send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0))
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

//"groupchat" command handler   groupid:message
void groupchat(int clientfd,string str)
{
    int idx = str.find(':');
    if(-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;        
    }

    int groupid = atoi((str.substr(0,idx)).c_str());
    string message = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    if(-1 == send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0))
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}

//"loginout" command handler
void loginout(int clientfd,string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    if(-1 == send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0))
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }

}

//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}


