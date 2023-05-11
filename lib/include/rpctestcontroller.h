#pragma once
#include<google/protobuf/service.h>
#include<string>


class RpctestController : public google::protobuf::RpcController{
public:
    RpctestController(){};
    inline void Reset() {
        m_failed = false;
        m_errText = std::string();
    }
    inline bool Failed() const {return m_failed;}
    inline std::string ErrorText() const {return m_errText;}
    inline void SetFailed(const std::string& reason){
        m_failed = true;
        m_errText = reason;
    }

    // 尚未实现的功能
    void StartCancel(){}
    bool IsCanceled() const{return false;}
    void NotifyOnCancel(google::protobuf::Closure* callback){}

private:
    bool m_failed = false;                 // 执行过程的状态
    std::string m_errText = std::string(); // 执行过程错误信息
};