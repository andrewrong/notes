# 枚举的个数

```
    enum State {
        UNLOCKED = 0,
        LOCK_PREP,  // Only for legacy 3 config servers.
        LOCKED,
        numStates
    };
```

今天在看mongodb的源码的时候，发现了它们在枚举的最后一个是`numStates`,非常取巧的一点；因为这样可以知道这个enum的对象个数，并且可以通过这个来判断其他的枚举对象是否是正常的; 可以通过下面的这种:

```
if (lockState < 0 || lockState >= State::numStates) {
        return {ErrorCodes::BadValue, str::stream() << "Invalid lock state: " << getState()};
    }
```

