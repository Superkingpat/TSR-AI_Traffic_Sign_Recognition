from confluent_kafka import Producer, Consumer, error
from confluent_kafka.admin import AdminClient, NewTopic
import tensorflow as tf
import numpy as np
from PIL import Image
from io import BytesIO
import cv2

class comunicationHandler:
    def __init__(self, server_address, consumer_group_name):
        self.server_address = server_address
        self.consumer_group_name = consumer_group_name
        self.admin_client = AdminClient({'bootstrap.servers': self.server_address})

        self.consumer = Consumer({
            'bootstrap.servers': server_address,
            'group.id': consumer_group_name,
            'auto.offset.reset': 'earliest'
        })

        self.producer = Producer({'bootstrap.servers': server_address})


    def create_topic(self, topic_name, num_partitions, replication_factor):
        topic_conf = NewTopic(topic_name, num_partitions, replication_factor)
        futures = self.admin_client.create_topics([topic_conf])

        for future in futures:
            try:
                future.result()
                print(f"Topic {future.topic} created with replication factor {future.replication_factor}")
            except Exception as e:
                raise(f"Failed to create topic {future.topic}: {e}")
            
    def __delivery_report(err, msg):
        if err is not None:
            print('Message delivery failed: {}'.format(err))
        
    def set_consumer_topic_subscribtion(self, topic, auto_topic_creation=True, num_partitions=1, replication_factor=1):
        try:
            self.consumer.subscribe([topic])
        except error.KafkaException as e:
            if 'Unknown topic or partition' in str(e):
                if auto_topic_creation:
                    self.create_topic(topic, num_partitions, replication_factor)
                else:
                    raise(f"Topic {topic} does not exist. auto_topic_creation set to false.")
            else:
                raise(f"An error occures. Error: {str(e)}")
    
    def produce(self, topic, massage):
        try:
            self.producer.produce(topic, value=massage, callback=self.__delivery_report)
            #self.producer.flush()
        except error.KafkaError as e:
            print(f"Kafka error occured: {str(e)}")
    
    def consume(self, timeout_len=1.0):
        consumer_msg = None
        try:
            consumer_msg = self.consumer.poll(timeout_len)
            if consumer_msg is None:
                print(f"Timeout reached. Timeout was set to {timeout_len} seconds.")
            else:
                return consumer_msg
        except error.KafkaError as e:
            raise(f"Kafka error occured: {str(e)}")
    
    def end_handler(self):
        self.consume.close()