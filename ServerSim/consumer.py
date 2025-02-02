import numpy as np
from PIL import Image, ImageOps
from io import BytesIO
import base64
from comunication_handler import comunicationHandler
from time import time
import json
from ultralytics import YOLO
from prometheus_client import start_http_server, Counter, Gauge
from graham_hull import grahm_algorithm
import tensorflow as tf
from datetime import datetime, timezone

import cv2
import numpy as np

def decompress_image(compressed_data: bytes) -> np.ndarray:
    try:
        np_array = np.frombuffer(compressed_data, dtype=np.uint8)
        image = cv2.imdecode(np_array, cv2.IMREAD_COLOR)
        
        if image is None:
            raise ValueError("Failed to decompress image. Check input data.")
        
        return image
    except Exception as e:
        print(f"Error decompressing image: {str(e)}")
        return None
    

SIGN_COUNT = Counter('signs_detected', 'Number of signs detected and classifyed by the DenseNet121 model')
ALL_SIGN_COUNT = Counter('all_signs_detected', 'Number of signs detected by the YOLOv8 model')
HEATMAP_POINTS = Gauge('heatmap_points', 'points in 2D space that represent the heatmap', ['x', 'y'])
POINTS_ALL = Gauge('points_all', 'points in 2D space that represent all signs', ['x', 'y'])
MOBILE_REQUESTS = Counter('mobile_requests', 'Number of reciaved requests from the mobile client')
CLASS_COUNT = Counter('sign_occurrences', 'Number of occurrences of each sign', ['class'])
CONFIDENCE_THRESHOLD = 0.80

start_http_server(8000)

model_tf = tf.keras.models.load_model('TSC_simulation_DN121.h5')
model_yolo = YOLO('bestSIM.pt')

class_index = {'-100': 0, '-110': 1, '-120': 2, '-130': 3, '-20': 4, '-30': 5, '-40': 6, '-50': 7, '-60': 8, '-70': 9, '-80': 10, '-90': 11, '100': 12, '110': 13, '120': 14, '130': 15, '20': 16, '30': 17, '40': 18, '50': 19, '60': 20, '70': 21, '80': 22, '90': 23, 'konec vseh omejitev': 24, 'odvzem prednosti': 25, 'stop': 26}
class_index = {v: k for k, v in class_index.items()}

ip = '10.8.2.2:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('sim-apk-pictures', True)

msg = handle.consume(1.0)

if msg is not None:
    print(msg.value().decode('utf-8'))

sign_positions = [[0,0]]
POINTS_ALL.labels(x=int(0), y=int(0)).set(1)
POINTS_ALL.labels(x=int(1080), y=int(720)).set(1)
index = 0

while True:
    msg = handle.consume(1.0)

    if msg is not None:
        try:
            decoded_payload = json.loads(msg.value().decode('utf-8'))
        except json.JSONDecodeError:
            print("Failed to decode JSON message.")
            continue

        MOBILE_REQUESTS.inc(1)
        index += 1

        packet = {
            "Result" : [],
            "DateTime" : str(datetime.fromtimestamp(time(), tz=timezone.utc))
        }

        image_bytes = np.array(decoded_payload.get("Image"), dtype=np.uint8).tobytes()
        image_bytes = Image.open(BytesIO(image_bytes))
        image_bytes = np.array(image_bytes)
        
        Image.fromarray(image_bytes).save(f"img/{index}.png")

        results = model_yolo(image_bytes)[0]

        for result in results:
            for conf, cutout, centre in zip(result.boxes.conf, result.boxes.xyxy, result.boxes.xywh):
                ALL_SIGN_COUNT.inc(1)
                x, y, _, _ = centre
                x1, y1, x2, y2 = cutout
                x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
                sign_image = image_bytes[y1:y2, x1:x2]
                sign_image = Image.fromarray(sign_image).resize((224, 224), Image.LANCZOS)
                sign_image = np.array(sign_image)
                sign_image = sign_image / 255.0
                sign_image = np.expand_dims(sign_image, axis=0)

                prediction_vec = model_tf(sign_image)
                predicted_class_index = np.argmax(prediction_vec)
                predicted_confidence = prediction_vec[0][predicted_class_index]

                if predicted_confidence >= CONFIDENCE_THRESHOLD:
                    CLASS_COUNT.labels(class_index[predicted_class_index]).inc()
                    POINTS_ALL.labels(x=int(x), y=int(y)).set(1)
                    packet["Result"].append(str(predicted_class_index))
                    print(class_index[predicted_class_index])
                    sign_positions = np.append(sign_positions, [[x1, y1]], axis = 0)
                    sign_positions = np.append(sign_positions, [[x2, y2]], axis = 0)

        if len(sign_positions) > 0:
            sign_positions = grahm_algorithm(np.array(sign_positions))
            print(sign_positions)
            for x, y in sign_positions:
                HEATMAP_POINTS.labels(x=x, y=y).set(1)
        print(packet["Result"])

        SIGN_COUNT.inc(len(packet["Result"]))

        packet["Result"] = ','.join(packet["Result"])
        if packet["Result"].strip() == "": packet["Result"] = "-1"
        pred = json.dumps(packet)
        handle.produce(decoded_payload.get("IP"), pred.encode('utf-8'))