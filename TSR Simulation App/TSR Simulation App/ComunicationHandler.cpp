#include "ComunicationHandler.h"

bool RUN_COMUNICATION = true;

Producer::Producer(const std::string& brokers, const std::string& topic_name) {
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (!conf) throw std::runtime_error("Failed to create global configuration object");

    if (conf->set("metadata.broker.list", brokers, errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set broker list: " + errstr);
    }

    producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) throw std::runtime_error("Failed to create producer: " + errstr);

    topic = RdKafka::Topic::create(producer, topic_name, nullptr, errstr);
    if (!topic) throw std::runtime_error("Failed to create topic: " + errstr);
}

void Producer::produceMessage(const cv::Mat input_image, int quality) {
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, quality };
    std::vector<uchar> encoded_image;
    if (!cv::imencode(".jpg", input_image, encoded_image, compression_params)) {
        throw std::runtime_error("Failed to encode image.");
    }

    //cv::imwrite("./img.jpg", input_image, compression_params);

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
    localtime_s(&local_time, &now_c);

    std::ostringstream time_stream;
    time_stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    std::string formatted_time = time_stream.str();

    // Initialize the JSON object
    nlohmann::json json_message = {
        {"IP", "10.8.2.8"},
        {"Image", encoded_image},
        {"DateTime", formatted_time},
    };

    std::string message = json_message.dump();

    RdKafka::ErrorCode resp = producer->produce(
        topic,  // Topic
        RdKafka::Topic::PARTITION_UA,  // Partition
        RdKafka::Producer::RK_MSG_COPY,  // Message option
        const_cast<char*>(message.c_str()), message.size(),  // Payload
        nullptr, 0  // Key (optional)
    );

    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Error producing message: " << RdKafka::err2str(resp) << std::endl;
    }
}

void Producer::flush() {
    producer->flush(10000);
}

Producer::~Producer() {
    delete topic;
    delete producer;
    delete conf;
}

void startConsumer(const std::string& brokers, const std::string& topic, const std::string& group_id) {
    std::string errstr;

    // Create configuration objects
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    if (!conf || !tconf) {
        throw std::runtime_error("Failed to create configuration objects");
    }

    // Set broker list
    if (conf->set("metadata.broker.list", brokers, errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set broker list: " + errstr);
    }

    // Set group.id
    if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set group.id: " + errstr);
    }

    // Set additional configurations for consumer behavior (e.g., auto commit, session timeout)
    if (conf->set("enable.auto.commit", "true", errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set enable.auto.commit: " + errstr);
    }

    // Create consumer
    RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(conf, errstr);
    if (!consumer) {
        throw std::runtime_error("Failed to create consumer: " + errstr);
    }

    // Subscribe to the topic
    std::vector<std::string> topics = { topic };
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if (err != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error("Failed to subscribe to topic: " + RdKafka::err2str(err));
    }

    std::cout << "Consumer created and subscribed to topic: " << topic << std::endl;

    // Consume messages in a loop
    while (RUN_COMUNICATION) {
        RdKafka::Message* msg = consumer->consume(1000);  // Timeout in milliseconds
        if (msg->err() == RdKafka::ERR_NO_ERROR) {
            std::string payload(static_cast<const char*>(msg->payload()), msg->len());
            std::cout << "Received message: " << payload << std::endl;
        } else if (msg->err() == RdKafka::ERR__TIMED_OUT) {
            // Handle timeout error if needed
            std::cerr << "Consumer timed out, retrying..." << std::endl;
        } else {
            // Handle other errors
            std::cerr << "Consumer error: " << msg->errstr() << std::endl;
        }

        delete msg;
    }

    // Cleanup
    delete tconf;
    delete conf;
    delete consumer;
}

int start() {
    try {
        std::string brokers = "10.8.2.2:9092";  // Replace with your Kafka broker
        std::string topic = "10.8.2.8";  // Replace with your topic name
        std::string group_id = "client_group";   // Replace with your consumer group ID

        startConsumer(brokers, topic, group_id);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}

int start_prod() {
    try {
        std::string broker = "10.8.2.2:9092";
        std::string topic_name = "sim-apk-pictures";
        std::string input_image_path = "C:/Users/patri/Downloads/img (142).jpg";

        // Load and compress the image
        cv::Mat input_image = cv::imread(input_image_path, cv::IMREAD_COLOR);
        if (input_image.empty()) {
            throw std::runtime_error("Failed to load image from " + input_image_path);
        }

        Producer producer(broker, topic_name);
        producer.produceMessage(input_image);
        producer.flush();

        std::cout << "JSON message sent to topic: " << topic_name << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
