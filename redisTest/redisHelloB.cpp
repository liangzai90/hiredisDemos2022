#include <iostream>
#include <string>
#include "hiredis/hiredis.h"
#include "errno.h"
#include <string.h>
#include <sstream>
#include "xmsg_server.h"
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>
#include <assert.h>
#include "hiredis/win32.h" ///strcasecmp
#include "hiredis/async.h" // hiredis redisAsyncConnect()

#include "redisRW.h"

using namespace std;


int main()
{
    printInfo("这是1条测试信息:%s, line:%d ","henry",159); //个性化打印日志，辅助调试信息

    struct config config;
    config.host = "127.0.0.1";
    config.passwd = "henry";
    config.port = 6379;
    cout << config.host << "|" << config.passwd << "|" << config.port << endl;
    redisContext *redisContextObj = do_connect(config);
    cout << "Connect to redisServer success!" << endl;

    select_database(redisContextObj, 3);
    int retval = 0;

    //=============================================基本结构的读写
    retval = testRedisString(redisContextObj);
    printInfo("testRedisString retVal:%d",retval);

    retval = testRedisHash(redisContextObj);
    printInfo("testRedisHash retVal:%d",retval);
    
    retval = testRedisList(redisContextObj);
    printInfo("testRedisList retVal:%d",retval);

    retval =testRedisSet(redisContextObj);
    printInfo("testRedisSet retVal:%d",retval);

    retval =testRedisZSet(redisContextObj);
    printInfo("testRedisZSet retVal:%d",retval);

    //==================================================appendcommand 考虑非阻塞模式
    retval = testAppendCommand(redisContextObj);
    printInfo("testAppendCommand retVal:%d",retval);

    retval = testCommand(redisContextObj);
    printInfo("testCommand retVal:%d",retval);
    
    retval = testCommand2(redisContextObj);
    printInfo("testCommand2 retVal:%d",retval);

    retval =  testCommand3(redisContextObj);


    disconnect(redisContextObj);
    cout << "redisFree  ~~~ " << endl;
    //=================================END redis test ================================



    // XMsgServer server;
    // server.Start();
    // for (int i = 0; i < 5; i++)
    // {
    //     stringstream ss;
    //     ss << " msg : " << i + 1;
    //     server.SendMsg(ss.str());
    //     this_thread::sleep_for(std::chrono::seconds(1));
    // }

    // cout << "=============================================wait 2 seconds=============================================" << flush << endl;

    // this_thread::sleep_for(std::chrono::seconds(2));

    // for (int i = 11; i < 15; i++)
    // {
    //     stringstream ss;
    //     ss << " msg : " << i + 1;
    //     server.SendMsg(ss.str());
    //     this_thread::sleep_for(std::chrono::seconds(1));
    // }
    //server.Stop();

    cout << " Server stoped!" << endl;

    cout << "Hello redis" << endl;
    return 0;
}
