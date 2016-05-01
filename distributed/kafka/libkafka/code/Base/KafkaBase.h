//
// Created by fenglin on 16/3/7.
//

#ifndef LOGAGENT_KAFKABASE_H
#define LOGAGENT_KAFKABASE_H

#include <unordered_map>
#include "Topic.h"

using std::unordered_map;
using std::string;
using std::shared_ptr;

namespace common {
    class KafkaBase {
    public:
        KafkaBase();

        /**
         * 真正创建一个kafka类
         */
        bool createKafka(rd_kafka_type_t type, RdKafka::EventCb* eventCb = nullptr, RdKafka::DeliveryReportCb* deliveryReportCb = nullptr);

        /**
         * get topic
         */
        const std::shared_ptr<Topic>& getTopic(const string& topicName);

        /**
         * add topic
         */
        void addTopic(const shared_ptr<Topic>& topic);

        /**
         * del topic
         */
        void delTopic(const string& topicName);

        /**
         * set config
         */
        void setConfig(const string& key, const string& value);

        virtual ~KafkaBase();
    protected:
        shared_ptr<RdKafka::Handle> handle_;
        shared_ptr<RdKafka::Conf> fConf_;
        unordered_map<string, shared_ptr<Topic>> topics_;
        unordered_map<string, string> configMap_;
    };
}


#endif //LOGAGENT_KAFKABASE_H
