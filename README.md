# mini-db

 别人 clone 你的仓库时怎么拉 submodule（你最好写进 README）

两种方式：

方式 A：clone 时带上
git clone --recursive <your_repo>

方式 B：clone 后初始化
git submodule update --init --recursive


不写这句，别人只会得到一个空的 third_party/googletest，然后 CMake 报错——这就是 submodule 最常见“新人坑”。