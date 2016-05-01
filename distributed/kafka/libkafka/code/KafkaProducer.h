//
// Created by fenglin on 16/3/3.
//

#ifndef LOGAGENT_KAFKAPRODUCER_H
#define LOGAGENT_KAFKAPRODUCER_H
#include "Base/KafkaBase.h"
#include <string>

using std::string;

namespace common {
    class KafkaProducer : public KafkaBase{
    public:
        KafkaProducer();
        ~KafkaProducer();

        void createProducer();
        void setConfig(const string& key, const string& value);

        void setExtraConfig(const string& key, const string& value);

        /**
         * 发送一个消息到队列（包含key值）
         * @param pKeySrc
         * @param iKeyLen
         * @param pSrc
         * @param iLen
         * @return
         */
        bool send(const string& topicName, const string& key, const char* content, const uint64_t& len);

        /**
         * 外界用来轮训事件
         */
        void poll(int32_t flag);
    private:
        string acks_;
    };
}


#endif //LOGAGENT_KAFKAPRODUCER_H
