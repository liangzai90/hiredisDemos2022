
#可执行文件的名称
NAME=redisDemo1

#需要包含的头文件目录，可以包含多个，用 空格 隔开
INC_DIR =  -I /home/heliang/package/redis-6.2.6/deps

#需要包含的cpp文件目录，不能在目录后面追加多个目录
SRC_DIR1 = ./*.cpp 
#SRC_DIR2 = ./*.cpp 

#如果有多个cpp文件目录，需要像这样分别定义多个SRC_DIRXXX（一般不会有多个src的目录），下面也有一处需要做对应的修改
#SRC_DIR1=./src/*.cpp
#SRC_DIR2=./src2/*.cpp

#可执行文件的存放路径
BIN_DIR=./

#如果有库文件，需要把库文件目录加入进来，可以包含多个，用空格隔开
LIB_FILE= -L /home/heliang/package/redis-6.2.6/deps/hiredis   -lhiredis -lpthread

#编译条件
CC=g++ 


#编译选项
CFLAGS=-Wall -std=c++11 -g 

#$(CC) $(CFLAGS) $(SRC_DIR1) $(SRC_DIR2) -o $(BIN_DIR)/$(NAME) $(INC_DIR) $(LIB_FILE) 

all:
	$(CC) $(CFLAGS) $(SRC_DIR1)  -o $(BIN_DIR)/$(NAME) $(INC_DIR) $(LIB_FILE) 
    
    
clean:
	-rm -rf *.o $(BIN_DIR)/$(NAME)