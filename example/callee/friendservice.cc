#include<iostream>
#include"friend.pb.h"
#include"rpctestapplication.h"
#include"rpctestprovider.h"
#include<vector>
#include"logger.h"

// 发布的rpc方法
class FriendService : public fixbug::FriendServiceRpc{
public:
    std::vector<std::string> GetFriendList(uint32_t userid){
        std::cout << "do FriendServiceRpc! userid:" << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("xie dong");
        vec.push_back("cao peng");
        vec.push_back("zhang wei");
        return vec;
    }

    // 重写基类中的方法：
    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 从request取出数据
        uint32_t userID = request->userid();
        // 执行本地业务
        std::vector<std::string> friendLists = GetFriendList(userID);
        // 将响应写入框架
        // fixbug::ResultCode* res = response->mutable_res();
        response->mutable_res()->set_errcode(0);
        response->mutable_res()->set_errmsg(std::string());
        for(std::string& name : friendLists){
            std::string* p = response->add_friends();
            *p = name;
        }
        // 执行回调函数(响应数据序列化以及网络发送)
        done->Run();
    }
};


int main(int argc,char** argv){
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s%d",__FILE__,__FUNCTION__,__LINE__);

    // 框架初始化
    RpctestApplication::Init(argc,argv);

    // rpc网络服务对象，定义好的rpc方法发布到节点上
    RpctestProvider provider;
    provider.NotifyService(new FriendService());

    // 进程阻塞等待远程的rpc请求
    provider.Run();

    return 0;
}
