# Lab 6.5 链表
最后我们来实现`Redis`中的链表。之前的`Lab`中, 作者对设计方案都进行了详细的介绍, 也许你觉得这限制了你自己的设计, 因此这一小节作者打算不给你任何提示, 你需要自己设计并实现一个链表, 并通过对应的单元测试。

# 1 代码实现
你需要修改的代码文件包括:
- `src/redis_wrapper/redis_wrapper.cpp`
- `include/redis_wrapper/redis_wrapper.h` (Optional)

> 下面的接口中, 你仍然需要进行`TTL`超时时间的判断, 同时你可能需要更新之前的`redis_ttl`和`redis_expire`以兼容`List`的`TTL`机制。

```cpp
// 链表操作
std::string RedisWrapper::redis_lpush(const std::string &key,
                                      const std::string &value) {
  // TODO: Lab 6.5 新建一个链表类型的`key`，并添加一个元素到链表头部
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":" + std::to_string(1) + "\r\n";
}

std::string RedisWrapper::redis_rpush(const std::string &key,
                                      const std::string &value) {
  // TODO: Lab 6.5 新建一个链表类型的`key`，并添加一个元素到链表尾部
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":" + std::to_string(1) + "\r\n";
}

std::string RedisWrapper::redis_lpop(const std::string &key) {
  // TODO: Lab 6.5 获取一个链表类型的`key`的头部元素
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return "$-1\r\n"; // 表示链表不存在
}

std::string RedisWrapper::redis_rpop(const std::string &key) {
  // TODO: Lab 6.5 获取一个链表类型的`key`的尾部元素
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return "$-1\r\n"; // 表示链表不存在
}

std::string RedisWrapper::redis_llen(const std::string &key) {
  // TODO: Lab 6.5 获取一个链表类型的`key`的长度
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":1\r\n"; // 表示链表不存在
}

std::string RedisWrapper::redis_lrange(const std::string &key, int start,
                                       int stop) {
  // TODO: Lab 6.5 获取一个链表类型的`key`的指定范围内的元素
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return "*0\r\n"; // 表示链表不存在或者范围无效
}
```

# 2 测试
现在你应该可以通过所有的单元测试:
```bash
✗ xmake
[100%]: build ok, spent 0.607s
✗ xmake run test_redis
[==========] Running 11 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 11 tests from RedisCommandsTest
[ RUN      ] RedisCommandsTest.SetAndGet
[       OK ] RedisCommandsTest.SetAndGet (10 ms)
[ RUN      ] RedisCommandsTest.IncrAndDecr
[       OK ] RedisCommandsTest.IncrAndDecr (8 ms)
[ RUN      ] RedisCommandsTest.Expire
[       OK ] RedisCommandsTest.Expire (2011 ms)
[ RUN      ] RedisCommandsTest.HSetAndHGet
[       OK ] RedisCommandsTest.HSetAndHGet (8 ms)
[ RUN      ] RedisCommandsTest.HDel
[       OK ] RedisCommandsTest.HDel (8 ms)
[ RUN      ] RedisCommandsTest.HKeys
[       OK ] RedisCommandsTest.HKeys (8 ms)
[ RUN      ] RedisCommandsTest.HGetWithTTL
[       OK ] RedisCommandsTest.HGetWithTTL (2108 ms)
[ RUN      ] RedisCommandsTest.HExpire
[       OK ] RedisCommandsTest.HExpire (1121 ms)
[ RUN      ] RedisCommandsTest.SetOperations
[       OK ] RedisCommandsTest.SetOperations (8 ms)
[ RUN      ] RedisCommandsTest.ZSetOperations
[       OK ] RedisCommandsTest.ZSetOperations (9 ms)
[ RUN      ] RedisCommandsTest.ListOperations
[       OK ] RedisCommandsTest.ListOperations (8 ms)
[----------] 11 tests from RedisCommandsTest (5314 ms total)

[----------] Global test environment tear-down
[==========] 11 tests from 1 test suite ran. (5314 ms total)
[  PASSED  ] 11 tests
```

此外, 不出意外, 整个`Lab`的所有单元测试你应该都能正常通过:
```bash
✗ xmake run test_redis
# ...
```

