#pragma once

#include <map>
#include <string>
#include "MonitorBase.h"

using std::map;
using std::string;

class TimeMonitorModel{
public:
    void setFunc(func fun) {
        fun_ = fun;
    }
    map<string,double> getStatus(){
        return fun_();
    }
protected:
    func fun_;
};

class TimeDb{
public:
    TimeDb(TimeMonitorModel* model) {
        model->setFunc([this](){
            return getStatus();
        });
    }
    map<string, double> getStatus(){
        map<string, double> tmp{
                {"fenglin", 1},
                {"sanke", 2}
        };
        return tmp;
    };
};

