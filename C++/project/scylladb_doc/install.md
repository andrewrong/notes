### 安装scylladb的教程
***

主要的依赖于官方的教程，这边主要是将一些细节进行串联描述，下次安装的时候会更加自动化;

我的安装环境如下：

* centos7.2
* 内核版本为3.18

#### 1. 关于scylladb几个rpm的介绍

* scylla-server(standard):scylladb主要的server端
* scylla-server(debuginfo):scylladb server端并且带有debuginfo
* scylla-jmx: 兼容cassandra通过jmx端口进行访问
* scylla-tools: scylla为了兼容cassandra而提供的类似的功能:
	* nodetool:很强大的功能，用来观察集群状态
	* cqlsh:
	* cassandra-stress:压测工具

#### 2.前期准备

* 删除abrt,主要是这个与scylladb本身的coredump配置冲突; `yum remove -y abrt`
* 必须有sudo权限
* 预安装的东西, `yum install -y wget epel-release`, `epel-release`是一个fedora维护的软件仓库，全名叫做*企业版Linux额外软件包*


#### 3. 正式开始装

* 下载最新的scylladb源，由于每次更新源都会有一定的变化，所以每一次更新的时候最好要更新一下源;下面的源是1.4版本的

```
sudo wget -O /etc/yum.repos.d/scylla.repo http://downloads.scylladb.com/rpm/centos/scylla-1.4.repo
```

* 是scylla yum源生效; `yum clean all; yum makecache`

* 安装scylla; `yum install -y scylla`

如果一些顺利的话，到这一步scylladb就已经安装完毕了

#### 4. 配置和脚本相关的

配置名字 | 配置作用 | 配置位置
-------| --------|--------
scylladb的主要配置 | 设定一些主要的参数，比如存储位置、开放端口和ip，也会有一些性能参数设置 | /etc/scylla/scylla.yaml
scylla启动脚本 | | /etc/sysconfig/scylla-server
系统资源限制 | 去掉对scylla用户的资源限制| /etc/security/limits.d/scylla.conf
启动脚本 | 设置参数、启动scylladb脚本 | /etc/sysconfig/scylla-server
coredump配置文件 | 设置coredump的配置文件 | /etc/sysconfig/sysctl.d/99-scylla.conf
collectd配置文件 | 设置collectd的一些配置 | /etc/collectd.d/scylla.conf


配置名字 | 配置作用 | 配置位置
-------| --------|--------
内核设置 | 在bootloader中设置内核参数 | /usr/lib/scylla/scylla_bootparam_setup
coredump配置文件生成器 | | /usr/lib/scylla/scylla_coredump_setup
ntp协议配置生成器 | | /usr/lib/scylla/scylla_ntp_setup
网络配置设定 | | /usr/lib/scylla/scylla_prepare
配置raid和文件系统的脚本 | | /usr/lib/scylla/scylla_raid_setup
压缩coredump脚本 | only ubuntu有效 | /usr/lib/scylla/save_coredump
重新设置网络模式 | 如果scylladb运行的virtio或者DPDK的话，就重新设置网络模式 | /usr/lib/scylla/scylla_stop
重新设置网络参数 | | /usr/lib/scylla/posix_net_conf.sh
io.conf生成器 | 用于测试io性能，并且把文件提供给scylladb | /usr/lib/scylla/scylla_io_setup 

