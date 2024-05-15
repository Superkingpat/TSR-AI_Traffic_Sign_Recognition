from confluent_kafka import Producer
import time
import cv2
from io import BytesIO
import numpy as np
from comunication_handler import comunicationHandler
from time import time

#ip = '10.8.2.3:9092'
ip = 'localhost:9092'

handle = comunicationHandler(ip, 'client_group')
handle.set_consumer_topic_subscribtion('to_client', True)

while True:
    image = cv2.imread(r'C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round\30\0000056.jpg')
    ret, buffer = cv2.imencode('.jpg', image)

    handle.produce('from_client', buffer.tobytes())
    msg = handle.consume(0.1)
    tim = time()
    if msg is not None:
        print(f"{msg.value()} {tim}")
    #time.sleep(5)