//
// Created by fenglin on 16/3/7.
//

#include <common/log/Log.h>
#include "KafkaBase.h"

namespace common{


    KafkaBase::KafkaBase() {
        handle_ = nullptr;
    }

    void KafkaBase::addTopic(const shared_ptr<Topic> &topic){
        if(handle_){
            topic->createTopic(handle_);
            topics_[topic->getTopicName()] = topic;
            return;
        }

        LOGAGENT_LOG_ERROR << "the kafka is incomplete";
    }

    bool KafkaBase::createKafka(rd_kafka_type_t type, RdKafka::EventCb* eventCb, RdKafka::DeliveryReportCb* deliveryReportCb) {
        if(!handle_){
            fConf_ = shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
            for(auto& item : configMap_){
                string errstr;
                if(fConf_->set(item.first, item.second, errstr) != RdKafka::Conf::CONF_OK){
                    LOGAGENT_LOG_ERROR << "set producer config (" << item.first << "," << item.second << ") is failure, message " << errstr;
                }
            }

            {
                string errstr;
                if(eventCb){
                    if(fConf_->set("event_cb", eventCb, errstr) != RdKafka::Conf::CONF_OK){
                        LOGAGENT_LOG_ERROR << "set producer config event_cb is failure, message" << errstr;
                        return false;
                    }
                }

                if(deliveryReportCb){
                    if(fConf_->set("dr_cb", deliveryReportCb, errstr) != RdKafka::Conf::CONF_OK){
                        LOGAGENT_LOG_ERROR << "set producer config dr_cb is failure, message" << errstr;
                        return false;
                    }
                }
            }

            switch (type){
                case RD_KAFKA_PRODUCER:{
                    string errstr;
                    handle_ = shared_ptr<RdKafka::Producer>(RdKafka::Producer::create(fConf_.get(), errstr));
                    if(!handle_){
                        LOGAGENT_LOG_ERROR << "create producer is failure, the message is " << errstr;
                        return false;
                    }else{
                        LOGAGENT_LOG_INFO << "producer name is " << handle_->name();
                        return true;
                    }
                }
                case RD_KAFKA_CONSUMER:
                    return false;
                default:
                    LOGAGENT_LOG_ERROR << "this type " << type << " is not valid";
                    return false;
            }
        }
        return true;
    }

    void KafkaBase::delTopic(const string &topicName) {
        if(topicName.empty()){
            return;
        }
        configMap_.erase(topicName);
    }

    void KafkaBase::setConfig(const string &key, const string &value) {
        if(key.empty() || value.empty()){
            return;
        }
        configMap_[key] = value;
    }

    KafkaBase::~KafkaBase() {
        handle_ = nullptr;
        configMap_.clear();
        topics_.clear();
        RdKafka::wait_destroyed(5000);
    }

    const shared_ptr<Topic> &KafkaBase::getTopic(const string &topicName) {
        static shared_ptr<Topic> _empty = nullptr;
        if(topicName.empty()){
            return _empty;
        }

        auto index = topics_.find(topicName);
        if(index == topics_.end()){
            return _empty;
        }
        return index->second;
    }
}
