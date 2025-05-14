# Lab 6.3 无序集合
# 1 实现原理
相比之前实现的哈希, `Redis`的无序数据结构功能简单不少, 其只需要将字符串存储在这个`Set`中即可, 也不要求数据的有序性, 其行为类似`C++`中的`std::unordered_set`。

因此, 这里你需要面对的问题就是, 如何组织这么多个字符串, 并且保证其唯一性?

在这里就不建议将`Set`中所有的字符串都编码或者拼接后存储在单个键值对的`value`中, 因为通常的业务中, 一个`Set`中的字符串数量是远大于哈希结构的`filed`的, 这里若还采用整体编码存储在单个键值对中, 在`CRUD`的过程中, 编解码会消耗大量的时间。

> 我们最后会兼容`Redis`的官方压测工具`redis-benchmark`, 你可以用这个测试工具比较不同实现方案的性能差距

这里唯一推荐的方式就是利用我们的`LSM Tree`中实现的谓词查询接口(这里的谓词就是判断前缀是否匹配), 为同一`Set`的所有字符串添加相同的前缀后进行分离的键值对存储, 然后通过这个前缀进行范围查询, 这样就可以保证查询的效率, 并且保证唯一性。

# 2 代码实现
你需要修改的代码文件包括:
- `src/redis_wrapper/redis_wrapper.cpp`
- `include/redis_wrapper/redis_wrapper.h` (Optional)

> 下面的接口中, 你仍然需要进行`TTL`超时时间的判断, 同时你可能需要更新之前的`redis_ttl`和`redis_expire`以兼容`Set`的`TTL`机制。
## 2.1 sadd
```cpp
std::string RedisWrapper::redis_sadd(std::vector<std::string> &args) {
  // TODO: Lab 6.3 如果集合不存在则新建，添加一个元素到集合中
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":1\r\n";
}
```
需要注意的是, `sadd`是支持一次性在集合中新增多个元素的, 这里你可能需要调用批量化操作的接口提高以性能。


## 2.2 srem
```cpp
std::string RedisWrapper::redis_srem(std::vector<std::string> &args) {
  // TODO: Lab 6.3 删除集合中的元素
  int removed_count = 0;
  return ":" + std::to_string(removed_count) + "\r\n";
}
```
需要注意的是, `srem`是支持一次性从集合中删除多个元素的, 这里你可能需要调用批量化操作的接口提高以性能。

## 2.3 sismember
```cpp
std::string RedisWrapper::redis_sismember(const std::string &key,
                                          const std::string &member) {
  // TODO: Lab 6.3 判断集合中是否存在某个元素
  return ":1\r\n";
}
```
对于查询单个字符串是否在集合中, 你只需要按照规则拼接这个分离存储的`key`, 再从`Lsm Tree`中查询即可。

## 2.4 scard
```cpp
std::string RedisWrapper::redis_scard(const std::string &key) {
  // TODO: Lab 6.3 获取集合的元素个数
  return ":1\r\n";
}
```
这里同样, 你可以直接在代表整个集合的大`key`中, 通过`value`中的元信息直接获取到集合的元素个数。如果你没有在大`key`中存储元信息, 调用谓词查询接口也可以获取到所有元素, 然后返回元素个数即可。

## 2.5 smembers
```cpp
std::string RedisWrapper::redis_smembers(const std::string &key) {
  // TODO: Lab 6.3 获取集合的所有元素
  return "*0\r\n";
}
```

`smembers`用于获取所有的元素, 这里你可以直接调用`LSM Tree`的谓词查询接口, 查询所有前缀匹配的键值对, 然后从这些键值对的`value`构造`RESP`协议的数组返回即可。

# 3 测试
在完成上面的功能后, 你应该能通过下面的测试:
```bash
✗ xmake
✗ xmake run test_redis
[==========] Running 11 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 11 tests from RedisCommandsTest
[ RUN      ] RedisCommandsTest.SetAndGet
[       OK ] RedisCommandsTest.SetAndGet (14 ms)
[ RUN      ] RedisCommandsTest.IncrAndDecr
[       OK ] RedisCommandsTest.IncrAndDecr (9 ms)
[ RUN      ] RedisCommandsTest.Expire
[       OK ] RedisCommandsTest.Expire (2009 ms)
[ RUN      ] RedisCommandsTest.HSetAndHGet
[       OK ] RedisCommandsTest.HSetAndHGet (17 ms)
[ RUN      ] RedisCommandsTest.HDel
[       OK ] RedisCommandsTest.HDel (9 ms)
[ RUN      ] RedisCommandsTest.HKeys
[       OK ] RedisCommandsTest.HKeys (9 ms)
[ RUN      ] RedisCommandsTest.HGetWithTTL
[       OK ] RedisCommandsTest.HGetWithTTL (2113 ms)
[ RUN      ] RedisCommandsTest.HExpire
[       OK ] RedisCommandsTest.HExpire (1111 ms)
[ RUN      ] RedisCommandsTest.ListOperations
```