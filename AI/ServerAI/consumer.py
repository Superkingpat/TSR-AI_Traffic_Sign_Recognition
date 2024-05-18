from confluent_kafka import Consumer
import numpy as np
from PIL import Image
from io import BytesIO
from comunication_handler import comunicationHandler
from time import time
import json
from ultralytics import YOLO
import base64
from prometheus_client import start_http_server, Counter

SIGN_COUNT = Counter('signs_detected', 'Number of signs detected')

start_http_server(8000)

model_yolo = YOLO('models/best.pt')
ip = 'localhost:9092'

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

        image_base64 = decoded_payload["image"]
        image_bytes = base64.b64decode(image_base64)
        image_bytes = Image.open(BytesIO(image_bytes))
        image_array = np.array(image_bytes)
        results = model_yolo(image_bytes)[0]

        classes = []
        filtered_results = []
        for result in results:
            filtered_boxes = []
            filtered_classes = []
            filtered_confidences = []
            for box, cls, conf in zip(result.boxes.xyxy, result.boxes.cls, result.boxes.conf):
                classes.append(cls.numpy().tolist())
                filtered_boxes.append(box)
                filtered_classes.append(cls.item())
                filtered_confidences.append(conf.item())
            filtered_result = {
                'boxes': filtered_boxes,
                'cls': filtered_classes,
                'conf': filtered_confidences
            }
            filtered_results.append(filtered_result)
        
        SIGN_COUNT.inc(len(classes))
        packet["sign_id"] = classes
        packet["time"] = tim
        pred = json.dumps(packet)
        handle.produce(decoded_payload["dev_id"], pred.encode('utf-8'))