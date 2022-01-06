#include "redisRW.h"
#include <iostream>
#include <string>
#include "errno.h"
#include <string.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>
#include <map>
#include <assert.h>
using namespace std;

//断开连接
void disconnect(redisContext *c)
{
    redisFree(c);
}

// 选择db
redisContext *select_database(redisContext *c, int dbIndex)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "SELECT %d", dbIndex);
    assert(reply != NULL);
    freeReplyObject(reply);
    return c;
}

//开始 连接，并认证
redisContext *do_connect(struct config config)
{
    redisContext *c = NULL;
    c = redisConnect(config.host, config.port);
    if (c == NULL)
    {
        cout << "do_connect NULL. config:" << config.host << "|" << config.port << "|" << config.passwd << ".Connection error: can't allocate redis context." << endl;
        exit(1);
    }
    else if (c->err)
    {
        cout << "do_connect err. config:" << config.host << "|" << config.port << "|" << config.passwd << ".Connection error: " << c->errstr << endl;
        disconnect(c);
        exit(1);
    }

    if (config.passwd != nullptr)
    {
        redisReply *reply = (redisReply *)redisCommand(c, "AUTH %s", config.passwd); //password为redis服务密码
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
    return c;
}

// 解析 redisReply 里面的 type 类型
const char* convertReplyTypeToStr(const int type)
{
    switch (type)
    {
    case REDIS_REPLY_STRING:
        return "REDIS_REPLY_STRING";
    case REDIS_REPLY_ARRAY:
        return "REDIS_REPLY_ARRAY";
    case REDIS_REPLY_INTEGER:
        return "REDIS_REPLY_INTEGER";
    case REDIS_REPLY_NIL:
        return "REDIS_REPLY_NIL";
    case REDIS_REPLY_STATUS:
        return "REDIS_REPLY_STATUS";
    case REDIS_REPLY_ERROR:
        return "REDIS_REPLY_ERROR";
    case REDIS_REPLY_DOUBLE:
        return "REDIS_REPLY_DOUBLE";
    case REDIS_REPLY_BOOL:
        return "REDIS_REPLY_BOOL";
    case REDIS_REPLY_MAP:
        return "REDIS_REPLY_MAP";
    case REDIS_REPLY_SET:
        return "REDIS_REPLY_SET";
    case REDIS_REPLY_ATTR:
        return "REDIS_REPLY_ATTR";
    case REDIS_REPLY_PUSH:
        return "REDIS_REPLY_PUSH";
    case REDIS_REPLY_BIGNUM:
        return "REDIS_REPLY_BIGNUM";
    case REDIS_REPLY_VERB:
        return "REDIS_REPLY_VERB";
    default:
    {
        stringstream ss;
        ss << "convertReplyTypeToStr BIG ERROR!!! type[" << type << "]" << endl;
        std::string strRet = ss.str();
        return strRet.c_str();
    }
    }
}

/* When an error occurs, the err flag in a context is set to hold the type of
 * error that occurred. REDIS_ERR_IO means there was an I/O error and you
 * should use the "errno" variable to find out what is wrong.
 * For other values, the "errstr" field will hold a description. */
// 命令执行如果出错，可以在 redisContext 对象里面检查  err 字段，如果不为空则打印 errstr字段的内容
const char *convertReplyErrorToStr(const int err)
{
    switch (err)
    {
    case REDIS_ERR_IO:
        return "REDIS_ERR_IO"; //Error in read or write
    case REDIS_ERR_EOF:
        return "REDIS_ERR_EOF"; // End of file
    case REDIS_ERR_PROTOCOL:
        return "REDIS_ERR_PROTOCOL"; //Protocol error
    case REDIS_ERR_OOM:
        return "REDIS_ERR_OOM"; //Out of memory
    case REDIS_ERR_TIMEOUT:
        return "REDIS_ERR_TIMEOUT"; //Timed out
    case REDIS_ERR_OTHER:
        return "REDIS_ERR_OTHER"; // Everything else...
    default:
    {
        stringstream ss;
        ss << "convertReplyErrorToStr err [" << err << "]" << endl;
        string strRet = ss.str();
        return strRet.c_str();
    }
    }
}

// ==================================  string  ==================================
/**
 * @brief 测试 redis 的 string 类型的读写
 * 
 * @param redisContextObj 上下文对象
 * @return int 返回0表示正常；-1表示异常
 */
int testRedisString(redisContext *redisContextObj)
{
    // set key val
    // get key

    // 写入一条数据
    redisReply *rReply = (redisReply *)redisCommand(redisContextObj, "set  %s  %s", "stringTest:aa:bb:cc", "value1"); //TODO:可以通过字符 ":"来实现分层
    if (nullptr == rReply)
    {
        cout << "Execute set failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    //strcasecmp 忽略大小写，如果字符相等返回0
    printInfo("set key val | type:%d | %s | %s",rReply->type,convertReplyTypeToStr( rReply->type), (rReply->str==nullptr?"[this type has no str]":rReply->str)  );
    if (!(rReply->type == REDIS_REPLY_STATUS && strcasecmp(rReply->str, "OK") == 0))
    {
        cout << "Failed to execute command " << endl;
        freeReplyObject(rReply);
        redisFree(redisContextObj);
        return -1;
    }


    //读取一条数据
    rReply = (redisReply *)redisCommand(redisContextObj, "get  %s", "stringTest:aa:bb:cc");
    if (nullptr == rReply)
    {
        cout << "Execute get failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }

    printInfo("get key | type:%d | %s | %s",rReply->type,convertReplyTypeToStr( rReply->type), ((rReply->str==nullptr)?"[this type has no str]":rReply->str)  );

    freeReplyObject(rReply);
    return 0;
}


// ==================================  hash  ==================================
/**
 * @brief 测试 redis 的 hash (散列)的读写
 * 
 * @param redisContextObj 上下文对象
 * @return int 返回0表示正常；-1表示异常
 */
int testRedisHash(redisContext *redisContextObj)
{

    //hset  hashkey key1 val1  key2 val2
    //hget hashkey key1 
    //hgetall  hashkey 
    //hkeys
    //hvals

    //订阅规则
    // 项目名称：股票市场：股票代码：用户id
    string strHashKey1="hash:dingpan:rule:1:600002:up0002";
    // map<string ,string> mVal1;
    // mVal1.insert(make_pair<string,string>("uid","up0001"));
    // mVal1.insert(make_pair<string,string>("mkt","1"));
    // mVal1.insert(make_pair<string,string>("code","600001"));
    // mVal1.insert(make_pair<string,string>("atp","1")); //预警类型（H_ALARM_TYPE）
    // mVal1.insert(make_pair<string,string>("ctm","20220105181200"));
    // mVal1.insert(make_pair<string,string>("utm","20220105181200"));
    // mVal1.insert(make_pair<string,string>("etm","20220105181200"));
    // mVal1.insert(make_pair<string,string>("name","到 此 一 游 2 345"));
    redisReply *rReply = (redisReply *)redisCommand(redisContextObj,"hset %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
    strHashKey1.c_str(),"uid","up0002","mkt","1", "code","600002", "atp","1","ctm","20220105181200","utm","20220105181200","etm","20220105181200","name","到 此 一 游 2 345");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ hset hashkey key1 val1 ] failure" << endl;
        redisFree(redisContextObj);
        return 0;
    }
    printInfo("hset hashkey key1 val1 key2 val2 | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)
    {
        printInfo("hset hashkey key1 val1 key2 val2 | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    //hget hashkey key1
    rReply = (redisReply *)redisCommand(redisContextObj,"hget %s %s",strHashKey1.c_str(),"uid");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ hget hashkey key1 ] failure" << endl;
        redisFree(redisContextObj);
        return 0;
    }
    printInfo("hget hashkey key1 | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_STRING==rReply->type)
    {
        printInfo("hget hashkey key1 | string:%s",rReply->str);
    }
    freeReplyObject(rReply);

    //hgetall hashkey    
    rReply = (redisReply *)redisCommand(redisContextObj,"hgetall %s",strHashKey1.c_str());
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ hgetall hashkey] failure" << endl;
        redisFree(redisContextObj);
        return 0;
    }
    printInfo("hgetall hashkey | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_ARRAY==rReply->type)
    {
        printInfo("hgetall hashkey | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ",rReply->elements,rReply->element[0]->str,rReply->element[1]->str,rReply->element[2]->str,rReply->element[3]->str);
    }
    freeReplyObject(rReply);

    //hkeys  hashkey
    rReply = (redisReply *)redisCommand(redisContextObj,"hkeys %s",strHashKey1.c_str());
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ hkeys hashkey] failure" << endl;
        redisFree(redisContextObj);
        return 0;
    }
    printInfo("hkeys hashkey | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_ARRAY==rReply->type)
    {
        printInfo("hkeys hashkey | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ",rReply->elements,rReply->element[0]->str,rReply->element[1]->str,rReply->element[2]->str,rReply->element[3]->str);
    }
    freeReplyObject(rReply);


    //hvals hashkey
    rReply = (redisReply *)redisCommand(redisContextObj,"hvals %s",strHashKey1.c_str());
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ hvals hashkey] failure" << endl;
        redisFree(redisContextObj);
        return 0;
    }
    printInfo("hvals hashkey | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_ARRAY==rReply->type)
    {
        printInfo("hvals hashkey | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ",rReply->elements,rReply->element[0]->str,rReply->element[1]->str,rReply->element[2]->str,rReply->element[3]->str);
    }
    freeReplyObject(rReply);


    //hincrby 
    //hexists

    return 0;
}

