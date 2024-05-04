from confluent_kafka import Producer
import time
import cv2
import numpy as np

#ip = '10.8.2.3:9092'
ip = 'localhost:9092'

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

    p.produce('mytopic', value=buffer.tobytes(), callback=delivery_report)
    time.sleep(15)

p.flush()