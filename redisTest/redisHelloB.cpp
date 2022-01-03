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

#include "hiredis/async.h"// hiredis redisAsyncConnect()
using namespace std;



// #define REDIS_ERR -1
// #define REDIS_OK 0

// /* When an error occurs, the err flag in a context is set to hold the type of
//  * error that occurred. REDIS_ERR_IO means there was an I/O error and you
//  * should use the "errno" variable to find out what is wrong.
//  * For other values, the "errstr" field will hold a description. */
// #define REDIS_ERR_IO 1 /* Error in read or write */
// #define REDIS_ERR_EOF 3 /* End of file */
// #define REDIS_ERR_PROTOCOL 4 /* Protocol error */
// #define REDIS_ERR_OOM 5 /* Out of memory */
// #define REDIS_ERR_TIMEOUT 6 /* Timed out */
// #define REDIS_ERR_OTHER 2 /* Everything else... */

// #define REDIS_REPLY_STRING 1
// #define REDIS_REPLY_ARRAY 2
// #define REDIS_REPLY_INTEGER 3
// #define REDIS_REPLY_NIL 4
// #define REDIS_REPLY_STATUS 5
// #define REDIS_REPLY_ERROR 6
// #define REDIS_REPLY_DOUBLE 7
// #define REDIS_REPLY_BOOL 8
// #define REDIS_REPLY_MAP 9
// #define REDIS_REPLY_SET 10
// #define REDIS_REPLY_ATTR 11
// #define REDIS_REPLY_PUSH 12
// #define REDIS_REPLY_BIGNUM 13
// #define REDIS_REPLY_VERB 14


// void testAsync(){
//     redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
//     if (c->err) {
//     printf("Error: %s\n", c->errstr);
//     // handle error
//     }

// }

