#include "thirdparty/json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
using namespace std;

string func1()
{
    json js;

    js["id"] = {1,2,3,4,5};

    js["name"] = "zhangsan";

    js["msg"]["zhangsan"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";

    js["msg"] = {{"zhangsan","hello wordl"},{"liu shuo","hello china"}};
    
    string sendbuf = js.dump();
    return sendbuf;
}

string func2()
{
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;

    string sendbuf = js.dump();
    return sendbuf;
}

int main()
{
    string jsonstr1 = func1();
    cout << jsonstr1 << endl;
    string jsonstr2 = func2();
    cout << jsonstr2 << endl;

    json js1 = json::parse(jsonstr1);
    json js2 = json::parse(jsonstr2);

    auto arr = js1["id"];
    string name = js1["name"];
    auto it = js1["msg"];
    vector<int> vec = js2["list"];
    map<int,string> mymap = js2["path"];

    cout << arr[0] << endl;
    cout << name << endl;
    cout << it["zhangsan"] << endl;
    cout << it["liu shuo"] << endl;
    
    for(auto val:vec)
    {
        cout << val << " ";
    }

    cout << endl;

    for(auto it = mymap.begin() ; it != mymap.end() ; it++)
    {
        cout << it->first << " " << it->second << endl;
    }

    return EXIT_SUCCESS;
}



