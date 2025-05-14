# Lab 6.1 简单字符串
# 1 简单字符串的设计
你也许会认为, 简单字符串不就是调用我们`LSM Tree`的`get`和`put`方法吗? 其实不然, 你是否忘记了我们的`Redis`是支持对键值对进行过期时间设置的? 

那么, 由于过期时间的存在, 你的代码实现需要解决以下几个难点:

## 1 如何实现过期时间?
你也许会认为, 我们只需要在`value`或者`key`中拼接一个字段来表示过期时间即可, 但虽然是一个可行的方案, 但因为我们过期时间是支持重新设置的, 这样以来你在查询数据后需要进行一定的字符串处理流程。

另一种方案，是每个实际的键值对绑定一个表示其生命周期的额外键值对，比如你插入的键值对是 `(a, b)`, 那你可以同时插入一个键值对 `(expire_a, expire_time)`, 其中`expire_time`表示与键`a`的绑定的表示过期时间的`key`。这样，当你查询`a`时，只需要查询`expire_a`的表示过期时间的`expire_time`即可，如果过期时间小于当前时间，则删除`a`和`expire_a`，并返回`nil`。

上面两种方案是最简单且容易想到的方案, 当然你也不一定局限于作者推荐的实现方案, 可以有自己的设计

## 2 采取何种过期清理策略?
那么`key`只要存在过期时间, 你的实现策略有以下3种:
- 惰性检查: 相同的`key`在下一次被查询时, 检查是否过期, 如果过期则删除, 返回`nil`
  - 优点: 实现简单
  - 缺点: 如果这个`key`是个冷`key`(即访问频率低), 那么即时其过期很久之后, 仍然占据了内存(虽然我们的`LSM Tree`是追加写入的, 但在`Compact`时, 我们是需要移除已经完成的事务且被覆写的键值对的)
- 后台线程检查: 在后台开启一个线程, 每隔一段时间检查所有键值对, 如果过期则删除
  - 优点: 过期的`key`能较为及时地被删除
  - 缺点: 需要额外的线程, 代码组织和并发控制复杂
- 前两种结合: 惰性检查+后台线程检查

# 3 代码组织简介
这一小节我们首先对`Redis`的兼容层代码进行简要介绍, 我们的代码组织为:
```bash
├── config.toml # 配置文件的常量 (你需要复制到单元测试编译的目录下才能生效)
├── include
│   ├── redis_wrapper # Redis 兼容层的头文件定义
│   │   └── redis_wrapper.h
├── server # 调用 Redis 兼容层的 Webserver
│   ├── include
│   │   └── handler.h # Redis 命令处理函数的声明
│   └── src
│       ├── handler.cpp # Redis 命令处理函数的实现, 就是对 redis_wrapper 的转发
│       └── server.cpp # Webserver 的实现
├── src
│   ├── redis_wrapper
│   │   └── redis_wrapper.cpp # Redis 兼容层的实现
├── test
│   ├── test_redis.cpp # Redis 兼容层的单元测试
└── xmake.lua
```

各个代码文件的作用如上所示, 这里我们主要介绍今天要修改的`redis_wrapper.cpp`和`redis_wrapper.h`文件。

首先看`redis_wrapper.h`文件:
```cpp
class RedisWrapper {
private:
  std::unique_ptr<LSM> lsm;
  std::shared_mutex redis_mtx;

public:
  RedisWrapper(const std::string &db_path);
  void clear();
  void flushall();

  // ************************* Redis Command Parser *************************
  // ...

private:
  // ************************* Redis Command Handler *************************
  // 基础操作
  std::string redis_incr(const std::string &key);
  std::string redis_decr(const std::string &key);
  std::string redis_expire(const std::string &key, std::string seconds_count);
  std::string redis_set(std::string &key, std::string &value);
  std::string redis_get(std::string &key);
  std::string redis_del(std::vector<std::string> &args);
  std::string redis_ttl(std::string &key);

  // 哈希操作
  std::string redis_hset(const std::string &key, const std::string &field,
                         const std::string &value);
  std::string redis_hset_batch(
      const std::string &key,
      std::vector<std::pair<std::string, std::string>> &field_value_pairs);
  std::string redis_hget(const std::string &key, const std::string &field);
  std::string redis_hdel(const std::string &key, const std::string &field);
  std::string redis_hkeys(const std::string &key);
  // 链表操作
  std::string redis_lpush(const std::string &key, const std::string &value);
  std::string redis_rpush(const std::string &key, const std::string &value);
  std::string redis_lpop(const std::string &key);
  std::string redis_rpop(const std::string &key);
  std::string redis_llen(const std::string &key);
  std::string redis_lrange(const std::string &key, int start, int stop);
  // 有序集合操作
  std::string redis_zadd(std::vector<std::string> &args);
  std::string redis_zrem(std::vector<std::string> &args);
  std::string redis_zrange(std::vector<std::string> &args);
  std::string redis_zcard(const std::string &key);
  std::string redis_zscore(const std::string &key, const std::string &elem);
  std::string redis_zincrby(const std::string &key,
                            const std::string &increment,
                            const std::string &elem);
  std::string redis_zrank(const std::string &key, const std::string &elem);
  // 无序集合操作
  std::string redis_sadd(std::vector<std::string> &args);
  std::string redis_srem(std::vector<std::string> &args);
  std::string redis_sismember(const std::string &key,
                              const std::string &member);
  std::string redis_scard(const std::string &key);
  std::string redis_smembers(const std::string &key);
};
```
这里的成员变量只有一把锁和一个`LSM`对象, 锁用于保护`LSM`对象, 防止并发访问。不过这个锁的只是一个可选的使用项, 如果你之前的`LSMEngine`的接口实现了对某些批量化操作的并发控制, 那么你可以直接使用`LSMEngine`的接口, 而不需要使用`RedisWrapper`的锁。

