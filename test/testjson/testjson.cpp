#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map> 
#include <string>

// json序列化实例1
std::string func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello world";

    std::string sendBuf = js.dump();
    // std::cout << sendBuf << std::endl;
    return sendBuf;
}

// json 序列化示例2
void func2() {
    json js;
    js["id"] = {1, 2, 3, 4, 5};
    js["name"] = "zhang san";
    js["to"] = "li si";
    js["msg"]["liu shuo"] = "hello world";

    std::string sendBuf = js.dump();
    std::cout << sendBuf << std::endl;
}

// json 序列化示例3
std::string func3() {
    json js;
    
    //直接序列化一个vector容器
    std::vector<int> vec;
    for (int i = 0; i < 3; ++i) {
        vec.push_back(i * i);
    }
    js["list"] = vec;

    return js.dump();
}

int main()
{
    const std::string& recvBuf = func3();
    
    //数据的反序列化
    json js = json::parse(recvBuf);
    std::cout << js["list"] << std::endl;
    //std::cout << js["from"] << std::endl;
    return 0;
}
