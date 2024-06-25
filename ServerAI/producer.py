from confluent_kafka import Producer
import time
import cv2
from io import BytesIO
import numpy as np
from comunication_handler import comunicationHandler
from time import time, sleep
import json
import base64
from time import time

#ip = '10.8.2.3:9092'
ip = 'localhost:9092'

handle = comunicationHandler(ip, 'client_group')
handle.set_consumer_topic_subscribtion('to_client', True)

while True:
    image = cv2.imread(r'C:\Users\steam\OneDrive\Namizje\YOLOv8_version_0.2\output_fr\frame_309_2200.jpg')
    ret, buffer = cv2.imencode('.jpg', image)
    buffer = base64.b64encode(buffer).decode('utf-8')
    tim = time()
    buf = {
        "dev_id" : 'to_client',
        "image" : buffer,
        "time" : tim
    }

    buffer = json.dumps(buf)
    #print(buffer)
    handle.produce('test-pictures-flutter', buffer.encode('utf-8'))
    msg = handle.consume(0.1)
    tim = time()
    if msg is not None:
        print(f"{msg.value()} {tim}")