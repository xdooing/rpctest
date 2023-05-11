#include"zookeeperutil.h"
#include"rpctestapplication.h"
#include<iostream>
#include<semaphore.h>

// 全局的watcher观察器
void global_watcher(zhandle_t* zh,int type,int state,const char* path,void* watcherCtx){
    if(type == ZOO_SESSION_EVENT){         // 会话消息类型
        if(state == ZOO_CONNECTED_STATE){  // 连接成功
            sem_t* sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}


ZKClient::ZKClient() : m_zhandle(nullptr){

}

ZKClient::~ZKClient(){
    if(m_zhandle != nullptr){
        zookeeper_close(m_zhandle);
    }
}

// 启动连接zkserver
void ZKClient::Start(){
    std::string host = RpctestApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = RpctestApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    // zookeeper的API客户端提供了三个线程：1.zookeeper_init函数 2.网络IO线程（poll） 3.watcher回调线程
    // 异步连接过程
    m_zhandle = zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(m_zhandle == nullptr){
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);

    // 阻塞在此处，因为被初始化了0,等着zkserver响应（回调）。等加1。成功了才向下走
    sem_wait(&sem); 
    std::cout << "zookeeper_init success" << std::endl;
}

// 根据path创建znode节点
void ZKClient::Create(const char* path,const char* data,int datalen,int state){
    char path_buffer[128];
    int buflen = sizeof path_buffer;
    // 首先判断节点是不是存在
    int flag = zoo_exists(m_zhandle,path,0,nullptr);
    if(flag == ZNONODE){
        // path的znode节点不存在，开始创建
        flag = zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,buflen);
        if(flag == ZOK){
            std::cout << "znode create success...path:" << path << std::endl;
        }
        else{
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create success...path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
// 根据path获取节点的值
std::string ZKClient::GetData(const char* path){
    char buffer[64];
    int buflen = sizeof buffer;
    int flag = zoo_get(m_zhandle,path,0,buffer,&buflen,nullptr);
    if(flag != ZOK){
        std::cout << "get znode error...path:" << path <<std::endl;
        return std::string();
    }
    else{
        return buffer;
    }
}
