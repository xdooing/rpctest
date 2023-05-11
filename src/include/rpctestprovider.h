#pragma once
#include"google/protobuf/service.h"
#include"muduo/net/TcpConnection.h"
#include"muduo/net/EventLoop.h"
#include<unordered_map>


// service服务类型信息
struct ServiceInfo{
    // 保存服务对象
    google::protobuf::Service* m_service;
    // 保存服务方法
    std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;
};


class RpctestProvider{
public:
    // 发布rpc方法，基类指针指向子类对象
    void NotifyService(google::protobuf::Service* service);
    // 启动服务
    void Run();
private:
    // 组合反应堆
    muduo::net::EventLoop m_evLoop;
    // 服务类型信息
    ServiceInfo m_serviceInfo;
    // 存储注册成功的服务对象和其服务方向的所有信息  名字<--->服务信息
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
    
private:
    // 新的socket连接回调
    void onConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接用户的读写事件回调
    void onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    // closure回调函数，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};