from confluent_kafka import Consumer
import tensorflow as tf
import numpy as np
from PIL import Image
from io import BytesIO
import cv2
from comunication_handler import comunicationHandler

def preprocess_image(image_bytes):
    image = Image.open(BytesIO(image_bytes))
    image_array = np.array(image)
    image_array = cv2.resize(image_array, (224, 224))
    image_array = image_array / 255.0
    image_array = np.expand_dims(image_array, axis=0)
    return image_array

model = tf.keras.models.load_model('speed_classification.h5')
class_ind = {'100': 0, '130': 1, '30': 2, '40': 3, '40-': 4, '50': 5, '50-': 6, '60': 7, '60-': 8, '70': 9, '70-': 10, '80': 11, '90': 12}
class_ind = {v: k for k, v in class_ind.items()}

ip = 'localhost:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('from_client')

while True:
    msg = handle.consume(1.0)

    if msg is not None:
        stream = BytesIO(msg.value())
        image = Image.open(stream).convert("RGB")
        image_array = preprocess_image(stream.getvalue())
        pred = model.predict(image_array)
        pred = bytes(class_ind[np.argmax(pred)], 'utf-8')
        handle.produce('to_client', pred)

'''
c = Consumer({
    'bootstrap.servers': ip,
    'group.id': 'mygroup',
    'auto.offset.reset': 'earliest'
})

c.subscribe(['img_stream'])

while True:
    msg = c.poll(1.0)

    if msg is None:
        continue
    if msg.error():
        print("Consumer error: {}".format(msg.error()))
        continue

    stream = BytesIO(msg.value())
    image = Image.open(stream).convert("RGB")
    image_array = preprocess_image(stream.getvalue())
    pred = model.predict(image_array)
    print(f"The sign prediction is {class_ind[np.argmax(pred)]}")
    #image.save('received_image.png')

c.close()
'''