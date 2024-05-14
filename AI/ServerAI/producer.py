from confluent_kafka import Producer
import time
import cv2
from io import BytesIO
import numpy as np
from comunication_handler import comunicationHandler

#ip = '10.8.2.3:9092'
ip = 'localhost:9092'

handle = comunicationHandler(ip, 'client_group')
handle.set_consumer_topic_subscribtion('to_client')

while True:
    image = cv2.imread(r'C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round\30\0000056.jpg')
    ret, buffer = cv2.imencode('.jpg', image)

    handle.produce('from_client', buffer.tobytes())
    msg = handle.consume(1.0)
    if msg is not None:
        print(BytesIO(msg.value()))
    time.sleep(25)


'''
p = Producer({'bootstrap.servers': ip})

def delivery_report(err, msg):
    if err is not None:
        print('Message delivery failed: {}'.format(err))
    else:
        print('Message delivered')

while True:
    p.poll(0)
    image = cv2.imread(r'C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round\30\0000056.jpg')
    ret, buffer = cv2.imencode('.jpg', image)

    p.produce('img_stream', value=buffer.tobytes(), callback=delivery_report)
    time.sleep(25)

p.flush()
'''