from confluent_kafka import Producer, Consumer
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


    def create_topic():
        pass