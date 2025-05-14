# Lab 6.4 有序集合
# 1 Redis ZSet 实现思路
## 1.1 实现思路
这里也不介绍`Redis zset`的语法了, 既然看这篇文章, 相比大家对`Redis`非常熟悉了, 这里本实验选择实现如下常见的`api`:

- `zadd`
- `zrem`
- `zrange`
- `zcard`
- `zscore`
- `zincrby`
- `zrank`

相较于之前实现的`set`, `zset`额外加入了了`score`字段, 并且需要支持按照`score`排序的查询, 因此这里的实现又类似于`hash`的实现, 因为存储的`filed`(就是集合中的数据成员)和`score`正好可以构成分离存储的一对键值对。

在分离存储键值对的方案下, `zset`最大的难点就是需要双向的查询索引: 既要能够通过`zscore`查询指定成员的分数, 也要能够通过`zrank`、`zrange`按照分数排序。对于按照分数排序查询, 由于我们的`LSM Tree`本来就是按照`key`排序的, 所以我们只需要将所有的成员按照他们的分数构建一个符合顺序的`key`就可以了; 对于按照成员查询分数, 我们只能再额外存储一个`key`.

因此我们使用如下的方案:
1. 整个`zset`控制结构的键值对只标记其存在, 不在`value`中存储有效信息(但不能为空, 因为`value`为空表示被删除)
2. 需要存储`(score, elem)`键值对, `score`为固定的前缀+`key`+真正的`score`拼接而成
3. 需要存储`(elem, score)`键值对, `elem`为固定的前缀+`key`+真正的`elem`拼接而成

> 需要注意的是`score`为固定的前缀+`key`+真正的`score`拼接而成, 为保证这个`key`在我们的`LSM Rree`中排序符合`score`的顺序, 这个`score`我们限制器为整型数, 且其长度对其到32位, 否则如果支持小数的话, 排序和解析就会复杂很多

## 1.2 TTL
类似上一章的`TTL`设计, 我们需要为这个`zset`也实现`TTL`机制. 首先需要明白, `Redis` 的 `TTL` 只能对整个键（`key`）设置过期时间，而不能针对列表（`list`）、集合（`set`）、哈希（`hash`）等数据结构中的单个成员单独设置过期时间。

类似上一章节的`lsit`和`hash`的工作流程, 我们每次读写`zset`时, 都需要检查`TTL`是否过期, 如果过期, 则删除`zset`

# 2 一个存储案例
这里`zset`应该是最复杂的数据结构, 因此这里用一个`demo`详细说明下作者推荐的实现方案是如何工作的:
```bash
ZADD student 100 tom # 添加一个学生及其成绩
# 主key: (student, 1) 1 表示成员的数量
#   -> 分离存储的key1:   (student_score_000100, tom) # 支持从分数查找 student, 这里填充为六位数`000100`, 保证字符串排序
#   -> 分离存储的key2:   (student_tom, 100) # 支持从 tom 分数查找分数

ZADD student 90 jerry # 添加一个学生及其成绩
# 主key: (student, 2) 2 表示成员的数量
#   -> 分离存储的key1:   (student_score_000100, tom)
#   -> 分离存储的key2:   (student_filed_tom, 100)
#   -> 分离存储的key1:   (student_score_000090, jerry)
#   -> 分离存储的key2:   (student_filed_jerry, 90)

ZSCORE student tom # 查询 tom 的成绩
# 直接调用kv引擎的 get接口, 查询 (student_tom, 100) 即可

ZCARD student # 查询学生数量
# 直接调用kv引擎的 get接口, 查询 (student, 2) 返回2

ZINCRBY student 10 tom # tom 成绩加10分
# 1. 先查询 tom 的成绩
# 2. 将 tom 的成绩加10分
# 3. 更新 (student_filed_tom, 110)
# 4. 插入 (student_score_000110, tom) , 删除 (student_score_000100, tom)
# PS: 如果 tom 不存在则直接新建

ZRANGE student 0 1 # 查询前两名学生
# 使用谓词查询, 谓词为查询所有前缀为 student_score_ 的key, 并按照key排序, 返回前两个key中的value即可
```

# 3 代码实现
通过上面的理论讲解和案例说明, 你应该对此非常熟悉了...

你需要修改的代码文件包括:
- `src/redis_wrapper/redis_wrapper.cpp`
- `include/redis_wrapper/redis_wrapper.h` (Optional)