其余部分的`redis_xxx`函数都是你需要在本大章节的`Lab`中需要实现的, 其对应于具体的`Redis`命令

# 4 代码实现
本小节我们实现字符串处理相关命令函数, 你需要修改的代码文件包括:
- `src/redis_wrapper/redis_wrapper.cpp`
- `include/redis_wrapper/redis_wrapper.h` (Optional)

## 4.1 set
```cpp
std::string RedisWrapper::redis_set(std::string &key, std::string &value) {
  // TODO: Lab 6.1 新建(或更改)一个`key`的值
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return "+OK\r\n";
}
```

这里我们不需要你支持在`set`一个`key`时就指定其过期时间, 我们的单元测试只会在`expire`中手动设置过期时间。

## 4.2 expire
```cpp
std::string RedisWrapper::redis_expire(const std::string &key,
                                       std::string seconds_count) {
  // TODO: Lab 6.1 设置一个`key`的过期时间
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":1\r\n";
}
```
该命令用于设置一个`key`的过期时间, 单位为秒。

如同之前理论部分的介绍, 你既可以选择为其额外设置一个表示过期时间的键值对, 也可以在键值对的字符串中拼接表示过期时间的部分, 亦或是其他方案。

### 4.3 ttl
```cpp
std::string RedisWrapper::redis_ttl(std::string &key) {
  // TODO: Lab 6.1 获取一个`key`的剩余过期时间
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":1\r\n"; // 表示键不存在
}
```
该命令是与`expire`成对的, 你在`expire`中如何设置过期时间, 就需要在`ttl`中如何获取剩余过期时间。

## 4.4 get
```cpp
std::string RedisWrapper::redis_get(std::string &key) {
  // TODO: Lab 6.1 获取一个`key`的值
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return "$-1\r\n"; // 表示键不存在
}
```
查询一个`key`的值, 如果不存在则返回`nil`。

你可能需要再此时判断一下`key`是否已经过期, 如果已经过期则删除该`key`。

## 4.5 incr && decr
```cpp
std::string RedisWrapper::redis_incr(const std::string &key) {
  // TODO: Lab 6.1 自增一个值类型的key
  // ? 不存在则新建一个值为1的key
  return "1";
}

std::string RedisWrapper::redis_decr(const std::string &key) {
  // TODO: Lab 6.1 自增一个值类型的key
  // ? 不存在则新建一个值为-1的key
  return "-1";
}
```
对一个值类型的`key`进行自增或自减操作, 如果不存在则新建一个值为1或-1的`key`。

如果该键值对的值不是数值类型, 则返回`error`。在`RESP`中如何表示`error`你需要自行回顾[Lab 6 Redis 兼容](./lab6-Redis.md)中的简单介绍, 或者看官方文档(甚至是问LLM)。

## 4.6 del
```cpp
std::string RedisWrapper::redis_del(std::vector<std::string> &args) {
  // TODO: Lab 6.1 删除一个key
  int del_count = 0;
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":" + std::to_string(del_count) + "\r\n";
}
```
删除一个`key`。

# 5 测试
完成上面的代码后, 你可以运行以下命令并通过对应的测试:
```bash
✗ xmake
[100%]: build ok, spent 2.013s
✗ xmake run test_redis
[==========] Running 11 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 11 tests from RedisCommandsTest
[ RUN      ] RedisCommandsTest.SetAndGet
[       OK ] RedisCommandsTest.SetAndGet (12 ms)
[ RUN      ] RedisCommandsTest.IncrAndDecr
[       OK ] RedisCommandsTest.IncrAndDecr (9 ms)
[ RUN      ] RedisCommandsTest.Expire
[       OK ] RedisCommandsTest.Expire (2014 ms)
[ RUN      ] RedisCommandsTest.HSetAndHGet # Failed
```

