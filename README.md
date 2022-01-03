# hiredisDemos2022
记录一些使用hiredis的示例



1.在执行了 redisCommand 获取到 返回值之后，记得用 freeReplyObject 释放掉返回的对象。
`C++
    redisReply *rRep0 = (redisReply *)redisCommand(redisObj, command0);
    freeReplyObject(rRep0);
`


2.使用 redisAppendCommand 命令，需要手动调用 redisGetReply ，然后再调用 freeReplyObject 释放资源
`C++

    //=================test redisAppendCommand
    {
        int ret = redisAppendCommand(redisObj, "set %s  %s", "redisAppendCmd1", "abcdefg");
        redisReply *reply;
        assert(redisGetReply(redisObj, (void **)&reply) == REDIS_OK);
        cout << "redisAppendCommand test1 ret:" << ret << ",replyType:" << reply->type << ",reply str:" << reply->str << endl;
        freeReplyObject(reply);
    }
`

3.不同的 redisCommand 命令，返回的 redisReply 对象的 type 不一样。而且有些有str，有些没有。错误使用，会导致程序崩溃。


