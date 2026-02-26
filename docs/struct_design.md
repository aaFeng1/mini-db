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

## 索引层

索引层目前只实现了b+树索引，因此详细介绍一下索引层实现。

从最上层说起，索引和表本身应为同级，因此在catalog中维护了indexinfo。

此处的需求是：

- 能通过索引名找到索引，方便调试
- 能通过表名以及一个字段名能找到对应索引，这样select的时候能判断是否走索引这条路
- 目前禁止同表名同字段名创建两份索引

- indexinfo里应维护一个b+树指针

然后是b+树层

此处的需求是：

- 支持插入（key，rid）对
- 支持查询key->rid
- [暂不支持]支持删除key

先不管下一层页如何设计，假设页的设计能提供我们想要的所有功能，现在讲讲b+树的状态机。

b+树中维护了一个根页页号，和bufferpool指针用于加载页，最初的状态页号为非法页，并且此刻没有任何有效数据。

以下为插入伪代码

``` 
bool helpfun（p，key，value，*pidret）：
	//辅助递归函数，功能为将kv插入到以p为根的树中
	//若出现与p同级的分裂，也就是p在此次插入后满了分裂
	//修改pidret为新页页号，返回true，
	//否则返回false
	if p为内部页：
		从p中找到子页nep
		newpid
		if helpfun（nep，key，value，&newpid）：
			将newpid插入到p
		endif
	else：
		//为叶子页
		插入kv
	endif
	
    if p is full：
    	新分配页v
    	将p后一半数据挪到v
    	*pidret=v
    	return true
    endif
	
	return false

插入：
	if 根页页号非法：
		新分配一页叶子页
		init
	endif
	newpid
	if helpfun（root，key，value，&newpid）：
		新分配一页内部页fa
		将root和newpid加入到fa
		更新root为fa
	endif
		
```

查询伪代码：

```
查询：
	p=root
	while p是内部页：
		找到下一层ne，此时应该找到左边第一个大于等于key的
		p=ne
	// 此时就找到了叶子页
	遍历链表，保存所有合法答案
	直到链表空或者遇到第一个大于的
	返回结果
```

因此，对于b+树页，做以下设计

BPlusTreePage基类维护共有的header，并且提供getter和setter

插入查询逻辑都类似，分裂逻辑对于内部页和叶子页不同





## 遇到的坑

### 技术层面

- page不需要构造和析构函数，已经是内存里的一块，只需要强制类型转换就行，当然有风险，比如不能包含虚函数
- bufferpool忘记unpin
- bufferpool忘记清掉map，导致已经下掉的页仍能查询到
- 模板的使用细节，模板只是模板，不是某个类
- b+树和bufferpool交互巨复杂
- 即便是拿到pageguard也有可能忘记setdirty
- 在页分裂后应该直接set，如果insert的话会扰乱顺序
- 叶子页链表忘记加了，或者加错了
- 如果某页首条数据被修改了，需要修改其parent的对应key数据
- 有些接口需要引用，但真正用的时候只有指针
- Value经常需要比较，要序列化和反序列化
- 在建索引的时候，需要考虑原表中有没有数据，有的话要先insert
- 在查找的时候只提供了列名，所以还有一步是找到列名所对的索引
- 索引的insert居然没有调用，但是居然有数据
- 二分经典坑，二分用int，别用无符号



### 工程能力层面

- 写代码只是最简单的一步，需要先想清楚状态机，伪代码，验收测试和极端测试，需要流程兜底
- 测试很重要，当功能出问题的时候，我能知道应该不是之前的模块，因为他们有测试包着
- 不要每次开始前想着还有好多大功能没做，怎么做地完，只需要想接下来20分钟要做什么就行
- 文档很重要，不管什么文档，伪代码文档，架构文档，经验文档，了，流程文档
- 代码注释很重要

















































