int main()
{

    redisContext *redisObj = redisConnect("127.0.0.1", 6379);
    if (redisObj->err)
    {
        redisFree(redisObj);
        cout << "Connect to redisServer failed." << endl;
        return 0;
    }
    cout << "Connect to redisServer success!" << endl;

    //================= redis 支持的6种类型
    //string     "set name henry "
    //list     "lpush  "
    //hash        "hset  keyname  key1 val1", "hset  keyname  key2 val2"   //hget
    //set
    //zset
    //string  redis 5.0之后，开始支持流式数据

    //================== Auth
    {
        redisReply *reply = (redisReply *)redisCommand(redisObj, "AUTH %s", "henry"); //password为redis服务密码
        if (reply->type == REDIS_REPLY_ERROR)
        {
            cout << "Redis 认证失败" << endl;
        }
        else
        {
            cout << "Redis 认证成功" << endl;
        }
        freeReplyObject(reply);
    }

    //================test one command
    {
        const char *command0 = "SELECT 3";
        const char *command1 = "set stest1:aa:bb:cc value3";
        redisReply *rRep0 = (redisReply *)redisCommand(redisObj, command0);
        freeReplyObject(rRep0);

        redisReply *rRep = (redisReply *)redisCommand(redisObj, command1);
        if (nullptr == rRep)
        {
            cout << "Execute command1 failure" << endl;
            redisFree(redisObj);
            return 0;
        }
        if (!(rRep->type == REDIS_REPLY_STATUS && strcasecmp(rRep->str, "OK") == 0))
        {
            cout << "Failed to execute command " << command1 << endl;
            freeReplyObject(rRep);
            redisFree(redisObj);
            return 0;
        }
        freeReplyObject(rRep);
    }

    //======================== 测试批量写入数据
    {
        stringstream ssKey;
        stringstream ssVal;
        stringstream ssCommand;

        for (int i = 20211101; i < 20211114; i++)
        {
            for (int j = 1; j < 6; j++)
            {
                // ssKey.clear(); //错，stringstream 这个方法不行
                // ssVal.clear();
                // ssCommand.clear();

                ssKey.str("");
                ssVal.str("");
                ssCommand.str("");

                ssKey << i << ":"
                      << "key"
                      << "_" << j;
                ssVal << "val_" << j;
                ssCommand << "set " << ssKey.str() << " " << ssVal.str();

                cout << "(" << i << "," << j << ")"
                                                "  ssCommand: "
                     << ssCommand.str() << endl;
                string strCmd = ssCommand.str();

                redisReply *rRep = (redisReply *)redisCommand(redisObj, strCmd.c_str());
                freeReplyObject(rRep);
            }
        }
    }

    //=================test redisAppendCommand
    {
        int ret = redisAppendCommand(redisObj, "set %s  %s", "redisAppendCmd1", "abcdefg");
        redisReply *reply;
        assert(redisGetReply(redisObj, (void **)&reply) == REDIS_OK);
        cout << "redisAppendCommand test1 ret:" << ret << ",replyType:" << reply->type << ",reply str:" << reply->str << endl;
        freeReplyObject(reply);
    }

    //hset 散列，设置散列值
    {
        vector<string> vCmdStr;

        //  股票 和用户的 映射关系
        vCmdStr.push_back("set dingpan:uid:1:600001 up123456 "); //股票市场
        vCmdStr.push_back("set dingpan:uid:1:600002 up123456 "); //股票市场
        vCmdStr.push_back("set dingpan:uid:1:600003 up123456 "); //股票市场

        //订阅规则
        // 项目名称：股票市场：股票代码：用户id
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  mkt 1");        //股票市场
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  code 600002");  //股票代码
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  uid up123458"); //用户id

        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  atp 1"); //预警类型（H_ALARM_TYPE）

        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  val 10086"); //预警阀值
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  ctm 10086"); //创建时间
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  utm 10086"); //...
        vCmdStr.push_back("hset dingpan:rule:1:600002:up123458  etm 10086"); //截止时间

        for (auto &oneCmd : vCmdStr)
        {
            redisReply *rRep = (redisReply *)redisCommand(redisObj, oneCmd.c_str());
            if (nullptr == rRep)
            {
                cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
                redisFree(redisObj);
                return 0;
            }
            // if (!(rRep->type == REDIS_REPLY_STATUS && strcasecmp(rRep->str, "OK") == 0))
            // {
            //     cout << "Failed to execute command [" << oneCmd <<" ] "<< endl;
            //     freeReplyObject(rRep);
            //     redisFree(redisObj);
            //     return 0;
            // }

            freeReplyObject(rRep);
        }
    }

    //lpush 向list添加元素
    {
        vector<string> vCmdStr;
        //lpush 最新插入的在最前面
        vCmdStr.push_back("lpush listtest up000001 ");
        vCmdStr.push_back("lpush listtest up000002 ");
        vCmdStr.push_back("lpush listtest up000003 ");
        vCmdStr.push_back("lpush listtest up000004 ");
        vCmdStr.push_back("lpush listtest up000005 ");
        vCmdStr.push_back("lpush listtest up000016  up000017  up000018  up000019  up000020  up000021  ");

        for (auto &oneCmd : vCmdStr)
        {
            redisReply *rRep = (redisReply *)redisCommand(redisObj, oneCmd.c_str());
            if (nullptr == rRep)
            {
                cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
                redisFree(redisObj);
                return 0;
            }
            // if (!(rRep->type == REDIS_REPLY_STATUS && strcasecmp(rRep->str, "OK") == 0))
            // {
            //     cout << "Failed to execute command [" << oneCmd <<" ] "<< endl;
            //     freeReplyObject(rRep);
            //     redisFree(redisObj);
            //     return 0;
            // }

            freeReplyObject(rRep);
        }
    }

    //sadd 将元素添加到集合
    {
        vector<string> vCmdStr;
        //lpush 最新插入的在最前面
        vCmdStr.push_back("sadd database  redis");
        vCmdStr.push_back("sadd database  MongoDB  CouchDB");

        //TODO:这种 sadd 可以批量写的命令，如果 写入的值里面 遇到了 空格怎么办？
        vCmdStr.push_back("sadd database  MySql  PostgreSQL  \"MS SQL\"  Oracle");

        for (auto &oneCmd : vCmdStr)
        {
            redisReply *rRep = (redisReply *)redisCommand(redisObj, oneCmd.c_str());
            if (nullptr == rRep)
            {
                cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
                redisFree(redisObj);
                return 0;
            }
            // if (!(rRep->type == REDIS_REPLY_STATUS && strcasecmp(rRep->str, "OK") == 0))
            // {
            //     cout << "Failed to execute command [" << oneCmd <<" ] "<< endl;
            //     freeReplyObject(rRep);
            //     redisFree(redisObj);
            //     return 0;
            // }
            freeReplyObject(rRep);
        }
    }

    // =================test redisAppendCommand
    {
        int ret = redisAppendCommand(redisObj, "set %s  %s", "redisAppendCmd2", "到 此 一 游");
        redisReply *reply;
        assert(redisGetReply(redisObj, (void **)&reply) == REDIS_OK);
        cout << "redisAppendCommand test2 ret:" << ret << ",replyType:" << reply->type << ",reply str:" << reply->str << endl;
        freeReplyObject(reply);
    }

    //zadd 将元素添加到 有序集合
    {
        vector<string> vCmdStr;
        //lpush 最新插入的在最前面
        vCmdStr.push_back("zadd salary  3500 petter ");
        vCmdStr.push_back("zadd salary  4000 jack ");
        vCmdStr.push_back("zadd salary  3800 tom  4200 henry2 ");

        for (auto &oneCmd : vCmdStr)
        {
            redisReply *rRep = (redisReply *)redisCommand(redisObj, oneCmd.c_str());
            if (nullptr == rRep)
            {
                cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
                redisFree(redisObj);
                return 0;
            }
            

            // test valgrind
           //cout <<"status:"<<rRep->type<<",str:"<<rRep->str<<endl;

            // todo 之类的type命令不能 瞎判断 ...  在这里崩溃几次了
            // // 不同的命令，返回的 type不同
            // if (!(rRep->type == REDIS_REPLY_INTEGER && strcasecmp(rRep->str, "OK") == 0))
            // {
            //     cout << "Failed to execute command [" << oneCmd <<" ] "<< endl;
            //     freeReplyObject(rRep);
            //     redisFree(redisObj);
            //     return 0;
            // }
            freeReplyObject(rRep);
        }
    }





    //================test one command
    {
            redisReply *reply = (redisReply *)redisCommand(redisObj, "AUTH %s", "henry"); //password为redis服务密码
           if (reply->type == REDIS_REPLY_ERROR)
            {
                cout << "Redis 认证失败" << endl;
            }
            else
            {
                cout << "Redis 认证成功" << endl;
            }

        const char *command1 = "set stest1:aB:14 redisAppendCmd";
        redisReply *rRep = (redisReply *)redisCommand(redisObj, command1);
        if (nullptr == rRep)
        {
            cout << "Execute command1 failure" << endl;
            redisFree(redisObj);
            return 0;
        }
        if (!(rRep->type == REDIS_REPLY_STATUS && strcasecmp(rRep->str, "OK") == 0))
        {
            cout << "Failed to execute command " << command1 << endl;
            freeReplyObject(rRep);
            redisFree(redisObj);
            return 0;
        }
        freeReplyObject(rRep);
    }


    

    // =================test redisAppendCommand
    {
        int ret = redisAppendCommand(redisObj, "set %s  %s", "redisAppendCmd5", "到 此 一 游");
        redisReply *reply;
        assert(redisGetReply(redisObj, (void **)&reply) == REDIS_OK);

        cout << "redisAppendCommand test3 ret:" << ret << ",replyType:" << reply->type << ",reply str:" << reply->str << endl;
        // assert(reply != NULL && reply->type == REDIS_REPLY_INTEGER);//TODO: 在这里崩溃了，返回类型不匹配
        freeReplyObject(reply);
    }




    redisFree(redisObj);
    cout << "redisFree  ~~~ " << endl;

    //=============================================================================================

    XMsgServer server;
    server.Start();
    for (int i = 0; i < 5; i++)
    {
        stringstream ss;
        ss << " msg : " << i + 1;
        server.SendMsg(ss.str());
        this_thread::sleep_for(std::chrono::seconds(1));
    }

    cout << "=============================================wait 2 seconds=============================================" << flush << endl;

    this_thread::sleep_for(std::chrono::seconds(2));

    for (int i = 11; i < 15; i++)
    {
        stringstream ss;
        ss << " msg : " << i + 1;
        server.SendMsg(ss.str());
        this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.Stop();

    cout << " Server stoped!" << endl;

    cout << "Hello redis" << endl;
    return 0;
}
