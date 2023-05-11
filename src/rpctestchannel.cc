#include"rpctestchannel.h"
#include<google/protobuf/message.h>
#include"rpcheader.pb.h"
#include"rpctestapplication.h"
#include"rpctestcontroller.h"
#include"zookeeperutil.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<errno.h>

/*
header_size + service_name method_name args_size + args
*/
void RpctestChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, 
                          google::protobuf::Closure* done)
{
    // method方法调用service()的话，
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializePartialToString(&args_str)){
        // 序列化成功
        args_size = args_str.size();
    }
    else{
        // 序列化失败
        controller->SetFailed("serialize request error!");
        return;
    }

    // 定义rpc请求的header
    rpctest::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcheader.SerializePartialToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }
    else{
        // 序列化失败
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "==========================================" << std::endl;

    // 使用简单的TCP编程完成远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd == -1){
        // 失败
        char errText[512] = {0};
        sprintf(errText,"create clientfd error! errno: %d",errno);
        controller->SetFailed(errText);
        return;
    }

    // 读取配置文件的信息
    // std::string ip = RpctestApplication::GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(RpctestApplication::GetConfig().Load("rpcserverport").c_str());

    // 从zk服务器读取所需服务的host信息
    ZKClient zkclient;
    zkclient.Start();

    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkclient.GetData(method_path.c_str());
    if(host_data == std::string()){
        controller->SetFailed(method_path + "is not exits!");
        return;
    }
    int index = host_data.find(":");
    if(index == -1){
        controller->SetFailed(method_path + "address is not exits!");
        return;
    }
    std::string ip = host_data.substr(0,index);
    uint16_t port = atoi(host_data.substr(index + 1,host_data.size() - index - 1).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    int ret = connect(clientfd,(struct sockaddr*)&server_addr,sizeof server_addr);
    if(ret == -1){
        char errText[512] = {0};
        sprintf(errText,"connect error! errno: %d",errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }

    // 连接成功，发送请求
    ret = send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0);
    if(ret == -1){
        char errText[512] = {0};
        sprintf(errText,"send error! errno: %d",errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }

    // 接收rpc响应
    char recv_buf[1024] = {0};
    int recv_size = recv(clientfd,recv_buf,1024,0);
    if(recv_size == -1){
        char errText[512] = {0};
        sprintf(errText,"recv error! errno: %d",errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }

    // 将接收到的数据反序列化到response当中
    if(!response->ParseFromArray(recv_buf,recv_size)){
        char errText[512] = {0};
        sprintf(errText,"parse error! errno: %d",errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }
    close(clientfd);
}
