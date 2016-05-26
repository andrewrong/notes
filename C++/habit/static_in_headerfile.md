### 当static变量初始化出现在头文件中的时候

```
    class Processor {
    public:
        Processor(const FileTaskPtr& config){
            config_ = config;
        }

        virtual void process() = 0;
        virtual ~Processor(){};

    protected:
        FileTaskPtr config_;
        static uint32_t limitLineCnt_;
    };
    
    uint32_t Processor::limitLineCnt = MetaConfig::getLimitLineCnt();
```

#### 问题

像上面这样定义的话，当多处包含这个头文件的时候会导致limitLineCnt重复定义;并且作为静态变量还依赖于其他的静态变量的初始化的；这是很危险的...因为C++中关于静态变量的初始化是强制的先后顺序的;

* 重复定义
* 不明确的初始化的关系

当然我同事问我，为什么不直接调用MetaConfig::getLimitLineCnt()来替换，非要用一个static来保存呢?我想到的原因可能是每一次都获取会比较耗时;

不过我leader给了我一个很好的解决方法，我一下子也惊呆了，用函数内部的静态变量来保证唯一性并且保证初始化过程的有序性，并且不会出现多处定义的问题;

```
    class Processor {
    public:
        Processor(const FileTaskPtr& config){
            config_ = config;
        }

        virtual void process() = 0;
        virtual ~Processor(){};

    protected:
        static uint32_t getLimitLineCnt() {
            static uint32_t limitCnt_ = MetaConfig::getLimitLineCnt();
            return limitCnt_;
        }
    protected:
        FileTaskPtr config_;
    };
```

#### 还存在的问题

* 我已经使用`#ifndef`这种判断了，不会应该只会包含一个头文件吗？
