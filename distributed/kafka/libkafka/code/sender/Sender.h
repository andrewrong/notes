//
// Created by fenglin on 16/3/16.
//

#ifndef LOGAGENT_SENDER_H
#define LOGAGENT_SENDER_H

#include <memory>
#include <string>
#include <common/kafka/KafkaProducer.h>
#include <common/log/Log.h>
#include <atomic>
#include <thread>

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using common::KafkaProducer;
using std::atomic_long;
using std::atomic_bool;

namespace LogAgent{
    class Sender;
    typedef shared_ptr<Sender> SenderPtr;

    class EventCB : public RdKafka::EventCb {
    public:
        EventCB(atomic_bool &run) : run_(run) { }
        void event_cb(RdKafka::Event &event) {
            switch (event.type()) {
                case RdKafka::Event::EVENT_LOG:
//                    LOGAGENT_LOG_INFO << event.str();
                    break;
                case RdKafka::Event::EVENT_ERROR:
//                    LOGAGENT_LOG_INFO << "Error (" << RdKafka::err2str(event.err()) << "):" << event.str();
                    if(event.err() == RdKafka::ERR__ALL_BROKERS_DOWN){
                        LOGAGENT_LOG_ERROR << " all brokers is down, so LogAgent is close";
                        run_ = false;
                    }
                default:
//                    LOGAGENT_LOG_INFO << "Event " << event.type() << "(" << RdKafka::err2str(event.err()) << "):" << event.str();
                    break;
            }
        }
    private:
        atomic_bool& run_;
    };

    class DeliveryCB : public RdKafka::DeliveryReportCb {
    public:
        DeliveryCB(atomic_long &msgCnt, atomic_long &msgSize, atomic_bool& run)
                : msgCnt_(msgCnt),
                  msgSize_(msgSize),
                  run_(run){ }
        virtual void dr_cb(RdKafka::Message &message) {
            msgCnt_--;
            msgSize_ -= message.len();
            // 能发送成功说明恢复通行
            run_ = true;
        }
    private:
        atomic_long& msgCnt_;
        atomic_long& msgSize_;
        atomic_bool& run_;
    };

    class Sender {
    public:
        Sender();
        static SenderPtr getInstance();
        void init();
        void stopAndWait();
        bool send(const string& topic, const char* msg, const uint64_t& msgLen, bool isOrder = false, bool isCompression = false);
        bool send(const string& partitionKey, const string& topic, const char* msg, const uint64_t& msgLen);
    private:
        bool innerSend(const string& partitionKey, const string& topic, const char* msg, const uint64_t& len);
    private:
        shared_ptr<KafkaProducer> producer_;
        shared_ptr<EventCB> eventCb_;
        shared_ptr<DeliveryCB> deliveryCb_;
        unique_ptr<std::thread> cbThread_;

        atomic_long msgCnt_;
        atomic_long msgSize_;
        atomic_long msgSizePerMin_;
        atomic_bool run_;
        atomic_bool pollRun_;
        string localIP_;
    };
}
#endif //LOGAGENT_SENDER_H