// ==================================  list  ==================================
int testRedisList(redisContext *redisContextObj)
{
    vector<string> vCmdStr;
    // lpush  key  val  左侧插入，最新插入的在最前面
    // rpush key val    右侧插入，最新的在最后面
    vCmdStr.push_back("rpush Rlisttest up000001 ");
    vCmdStr.push_back("rpush Rlisttest up000002 ");
    vCmdStr.push_back("rpush Rlisttest up000003 ");
    vCmdStr.push_back("rpush Rlisttest up000016  up000017  up000018  up000019  up000020  up000021  ");

    for (auto &oneCmd : vCmdStr)
    {
        redisReply *rReply = (redisReply *)redisCommand(redisContextObj, oneCmd.c_str());
        if (nullptr == rReply)
        {
            cout << "Execute oneCmd [ rpush Rlisttest] failure" << endl;
            redisFree(redisContextObj);
            return -1;
        }
        printInfo("rpush Rlisttest  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
        if(REDIS_REPLY_INTEGER==rReply->type)
        {
            printInfo("rpush Rlisttest | INTEGER:%lld",rReply->integer);//返回插入之后的list的总长度
        }
        freeReplyObject(rReply);
    }

    // lpop key  左侧操作， 删除最前面的元素
   redisReply *rReply = (redisReply *)redisCommand(redisContextObj, "lpop Rlisttest");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ lpop Rlisttest] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("lpop listtest  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if (REDIS_REPLY_STRING  == rReply->type)
    {
        printInfo("lpop listtest | str:%s",rReply->str); //返回插入之后的list的总长度
    }
    freeReplyObject(rReply);


    //lrange key 
    rReply = (redisReply *)redisCommand(redisContextObj, "lrange Rlisttest 0 -1");//如果访问的数据没有，则返回一个空数组
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ lrange Rlisttest] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("lrange Rlisttest  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_ARRAY==rReply->type)
    {
        //在获取 elememt下面元素之前，必须先判断 elements 长度，否则越界 
        printInfo("lrange Rlisttest | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ",rReply->elements,rReply->element[0]->str,rReply->element[1]->str,rReply->element[2]->str,rReply->element[3]->str);
    }
    freeReplyObject(rReply);

    return 0;
}

// ==================================  set  ==================================
int testRedisSet(redisContext *redisContextObj)
{
    vector<string> vCmdStr;
    //lpush 最新插入的在最前面
    vCmdStr.push_back("sadd setData  redis");
    vCmdStr.push_back("sadd setData  MongoDB  CouchDB");

    //TODO:这种 sadd 可以批量写的命令，如果 写入的值里面 遇到了 空格怎么办？
    vCmdStr.push_back("sadd setData  MySql  PostgreSQL 'MS_SQL'  Oracle");

    for (auto &oneCmd : vCmdStr)
    {
        redisReply *rReply = (redisReply *)redisCommand(redisContextObj, oneCmd.c_str());
        if (nullptr == rReply)
        {
            cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
            redisFree(redisContextObj);
            return -1;
        }
        printInfo("sadd setData   | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
        if(REDIS_REPLY_ARRAY==rReply->type)
        {
            //在获取 elememt下面元素之前，必须先判断 elements 长度，否则越界 
            printInfo("sadd setData   | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ",rReply->elements,rReply->element[0]->str,rReply->element[1]->str,rReply->element[2]->str,rReply->element[3]->str);
        }
        freeReplyObject(rReply);
    }


    // srem  hashkey  val  # 删除指定的元素
    redisReply *rReply = (redisReply *)redisCommand(redisContextObj, "srem setData CouchDB");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ srem setData CouchDB ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("srem setData CouchDB  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)//0:表示删除失败，1:删除成功
    {
        printInfo("srem setData CouchDB | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    // spop  hashkey  # 从最末尾删除元素
    rReply = (redisReply *)redisCommand(redisContextObj, "spop setData");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ spop setData ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("spop setData | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_STRING==rReply->type) //返回被删除的元素
    {
        printInfo("spop setData | string:%s",rReply->str);
    }
    freeReplyObject(rReply);

    // sismember  # 是不是成员
    rReply = (redisReply *)redisCommand(redisContextObj, "sismember setData MongoDB"); //Oracle
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ sismember setData MongoDB ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("sismember setData MongoDB  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)//0:表示不存在，1:表示存在
    {
        printInfo("sismember setData MongoDB | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);


    // smembers  # 打印所有的成员
    rReply = (redisReply *)redisCommand(redisContextObj, "smembers setData");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ smembers setData ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("smembers setData   | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if (REDIS_REPLY_ARRAY == rReply->type)//REDIS_REPLY_ARRAY
    {
        //在获取 elememt下面元素之前，必须先判断 elements 长度，否则越界
        printInfo("smembers setData  | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ", rReply->elements, rReply->element[0]->str, rReply->element[1]->str, rReply->element[2]->str, rReply->element[3]->str);
    }
    freeReplyObject(rReply);

    return 0;
}


// ==================================  zset  ==================================
int testRedisZSet(redisContext *redisContextObj)
{
    vector<string> vCmdStr;

    // zadd hashstr  score  keyname
    vCmdStr.push_back("zadd zsetTest:salary  100  henry ");
    vCmdStr.push_back("zadd zsetTest:salary  8500 heliang ");
    vCmdStr.push_back("zadd zsetTest:salary  3500 petter ");
    vCmdStr.push_back("zadd zsetTest:salary  4000 jack ");
    vCmdStr.push_back("zadd zsetTest:salary  3800 tom  4200 henry2   6660 henry3 ");

    for (auto &oneCmd : vCmdStr)
    {
        redisReply *rReply = (redisReply *)redisCommand(redisContextObj, oneCmd.c_str());
        if (nullptr == rReply)
        {
            cout << "Execute oneCmd [ " << oneCmd << " ] failure" << endl;
            redisFree(redisContextObj);
            return 0;
        }
        printInfo("zadd zsetTest:salary | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
        freeReplyObject(rReply);
    }

    //zrem  hashstr member
    redisReply *rReply = (redisReply *)redisCommand(redisContextObj, "zrem zsetTest:salary petter ");//移除指定的元素
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zrem hashstr ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zrem hashstr  | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)//0：移除失败；1：移除成功
    {
        printInfo("zrem hashstr | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    // zrange hashKey 0 -1 # 默认是输出 member成员
    // zrange hashKey 0 -1 withscores  ##zrange 默认是按照从小到大的顺序排序数据
    rReply = (redisReply *)redisCommand(redisContextObj, "zrange zsetTest:salary 0 -1 withscores");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zrange hashstr score val ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zrange hashstr 0 -1 | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));    
    if (REDIS_REPLY_ARRAY == rReply->type)//REDIS_REPLY_ARRAY
    {
        //在获取 elememt下面元素之前，必须先判断 elements 长度，否则越界
        printInfo("zrange hashstr 0 -1 | elements:%d,[0]:%s,[1]:%s,[2]:%s,[3]:%s ", rReply->elements, rReply->element[0]->str, rReply->element[1]->str, rReply->element[2]->str, rReply->element[3]->str);
        //[0] henry
        //[1] 100
        //[2] petter 
        //[3] 3500
    }
    freeReplyObject(rReply);

    //zcount hashstr  100 200 #获取给定区间内的元素数量
    //zcard  hashstr  #获取元素数量 
    rReply = (redisReply *)redisCommand(redisContextObj, "zcard zsetTest:salary");//获取有序集合的成员数
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zcard hashstr ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zcard hashstr | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)//返回有序集合的成员数
    {
        printInfo("zcard hashstr | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    // ZINCRBY key increment member 
    // 有序集合中对指定成员的分数加上增量 increment
    //zincrby  hashstr  10  keyname #给元素增加10
    rReply = (redisReply *)redisCommand(redisContextObj, "zincrby zsetTest:salary 10 heliang");
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zincrby hashstr score val ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zincrby hashstr score val | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_STRING==rReply->type) //返回的元素是string类型
    {
        printInfo("zincrby hashstr  | string:%s",rReply->str);
    }
    freeReplyObject(rReply);

    //zscore 
    rReply = (redisReply *)redisCommand(redisContextObj, "zscore zsetTest:salary heliang");//获取member的值
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zscore hashstr score val ] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zscore hashstr member | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_STRING==rReply->type) //返回的元素是string类型
    {
        printInfo("zscore hashstr  | string:%s",rReply->str);
    }
    freeReplyObject(rReply);


    //zrank
    rReply = (redisReply *)redisCommand(redisContextObj, "zrank zsetTest:salary heliang");//获取 member 的排名(默认是按照值的大小从小到大排序)
    if (nullptr == rReply)
    {
        cout << "Execute oneCmd [ zrank hashstr] failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("zrank hashstr | type:%d | %s | %s", rReply->type, convertReplyTypeToStr(rReply->type), ((rReply->str == nullptr) ? "[this type has no str]" : rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)//获取member的排名(索引从0开始)
    {
        printInfo("zrank hashstr | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);
    return 0;
}


// 测试 append 非阻塞操作 
int testAppendCommand(redisContext *redisContextObj)
{
    // 在阻塞的 redisContext 里面，执行了redisAppendCommand N次，则需要 执行 redisGetReply N次；
    // 在服阻塞的 redisContext 里面，redisGetReply 只需要执行1次
    int ret = redisAppendCommand(redisContextObj, "set %s  %s", "redisAppendCmd2", "到 此 一 游");//REDIS_OK  REDIS_ERR
    redisReply *rReply;
    assert(redisGetReply(redisContextObj, (void **)&rReply) == REDIS_OK);
    printInfo("redisAppendCommand |ret:%d| type:%d | %s | %s",ret, rReply->type,convertReplyTypeToStr( rReply->type), ((rReply->str==nullptr)?"[this type has no str]":rReply->str));
    freeReplyObject(rReply);
    return 0;
}


// 测试 一些 操作命令
int testCommand(redisContext *redisContextObj)
{
    //inc
    //dec
    // multi

    // =================================================
    // //命令
    // //给指定key设置生存时间（单位秒）
    // EXPIRE key seconds
    // //给指定key设置生存时间（单位毫秒）
    // PEXPIRE key milliseconds
    const char *commandExpire = "expire stest1:aa:bb:cc 30"; //设置 "stest1:aa:bb:cc" key 30秒后自动删除
    redisReply *rReply = (redisReply *)redisCommand(redisContextObj, commandExpire);
    if (nullptr == rReply)
    {
        cout << "Execute expire failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("expire key val | type:%d | %s | %s ",rReply->type,convertReplyTypeToStr( rReply->type), ((nullptr == rReply->str)?"[this type has no str]":rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)
    {
        printInfo("expire key val | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    // setex  key  ttl  value
    const char *commandSeyEx = "setex stest1:aa:bb:dd  40 6666"; //原子操作，设置值，并设置超时时间
    rReply = (redisReply *)redisCommand(redisContextObj, commandSeyEx);
    if (nullptr == rReply)
    {
        cout << "Execute setex failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("setex key ttl val | type:%d | %s | %s",rReply->type,convertReplyTypeToStr( rReply->type), ((rReply->str==nullptr)?"[this type has no str]":rReply->str));
    printInfo("setex key integer is: %lld", rReply->integer);
    freeReplyObject(rReply);

    // =================================================
    // //获取key剩余的生存时间（单位秒）
    // TTL key
    // ttl  value
    const char *commandTtl = "ttl  stest1:aa:bb:dd ";
    rReply = (redisReply *)redisCommand(redisContextObj, commandTtl);
    if (nullptr == rReply)
    {
        cout << "Execute ttl failure" << endl;
        redisFree(redisContextObj);
        return -1;
    }
    printInfo("ttl val | type:%d | %s | %s ",rReply->type,convertReplyTypeToStr( rReply->type), ((rReply->str==nullptr)?"[this type has no str]":rReply->str));
    if(REDIS_REPLY_INTEGER==rReply->type)
    {
        printInfo("ttl val | INTEGER:%lld",rReply->integer);
    }
    freeReplyObject(rReply);

    return 0;
}



int testCommand2(redisContext *redisContextObj)
{
       //=================test redisAppendCommand
    {
        int ret = redisAppendCommand(redisContextObj, "set %s  %s", "redisAppendCmd1", "abcdefg");
        redisReply *reply;
        assert(redisGetReply(redisContextObj, (void **)&reply) == REDIS_OK);
        cout << "redisAppendCommand test1 ret:" << ret << ",replyType:" << reply->type << ",reply str:" << reply->str << endl;
        freeReplyObject(reply);
    }



    //TODO:测试阻塞写10w条； 非阻塞写10w条数据，并记录执行时间

    // //======================== 测试批量写入数据
    // {
    //     stringstream ssKey;
    //     stringstream ssVal;
    //     stringstream ssCommand;
    //     for (int i = 20211101; i < 20211104; i++)
    //     {
    //         for (int j = 1; j < 3; j++)
    //         {
    //             // ssKey.clear(); //错，stringstream 这个方法不行
    //             // ssVal.clear();
    //             // ssCommand.clear();
    //             ssKey.str("");
    //             ssVal.str("");
    //             ssCommand.str("");
    //             ssKey << i << ":"
    //                   << "key"
    //                   << "_" << j;
    //             ssVal << "val_" << j;
    //             ssCommand << "set " << ssKey.str() << " " << ssVal.str();
    //             cout << "(" << i << "," << j << ")"
    //                                             "  ssCommand: "
    //                  << ssCommand.str() << endl;
    //             string strCmd = ssCommand.str();
    //             redisReply *rReply = (redisReply *)redisCommand(redisContextObj, strCmd.c_str());
    //             freeReplyObject(rReply);
    //         }
    //     }
    // }


    return 0;
}

int testCommand3(redisContext *redisContextObj)
{

    test("Is able to deliver commands: ");
    redisReply *reply = (redisReply *)redisCommand(redisContextObj, "PING");
    test_cond(reply->type == REDIS_REPLY_STATUS &&
              strcasecmp(reply->str, "pong") == 0)
        freeReplyObject(reply);

    /* test 7 */
    test("Can parse integer replies: ");
    reply = (redisReply *)redisCommand(redisContextObj, "INCR mycounter");
    test_cond(reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
        freeReplyObject(reply);

    test("Can parse multi bulk replies: ");
    freeReplyObject(redisCommand(redisContextObj, "LPUSH mylist foo"));
    freeReplyObject(redisCommand(redisContextObj, "LPUSH mylist bar"));
    reply = (redisReply *)redisCommand(redisContextObj, "LRANGE mylist 0 -1");
    test_cond(reply->type == REDIS_REPLY_ARRAY &&
              reply->elements == 2 &&
              !memcmp(reply->element[0]->str, "bar", 3) &&
              !memcmp(reply->element[1]->str, "foo", 3))
        freeReplyObject(reply);

    //TODO:redis 的事务。 multi 开始， exec 执行
    /* m/e with multi bulk reply *before* other reply.
     * specifically test ordering of reply items to parse. */
    test("Can handle nested multi bulk replies: ");
    freeReplyObject(redisCommand(redisContextObj, "MULTI"));
    freeReplyObject(redisCommand(redisContextObj, "LRANGE mylist 0 -1"));   //type  REDIS_REPLY_ARRAY
    freeReplyObject(redisCommand(redisContextObj, "LRANGE listtest 0 -1")); //type  REDIS_REPLY_ARRAY
    freeReplyObject(redisCommand(redisContextObj, "PING"));                 //type  REDIS_REPLY_STATUS
    reply = (redisReply *)(redisCommand(redisContextObj, "EXEC"));
    cout << "reply->type:" << reply->type << ",reply->elements:" << reply->elements << endl
         << "[0]" << reply->element[0]->type << "|" << reply->element[0]->elements << "|" << reply->element[0]->element[0]->str << "|" << reply->element[0]->element[1]->str << endl
         << "[1]" << reply->element[1]->type << "|" << reply->element[1]->elements << "|" << reply->element[1]->element[0]->str << "|" << reply->element[1]->element[1]->str << endl
         << "[2]" << reply->element[2]->type << "|" << reply->element[2]->str << endl
         << endl;

    freeReplyObject(reply);

    // 对 nil 类型做判断
    test("Can parse nil replies: ");
    reply = (redisReply *)redisCommand(redisContextObj, "GET nokey");
    test_cond(reply->type == REDIS_REPLY_NIL)
        freeReplyObject(reply);
}
