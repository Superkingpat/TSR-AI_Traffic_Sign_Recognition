from confluent_kafka import Consumer
import numpy as np
from PIL import Image
from io import BytesIO
from comunication_handler import comunicationHandler
from time import time
import json
from ultralytics import YOLO
from prometheus_client import start_http_server, Counter, Gauge
from graham_hull import grahm_algorithm

SIGN_COUNT = Counter('signs_detected', 'Number of signs detected')
HEATMAP_POINTS = Gauge('heatmap_points', 'points in 2D space that represent the heatmap', ['x', 'y'])

start_http_server(8000)

model_yolo = YOLO('models/best.pt')
ip = '10.8.2.2:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('test-pictures-flutter', False)

msg = handle.consume(1.0)

if msg is not None:
    print(msg.value().decode('utf-8'))

while True:
    msg = handle.consume(1.0)
    tim = time()

    if msg is not None:
        decoded_payload = json.loads(msg.value().decode('utf-8'))

        packet = {
            "sign_id" : [],
            "time" : tim
        }

        image_bytes = np.array(decoded_payload.get("Image"), dtype=np.uint8).tobytes()
        image_bytes = Image.open(BytesIO(image_bytes))
        image_bytes = np.array(image_bytes)

        results = model_yolo(image_bytes)[0]

        classes = []
        sign_positions = []
        for result in results:
            filtered_boxes = []
            filtered_classes = []
            filtered_confidences = []
            for box, cls, conf in zip(result.boxes.xywh, result.boxes.cls, result.boxes.conf):
                x, y, _, _ = box
                classes.append(cls.numpy().tolist())
                sign_positions.append([x.item(), y.item()])
        
        if len(sign_positions) > 0:
            hull = grahm_algorithm(np.array(sign_positions))
            print(hull)
            for x, y in hull:
                HEATMAP_POINTS.labels(x=x, y=y).set(1)
        print(classes)

        SIGN_COUNT.inc(len(classes))
        packet["sign_id"] = classes
        packet["time"] = tim
        pred = json.dumps(packet)
        print(decoded_payload.get("IP"))
        handle.produce(decoded_payload.get("IP"), pred.encode('utf-8'))