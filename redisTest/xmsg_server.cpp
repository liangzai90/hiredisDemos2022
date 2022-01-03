#include "xmsg_server.h"
#include <iostream>

using namespace std;
using namespace this_thread;

void XMsgServer::Stop(){
    is_exit_ = true;
    cv_.notify_all();
    Wait();
}

//给当前线程发消息
void XMsgServer::SendMsg(std::string msg){
    unique_lock<mutex>  lock(mux_);
    msgs_.push_back(msg);

    lock.unlock();
    cv_.notify_one();
}


//处理消息的线程入口函数
void XMsgServer::Main(){
    while(!is_exit()){
        unique_lock<mutex>  lock(mux_);
        cv_.wait(lock, [this]{
            cout <<" wait cv"<< flush<<endl;
            if(is_exit()) return true;//处理退出逻辑
            return !msgs_.empty();
        });

        //特别注意，这里的 msgs_ 消息处理的业务逻辑，是在 上面的 while 循环里面。
        while(!msgs_.empty()){
            //消息处理业务逻辑
            cout <<"recv : "<<msgs_.front()<<flush<< endl;
            msgs_.pop_front();
        }
    }

}