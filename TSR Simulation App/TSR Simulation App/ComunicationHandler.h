#include "Water.h"

class Producer {
public:
    Producer(const std::string& brokers, const std::string& topic_name);

    void produceMessage(const cv::Mat input_image, int quality = 50);

    void flush();

    ~Producer();

private:
    RdKafka::Conf* conf;
    RdKafka::Producer* producer;
    RdKafka::Topic* topic;
    std::string errstr;
};


static void startConsumer(const std::string& brokers, const std::string& topic, const std::string& group_id);
int start();
int start_prod();