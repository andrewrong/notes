#pragma once

#include <boost/mpl/vector.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/int.hpp>
#include <iostream>

#include "ThreadPool.h"
#include "TimeDb.h"

typedef boost::mpl::vector<ThreadPoolMonitorModel, TimeMonitorModel> __STATE_MEMBERS;

template <class T, size_t N = boost::mpl::size<T>::value>
class _MONITOR_IMPL : public boost::mpl::at<T, boost::mpl::int_<N-1>>::type, public _MONITOR_IMPL<T, N - 1> {
    typedef typename boost::mpl::at<T, boost::mpl::int_<N-1>>::type _REAL_TYPE;
    typedef typename _MONITOR_IMPL<T, N-1> _IMPL_TYPE;
public:
    map<string, double> realGetStatus(){
        auto res1 = _REAL_TYPE::getStatus();
        auto res2 = _MONITOR_IMPL<T, N - 1>::realGetStatus();

        res1.insert(res2.cbegin(), res2.cend());

        return res1;
    }
};
template <class T>
class _MONITOR_IMPL<T, 0>{
public:
    map<string, double> realGetStatus(){
        return map<string, double>();
    }
};

class monitor: public _MONITOR_IMPL<__STATE_MEMBERS> {
public:
    void status(){
        auto rs = realGetStatus();

        for(auto& item : rs){
            std::cout << item.first << ":" << item.second;
        }
    }
};