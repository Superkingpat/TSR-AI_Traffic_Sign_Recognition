from confluent_kafka import Consumer
import base64
import tensorflow as tf
import numpy as np

model = tf.keras.models.load_model('speed_classification.h5')
class_ind = {'100': 0, '130': 1, '30': 2, '40': 3, '40-': 4, '50': 5, '50-': 6, '60': 7, '60-': 8, '70': 9, '70-': 10, '80': 11, '90': 12}
class_ind = {v: k for k, v in class_ind.items()}

ip = '10.8.2.3:9092'

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

    decoded_image = base64.b64decode(msg.value().decode('utf-8'))
    decoded_image = model.predict(np.expand_dims((tf.image.resize(tf.io.decode_image(base64.b64decode(msg.value().decode('utf-8')), channels=3, dtype=tf.uint8), [224, 224]) / 255.0).numpy(), axis=0))
    print(f"The sign prediction is {class_ind[decoded_image]}")

c.close()