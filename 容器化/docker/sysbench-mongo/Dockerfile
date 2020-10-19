# 操作系统的基础镜像，公司内部用的是ubuntu18.04
FROM 
RUN apt-get update && apt-get --yes install git g++ curl cmake automake build-essential libtool libmysqlclient-dev libsasl2-dev
RUN mkdir /home/sysbench
WORKDIR /home/sysbench

RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.3.5/mongo-c-driver-1.3.5.tar.gz 
RUN tar zxf mongo-c-driver-1.3.5.tar.gz
RUN cd mongo-c-driver-1.3.5
WORKDIR /home/sysbench/mongo-c-driver-1.3.5
RUN ./configure && make && make install

# RUN echo $(ls /bin | grep sleep)
# RUN cp /usr/local/include/libbson-1.0/bson/* /usr/local/include/libbson-1.0/
# RUN echo $(ls /usr/local/include/libbson-1.0)

WORKDIR /home/sysbench/
RUN git clone -b dev-mongodb-support-1.0 https://github.com/Percona-Lab/sysbench.git ./sysbench
WORKDIR /home/sysbench/sysbench
RUN ./autogen.sh
RUN ./configure
RUN make
RUN make install

RUN echo "export  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib" >> /root/.bashrc
CMD /bin/sleep 1000000000000
