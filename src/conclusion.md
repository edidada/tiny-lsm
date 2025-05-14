# 结语
本项目的初衷是作为`LSM Tree`的入门课程, 使参与者入门数据存储领域。这个`Lab`从代码实现, 到最后`Lab`文档的设计, 耗时半年, 花费了作者非常多的心血。因此, 如果您觉得这个`Lab`对您有帮助, 请在`GitHub`上给这个`Lab`一个`Star`。

同时也感谢网友[koi](https://github.com/koi2000)对本项目文档的校正与补全, 让这个`Lab`的文档更加完善。

感谢[Goodenough-hu](https://github.com/Goodenough-hub)、[snowcgj](https://github.com/snowcgj)、[mapleFU](https://github.com/mapleFU)对`toni-lsm`代码建设做出的贡献。

# 下一步？
当然，这个项目的定位只是让你初步入门`LSM Tree和存储领域`, 如果你想深入理解`LSM Tree`，建议阅读`leveldb/rocksdb`的源码。

此外，以下是一些存储领域的开源学习资料和项目，供进一步学习：

- **[Apache Cassandra](https://cassandra.apache.org/)**: 一个分布式 NoSQL 数据库，广泛使用 LSM Tree 作为其存储引擎的核心。
- **[HBase](https://hbase.apache.org/)**: 一个基于 Hadoop 的分布式数据库，适合处理大规模结构化数据。
- **[TiDB](https://github.com/pingcap/tidb)**: 一个开源的分布式 SQL 数据库，支持水平扩展和强一致性。
- **[ClickHouse](https://clickhouse.com/)**: 一个用于在线分析处理 (OLAP) 的列式数据库管理系统。
- **[FoundationDB](https://github.com/apple/foundationdb)**: 一个分布式数据库，提供事务支持，适合构建复杂的存储系统。
- **[DuckDB](https://duckdb.org/)**: 一个嵌入式分析数据库，适合处理小型到中型数据集。
- **[SQLite](https://sqlite.org/)**: 一个轻量级嵌入式数据库，适合学习存储引擎的基础知识。
- **[bbolt](https://github.com/etcd-io/bbolt)**: 一个嵌入式键值数据库，基于 B+ 树实现，适合学习与 LSM Tree 不同的存储结构。
- **[Ceph](https://ceph.io/)**: 一个统一的分布式存储系统，支持对象存储、块存储和文件存储，广泛应用于云计算和大规模存储场景。

通过研究这些项目的源码和文档，你可以更全面地了解存储系统的设计与实现。