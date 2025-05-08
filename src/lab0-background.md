# 1. LSM Tree 简介
在具体进入本实验之前，我们先来简单介绍`LSM Tree`。

`LSM Tree`是一种`KV`存储架构。其核心思想是，将`KV`存储的数据以`SSTable`的形式进行持久化，并通过`MemTable`进行内存缓存，当`MemTable`的数据量达到一定阈值时，将其持久化到磁盘中，并重新创建一个`MemTable`。`LSM Tree`的核心思想是，将`KV`存储的数据以`SSTable`的形式进行持久化，并通过`MemTable`进行内存缓存。并且， 数据以追加写入的方式进行，删除数据也是通过更新的数据进行覆盖的方式实现。

![Fig 1](images/intro/toni-lsm-arch.drawio.png)

如图所示为`LSM Tree`的核心架构。我们通过`Put`, `Remove`和`Get`操作的流程对其进行介绍。

## 1.1 Put 操作流程
`Put`操作流程如下：
1. 将`Put`操作的数据写入`MemTable`中
   1. `MemTable`中包括多个键值存储容器(本项目是采用的跳表`SkipList`)
      1. 其中有一份称为`current_table`, 即活跃跳表, 其可读可写
      2. 其余的多份跳表均为`frozen_tables`, 即即冻结跳表, 其只能进行读操作
   2. `Put`的键值对首先插入到`current_table`中
      1. 如果`current_table`的数据量未达到阈值, 直接返回给客户端
      2. 如果`current_table`的数据量达到阈值, 则将`current_table`被冻结称为`frozen_tables`中的一份, 并重新初始化一份`current_table`
2. 如果前述步骤导致了新的`frozen_table`产生, 判断`frozen_table`的容量是否超出阈值
3. 如果超出阈值, 则将`frozen_table`持久化到磁盘中, 形成`SST`(全程是`Sorted String Table`), 单个`SST`是有序的。
   1. `SST`按照不同的层级进行划分, 内存中的`MemTable`刷出的`SST`位于`Level 0`, `Level 0`的`SST`是存在重叠的。（例如`SST 0`的`key`范围是`[0, 100)`, `SST 1`的`key`范围是`[50, 150)`, 那么`SST 0`和`SST 1`的`key`在`50, 100)`范围是重叠的, 因此无法在整个层级进行二分查询）
   2. 当`Level 0`的`SST`数量达到一定阈值时, 会进行`Level 0`的`SST`合并, 将`SST`进行`compact`操作, 新的`SST`将放在`Level 1`中。同时，为保证此层所有`SST`的`key`有序且不重叠, `compact`的`SST`需要与原来`Level 1`的`SST`进行重新排序。由于`compact`时将上一层所有的`SST`合并到了下一层, 因此每层单个`SST`的容量是呈指数增长的。
   3. 当每一层的`SST`数量达到一定阈值时, `compact`操作会递归地向下一层进行。

## 1.2 Remove 操作流程
由于`LSM Tree`的操作是追加写入的, 因此`Remove`操作与`Put`操作本质上没有区别, 只是`Remove`的`value`被设定为空以标识数据的删除。

## 1.3 Get 操作流程
`Get`操作流程如下:
1. 先在`Memtable`中查找, 如果找到则直接返回。
   1. 优先查找活跃跳表`current_table`
   2. 其次尝试查找`frozen_tables`, 按照`id`倒序查找(因为`id`越小, 表示`SkipList`越旧)
2. 如果在`Memtable`中未找到, 则在`SST`中查找。
   1. 先在`Level 0`层查找, 如果找到则直接返回。注意的是, `Level 0`层不懂的`SST`没有进行排序, 因此需要按照`SST`的`id`倒序逐个查找(因为`id`越小, 表示`SST`越旧, 优先级越低)
   2. 随后逐个在后面的`Level`层查找, 此时所有的`SST`都已经进行过排序, 因此可在`Level`层面进行二分查询。
3. 如果`SST`中为查询到, 返回。

至此，你对`LSM Tree`的`CRUD`操作流程有了一个初步的印象, 如果你现在看不懂的话, 没关系。后续的`Lab`中会对每个模块有详细的讲解。