> 下面的接口中, 你仍然需要进行`TTL`超时时间的判断, 同时你可能需要更新之前的`redis_ttl`和`redis_expire`以兼容`ZSet`的`TTL`机制。

## 3.1 zadd
```cpp
std::string RedisWrapper::redis_zadd(std::vector<std::string> &args) {
  // TODO: Lab 6.4 如果有序集合不存在则新建，添加一个元素到有序集合中
  // ? 返回值的格式, 你需要查询 RESP 官方文档或者问 LLM
  return ":1\r\n";
}
```
注意, 这里支持一次性添加多个`filed`, 返回值表示多少个`filed`添加成功

## 3.2 zrem
```cpp
std::string RedisWrapper::redis_zrem(std::vector<std::string> &args) {
  // TODO: Lab 6.4 删除有序集合中的元素
  int removed_count = 0;
  return ":" + std::to_string(removed_count) + "\r\n";
}
```
注意, 这里支持一次性删除多个`filed`, 返回值表示多少个`filed`删除成功

## 3.3 zrange
```cpp
std::string RedisWrapper::redis_zrange(std::vector<std::string> &args) {
  // TODO: Lab 6.4 获取有序集合中指定范围内的元素
  return "*0\r\n";
}
```
查询指定范围的元素, 返回值表示查询到的元素的数组(`RESP`格式), 你需要使用`LSM`的谓词查询接口

## 3.4 zcard
```cpp
std::string RedisWrapper::redis_zcard(const std::string &key) {
  // TODO: Lab 6.4 获取有序集合的元素个数
  return ":1\r\n";
}
```
查询有序集合的元素个数

## 3.5 zscore
```cpp
std::string RedisWrapper::redis_zscore(const std::string &key,
                                       const std::string &elem) {
  // TODO: Lab 6.4 获取有序集合中元素的分数
  return "$-1\r\n";
}
```
查询有序集合中指定`filed`的分数, 你只需要按照指定格式拼接`key`进行查询即可

## 3.6 zincrby
```cpp
std::string RedisWrapper::redis_zincrby(const std::string &key,
                                        const std::string &increment,
                                        const std::string &elem) {
  // TODO: Lab 6.4 对有序集合中元素的分数进行增加
  return "$-1\r\n";
}
```
对有序集合中指定元素的分数进行增加, 返回值表示增加后的分数

## 3.7 zrank
```cpp
std::string RedisWrapper::redis_zrank(const std::string &key,
                                      const std::string &elem) {
  //  TODO: Lab 6.4 获取有序集合中元素的排名
  return "$-1\r\n";
}
```
获取有序集合中指定元素的排名, 返回值表示排名

# 4 测试
在完成上面的功能后, 你应该能通过下面的测试:
```bash
✗ xmake
✗ xmake run test_redis
✗ xmake run test_redis
[==========] Running 11 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 11 tests from RedisCommandsTest
[ RUN      ] RedisCommandsTest.SetAndGet
[       OK ] RedisCommandsTest.SetAndGet (13 ms)
[ RUN      ] RedisCommandsTest.IncrAndDecr
[       OK ] RedisCommandsTest.IncrAndDecr (9 ms)
[ RUN      ] RedisCommandsTest.Expire
[       OK ] RedisCommandsTest.Expire (2011 ms)
[ RUN      ] RedisCommandsTest.HSetAndHGet
[       OK ] RedisCommandsTest.HSetAndHGet (17 ms)
[ RUN      ] RedisCommandsTest.HDel
[       OK ] RedisCommandsTest.HDel (9 ms)
[ RUN      ] RedisCommandsTest.HKeys
[       OK ] RedisCommandsTest.HKeys (9 ms)
[ RUN      ] RedisCommandsTest.HGetWithTTL
[       OK ] RedisCommandsTest.HGetWithTTL (2111 ms)
[ RUN      ] RedisCommandsTest.HExpire
[       OK ] RedisCommandsTest.HExpire (1109 ms)
[ RUN      ] RedisCommandsTest.SetOperations
[       OK ] RedisCommandsTest.SetOperations (8 ms)
[ RUN      ] RedisCommandsTest.ZSetOperations
[       OK ] RedisCommandsTest.ZSetOperations (8 ms)
[ RUN      ] RedisCommandsTest.ListOperations # failed
```