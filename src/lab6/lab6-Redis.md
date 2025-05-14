# Lab 6 Redis 兼容
之前章节的`Lab`, 你已经实现了一个功能较为完备的`LSM Tree`, 这一章我们将目光向上层移动, 来设计一个兼容`Redis`协议的服务层, 使其能替代`redis-server`处理`redis-cli`的请求。

需要注意的是，本章的实验实现了`Redis`的`RESP`协议的解析与命令功能的实现, 底层使用的是我们的`LSM Tree`存储引擎的`KV`接口。

此外本章不会额外介绍`Redis`是什么, 你可以参考[Redis官网](https://redis.io/learn/howtos/quick-start)进行简短的学习(只需要了解基础命令的使用即可)。


# 1 Resp协议简介
这里就不再对`Redis`本身的基础概念进行介绍了, 毕竟`Redis`是校招八股必背知识点, 大家想必都非常熟悉了. 但大多数朋友可能对`Redis`通信的`Resp`协议完全不熟悉, 这里简单介绍一下`Resp`协议.

Redis的RESP（REdis Serialization Protocol）是Redis客户端与服务器之间通信的协议, 也就是`redis-cli`和`redis-server`之间进行通信的协议，它简单、高效，支持多种数据类型。因此, 其只需要描述`Redis`中的数据类型和请求类型就可以了。

需要说明的是, `Resp`协议应该属于`TCP`这一层的协议, 其没有`HTTP`等协议的头, 实现时我们也不需要`http`层的框架

## 1.1 一个简单的案例
假设你在 `redis-cli` 中输入了以下命令：
```bash
SET k1 v1
```
`redis-cli`客户端会将该命令转换为 `RESP` 协议格式并发送给 `Redis` 服务器。具体表示如下：
```text
*3
$3
SET
$2
k1
$2
v1
```
> 这里其实显式地表达了换行符`\r\n`, 真实的内容是: `*3\r\n$3\r\nSET\r\n$2\r\nk1\r\n$2\r\nv1\r\n`

**解释：**

- `*3`：表示这是一个包含 3 个元素的数组。
- `$3`：表示第一个元素是一个长度为 3 的批量字符串（Bulk String），内容为 `SET`。
- `$2`：表示第二个元素是一个长度为 2 的批量字符串，内容为 `k1`。
- `$2`：表示第三个元素是一个长度为 2 的批量字符串，内容为 `v1`。

> `\r\n`为不同字段之间的分隔符, 且不计入长度

`Redis` 服务器接收到上述请求后，执行 `SET k1 v1` 操作，并返回响应。例如：

```bash
+OK
```

**解释：**

- `+OK`：表示这是一个简单字符串（Simple String），值为 `OK`，表示操作成功。

如果客户端发送的命令或参数有误，`Redis` 服务器可能会返回错误信息。例如：
```bash
-ERR syntax error
```

**解释：**

- `-ERR`：表示这是一个错误消息（Error），内容为 `syntax error`。


## 1.2 数据类型语法
通过之前的案例我们可以看到, `RESP`使用一些符号来对数据类型进行标记, 这里简单总结如下:

- **简单字符串（Simple Strings）**：以"+"开头，如`+OK\r\n`。
- **错误（Errors）**：以"-"开头，如`-ERR unknown command\r\n`。
- **整数（Integers）**：以":"开头，如`:1000\r\n`。
- **批量字符串（Bulk Strings）**：以"$"开头，如`$6\r\nfoobar\r\n`。
- **数组（Arrays）**：以"*"开头，如`*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n`。


# 2 Redis实现思路
这里我们将利用自身的`LSM Tree`接口来设计一个兼容`Redis`协议的服务层, 使其能替代`redis-server`处理`redis-cli`的请求.

首先我们回顾一下我们的`LSM Tree`接口支持什么api:
1. `Put(key, value)`: 将键值对插入到数据库中。
2. `Get(key)`: 根据键获取对应的值。
3. `Delete(key)`: 根据键删除对应的值。
4. `Scan(start_key, end_key)`: 根据起始键和结束键范围获取键值对。(就是上一章实现的谓词查询, 这里的`Scan`是一个虚拟的接口)

然后我们想一下`Redsis`的不同数据结构的接口
1. **字符串（String）**：和`LSM Tree`一样, 我们也只需要实现`Put(key, value)`和`Get(key)`即可
2. **列表（List）**：相当于不同字符串之间有连接
3. **哈希（Hash）**：一个哈希的`key`由很多个`filed`即`value`组成
4. **集合（Set）**: 一个集合的`key`由很多个`member`, 但不需要排序
5. **有序集合（Sorted Set）**: 集合的`key`由很多个`member`和`score`组成, 并且需要按照`score`排序

同时很多`key`还有一些基础属性, 最常用的就是`TTL`(过期时间), 当`TTL`过期时, 该`key`将不再存在, 我们也可以通过`TTL`来判断一个`key`是否过期, 也可以使用`EXPIRE`来设置一个`key`的过期时间

其实本质上, 由于我们的存储引擎是`KV`存储, 我们的哈希的所有数据都将作为基础的`key`和`value`进行存储, 这与`Redis`中的`字符串`是一致的。而`List`, `Set`, `Sorted Set`等数据结构就需要将多对`key`进行组合, 并且需要根据一定的规则进行排序, 而且回想我的`LSM Tree`接口支持的api, 核心思路就只有2类:

1. `List`, `Set`, `Sorted Set`等数据结构的`key`的`value`中要记录所管理成员的元信息
2. 归属于某个大数据结构(`List`, `Set`, `Sorted Set`等)的成员的`key`需要包含统一的前缀, 这样才能通过我的的存储引擎进行前缀查询

现在你已经对`RESP`和`Redis`的兼容层的设计思想有一个大致的认知, 接下来我们就可以开始实现我们的兼容层了: [Lab 6.1 简单字符串](./lab6.1-String.md)

> !!! 你做`Lab`时如果对`RESP`协议的格式不熟悉, 可以参考附录中的[RESP 常见格式](../appendix/RESP.md)进行学习