/*
#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <sstream>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <fstream>

// Function to compress an image
std::vector<uchar> compressImage(const cv::Mat& image, const std::string& format = "jpg", int quality = 90) {
    std::vector<uchar> buffer;
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, quality };
    cv::imencode("." + format, image, buffer, compression_params);
    return buffer;
}

// Function to send an image to Kafka topic
void sendImageToKafka(RdKafka::Producer* producer, const std::string& topic, const cv::Mat& image) {
    std::vector<uchar> compressedImage = compressImage(image);
    std::string payload(compressedImage.begin(), compressedImage.end());

    RdKafka::ErrorCode resp = producer->produce(
        topic, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(payload.data()), payload.size(),
        nullptr, nullptr);

    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to produce message: " << RdKafka::err2str(resp) << std::endl;
    }
    else {
        std::cout << "Message produced to topic " << topic << std::endl;
    }
}

// Function to decode and process a Kafka message
void decodeKafkaMessage(const std::string& payload) {
    try {
        json packet = json::parse(payload);
        auto results = packet["Result"].get<std::vector<std::string>>();
        auto dateTime = packet["DateTime"].get<std::string>();

        std::cout << "Received packet:\n";
        std::cout << "DateTime: " << dateTime << std::endl;
        std::cout << "Results:\n";
        for (const auto& result : results) {
            std::cout << "  - " << result << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse message: " << e.what() << std::endl;
    }
}

// Function to consume messages from Kafka topic
void consumeMessagesFromKafka(RdKafka::KafkaConsumer* consumer) {
    while (true) {
        std::unique_ptr<RdKafka::Message> msg(consumer->consume(1000));

        if (msg->err() == RdKafka::ERR__TIMED_OUT) {
            continue;
        }
        else if (msg->err()) {
            std::cerr << "Consumer error: " << msg->errstr() << std::endl;
            break;
        }

        std::cout << "Message received from topic " << msg->topic_name() << std::endl;
        decodeKafkaMessage(std::string(static_cast<const char*>(msg->payload()), msg->len()));
    }
}

int main() {
    // Kafka configuration
    std::string brokers = "localhost:9092";
    std::string produceTopic = "image_topic";
    std::string consumeTopic = "data_topic";

    // Create Kafka producer configuration
    RdKafka::Conf* producerConfig = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::unique_ptr<RdKafka::Producer> producer(RdKafka::Producer::create(producerConfig, nullptr));

    if (!producer) {
        std::cerr << "Failed to create producer" << std::endl;
        return -1;
    }

    // Create Kafka consumer configuration
    RdKafka::Conf* consumerConfig = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::unique_ptr<RdKafka::KafkaConsumer> consumer(RdKafka::KafkaConsumer::create(consumerConfig, nullptr));

    if (!consumer) {
        std::cerr << "Failed to create consumer" << std::endl;
        return -1;
    }

    // Subscribe to consumeTopic
    std::vector<std::string> topics = { consumeTopic };
    consumer->subscribe(topics);

    // Example: Load and send an image
    cv::Mat image = cv::imread("example.jpg");
    if (!image.empty()) {
        sendImageToKafka(producer.get(), produceTopic, image);
    }
    else {
        std::cerr << "Failed to read image" << std::endl;
    }

    // Consume messages
    consumeMessagesFromKafka(consumer.get());

    return 0;
}
*/