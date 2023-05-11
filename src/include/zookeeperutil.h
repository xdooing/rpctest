#pragma once
#include<string>
#include<zookeeper/zookeeper.h>

// 封装zk客户端类
class ZKClient{
public:
    ZKClient();
    ~ZKClient();
    // 启动连接zkserver
    void Start();
    // 根据path创建znode节点
    void Create(const char* path,const char* data,int datalen,int state = 0); // 0 -- 永久性节点
    // 根据path获取节点的值
    std::string GetData(const char* path);

private:
    zhandle_t* m_zhandle;  // 句柄
};