### stackoverflow_in_thread

```
    void DiskProcessor::process() {
        /**
         * 1. 获得文件偏移量
         * 2. 都一行日志,
         * 3. 发送到kafka中
         */
        auto &fileHandle = config_->getFileHandle();
        if (!fileHandle) {
            return;
        }
        uint64_t offset = config_->getOffset();

        uint8_t buffer[CACHESIZE];
        uint8_t line[LINECHARCNT];

        memset(buffer, 0, CACHESIZE);
        memset(line, 0, LINECHARCNT);

        uint32_t count = 0;
        uint32_t cursor = 0;

        while (count <= getLimitLineCnt()) {
            int byteSize = fileHandle->pread(buffer, CACHESIZE, offset);
            if (byteSize <= 0) {
                LOG_INFO << config_->getKey() << " have been read is over";
                break;
            }

            for (int i = 0; i < byteSize; i++) {
                if (buffer[i] != '\n') {
                    line[cursor++] = buffer[i];
                    //上限单行日志为2M
                    if (cursor == (LINECHARCNT - 1)) {
                        Sender::getInstance()->send(config_->getTopic(), string((char *) line), false);
                        offset += cursor;
                        cursor = 0;
                        count++;
                        memset(line, 0, LINECHARCNT);
                        Util::writeOffSetFile(config_->getOffsetFileHandle(), offset);
                    }
                } else {
                    if (!cursor) {
                        continue;
                    }

                    Sender::getInstance()->send(config_->getTopic(), string((char *) line), false);
                    offset += cursor + 1;
                    cursor = 0;
                    count++;
                    memset(line, 0, LINECHARCNT);
                    Util::writeOffSetFile(config_->getOffsetFileHandle(), offset);
                }
            }
        }
        Dispatcher::getInstance()->setDiskRunStatus(config_->getKey(), NOWORK);
    }
```

### 现象

我的线程池运行这个process函数的时候，直接就在process开头就挂了，完全看不出问题所在的；在三刻的帮忙下，后来意识到是函数内部申请了6M的大空间，导致stackoverflow;

三刻的处理办法是用排除法:

* 排除是线程池的问题
* 排除是函数本身的问题

测试发现是函数本身的问题;排除法真的很重要，有的时候在毫无头绪的情况下比较快定位问题;

### 有问题的代码

```
        uint8_t buffer[CACHESIZE];
        uint8_t line[LINECHARCNT];
```

解决的方式就是

* 确定是否有必要申请那么多空间的stack空间
* 如果有必要就用new的方式，让内存放到堆上去; 必须要注意内存泄露问题;

### 感想

这种事情以后不会发现在自己身上，但是当真的在写代码的时候依然还是发生了; 这应该是一种意识问题；平时没有意识到这些问题，导致自己忽略了...

### 存在的疑问

我使用`ulimt -s` 看了我的系统限制是8M，按理来说应该还没有到达上限，不过程序还是挂了....
