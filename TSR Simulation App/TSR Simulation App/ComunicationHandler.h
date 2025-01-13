#pragma once
#include <librdkafka/rdkafkacpp.h>
#include "opencv2/opencv.hpp"
#include <json/json.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <iomanip>

using json = nlohmann::json;

// Compress an image into a specific format with optional quality
std::vector<uchar> compressImage(const cv::Mat& image, const std::string& format, int quality);

// Send an image to a Kafka topic
void sendImageToKafka(RdKafka::Producer* producer, const std::string& topic, const cv::Mat& image);

// Decode and process a Kafka message
void decodeKafkaMessage(const std::string& payload);

// Consume messages from a Kafka topic
void consumeMessagesFromKafka(RdKafka::KafkaConsumer* consumer);