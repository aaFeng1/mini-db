## 基本类

### page

维护了：

内存中的一片内存

### page guard

析构时自动调用buffer pool的unpin，避免使用后忘记释放pin

维护了：

- buffer pool指针
- 页号
- 页指针
- 脏页标记

提供接口：

- get page
- set dirty

### rid

结构体，包含页号和槽位，这样能定位一条记录

## 存储层

### buffer pool

维护了：

- 页数组（大小即为pool size）

- 每个页元信息（pid，pincount，脏页）
- 一个map（用于查询pid对应的帧id）
- 空闲列表
- disk manager指针（用于脏页刷盘和页加载）

支持以下接口：

- **fetch page**

返回一个页指针

- **unpin page**

释放对某个页的pin，并标记是否写

- **flush page**

将某个页刷新到磁盘

- **flush all pages**

将所有页刷新到磁盘

- **fetch page guarded**

提供更好的页使用的接口

### disk manager

维护了：

- 文件路径
- fd

提供以下接口：

- 写页
- 读页

### table page

作为table存储数据的页，提供一些增删查的接口

提供以下接口：

- from，将某片内存转为table page指针
- init，初始化
- insert tuple，插入元组
- get tuple，查询接口
- mark delete，逻辑删除，不支持真正删除
- is delete，检查某个槽位元组是否删除
- get free space，得到空闲空间
- get slot count，得到目前已占用槽数量
- get next page id，由于table page是按链表的结构串起来的，在头部有下一页的pid
- set next page id，npid修改接口

### table heap

表本身，维护table page的链表，提供增删接口

维护了：

- buffer pool的指针，加载和刷新页需要buffer pool
- first page id，链表头页号
- last page id，链表尾页号

提供以下接口：

- insert tuple，返回rid，也就是记录唯一位置
- get tuple
- delete tuple
- begin，模仿stl的迭代器，用于方便遍历所有的元组
- end，结尾迭代器
- get first page id，拿到链表头页id

### table iterator

提供解引用，前置++，==和!=的接口

维护了：

- table heap指针，因为它需要知道自己来自哪张表
- rid，目前指向什么位置
- end，是否结束

### tuple

本质就是一串连续内存

## catalog

目前只做了内存目录，也就是说目录信息没有持久化

### column

结构体，包含了一列的元信息，包括列名字，列类型，偏移，长度

### schema

本质就是维护了column数组，表示一张表里的所有列信息。主要接口就是add column，以及get

### table info

结构体，包含表名，schema，tableheap指针

### catalog

维护了：

- 表名到table info的map
- buffer pool指针
- next table id

提供create和get table接口

## parser

### token

一串或一个有意义的字符

维护了：

- type
- span位置偏移
- 一个string view

### lexer

用于将sql切成一个个token，供上层消费

### literal

字面量，包括整数字面量和字符串字面量

只维护了string view

### statement

目前支持插入声明和查询声明

声明里维护了表名和插入的数据或者查询的数据

### parser

维护了一个lexer，并不断消费token产出statement

## binder

### binder

将statement转为bound statement，也就是说它的工作就是将表名对应tableheap，将字面量转为对应value，生成可以执行的bound statment

维护了：

- catalog

### bound statement

目前支持插入statement和查询statement，例如插入statement维护了table info和插入的value

### value

真实值，目前支持整型和字符串

## execution

### executor

目前支持插入和查询，每个executor支持init和next接口，init后只需要不断调用next即可执行对应逻辑



















