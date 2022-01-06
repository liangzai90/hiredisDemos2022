#ifndef  __REDIS_RW_H__
#define  __REDIS_RW_H__
/**
 * @brief 二次封装 hiredis 的若干方法
 * @brief reload hiredis functions
 * author: henry
 * date: 2020.01.05
 * 
 */
#include <string>
#include "hiredis/hiredis.h"
#include "hiredis/win32.h" ///strcasecmp
#include "hiredis/async.h" // hiredis redisAsyncConnect()
// // 在返回类型为 int 的 redisXXX 函数里面，如果执行成功返回0，失败返回-1
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



static int tests = 0, fails = 0, skips = 0;
#define test(_s)                   \
    {                              \
        printf("#%02d ", ++tests); \
        printf(_s);                \
    }
#define test_cond(_c)                          \
    if (_c)                                    \
        printf("\033[0;32mPASSED\033[0;0m\n"); \
    else                                       \
    {                                          \
        printf("\033[0;31mFAILED\033[0;0m\n"); \
        fails++;                               \
    }
#define test_skipped()                           \
    {                                            \
        printf("\033[01;33mSKIPPED\033[0;0m\n"); \
        skips++;                                 \
    }

/* This is the reply object returned by redisCommand() */
typedef struct redisReplyTTTT
{
    int type;                    /* REDIS_REPLY_* */
    long long integer;           /* The integer when type is REDIS_REPLY_INTEGER */
    double dval;                 /* The double when type is REDIS_REPLY_DOUBLE */
    size_t len;                  /* Length of string */
    char *str;                   /* Used for REDIS_REPLY_ERROR, REDIS_REPLY_STRING
                  REDIS_REPLY_VERB, REDIS_REPLY_DOUBLE (in additional to dval),
                  and REDIS_REPLY_BIGNUM. */
    char vtype[4];               /* Used for REDIS_REPLY_VERB, contains the null
                      terminated 3 character content type, such as "txt". */
    size_t elements;             /* number of elements, for REDIS_REPLY_ARRAY */
    struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReplyTTTT;


/* Context for a connection to Redis */
typedef struct redisContextTTTT {
    const redisContextFuncs *funcs;   /* Function table */

    int err; /* Error flags, 0 when there is no error */
    char errstr[128]; /* String representation of error when applicable */
    redisFD fd;
    int flags;
    char *obuf; /* Write buffer */
    redisReader *reader; /* Protocol reader */

    enum redisConnectionType connection_type;
    struct timeval *connect_timeout;
    struct timeval *command_timeout;

    struct {
        char *host;
        char *source_addr;
        int port;
    } tcp;

    struct {
        char *path;
    } unix_sock;

    /* For non-blocking connect */
    struct sockaddr *saddr;
    size_t addrlen;

    /* Optional data and corresponding destructor users can use to provide
     * context to a given redisContext.  Not used by hiredis. */
    void *privdata;
    void (*free_privdata)(void *);

    /* Internal context pointer presently used by hiredis to manage
     * SSL connections. */
    void *privctx;

    /* An optional RESP3 PUSH handler */
    redisPushFn *push_cb;
} redisContextTTTT;



#define panicAbort(fmt, ...) \
    do { \
        fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
        exit(-1); \
    } while (0)

#define printInfo(fmt, ...) \
    do { \
        fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
        fprintf(stderr,"\r\n"); \
    } while (0)


// 例如：
// static void assertReplyAndFree(redisContext *context, redisReply *reply, int type) {
//     if (reply == NULL)
//         panicAbort("NULL reply from server (error: %s)", context->errstr);
//     if (reply->type != type) {
//         if (reply->type == REDIS_REPLY_ERROR)
//             fprintf(stderr, "Redis Error: %s\n", reply->str);
//         panicAbort("Expected reply type %d but got type %d", type, reply->type);
//     }
//     freeReplyObject(reply);
// }


//================= redis 支持的6种类型
//string     "set name henry "
//list     "lpush  "
//hash        "hset  keyname  key1 val1", "hset  keyname  key2 val2"   //hget
//set
//zset
//stream  redis 5.0之后，开始支持流式数据




struct config
{
    const char *host = nullptr;   //主机ip地址
    int port = 6379;              //端口
    const char *passwd = nullptr; //Authen密码
    struct timeval timeout;       //超时时间
};

class TC_Port
{
public:
    static int gettimeofday(struct timeval &tv);
};


//class UTIL_DLL_API TC_Common
class  TC_Common
{
public:
    /**
	* @brief  获取当前的秒和毫秒
    * @brief  Get the current seconds and milliseconds
	*
	* @param t        时间结构
    * @param t        time structure
	*/
    static int gettimeofday(struct timeval &tv);

    
    /**
     * @brief  获取当前时间的的毫秒数.
     * @brief  Get the value of milliseconds of current time.
     *
     * @return int64_t 当前时间的的毫秒数
     * @return int64_t current milliseconds of this time
     */
    static int64_t now2ms();

    /**
     * @brief  取出当前时间的微秒.
     * @brief  Take out microseconds of current time.
     *
     * @return int64_t 取出当前时间的微秒
     * @return int64_t Take out microseconds of current time.
     */
    static int64_t now2us();
};


//断开连接
void disconnect(redisContext *c);
// 选择db
redisContext *select_database(redisContext *c, int dbIndex = 0);
//开始 连接，并认证
redisContext *do_connect(struct config config);

// 解析 redisReply 里面的 type 类型
const char* convertReplyTypeToStr(const int type);

/* When an error occurs, the err flag in a context is set to hold the type of
 * error that occurred. REDIS_ERR_IO means there was an I/O error and you
 * should use the "errno" variable to find out what is wrong.
 * For other values, the "errstr" field will hold a description. */
// 举例用法。下面的例子里面，用c->err判断是否有错误
// redisContext *c = redisConnect("127.0.0.1", 6379);
// if (c == NULL || c->err) {
//     if (c) {
//         printf("Error: %s\n", c->errstr);
//         // handle error
//     } else {
//         printf("Can't allocate redis context\n");
//     }
// }
// 命令执行如果出错，可以在 redisContext 对象里面检查  err 字段，如果不为空则打印 errstr字段的内容
const char* convertReplyErrorToStr(const int err);


// ==================================  string  ==================================
int testRedisString(redisContext *redisContextObj);
// ==================================  hash  ==================================
int testRedisHash(redisContext *redisContextObj);
// ==================================  list  ==================================
int testRedisList(redisContext *redisContextObj);
// ==================================  set  ==================================
int testRedisSet(redisContext *redisContextObj);
// ==================================  zset  ==================================
int testRedisZSet(redisContext *redisContextObj);
// 测试 一些 操作命令
int testCommand(redisContext *redisContextObj);
// 测试 一些 操作命令
int testCommand2(redisContext *redisContextObj);
int testCommand3(redisContext *redisContextObj);

// 测试 append 非阻塞操作 
int testAppendCommand(redisContext *redisContextObj);



/** 
 对空格的处理
0.使用hiredis自带的多参数函数
 缺点：1.需要多次调用执行；优点：1.没有不准确，或者字符边长的问题

1.将空格替换为其他字符
  缺点：可能不太准确

2.用base16编码，将字符转码
  缺点：1.影响阅读，2.所有数据都转码影响效率；

*/
#endif 
