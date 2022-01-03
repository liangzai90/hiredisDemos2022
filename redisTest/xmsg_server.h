#ifndef  __X_MSG_SERVER_H__
#define __X_MSG_SERVER_H__

#include "xthread.h"
#include <string>
#include <list>
#include <mutex>
#include <condition_variable> 

class XMsgServer:public XThread{
public:
    //给当前线程发消息
    void SendMsg(std::string msg);
    void Stop() override;

    private:
    //处理消息的线程入口函数
    void Main()  override;

    //可以考虑给消息队列设置一个最大值，超过最大值就阻塞，或者发送失败等等
    //消息队列缓冲区
    std::list<std::string> msgs_;

    //互斥访问消息队列
    std::mutex mux_;

    std::condition_variable cv_;
};

#endif 