本实验的`1.0`版本已经完成，但目前该实验存在以下优化方向:

1. 自由度提升: `1.0`版本的`Lab`, 基本上就是在既有的项目代码下进行关键函数的挖空, 在组件设计层面没有给实验参与者太多的发挥空间, 后续应该添加更多的自由度, 允许实验者自由设计组件, 并且在组件设计中添加更多的测试用例。
2. 测试用例覆盖率不足: `1.0`版本的`Lab`中, 测试用例的覆盖率比较低, 比如对崩溃恢复的各种边界条件的考虑不足, 当然这确实也比较难控制就是了。
3. 各个`Lab`工作量不尽相同, 现在的`Lab`设计是按照功能模块进行划分的, 但这导致[Lab 5](./lab5/lab5-Tranc-MVCC.md)和[Lab 6](./lab6/lab6-Redis.md)的测代码量和难度远大于之前的`Lab`, 难度曲线可能不太合理, 后续应考虑对各个`Lab`进行更加均衡的划分。
4. 进一步补充一些背景理论知识, 尤其是实际场景中的各种性能优化方案。

如果你有兴趣参与本实验的建设，欢迎在下面的分支上提交PR: 

- [Lab代码分支](https://github.com/ToniXWD/toni-lsm/tree/lab)
- [Lab文档分支](https://github.com/ToniXWD/toni-lsm/tree/lab-doc)
- [代码开发分支](https://github.com/ToniXWD/toni-lsm/tree/master)

如果你有什么问题，可以通过 [QQ讨论群](https://qm.qq.com/q/wDZQfaNNw6) 或者 [📧邮件](mailto:xwdtoni@126.com) 联系到作者。

