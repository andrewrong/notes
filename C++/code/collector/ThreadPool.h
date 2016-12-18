//
#pragma once

#include <string>
#include <map>
#include "MonitorBase.h"

using std::map;
using std::string;

class ThreadPoolMonitorModel{
public:
    void setFunc(func fun) {
        fun_ = fun;
    }

    map<string, double> getStatus(){
        return fun_();
    }
protected:
    func fun_;
};

class ThreadPool {
public:
    ThreadPool(ThreadPoolMonitorModel* monitor) {
        monitor->setFunc([this](){
            return getStatus();
        });
    }
    map<string, double> getStatus() {
        map<string, double> tmp{
                {"yumo", 1},
                {"bojue", 2}
        };
        return tmp;
    };
};

