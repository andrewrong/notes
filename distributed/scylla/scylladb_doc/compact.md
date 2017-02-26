### compact操作
***

#### 定义
compact操作是将读取多个sstable写入到一个sstable的过程，这个过程本身是包含聚合过程，将相同的数据做聚合，比如更新、删除等一些变更操作。完成了compact之后之前的sstable就会删除，而从此刻开始查询就以当前的sstable为主. 

scylladb的compact会在一个新的sstable被写到磁盘的时触发，这样可以降低磁盘的使用率;

#### compact策略

1. 基于大小分级的compaction

	这种compaction策略是cassandra最早的策略；它的触发条件是当系统有足够多相似大小的sstable，这个时候系统就会将几个相似大小的sstable合并成一个大的sstable; 这其实就意味着在系统中会有按照尺寸大小的分级(small，large，even-large),而且每一个级别都有着大概类似的文件个数，当某一个级别的文件个数已经满了之后，系统就会合并所有本层次的sstable，变成下一个层次的一个sstable;

	对于那种写完之后很少会修改的场景十分合适用基于大小分级的compact策略；因为每一行最终会被写到一个巨大的sstable中去，这样会极大的提高read的效率；而如果面对某一个行需要被重复的修改或者覆盖的场景的话，这个策略就不会很合适；原因是你的行会存在于很多的sstable中，因为你一直会修改；这样就会导致查询的过程很慢，毕竟要轮训多个文件来查找这个key;

	对于此策略有两个缺点：

	* 存在于一些大的sstable中的废弃数据将会被保存很长时间；这样是非常浪费空间的；原因其实很简单，本身系统就很难生成足够多的超大的sstable，所以如果对一些存在于这些sstable中的废弃的数据，必须到下一次这些sstable足够多，触发compact的时候才会删除；所以越是到后期，这样的数据可能会越来越多，导致很占空间;
	* compact的时候需要很多的磁盘空间；在极端情况下，我们需要合并所有的已经存在的sstable，那么这个时候磁盘就必须存在足够的sstable，磁盘的利用率大概就只能有50%的利用率;


2. 层级的compaction策略

	此策略出现在Cassandra 1.0的时候，这个方式主要是来替换上面的巨大的sstable；它使用小的、固定的sstable，并且有层级的划分；具体的技术如下：
	
	* 从memtable 刷到sstable，这种新sstable是处于level 0层级的
	* 除了level 0 层级以外的层级叫做run的sstable,每一层之间是sstable文件数量上的差距，在scylladb系统中是10倍的差距，比如level1是10的sstable，那么level2是100个sstable,这是一个指数增加的过程;
	* A run of sstable是一个LSM结构的术语，其中的含义是：一组key没有交叉的sstable的集合，说白了就是一组sstable中的key没有交叉，每一个sstable中的key都是不一样的,无重叠的，这个前提是它们处于同一个层级;
	* 在同一个层级的run sstable其实类似于一个被分离的巨大的sstable；而run的好处在于：巨大的sstable对于修改就必须整个sstable重新compact，而run sstable 只要在保证所有的sstable key不重叠的基础上只修改一部分的sstable，而不需要修改所有的sstable，这也是相互独立的sstable带来的灵活性；
	
	下面是具体的compact过程:
	
	* 当l0层有充足的sstable，这个时候就会触发compact操作；这个过程类似于:我们会并行的读4个L0的sstable和10L1的sstable，然后写新的sstable到L1，这些sstable会在后期替换之前的sstable，后old sstable会被delete在compact完成之后;因为scylladb的sstable大小是固定的，所以到超过一定范围之后会写到新的sstable中去; 这些sstable依然是run sstable，无key重叠;
		* 当L0写到L1的时候，肯定会导致L1的sstable个数超过10个，这个时候就会触发从L1到L2的compact的过程；
			
			* 选择多出来的一个sstable
			* 读取这个sstable的key的返回，并且在L2层中找到与它有重叠的sstable; 这边有一个最大值的计算方式;因为L1层的一个sstable大概占有全局的key的1/10,而L2的一个sstable大概占有全局的key的1/100,所以通常10个sstable就可以覆盖L1的一个sstable，再加上边缘的两个sstable，所以最多能找到的sstable为12个;
			* 然后就对这L1中的一个sstable和12个L2的sstable进行重新的排列生成新的sstable，替换之前的sstable;

	上面的过程会慢慢的持续到最底层；而这个过程基本类似，除了L0与L1不一样以外，其他的基本类似，而触发的条件是当前层级的个数超过了当前层级规定的个数；
	
	层级的compact的好处是对read非常的有效，通常查找的过程是先查memtable、再查L0，一尺类推；因为最新的数据肯定是在最上面的层级；所以小文件比较多，但是因为每一个文件的key是有一定返回的并且排序的，通过都会对这些文件建立二次索引，所以查找某一个key的速度是很快的;
	
	另一个好处是:
	
	* 不会因为写入重复的值而极大的浪费磁盘中间，据scylladb官方写:最多10%的磁盘浪费；其实也很能理解，因为L0到L1发生的频繁就会导致数据会被频繁的更新到下面的层级，当然这个的原因是因为触发的条件和sstable个数有关系..
	* compact操作的时候不需要大量的空余的磁盘空间;从上面我们知道，每一次compact操作最多就只需要大于10 的sstable空间，这对比按照大小进行compact就是一个很大的优势;

	层级compact的缺点是: 写放大、


3. 基于时间的compact方式

	这个方式是在Cassandra 2.1中加入的，设计的主要针对时间序列的数据；时间序列在scylladb中主要是用时间戳来做列键，在时间序列的case中，通常有一下几个共同的特征：
	
	* 列键与写入的时间是相互关联的;
	* 数据以时间序列来添加，只有小部分的乱序的写入数据，经典的场景就是几秒乱序
	* 数据的删除依赖于TTL或者整个分析删除
	* 数据写入的qps相对比较稳定，接近常量
	* 数据查询通常是对特定分区进行时间范围的查询，最最常用的方式是最近一分钟、一小时、一天等;

	
	基于以上假设，我们很容易知道sstable的compact操作应该是和时间区间有关，然而 按照大小分级和层级的compact肯定是会破坏这个关系，因为它们两者的compact的方式是将新老数据合并到一起；
	
	按照时间维度的compact在压缩几个相似的sstable的时候(按照时间维度)，首先会对sstable按照时间序列排序；这样的结果就是sstable的大小会随着时间的累计而慢慢变大；比如我们有4个1分钟的sstable(scylladb 默认的base_time_seconds = 60s),那么可能就触发它进行compact，将4个sstable compact成1个4分钟的sstable,其实这个有点类似于按照大小分级的compact方法，它是以大小进行compact，而这个是以时间的区间为依据;
	
scylladb中比max_sstable_age_days(默认是365)还要古老的sstable是不会再做任何压缩的，所以对这些区间的查询将变得很慢，并且需要大量的临时的磁盘空间; 

