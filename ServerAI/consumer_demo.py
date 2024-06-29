import numpy as np
from PIL import Image, ImageOps, ImageDraw, ImageFont
from io import BytesIO
from comunication_handler import comunicationHandler
from time import time
import json
from ultralytics import YOLO
from prometheus_client import start_http_server, Counter, Gauge
from graham_hull import grahm_algorithm
import tensorflow as tf
from datetime import datetime, timezone
import matplotlib.pyplot as plt
import csv

sesion_key = time()
headernames=["created_time", "recived_time", "completed_time", "num_of_detections", "num_of_clasifications"]

with open(f"server_{sesion_key}.csv","w",newline="") as csvfile:
    writer=csv.DictWriter(csvfile,headernames)
    writer.writeheader()

def show_array_interactive(array):
    plt.ion()
    plt.imshow(array)
    plt.title('Detection')
    plt.draw()
    plt.pause(0.001)

SIGN_COUNT = Counter('signs_detected', 'Number of signs detected and classifyed by the DenseNet121 model')
ALL_SIGN_COUNT = Counter('all_signs_detected', 'Number of signs detected by the YOLOv8 model')
HEATMAP_POINTS = Gauge('heatmap_points', 'points in 2D space that represent the heatmap', ['x', 'y'])
MOBILE_REQUESTS = Counter('mobile_requests', 'Number of reciaved requests from the mobile client')
POINTS_ALL = Gauge('points_all', 'points in 2D space that represent all signs', ['x', 'y'])
CLASS_COUNT = Counter('sign_occurrences', 'Number of occurrences of each sign', ['class'])
CONFIDENCE_THRESHOLD = 0.80

start_http_server(8000)

model_tf = tf.keras.models.load_model(r'models\TSC_final_2_DN121.h5')
model_yolo = YOLO('models/best.pt')
model_yolo.to("cuda")

class_index = {'10': 0, '100': 1, '120': 2, '20': 3, '30': 4, '30-': 5, '40': 6, '40-': 7, '50': 8, '50-': 9, '60': 10, '60-': 11, '70': 12, '70-': 13, '80': 14, '80-': 15, 'delo_na_cestiscu': 16, 'kolesarji_na_cestiscu': 17, 'konec_omejitev': 18, 'odvzem_prednosti': 19, 'otroci_na_cestiscu': 20, 'prednost': 21, 'prehod_za_pesce': 22, 'stop': 23, 'unknown': 24}
class_index = {v: k for k, v in class_index.items()}

ip = '10.8.2.2:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('test-pictures-flutter', False)

msg = handle.consume(1.0)

if msg is not None:
    print(msg.value().decode('utf-8'))

sign_positions = [[0,0]]
POINTS_ALL.labels(x=int(0), y=int(0)).set(1)
POINTS_ALL.labels(x=int(1080), y=int(720)).set(1)

while True:
    msg = handle.consume(1.0)
    info = {"recived_time":time(), "num_of_detections" : 0, "num_of_clasifications" : 0}

    if msg is not None:
        try:
            decoded_payload = json.loads(msg.value().decode('utf-8'))
        except json.JSONDecodeError:
            print("Failed to decode JSON message.")
            continue

        MOBILE_REQUESTS.inc(1)

        packet = {
            "Result" : [],
            "DateTime" : str(datetime.fromtimestamp(time(), tz=timezone.utc))
        }

        image_bytes = np.array(decoded_payload.get("Image"), dtype=np.uint8).tobytes()
        image = Image.open(BytesIO(image_bytes))
        image_bytes = np.array(image)

        results = model_yolo(image_bytes)[0]

        draw = ImageDraw.Draw(image)
        font = ImageFont.truetype("BIZUDPMincho-Regular.ttf", 20)

        for result in results:
            for conf, cutout, centre in zip(result.boxes.conf, result.boxes.xyxy, result.boxes.xywh):
                info["num_of_detections"] += 1
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

                if predicted_class_index == 24:
                    continue

                if predicted_confidence >= CONFIDENCE_THRESHOLD:
                    if str(predicted_class_index) not in packet["Result"]: packet["Result"].append(str(predicted_class_index))
                    
                    info["num_of_clasifications"] += 1
                    CLASS_COUNT.labels(class_index[predicted_class_index]).inc()
                    POINTS_ALL.labels(x=int(x), y=int(y)).set(1)
                    print(class_index[predicted_class_index])
                    sign_positions = np.append(sign_positions, [[x1, y1]], axis = 0)
                    sign_positions = np.append(sign_positions, [[x2, y2]], axis = 0)

                    draw.rectangle([x1, y1, x2, y2], outline="red", width=3)
                    label = f"{class_index[predicted_class_index]}: {predicted_confidence:.2f}"
                    bbox = draw.textbbox((x1, y1), label, font=font, anchor="ls")
                    draw.rectangle(bbox, fill='black')
                    draw.text((x1, y1), label, fill="white", font=font, anchor="ls")
        
        if len(sign_positions) > 0:
            sign_positions = grahm_algorithm(np.array(sign_positions))
            #print(sign_positions)
            for x, y in sign_positions:
                HEATMAP_POINTS.labels(x=x, y=y).set(1)
        #print(packet["Result"])

        SIGN_COUNT.inc(len(packet["Result"]))

        packet["Result"] = ','.join(packet["Result"])
        pred = json.dumps(packet)
        handle.produce(decoded_payload.get("IP"), pred.encode('utf-8'))
        
        t = datetime.fromisoformat(packet['DateTime']).timestamp()

        info["created_time"] = t
        info["completed_time"] = time()

        with open(f"server_{sesion_key}.csv","a",newline="") as csvfile:
            writer=csv.DictWriter(csvfile,headernames)
            writer.writerow(info)

        Image.fromarray(image_bytes).save(f"img/{t}.png")
        image.save(f"img_marked/{t}.png")

        #show_array_interactive(np.array(image))