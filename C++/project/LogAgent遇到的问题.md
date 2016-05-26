### 新版 LogAgent 
***

#### 1. LogAgent的功能

新版 LogAgent 是使用C++重新开发的，整个项目整体意义在于将日志收集的 kafka 中的，中间包含通过任务调度将不同文件和共享内存中的日志发送到 kafka 中去; 功能比较简单，所以开发难度不是很大; 

* 本次开发使用到的库:[folly](https://github.com/facebook/folly)、[rdkafka](https://github.com/edenhill/librdkafka)、boost 等一系列的库;
* 包管理工具: 公司中C++大神搭建的 [conan](https://github.com/conan-io/conan), 这东西非常的方便...有时间准备自己搭建一下..
* 其他: Clion、cmake等

#### 2. 性能如何

公司之前有一款 Java 的 LogAgent，但是因为是开发周期比较早，所以在设计和效率上都有点欠缺；新版在性能方面还是做了很多的优化测试的;

对于新版的LogAgent主要做了一下的性能测试:

1. 单个文件17G，一个broker，一个分区，ack = 1, 无消费者消费kafka;

> 新版LogAgent在这样的情况下，能跑满网卡; kafka client内部的缓存队列基本无消息堆积;

2. 单个文件17G，一个broker，一个分区，ack = 1, kafka出口流量跑满;

> 在这样的情况下，新版LogAgent的生产速度为15~16m/s, 相比与上一个压测，发现当kafka出口流量满的时候，会知道新版LogAgent有大量的日志堆积等待kafka的ack; 当然15m/s的速度已经能抗住大促时候的峰值流量，而且现在的kafka还只是一个broker和一个分区;

3. 单个文件17G, 2个broker，2个分区， ack = 1，kafka 出口流量跑满

> 新版LogAgent的生产速度为50m/s; 两台机器的kafka的出口流量也全部跑满；就这个出口流量，可以保证大促期间的日志不会延迟;

#### 3. 中间值得总结的技术

#### 4. 第一版本release 遇到的问题

1. 文件不能自动切换(已解决); 来源于在遍历文件内容的时候，没有将回车符作为正常字符处理；主要发生与当内容中出现空行的时候就会有问题;
2. 上下文切换过于频繁，在系统压力很大的时候，会影响系统性能; 这个原因目前估计是: 使用 rdkafka 的 poll 函数的时候，等待时间过短，仅仅只等待了10ms，导致线程不断从阻塞状态切换成运行状态，导致就算是没有任务也会有频繁的上下文切换问题；
3. kafka 回流到 LogAgent 消息: 这个现象比较怪异，通常都发生在没有任务的 LogAgent 机器上，发现这些机器上的 LogAgent 每秒都接受400kb/s 的数据，查看 tcpdump 发现都是来源于 Kafka，类似于 ack 的消息，这个问题还在查询中;
4. 这个应该是 rdkafka 的一个问题，当 broker 下线几台机器之后，会一直尝试重连，但是因为机器已经下线，所以就会导致连三次握手都建立不了，这样就导致频繁的 tcp重连；在 github 看到了关于这个的 [issue](https://github.com/edenhill/librdkafka/issues/238)；

#### 5. 关于这个项目的总结
