# 阶段2-SST
在完成了`Block`组件和`BlockIterator`组件之后, 我们就可以开始实现`SST`组件了。

`SST`组件的核心工作就是管理器内部的多个`Block`组件, 利用`Block`组件和`BlockIterator`的各种`CRUD`和初始化、构建等基本接口，来对外提供各类功能。
