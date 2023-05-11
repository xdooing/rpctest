#include<string>
#include<functional>
#include"rpctestconfig.h"
#include"rpctestapplication.h"
#include"rpctestprovider.h"
#include"google/protobuf/descriptor.h"
#include"muduo/net/TcpServer.h"
#include"rpcheader.pb.h"
#include"zookeeperutil.h"
#include"logger.h"


// 发布rpc方法，基类指针指向子类对象
void RpctestProvider::NotifyService(google::protobuf::Service* service){
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* serviceDesc = service->GetDescriptor();
    // 获取服务名字
    std::string service_name = serviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCount = serviceDesc->method_count();

    LOG_INFO("service_name: %s",service_name.c_str());

    for(int i = 0; i < methodCount; ++i){
        // 获取服务对象指定下标的服务方法的抽象描述
        const google::protobuf::MethodDescriptor* methodDesc = serviceDesc->method(i);
        std::string method_name = methodDesc->name();
        m_serviceInfo.m_methodMap.insert({method_name,methodDesc});

        LOG_INFO("method_name: %s",method_name.c_str());
    }
    m_serviceInfo.m_service = service;
    m_serviceMap.insert({service_name,m_serviceInfo});
}

// 启动服务
void RpctestProvider::Run(){
    // 读取配置文件信息
    std::string ip = RpctestApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(RpctestApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress address(ip,port);
    // 创建Tcpserver对象
    muduo::net::TcpServer server(&m_evLoop,address,"RpcProvider");
    // 绑定回调函数
    server.setConnectionCallback(std::bind(&RpctestProvider::onConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpctestProvider::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    // 设置线程数量
    server.setThreadNum(4);

    // 将当前的rpc节点上要发布的服务全部发布到zk上面
    // 默认session timeout 30s 
    ZKClient zkClient;
    zkClient.Start();

    // service_name -- 永久节点  method_name -- 临时节点
    for(auto& sp : m_serviceMap){
        // /service_name: /friendserviceRpc
        std::string service_path = "/" + sp.first;
        zkClient.Create(service_path.c_str(),nullptr,0);
        for(auto& mp : sp.second.m_methodMap){
            // /service_name/method_name:  /friendserviceRpc/GetFriendList
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            // 创建临时节点
            zkClient.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }
    std::cout << "RpcProvider start service at ip:" << ip <<" port:" << port << std::endl;

    // 启动网络服务
    server.start();
    m_evLoop.loop();
}

// 新的socket连接回调
void RpctestProvider::onConnection(const muduo::net::TcpConnectionPtr& conn){
    // rpc请求是短连接，一次请求回复之后，就关闭连接
    if(!conn->connected()){
        conn->shutdown();
    }
}

// 已建立连接用户的读写事件回调
void RpctestProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp){
    // 网络上接收到的请求信息,保存在recv_buf中
    std::string recv_buf = buffer->retrieveAllAsString();

    // 获取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);
    // 获取数据头 --- service_name method_name args_size
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    rpctest::RpcHeader rpcheader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if(rpcheader.ParseFromString(rpc_header_str)){
        // 数据头反序列化成功
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();
    }
    else{
        LOG_ERR("rpc_header_str:%s parse error!",rpc_header_str.c_str());
        return;
    }

    // 获取rpc方法参数
    std::string args_str = recv_buf.substr(4 + header_size,args_size);

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "==========================================" << std::endl;

    // 获取service对象和method对象(开始调用)
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        LOG_ERR("%s is not exits!",service_name.c_str());
        return;
    }
    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end()){
        LOG_ERR("%s:%s is not exits!",service_name.c_str(),method_name.c_str());
        return;
    }
    google::protobuf::Service* service = it->second.m_service;      // 获取service对象
    const google::protobuf::MethodDescriptor* method = mit->second; // 获取method方法

    // 生成rpc方法调用的请求request和response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        // 解析失败
        LOG_ERR("request parse error,content:%s",args_str.c_str());
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 给method方法的调用绑定一个Closure的回调函数 这里自己定义一个Closure的派生类，重写里面的Run方法也是可行的
    google::protobuf::Closure* done = google::protobuf::NewCallback<RpctestProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                                    (this,
                                                                    &RpctestProvider::SendRpcResponse,
                                                                    conn,
                                                                    response);
    // 调用rpc节点上发布的方法:将响应的参数和方法传到业务层
    service->CallMethod(method,nullptr,request,response,done);
}

// closure回调函数，用于序列化rpc的响应和网络发送
void RpctestProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response){
    std::string response_str;
    // response进行序列化
    if(response->SerializePartialToString(&response_str)){
        // 序列化成功
        conn->send(response_str);
    }
    else{
        LOG_ERR("Serialize response_str error!");
    }
    conn->shutdown();  // 短连接服务，发送完就断开连接
}
