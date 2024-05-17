from confluent_kafka import Consumer
import tensorflow as tf
import numpy as np
from PIL import Image
from io import BytesIO
import cv2
from comunication_handler import comunicationHandler
from time import time
import json
from ultralytics import YOLO
import base64

def preprocess_image(image_bytes):
    image = Image.open(BytesIO(image_bytes))
    image_array = np.array(image)
    image_array = cv2.resize(image_array, (224, 224))
    image_array = image_array / 255.0
    image_array = np.expand_dims(image_array, axis=0)
    return image_array

model = tf.keras.models.load_model('models/speed_classification.h5')
model_yolo = YOLO('models/best.pt')

class_ind = {'100': 0, '130': 1, '30': 2, '40': 3, '40-': 4, '50': 5, '50-': 6, '60': 7, '60-': 8, '70': 9, '70-': 10, '80': 11, '90': 12}
class_ind = {v: k for k, v in class_ind.items()}

ip = 'localhost:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('test-pictures-flutter', False)

msg = handle.consume(1.0)
'''if msg is not None:
    print(msg.value().decode('utf-8'))'''

while True:
    msg = handle.consume(1.0)
    tim = time()

    if msg is not None:
        stream = json.loads(msg.value().decode('utf-8'))
        print(stream)
        packet = {
            "sign_id" : [],
            "time" : tim
        }

        image = base64.b64decode(stream["image"])
        result = model_yolo(image).xyxy[0].numpy()

        for i, bbox in enumerate(result):
            x1, y1, x2, y2, conf, cls = map(int, bbox)
            cropped_image = image_array[y1:y2, x1:x2]

            image_array = preprocess_image(cropped_image)
            packet["sign_id"].append(class_ind[model.predict(image_array)])

        pred = json.dumps(packet)
        handle.produce(stream["dev_id"], pred.encode('utf-8'))