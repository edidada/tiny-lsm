# Lab 6.6 redis-server实现
# 1 概述
之前我们完成了利用`KV`接口实现常见`Redis`命令的常见命令, 我们的`Lab`即将迎来最后一个部分, 即实现`Redis`服务器的部分。

这一部分内容其实也很简单，网络框架作者已经为你搭建好了，你只需要实现`Resp`请求的解析, 利用解析出的参数调用我们之前实现的各个命令的接口即可。

# 2 代码实现
这节课你可以修改`server`文件夹下的任何文件
```bash
├── server # 调用 Redis 兼容层的 Webserver
│   ├── include
│   │   └── handler.h # Redis 命令处理函数的声明
│   └── src
│       ├── handler.cpp # Redis 命令处理函数的实现, 就是对 redis_wrapper 的转发
│       └── server.cpp # Webserver 的实现
```

你需要实现的接口为:
```cpp
// server/src/server.cpp
std::string handleRequest(const std::string &request) {
    // TODO: Lab 6.6 处理网络传输的RESP字节流
    // TODO: Lab 6.6 形成参数并调用 redis_wrapper 的api
    // TODO: Lab 6.6 返回结果
return "";
}
```

`handleRequest`前后的网络包收发逻辑已经为你写好, 你只需要在这个函数中解析`RESP`协议, 调用`redis_wrapper`的接口即可。当然, 你也可以直接新增各种辅助函数。

> 除了我们之前实现的各种命令外, 你还需要实现`PING`命令, 这个命令不需要任何参数, 只需要返回`"+PONG\r\n"`即可。其内在含义表示服务器正在运行。

# 3 测试
你可以安装并使用`redis-cli`来测试你的`Redis`服务器, 过程为
```bash
cd  tiny-lsm
xmake
xmake run server
```
上述操作将启动你实现的`Redis`服务器

然后在另一个终端中运行:
```bash
redis-cli 
```
此时你就可以在`redis-cli`中输入我们之前`Lab`实现的命令和`server`进行交互了

预期的结果应该如下所示:

![redis-example](../images/lab6/redis-example.png)
