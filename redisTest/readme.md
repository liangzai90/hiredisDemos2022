

# gdb的断点调试

`shell
gdb  ./redisHello            # gdb 调试文件
run                          # 开始运行
break  redisHelloB.cpp:100   # 在 redisHelloB.cpp 文件的 第100行打断点.  break 的简写为b 
p   config                   # 输出变量 config 的内容. print的简写为p
c                            # 程序从断点出继续运行

delete break  # 删除所有断点
delete break redisHelloB.cpp:100  # 删除此处的断点
clear  redisHelloB.cpp:100  # 删除设在某一行的断点 

disable  break  n  # 禁用某个断点 n 为断点号
enable  break  n  # 启用某个断点 n 为断点号

info  b  # 查看所有断点信息
info  b  n # 查看第 n 个断点的信息 

list  100  # 打印出第100行 周围的源程序 
list  main # 打印函数名称为 main 的函数双下文的源程序
list   # 输出当前行后面的代码
list  - # 显示当前行前面的代码


break  if i=100  # 在处理某些循环体中使用此方法进行调试
`

### 3.4 逐步调试
使用gdb工具调试可以使用next命令单步执行程序代码，
next的单步不会进入函数的内部，
与next对应的step命令则在单步执行一个函数时进入函数内部，类似于VC++中的step into.
其用法如下

next  5  # 单步跟踪，如果有函数调用不会进入函数，如果后面不加count表示一条一条的执行，加count表示执行后面的count条指令