# 2. LSM Tree VS B-Tree

接下来对比分析一下LSM Tree 和 B-Tree。B-Tree 和 LSM-Tree 是两种最常用的存储数据结构，广泛应用于各种数据库和存储引擎中。

## 2.1 B-Tree 概述
### 2.1.1 基本结构
* **多路搜索树**：每个节点可包含多个键值对及对应子节点指针。
* **页（Page）存储**：通常以固定大小页（如 4 KB）为单位，将数据保存在叶子节点或中间节点中。

### 2.1.2 读写流程

* **查找**：从根节点开始，依次向下比较键值，直到叶子节点。
* **写入**：在页内定位并修改；若页已满，则分裂节点并向上调整，保持树的平衡。

### 2.1.3 性能特点

* **查找**：时间复杂度 O(log n)，适合读密集型应用。
* **写入**：每次随机写与节点分裂会带来较高的 I/O 开销，不利于写密集型场景。

---

## 2.2. LSM-Tree 概述

### 2.2.1 设计动机

为了解决传统 B-Tree 在写密集场景下的随机写和高写放大问题，LSM-Tree 采取“先内存写、再后台合并”的思路，以空间换时间，提升写入吞吐。

### 2.2.2 主要组件与流程

1. **MemTable（内存表）**：所有写操作首先追加到内存中的有序结构（如跳表）。
2. **SSTable（磁盘表）**：当 MemTable 达到阈值时，将其定期刷新为只读的磁盘文件。
3. **后台 Compaction（合并）**：将多个旧的 SSTable 合并、去重，并生成新的 SSTable，控制文件数量与数据冗余。

### 2.2.3 优化手段

* **Bloom Filter**：快速判断某 key 是否存在于某个 SSTable，减少不必要的磁盘查找。
* **Index Block / Block Cache**：缓存热点数据页，加速读请求。
* **Tiered / Leveled Compaction**：通过分层或分阶段合并，平衡写放大与读放大。

---

## 2.3. B-Tree vs. LSM-Tree 对比

| 特性     | B-Tree          | LSM-Tree                         |
| ------ | --------------- | -------------------------------- |
| 写入模式   | 随机写，页内更新，可能分裂节点 | 顺序写（追加），后台合并                     |
| 写放大    | 较低              | 较高（受合并策略影响）                      |
| 读取路径   | 单次树搜索           | 多级查找（MemTable + 多个 SSTable + 合并） |
| 读写性能场景 | 读密集型            | 写密集型                             |
| 磁盘友好性  | 随机 I/O          | 顺序 I/O，更适合 SSD                   |

> **小结**：LSM-Tree 在写入吞吐上优于 B-Tree，但读取延迟与 I/O 成本相对更高。
> 更深入的对比分析可以阅读：
> [https://tikv.org/deep-dive/key-value-engine/b-tree-vs-lsm/](https://tikv.org/deep-dive/key-value-engine/b-tree-vs-lsm/)

---

## 2.4 常见系统及应用场景

| 存储引擎 / 数据库   | 类型          |
| ------------ | ----------- |
| LevelDB      | 嵌入式 KV 存储引擎 |
| RocksDB      | KV 存储引擎     |
| HyperLevelDB | 分布式 KV 存储   |
| PebbleDB     | 嵌入式 KV 存储   |
| Cassandra    | 分布式列存储数据库   |
| ClickHouse   | 分析型数据库      |
| InfluxDB     | 时间序列数据库     |

---

## 2.5 LSM-Tree 优化研究

LSM Tree自1997年提出后，有很多研究人员在LSM Tree的基础上进行了改进，部分代表性工作比如：


[RocksDB](), [LevelDB]()：经典的LSM Based KV存储引擎.

[WiscKey: Separating Keys from Values in SSD-Conscious Storage](https://www.usenix.org/system/files/conference/fast16/fast16-papers-lu.pdf)：分离 Key 和 Value，降低写放大

[Monkey: Optimal Bloom Filters and Tuning for LSM-Trees](https://www.usenix.org/system/files/conference/fast16/fast16-papers-lu.pdf)：优化布隆过滤器分配策略，减少读放大


这里不再进行详细介绍，感兴趣的可以阅读相关论文。



