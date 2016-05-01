//
// Created by fenglin on 16/3/7.
//

#ifndef LOGAGENT_TOPIC_H
#define LOGAGENT_TOPIC_H

#include <memory>
#include <unordered_map>
#include <string>
#include <rdkafka.h>
#include <rdkafkacpp.h>

using std::shared_ptr;
using std::unordered_map;
using std::string;

namespace common{

    class Topic {
    public:
        Topic(const string& topic);

        const string& getTopicName() const;
        bool createTopic(shared_ptr<RdKafka::Handle>& handle);
        const shared_ptr<RdKafka::Topic>& getTopic() const;
        bool isValidTopic() const;
        void setTopicConfig(const string& key, const string& value);
        ~Topic();
    private:
        shared_ptr<RdKafka::Topic> topic_;
        unordered_map<string, string> config_;
        shared_ptr<RdKafka::Handle> handle_;
        shared_ptr<RdKafka::Conf> fConf_;
        string topicName_;
    };
}



#endif //LOGAGENT_TOPIC_H
