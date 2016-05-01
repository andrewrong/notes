//
// Created by fenglin on 16/3/3.
//

#include <sys/errno.h>
#include <folly/Singleton.h>
#include "KafkaProducer.h"
#include <common/log/Log.h>

common::KafkaProducer::KafkaProducer() {

}

common::KafkaProducer::~KafkaProducer() {

}

void common::KafkaProducer::createProducer() {
    createKafka(RD_KAFKA_PRODUCER);
}

void common::KafkaProducer::setConfig(const string &key, const string &value) {
    KafkaBase::setConfig(key, value);
}

bool common::KafkaProducer::send(const string &topicName, const string &key, const char* content, const uint64_t& len) {
    if(topicName.empty() || key.empty() || content == nullptr || len == 0){
        return true;
    }
    auto topic = getTopic(topicName);
    if(topic == nullptr){
        topic = std::make_shared<Topic>(topicName);
        topic->setTopicConfig("request.required.acks", acks_);
        addTopic(topic);
    }

    RdKafka::ErrorCode  resp = dynamic_cast<RdKafka::Producer*>(handle_.get())->produce(topic->getTopic().get()
            , RdKafka::Topic::PARTITION_UA
            , RdKafka::Producer::RK_MSG_COPY
            , (void*)content
            , len
            , &key
            , nullptr
    );

    if(resp != RdKafka::ERR_NO_ERROR){
//        LOGAGENT_LOG_ERROR << "producer failed:" << RdKafka::err2str(resp);
        return false;
    }
    return true;
}

void common::KafkaProducer::poll(int32_t flag) {
    if(handle_){
        handle_->poll(flag);
    }
}

void common::KafkaProducer::setExtraConfig(const string &key, const string &value) {
    if(key == "request.required.acks"){
        acks_ = value;
    }
}
