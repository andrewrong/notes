//
// Created by fenglin on 16/3/7.
//

#include <common/log/Log.h>
#include "Topic.h"
namespace common{
    Topic::Topic(const string &topic)
            :topicName_(topic) {
        topic_ = nullptr;
        handle_ = nullptr;
        LOGAGENT_LOG_INFO << "the topic name:" << topic << " is initial";
    }

    bool Topic::createTopic(shared_ptr<RdKafka::Handle>& handle) {
        if(handle){
            /**
             * 构建一个topic的过程
             * 1. 构建topic_config
             * 2. 用rd_kafka_t, name, conf来创建一个topic对象
             */
            handle_ = handle;
            fConf_ = shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
            for(auto& item : config_){
                string errstr;
                if(fConf_->set(item.first, item.second, errstr) != RdKafka::Conf::CONF_OK){
                    LOGAGENT_LOG_ERROR << "topic config(key:" << item.first << "," << item.second << ") is error,message:" << errstr;
                }
            }

            string errstr;
            topic_ = shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(handle_.get(), topicName_, fConf_.get(), errstr));

            if(!topic_){
                LOGAGENT_LOG_ERROR << "create topic:" << topicName_ << " is failure, message " << errstr;
                return false;
            }
            return true;
        }

        return false;
    }

    const string& Topic::getTopicName() const {
        static string _empty = "";
        if(topicName_.empty()){
            return _empty;
        }
        return topicName_;
    }

    bool Topic::isValidTopic() const {
        if(topicName_.empty() || handle_ == nullptr || topic_ == nullptr){
            return false;
        }
        return true;
    }

    void Topic::setTopicConfig(const string &key, const string &value) {
        config_[key] = value;
    }

    Topic::~Topic() {
        LOGAGENT_LOG_INFO << "the topic name is " << topicName_ << " is destory";
    }

    const shared_ptr<RdKafka::Topic>& Topic::getTopic() const {
        return topic_;
    }
}
