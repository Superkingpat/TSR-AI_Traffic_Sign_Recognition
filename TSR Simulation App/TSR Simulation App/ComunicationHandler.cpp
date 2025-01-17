#include <librdkafka/rdkafkacpp.h>
#include <json/json.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

class Producer {
public:
    Producer(const std::string& brokers, const std::string& topic_name) {
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf->set("metadata.broker.list", brokers, errstr);
        conf->set("acks", "all", errstr);
        producer = RdKafka::Producer::create(conf, errstr);
        topic = RdKafka::Topic::create(producer, topic_name, NULL, errstr);
    }

    void produceMessage(const nlohmann::json& json_message) {
        // Serialize the JSON object to a string
        std::string message = json_message.dump();

        // Produce the message
        RdKafka::ErrorCode resp = producer->produce(
            topic,  // Topic
            RdKafka::Topic::PARTITION_UA,  // Partition: Use the default partition
            RdKafka::Producer::RK_MSG_COPY, // Message option
            const_cast<char*>(message.c_str()), message.size(), // Payload and size
            nullptr, 0 // Optional: key and key size (null in this case)
        );

        if (resp != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Error producing message: " << RdKafka::err2str(resp) << std::endl;
        }
    }

    void flush() {
        // Wait for all messages to be delivered
        producer->flush(10000);
    }

    ~Producer() {
        delete topic;
        delete producer;
        delete conf;
    }

private:
    RdKafka::Conf* conf;
    RdKafka::Producer* producer;
    RdKafka::Topic* topic;
    std::string errstr;
};


int main() {
    std::string broker = "10.8.2.2:9092"; // Kafka broker address
    std::string topic_name = "sim-apk-pictures"; // Topic name

    std::string input_image_path = "C:/Users/patri/Downloads/img (142).jpg";
    cv::Mat input_image = cv::imread(input_image_path, cv::IMREAD_COLOR);

    // Create producer
    Producer producer(broker, topic_name);

    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(90);

    std::vector<uchar> encoded_image;
    cv::imencode("12.jpg", input_image, encoded_image, compression_params);

    std::string output_image_path = "compressed_image.jpg";
    bool success = cv::imwrite(output_image_path, encoded_image);

    // Create a JSON object to send
    nlohmann::json json_obj = {
        {"name", "John Doe"},
        {"age", 30},
        {"Image", "john.doe@example.com"},
        {"is_active", true}
    };

    // Send the JSON object as a message
    producer.produceMessage(json_obj);

    // Ensure the message is delivered
    producer.flush();

    std::cout << "JSON message sent to topic: " << topic_name << std::endl;

    return 0;
}