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
import csv
from pytz import UTC
import cv2
import numpy as np

MODEL_INDEX = "N640"

CSV_FILE = f"performance_time{MODEL_INDEX}MN.csv"

with open(CSV_FILE, mode='w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)
    csv_writer.writerow(["Procedure", "Timestamp", "Timestamp_Sent", "Timestamp_Recived","Seconds_Deliver", 
                         "Seconds_Detected", "Seconds_Classified", "No_Detected", "No_Classified"])

def record_time_to_csv(arr):
    try:
        with open(CSV_FILE, mode='a', newline='') as csvfile:
            csv_writer = csv.writer(csvfile)
            csv_writer.writerow(arr)
    except Exception as e:
        print(f"Error writing to CSV: {str(e)}")

def calculate_elapsed_time(cpp_timestamp, current_time = datetime.now()):
    try:
        if type(cpp_timestamp) == type("eda"): cpp_timestamp = datetime.strptime(cpp_timestamp, "%Y-%m-%d %H:%M:%S.%f")
        if type(current_time) == type("eda"): current_time = datetime.strptime(current_time, "%Y-%m-%d %H:%M:%S.%f")
    
        elapsed_time = current_time - cpp_timestamp
        return elapsed_time.total_seconds()
    except Exception as e:
        print(f"Error calculating elapsed time: {str(e)}")
        return None

SIGN_COUNT = Counter('signs_detected', 'Number of signs detected and classifyed by the DenseNet121 model')
ALL_SIGN_COUNT = Counter('all_signs_detected', 'Number of signs detected by the YOLOv8 model')
HEATMAP_POINTS = Gauge('heatmap_points', 'points in 2D space that represent the heatmap', ['x', 'y'])
POINTS_ALL = Gauge('points_all', 'points in 2D space that represent all signs', ['x', 'y'])
MOBILE_REQUESTS = Counter('mobile_requests', 'Number of reciaved requests from the mobile client')
CLASS_COUNT = Counter('sign_occurrences', 'Number of occurrences of each sign', ['class'])
CONFIDENCE_THRESHOLD = 0.80

sign_positions = [[0,0]]
POINTS_ALL.labels(x=int(0), y=int(0)).set(1)
POINTS_ALL.labels(x=int(1080), y=int(720)).set(1)
index = 0

def simulation_procedure(decoded_payload):
    global sign_positions
    start_time = datetime.now()

    packet = {
        "Result" : [],
        "DateTime" : str(datetime.fromtimestamp(time(), tz=timezone.utc))
    }

    image_bytes = np.array(decoded_payload.get("Image"), dtype=np.uint8).tobytes()
    image_bytes = Image.open(BytesIO(image_bytes))
    image_bytes = np.array(image_bytes)

    results = model_yolo_S(image_bytes)[0]

    classification_start_time = datetime.now()
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

            prediction_vec = model_tf_S(sign_image)
            predicted_class_index = np.argmax(prediction_vec)
            predicted_confidence = prediction_vec[0][predicted_class_index]

            if predicted_confidence >= CONFIDENCE_THRESHOLD:
                CLASS_COUNT.labels(class_index_S[predicted_class_index]).inc()
                POINTS_ALL.labels(x=int(x), y=int(y)).set(1)
                packet["Result"].append(str(predicted_class_index))
                print(class_index_S[predicted_class_index])
                sign_positions = np.append(sign_positions, [[x1, y1]], axis = 0)
                sign_positions = np.append(sign_positions, [[x2, y2]], axis = 0)
    classification_end_time = datetime.now()
    arr = ["simulation_procedure", datetime.now(), decoded_payload.get("DateTime"), start_time, calculate_elapsed_time(decoded_payload.get("DateTime"), start_time),
           calculate_elapsed_time(start_time, classification_start_time), calculate_elapsed_time( classification_start_time, classification_end_time),
             len(results), len(packet["Result"])]
    
    record_time_to_csv(arr)

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

def application_procedure(decoded_payload):
    global sign_positions
    packet = {
        "Result" : [],
        "DateTime" : str(datetime.fromtimestamp(time(), tz=timezone.utc))
    }

    image_bytes = decompress_image(base64.b64decode(decoded_payload.get("Image")))

    detection_start_time = datetime.now(timezone.utc)
    results = model_yolo_A(image_bytes)[0]

    classification_start_time = datetime.now(timezone.utc)
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

            prediction_vec = model_tf_A(sign_image)
            predicted_class_index = np.argmax(prediction_vec)
            predicted_confidence = prediction_vec[0][predicted_class_index]

            if predicted_confidence >= CONFIDENCE_THRESHOLD:
                CLASS_COUNT.labels(class_index_A[predicted_class_index]).inc()
                POINTS_ALL.labels(x=int(x), y=int(y)).set(1)
                packet["Result"].append(str(predicted_class_index))
                print(class_index_A[predicted_class_index])
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
    
start_http_server(8000)

model_tf_A = tf.keras.models.load_model('TSC_EXT_DN121.h5')
model_yolo_A = YOLO('best1080.pt')

model_tf_S = tf.keras.models.load_model('TSC_simulation_MNV2.h5')
model_yolo_S = YOLO(f'best{MODEL_INDEX}.pt')

class_index_S = {'-100': 0, '-110': 1, '-120': 2, '-130': 3, '-20': 4, '-30': 5, '-40': 6, '-50': 7, '-60': 8, '-70': 9, '-80': 10, '-90': 11, '100': 12, '110': 13, '120': 14, '130': 15, '20': 16, '30': 17, '40': 18, '50': 19, '60': 20, '70': 21, '80': 22, '90': 23, 'konec vseh omejitev': 24, 'odvzem prednosti': 25, 'stop': 26}
class_index_S = {v: k for k, v in class_index_S.items()}

class_index_A = {'10': 0, '100': 1, '110': 2, '120': 3, '130': 4, '20': 5, '30': 6, '30-': 7, '40': 8, '40-': 9, '50': 10, '50-': 11, '60': 12, '60-': 13, '70': 14, '70-': 15, '80': 16, '80-': 17, '90': 18, 'delo_na_cestiscu': 19, 'kolesarji_na_cestiscu': 20, 'konec_omejitev': 21, 'odvzem_prednosti': 22, 'otroci_na_cestiscu': 23, 'prednost': 24, 'prehod_za_pesce': 25, 'stop': 26, 'unknown': 27}
class_index_A = {v: k for k, v in class_index_A.items()}

ip = '10.8.2.2:9092'

handle = comunicationHandler(ip, 'server_group')
handle.set_consumer_topic_subscribtion('sim-apk-pictures', True)

msg = handle.consume(1.0)

if msg is not None:
    print(msg.value().decode('utf-8'))



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
        
        #Image.fromarray(image_bytes).save(f"img/{index}.png")

        payloadType = decoded_payload.get("Position")

        if payloadType == None:
            simulation_procedure(decoded_payload)
        else:
            application_procedure(decoded_payload)