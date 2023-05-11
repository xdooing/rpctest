#include"rpctestapplication.h"
#include"friend.pb.h"
#include"rpctestchannel.h"
#include"rpctestcontroller.h"
#include<iostream>

int main(int argc,char**argv){
    // 首先要初始化框架
    RpctestApplication::Init(argc,argv);

    // 使用远程rpc方法调用的时候，需要定义代理对象stub
    fixbug::FriendServiceRpc_Stub stub(new RpctestChannel());

    // rpc请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    // rpc响应
    fixbug::GetFriendListResponse response;

    // 通过代理对象发起rpc方法调用，调用的底层为RpctestChannel::CallMethod
    RpctestController controller;
    stub.GetFriendList(&controller,&request,&response,nullptr);
    
    // 判断调用结果
    if(controller.Failed()){
        std::cout << controller.ErrorText() << std::endl;
    }
    else{
        // 远程调用完成，读取调用结果
        if(response.res().errcode() == 0){
            std::cout << "rpc GetFriendList response success!" << std::endl;
            int size = response.friends_size();
            for(int i = 0;i < size;++i){
                std::cout << "index:"<< i+1 << " name:" << response.friends(i) <<std::endl;
            }
        }
        else{
            std::cout << "rpc GetFriendList response error:" << response.res().errmsg() << std::endl;
        }
    }
    return 0;
}