from confluent_kafka import Producer, Consumer, KafkaException
from confluent_kafka.admin import AdminClient, NewTopic
import tensorflow as tf
import numpy as np
from PIL import Image
from io import BytesIO
import cv2

class comunicationHandler:
    def __init__(self, server_address, group_name):
        self.server_address = server_address
        self.group_name = group_name
        self.admin_client = AdminClient({'bootstrap.servers': self.server_address})
        self.consumer = Consumer({
            'bootstrap.servers': server_address,
            'group.id': group_name,
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
        
    def set_consumer_topic_subscribtion(self, topic, auto_topic_creation=True, num_partitions=1, replication_factor=1):
        try:
            self.consumer.subscribe([topic])
        except KafkaException as e:
            if 'Unknown topic or partition' in str(e):
                if auto_topic_creation:
                    self.create_topic(topic, num_partitions, replication_factor)
                else:
                    raise(f"Topic {topic} does not exist. auto_topic_creation set to false.")
            else:
                raise(f"An error occures. Error: {str(e)}")
    
