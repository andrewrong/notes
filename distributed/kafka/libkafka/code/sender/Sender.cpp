//
// Created by fenglin on 16/3/16.
//

#include "Sender.h"
#include <folly/Singleton.h>
#include <common/log/Log.h>
#include <common/util/Util.h>
#include <biz/config/KafkaConfig.h>
#include <biz/monitor/Monitor.h>
#include <biz/util/Util.h>

namespace LogAgent{
    folly::Singleton<Sender> gSender;

    SenderPtr Sender::getInstance() {
        return gSender.try_get();
    }

    Sender::Sender() {
        //配置kafka producer;
        producer_ = std::make_shared<KafkaProducer>();
        localIP_ = Util::getLocalIP();
        run_ = true;
        pollRun_ = true;
        msgCnt_ = 0;
        msgSize_ = 0;
        msgSizePerMin_ = 0;

        eventCb_ = std::make_shared<EventCB>(run_);
        deliveryCb_ = std::make_shared<DeliveryCB>(msgCnt_, msgSize_, run_);
    }

    void Sender::init() {
        LOGAGENT_LOG_INFO << "Sender is init start";

        producer_->setConfig("batch.num.messages", std::to_string(KafkaConfig::getBatchNumMessages()));
        producer_->setConfig("queue.buffering.max.ms", std::to_string(KafkaConfig::getQueueBufferMaxMs()));
        producer_->setConfig("queue.buffering.max.messages", std::to_string(KafkaConfig::getQueryBufferMaxMessages()));
        producer_->setConfig("metadata.broker.list", KafkaConfig::getMetadataBrokerList());
        producer_->setConfig("message.max.bytes", std::to_string(KafkaConfig::getMaxMessageSize()));
        producer_->setExtraConfig("request.required.acks", KafkaConfig::getRequestRequiredAcks());
        producer_->createKafka(RD_KAFKA_PRODUCER, eventCb_.get(), deliveryCb_.get());

        cbThread_.reset(new std::thread([this](){
            while(pollRun_){
                producer_->poll(10);
            }
        }));

        Monitor::getInstance()->appendTimedTask([this](const std::shared_ptr<AgentReport>& agent){
            static string undealMetric = "kafka.undeal.data.package.cnt";
            static string undealMsgSize = "kafka.undeal.data.package.size";
            static map<string,string> tags = {{"sentryIP", Util::getLocalIP()}};
            uint32_t now = time(nullptr);

            long cnt = msgCnt_;
            SentryDpsUtil::DpValue value = {now, cnt * 1.0};
            agent->Report(undealMetric, tags, &value, 1);
            long msgSize = msgSize_;
            SentryDpsUtil::DpValue sizeValue = {now, msgSize * 1.0};
            agent->Report(undealMsgSize, tags, &sizeValue, 1);

            msgSizePerMin_ = 0;
        });
        LOGAGENT_LOG_INFO << "Sender is init end";
    }

    bool Sender::send(const string &topic, const char* msg, const uint64_t& msgLen, bool isOrder, bool isCompression) {
        if(topic.empty() || msg == nullptr || msgLen == 0){
            return true;
        }

        Monitor::getInstance()->put(topic, 1, msgLen);

        if(isOrder){
            return innerSend(localIP_, topic, msg, msgLen);
        }else{
            return innerSend(localIP_ + ":" + std::to_string(time(nullptr) * 1000), topic, msg, msgLen);
        }
    }

    bool Sender::send(const string &partitionKey, const string& topic, const char* msg, const uint64_t& msgLen) {
        if(partitionKey.empty() || topic.empty() || msg == nullptr || msgLen == 0){
            return true;
        }
        return innerSend(partitionKey, topic, msg, msgLen);
    }

    bool Sender::innerSend(const string &partitionKey, const string &topic, const char* msg, const uint64_t& len) {
        static int64_t highWaterMark = KafkaConfig::getQueryBufferMaxMessages() * 0.8;
        static int64_t maxNetworkSpeed = KafkaConfig::getMaxNetworkSpeed();
        static int64_t maxMessageSizePerMin = KafkaConfig::getMaxMsgSizePerMin() * 0.6 * 2;

        if(run_){
            while(msgCnt_ > highWaterMark || msgSize_ > maxNetworkSpeed || msgSizePerMin_ > maxMessageSizePerMin ){
                Monitor::getInstance()->putWithHighWaterMark(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            msgCnt_++;
            msgSize_ += len;
            msgSizePerMin_ += len;
            return producer_->send(topic, partitionKey, msg, len);
        }else{
            //表示所有的broker都down掉了
            while(!run_){
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        return false;
    }

    void Sender::stopAndWait() {
        while(msgCnt_){
            //等待数据全部发送完毕才结束kafka sender
            std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
        }

        run_ = false;
        pollRun_ = false;
        if(cbThread_){
            if(cbThread_->joinable()){
                cbThread_->join();
                cbThread_ = nullptr;
            }
        }
        LOGAGENT_LOG_INFO << "the Sender is stop";
    }


}


