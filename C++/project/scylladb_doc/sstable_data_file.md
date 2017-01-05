### SSTable Data File

在scylladb中，data 文件包含了一部分的真实的数据；这个文件基本上有一个行的列表组成，每一个行包含了对应的key和对应的列;

单独的data文件不能提供有效查找的方式(通过具体key找到对应的行)；为了更加有效的查询对应的行，scylladb提供了sstable index file和sstable summary file，通过建立索引来加快查询速度；而且为了加速定位一个key是否存在在一个sstable中，scylladb提供的bloom filter的文件，通过这个文件可以很快的定位当前的key是否存在在当前的sstable；bloom filter 这种方式如果判断是不存在那么key就一定不会存在在sstable，如果key被判断是存在的，那么这个key仅仅只是可能在sstable中;

本文档主要是解释sstable data文件的格式，不包含其他高级的概念，比如列簇、静态列、集合等，这些概念将会在其他的文档中介绍.

#### 1. the data file

the data file 仅仅只是一连串的行;

```
struct data_file {
	struct row[];
}
```

通常在查询过程中，系统通过index file得到对应的位置，然后获得对应的行的数据;

#### 2. 行对应的数据结构

每一个行都有一个头，头中包含了这个行的对应的key、是否被删除的数据结构、一个cell数组，cell是对应的列名和列对应的value,这个数组最后有一个特殊的cell，用来标示这个row已经结束，这也保证了row不需要去保存cell数组长度;

```
struct row {
	be16 key_length;
	char key[key_length];
	struct deletion_time deletion_time;
	struct atom atoms[];
}
```
上面提到这个数据结构中没有包含atoms本身的长度，而是通过一个特殊的atom来进行区分；假如我们想要将整个行读入内存然后再解析它的话，那我们可以通过sstable-index-file来得到对应的长度; 因为假如你已经通过sstable-index-file得到了对应key的数据的offset，那么紧接这个key后面的key的前一个字节就是这个行数据的最后的位置，所以可以根据这个关系来得到数据的长度;

这个`deletion_time`定义了一个行是否已经终结或者是否被标示删除;

```
struct deletion_time {
	be32 local_deletion_time;
	be64 marked_for_delete_at;
}
```
系统中通过一个特殊值来表示行存活的信息; LIVE = (MAX_BE32, MIN_BE64), 比如(7f ff ff 00 00 00 00 00 00 00 00 这样的值表示的就是没有被删除的行); `marked_for_delete_at`是表示当数据被删除之时的一个unix毫秒时间戳； 假如`marked_for_delete_at`被设置为MIN_BE64的话，那么这行就表示没有被删除; `local_deletion_time`是当行的墓碑被创建的一个本地的秒级时间戳；这个时间戳主要是为了有一个标准，这个标准是说这个被标志了墓碑的行会在合适被真正删除，有一个配置(gc_grace_seconds),也就是一个被标记为删除的行会在gc_grace_seconds之后才会被删除;

#### 3. Atoms 数据结构

一行的真实数据是其实cell的列表，这些cell还有一些额外的类型;

任何类型的atom类型开头都是列名; 假如`column_name_length`的长度为0，换而言之就是atom开始的两个直接为null 字节，那么这就是end of row atom，这是一个特殊的atom，用来表示一行的结束;其他类型的atom是不会以空的列名开头的；从上面的data file的数据类型可知，每一个行都会保存对应的列名，这样是比较消耗磁盘空间的，这部分消耗会在后期的压缩环节来进行压缩;

```
struct atom {
	be16 column_name_length;
	char column_name[column_name_length];
}
```
一个非空类型的atom还有一个字节的掩码，下面的各种类型都是使用继承的方式来扩展:

```
enum mask {
		DELETE_MASK = 0x01,
		EXPIRATION_MASK      = 0x02,
   	COUNTER_MASK         = 0x04,
    	COUNTER_UPDATE_MASK  = 0x08,
    	RANGE_TOMBSTONE_MASK = 0x10,
}
```
```
struct noempty_atom : atom {
	char mask;
}
```

这个掩码从一定程度上反映了atom的类型; 假如 `mask & (RANGE_TOMBSTONE_MASK | COUNTER_MASK | EXPIRATION_MASK) == 0`,那么表示这个atom是一个正常的atom；一个正常的atom有64位的时间戳，那么这个列对应的value组成，value是序列化的字符数组;

```
struct cell_atom : noempty_atom{
	be64 timestamp;
	be32 value_length;
	char value[value_length];
}
```

掩码是`COUNTER_UPDATE_MASK or DELETE_MASK`的cell可以被转化成cell_atom;

假如掩码是`RANGE_TOMBSTONE_MASK`,那么对应的cell的数据结构为:

```
struct range_tombstone_atom : noempty_atom {
	u16 last_column_length;
	char last_column_name[last_column_length];
	struct deletion_time dt;
}
```
range-tombstone atom的效果不只是针对单个列，而且会影响到column-name与last column name之间的所有列(这个比较是通过列名对应类型的比较器来进行比较的);

假如掩码是`COUNTER_MASK`,它对应的数据类型是：

```
struct counter_cell_atom: noempty_atom {
	be64 timestamp_of_last_delete;
	be64 timestamp;
	be32 value_length;
	char value[value_length];
}
```

假如掩码是`EXPIRATION_MASK`,它对应的数据类型是：

```
struct expiring_cell_atom: noempty_atom {
	be32 ttl;
	be32 expiration;
	be64 timestamp;
	be32 value_length;
	char value[value_length];
}
```

如果一个atom的掩码同时覆盖了`RANGE_TOMBSTONE_MASK, COUNTER_MASK or EXPIRATION_MASK`中的两个，都是不正常的;

#### 4. 列名和value的序列化

必须要牢记的是：不管是列名还是对应的value都会被序列化成一个字节序列(类似于字节数组); 但是由于Cassandra不管是列名还是value都是支持多种类型；所以在这些值被写入到磁盘之前都必须被序列化成字符数组的类型;

从cassandra 1.2开始，除非在创建的table的时候使用`with compact storage`,大部分的列名都是组合的，是有几个部分组成的；一个组合的列名被序列化成为字节数组:

```
struct serialized_composite_name {
	struct {
		be16 component_length;
		char[] component; //长度为component_length
		char end_of_component; // 通常是 0， 也可以是 -1(0xff) or 1(0x01)
	}component[];
}
```

这个`end_of_component`通常是0，可以是1 or -1;`can also be -1 or 1 for specifying not a specific column but ranges, as explained in comments in Composite.java and CompositeType.java.`

上面是将一个列名序列化成为一个component_name,然后这个序列化之后的字符数组会被作为列名再次被序列化，这样就会导致两次序列化，这是很浪费的过程; 比如:列名为: age, 那么首先它先序列化成component_name的结构,这个时候它就是`\0\3age\0`,然后会将这个结构作为真正的列名存放到sstable中去就会变成`\0\6\0\3age\0`; 当然上面说的那么多都是没有使用`with compact storage`,但是其实现在table基本都会使用`with compact storage`;

#### 5. CQL Row Marker

无