上面这些脚本都是在`/usr/lib/scylla`目录下面的，并且还有其他的脚本，这些脚本会在运行scylla_setup的时候会调用;详情请看[url](http://www.scylladb.com/doc/system-configuration/)

作用 | 端口值
----|------
cql | 9042
内部rpc | 7000
ssl内部rpc | 7001
jmx 端口 | 7199
scylla rest api | 10000
Scylla Prometheus API(不知道是什么) | 9180
node_exporter | 9100


#### 5. 具体的配置

配置项 | 含义
------|-----
seeds | 种子ip列表，建议多几个ip
listen_address | scylladb内部通信的ip
rpc_address | 客户端连接scylladb的ip
broadcast_address | 默认listen_address，从其他节点角度看这个node的ip，可能是不同网络之间不同，而通过外网来进行互联
broadcast_rpc_address | 默认是rpc_address,从client角度上来你应该是什么ip，同上

#### 6. scylladb管理命令

命令 | 作用
-----|-----
scylla --version | 查看版本
nodetool snapshot | 将node数据进行快照;关于scylladb snapshot的原理是：通过linux的hardlink来进行备份，因为scylladb的文件产生之后会不会改变所以这样能很好的保证备份的作用，并且用来了hardlink保证文件的可用性，和不需要通过拷贝也加重系统的负载;
恢复备份 | 1. 清空commitlog 2. 清空data目录下面的数据文件 3. 把snapshot备份的文件mv过来即可 4. 重启
提供rest接口	| 
scyllatop | scylla自己的top, 目测可以与collectd一起使用，并且提供比较好的功能
Prometheus | scylla自己的监控系统
collectd | scylladb本地会启动collectd的进程，用来接收scylladb抛送过来的数据，可以通过插件的方式来修改，比较方便，并且会增加scyllatop看到的数据
log | scylla log是通过centos7的 journalctl 来控制的；最常用的有`journalctl _COMM=scylla -p warning` 或者 `journalctl _COMM=scylla --since "2015-01-10" --until "2015-01-11 03:00` 或者 `journalctl _COMM=scylla -b(从最近一次重启的日志)`


node tool命令介绍

命令 | 作用
----| ----
nodetool status | 看集群的状态,[详情](http://www.scylladb.com/doc/nodetool-commands/status/)
nodetool snapshot | 上面已提过
nodetool cfhistograms | 提供每一个表的静态数据，包括sstable的个数、读写延迟、分区的尺寸和列簇的个数;`nodetoll sfhistograms keyspace tablename`,[详情](http://www.scylladb.com/doc/nodetool-commands/cfhistograms/)
nodetool cfstats | 提供对特定的表的一个深层的分析;`nodetool cfstats keyspace.tablename`,[详情](http://www.scylladb.com/doc/nodetool-commands/cfstats/)
nodetool cleanup | 立即触发清理不属于本机的key；`nodetool cleanup -h 127.0.0.1 keyspace`
nodetool clearsnapshot | 清理snapshot文件,`nodetool clearsnapshot keyspace`,不写keyspace就默认删除全部的snapshot 
compactionhistory | 打印compact的历史
compactionstats | 打印目前正在compact的进度和一些信息
compact | 对可能的keyspac而进行强制的compact操作; `nodetool compact keyspace`
describecluster | 打印一些信息
decommission | 
describering | 打印某一个keyspace的一致性hash分布情况
disablebackup | 关闭增量备份
disablebinary | 关闭cql
statusbinary | 看cql当前运行的状态
enablebinary | 开启cql
statusgossip | gossip协议的运行状态
disablegossip | 关闭gossip
enablegossip | 开启gossip
drain | 通常用于升级scylladb之前使用，主要的操作是将所有的memtable全部写入到sstable，然后停止listen各种端口，之后要恢复必须通过重启
flush | 将当然的memtable刷成sstable
getendpoints |  `nodetool getendpoints keyspace tablename key`,现在还不知道用来做什么
getlogginglevels | 获得运行时中的日志等级
gossipinfo | 展示gossip协议中传播的东西
info | 展现当前节点的一些信息; [详情](http://www.scylladb.com/doc/nodetool-commands/info/)
listsnapshots | 展示所有的snapshot在磁盘上的占用率
move | 将node 分配到新的token
netstats | 打印一些网络信息
proxyhistograms | 对于网络操作打印一些静态统计
rebuild <src-dc-name>| 从另外一个节点重建数据
refrash <keyspace> <tablename>| 在不重启的前提下，重新reload文件中的sstable
removenode <ID>| 移除名为id的节点
repair |  修复一个或者多个的列簇； 参数比较多，使用前查询;
ring | 显示一致性hash的列表
setlogginglevel <class> <threshold> | 设置某一个类的运行时日志等级
settraceprobability -- <value>| 设置跟踪请求的概率，value在0~1之间
snapshot [-t tag] [-cf tablename] <keyspace>| 针对具体的表或keyspace做快照
statusbackup | 增量backup的状态
stop | 停止compact任务
version | db的版本  


nodetool info几个参数介绍

参数 | 描述
-----|-----
Gossip active | gossip状态
Thrift active | thrift 状态
native transport active | cql的状态
Load | sstable占磁盘多少空间
Generation NO | 主版本，当节点重启、token更新，这个版本就会往上增加
Uptime | scylla没有这个
Heap Memory | scylla没有这个
off Heap memory (MB) | 所有的memtables、bloom filters 、indexs、compression metadata占用的内存
Data center | 这个节点所在的数据中心
Rack | ？？、
Exception | scylla没有这个
Key Cache | scylla没有这个
Row Cache | Row cache的信息
Counter Cache | scylla没有这个
Token | token展示

#### 7. 关于scylladb的monitor安装

目前scylladb的监控是使用[Grafana and Prometheus
](https://github.com/scylladb/scylla-grafana-monitoring), 具体的安装过程如下:

1. 首先安装docker在服务器上; `sudo yum install -y docker`
2. 启动docker服务;`sudo service docker start`
3. 验证docker安装情况; `sudo docker ps -a or sudo docker images`

其实监控安装比较简单，但是由于网络问题安装就变成很蛋疼的过程;

1. 首先通过mac翻墙下载docker 镜像; `prom/prometheus` and `grafana/grafana`；
	
	```
	docker pull prom/prometheus
	docker pull grafana/grafana
	``` 

2. 将镜像导出成tar格式的文件

	```
	docker save imageid
	```

镜像id可以通过`docker images`来得到

```
	$ docker images
	REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
	grafana/grafana     latest              a892c250adfa        11 days ago         266.2 MB
		prom/prometheus     latest              bdeacb538ef9        2 weeks ago         79.25 MB
```

3. 将tar文件上传到服务器，然后导入服务器的docker

```
docker load -i xxx.tar
#修改对应的name和版本
docker tag imageId name:tag

sudo docker tag  047fd14b7251 prom/prometheus:v1.0.0
sudo docker tag d7528263f75a grafana/grafana:3.1.0
```

4. 下载 scylla-grafana-monitoring 项目

```
git clone https://github.com/scylladb/scylla-grafana-monitoring.git
```

5. 修改prometheus的配置文件

```
global:
	# 采集周期
  scrape_interval: 15s # By default, scrape targets every 15 seconds.

  # Attach these labels to any time series or alerts when communicating with
  # external systems (federation, remote storage, Alertmanager).
  external_labels:
    monitor: 'scylla-monitor'

scrape_configs:
- job_name: scylla
  honor_labels: true
  static_configs:
  # 这个是scylladb服务的地址，这边是主动采集为主，不是被动上报，主要收集scylla的metric信息
  - targets: ["10.19.11.23:9180"]
- job_name: node_exporter
  honor_labels: true
  static_configs:
  # 这部分主要是收集scylla的机器的信息，需要通过node_exporter 来开启这个上报;
  - targets: ['10.19.11.23:9100']

## two servers example: - targets: ["172.17.0.3:9103","172.17.0.2:9103"]
```

6. 用启动脚本启动

```
sudo sh start-all.sh -d data_dir
```

data路径最好要配置一个，不然两次启动会把数据删除

这个可能会碰到一个问题，

```
598cf3f9e16f0e5df2a1f0cf79df8be2ee0909f80307e392911d140b2ff6dac8
Wait for Prometheus container to start..4721e32c86ea09f1e16d270a194eb979cbf61f249c066435234e7d5cafee8631
Wait for Grafana container to start........curl: (7) Failed connect to localhost:3000; Connection refused
curl: (7) Failed connect to localhost:3000; Connection refused
curl: (7) Failed connect to localhost:3000; Connection refused
curl: (7) Failed connect to localhost:3000; Connection refused
```

Grafana 没有起来 或者起来，但是还没有监听端口，然后脚本最多只是重试7次，这个时候就会报错，其实后期grafana是成功起来的；

解决方式有两个：

	1. 先执行脚本，然后等到grafana起来，然后关掉prometheus,然后修改start-all.sh中的脚本，把docker run grafana那个部分去掉，然后在执行start-all.sh, 这样就ok了
	2. 将重试几次增大

7. 关闭监控

```
sudo sh kill-all.sh
```






