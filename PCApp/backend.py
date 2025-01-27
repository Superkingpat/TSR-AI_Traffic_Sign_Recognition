from flask import Flask, render_template, Response, jsonify
import cv2
import time
import os
from datetime import datetime
import threading
from flask_cors import CORS
import json
import numpy as np
import psutil
import socket
from confluent_kafka import Producer, Consumer, KafkaError
from typing import Optional, Dict, Any
import threading
from test import Controller
import base64
import requests
from flask import request
import queue

GLOBAL_PORT = 9999
GLOBAL_SIGN_ARRAY = []  
shared_queue = queue.Queue()

# from test import processGPSCoordinates, timeGPS, latitudeGPS, longitudeGPS

app = Flask(__name__)
CORS(app)

data_array = []

def get_vpn_ip():
    return "10.8.2.5"
    for interface, addrs in psutil.net_if_addrs().items():
        if "tun" in interface.lower() or "vpn" in interface.lower():
            for addr in addrs:
                if addr.family == socket.AF_INET:
                    return addr.address
    return None

def send_data_to_flask(packet):
    url = f"http://127.0.0.1:{GLOBAL_PORT}/receive_data"
    response = requests.post(url, json=packet)

    if response.status_code == 200:
        print("Data sent successfully")
    else:
        print(f"Failed to send data: {response.status_code}")

def send_gps_data_to_flask(packet):
    url = f"http://127.0.0.1:{GLOBAL_PORT}/receive_data"
    response = requests.post(url, json=packet)

    if response.status_code == 200:
        print("Data sent successfully")
    else:
        print(f"Failed to send data: {response.status_code}")

class CameraManager:
    def __init__(self):
        self.camera: Optional[cv2.VideoCapture] = None
        self.is_capturing: bool = False
        self.capture_thread: Optional[threading.Thread] = None
        self.gps_thread: Optional[threading.Thread] = None
        self.latest_frame_data: Dict[str, Any] = {
            "compressed_frame": None,
            "timestamp": None,
            "location": {"time": None, "latitude": None, "longitude": None},
        }
        self.gps_controller = None

    def update_gps_data(self, gps_info):
        self.latest_frame_data["location"] = {
            "time": gps_info["time"],
            "latitude": gps_info["latitude"],
            "longitude": gps_info["longitude"],
        }

    def start_gps_tracking(self):
        global shared_queue
        def run_gps():
            self.gps_controller = Controller(callback=self.update_gps_data)
            self.gps_controller.run_continuous(shared_queue)

        self.gps_thread = threading.Thread(target=run_gps)
        self.gps_thread.daemon = True
        self.gps_thread.start()

    def get_camera(self) -> cv2.VideoCapture:
        video_path = "2024_5_3-Pat (3).MP4"
        if self.camera is None:
            self.camera = cv2.VideoCapture(0)  # Spremeni na 1
            if not self.camera.isOpened():
                raise RuntimeError("Could not open camera")
        return self.camera

    def cleanup(self):
        if self.camera:
            self.camera.release()
            self.camera = None
        if self.gps_controller:
            if hasattr(self.gps_controller, "ser"):
                self.gps_controller.ser.close()

class KafkaImageProducer:
    def __init__(
        self,
        bootstrap_servers: str = "10.8.2.2:9092",
        topic_name: str = "test-pictures-flutter",
    ):
        self.conf = {
            "bootstrap.servers": bootstrap_servers,
            "client.id": socket.gethostname(),
        }
        self.producer = Producer(self.conf)
        self.topic_name = topic_name

    def delivery_callback(self, err, msg):
        if err:
            print(f"Message delivery failed: {err}")
        else:
            print(f"Message delivered to {msg.topic()} [{msg.partition()}]")

    def send_frame(self, compressed_frame: np.ndarray):
        encoded_frame = base64.b64encode(compressed_frame).decode("utf-8")

        message = {
            "IP": str(get_vpn_ip()),
            "Image": encoded_frame,
            "Time": datetime.now().isoformat(),
            "Position": {
                "time": None,
                "latitude": None,
                "longitude": None,
                "altitude": None,
                "accuracy": None,
                "speed_accuracy": None,
            },
        }
        try:
            self.producer.produce(
                self.topic_name,
                value=json.dumps(message).encode("utf-8"),
                callback=self.delivery_callback,
            )
            self.producer.poll(0)
        except BufferError:
            print("Local producer queue is full")
            self.producer.poll(0.1)
            self.producer.produce(
                self.topic_name,
                value=json.dumps(message).encode("utf-8"),
                callback=self.delivery_callback,
            )

    def close(self):
        self.producer.flush()

class KafkaTrafficSignConsumer:
    def __init__(self, bootstrap_servers="10.8.2.2:9092"):
        self.bootstrap_servers = bootstrap_servers
        self.consumer = None
        self.is_consuming = False
        self.consumer_thread = None

        self.traffic_signs = {'10': 0, '100': 1, '110': 2, '120': 3, '130': 4, '20': 5, '30': 6, '30-': 7, '40': 8, '40-': 9, '50': 10, '50-': 11, '60': 12, '60-': 13, '70': 14, '70-': 15, '80': 16, '80-': 17, '90': 18, 'delo_na_cestiscu': 19, 'kolesarji_na_cestiscu': 20, 'konec_omejitev': 21, 'odvzem_prednosti': 22, 'otroci_na_cestiscu': 23, 'prednost': 24, 'prehod_za_pesce': 25, 'stop': 26, 'unknown': 27}
        self.reverse_traffic_signs = {v: k for k, v in self.traffic_signs.items()}
        self.current_signs = []

    def start_consuming(self, topic_name, group_id="server_group"):
        try:
            conf = {
                "bootstrap.servers": self.bootstrap_servers,
                "group.id": group_id,
                #"auto.offset.reset": "earliest",
                "enable.auto.commit": True,
            }

            self.consumer = Consumer(conf)
            self.consumer.subscribe([topic_name])

            self.is_consuming = True
            self.consumer_thread = threading.Thread(target=self._consume_messages)
            self.consumer_thread.daemon = True
            self.consumer_thread.start()

            print(f"Started consuming from topic: {topic_name}")

        except Exception as e:
            print(f"Failed to start Kafka consumer: {e}")
            self.is_consuming = False
            raise

    def _consume_messages(self):
        try:
            while self.is_consuming:
                msg = self.consumer.poll(1.0)
                if msg is None:
                    continue
                if msg.error():
                    print(f"Kafka error: {msg.error()}")
                    continue

                try:
                    self.process_message(msg)
                    self.consumer.commit()
                except Exception as e:
                    print(f"Error processing message: {e}")
                    continue

        except Exception as e:
            print(f"Error consuming messages: {e}")
        finally:
            self.stop_consuming()

    def process_message(self, msg):
        global shared_queue, GLOBAL_SIGN_ARRAY
        try:
            payload = json.loads(msg.value().decode("utf-8"))
            datetime = payload.get("DateTime", "unknown datetime")

            results = payload.get("Result", "").split(",")
            # Update current signs
            self.current_signs = [
                self.reverse_traffic_signs.get(int(r.strip()), "") for r in results
            ]

            shared_queue.put(self.current_signs)

            #self.current_signs = ["120"]
            #sGLOBAL_SIGN_ARRAY.clear()
            lenght_of_current_signs = len(self.current_signs)
            GLOBAL_SIGN_ARRAY.append(self.current_signs)
            #GLOBAL_SIGN_ARRAY = GLOBAL_SIGN_ARRAY[:-lenght_of_current_signs],
            if len(self.current_signs) > 0:
                GLOBAL_SIGN_ARRAY = GLOBAL_SIGN_ARRAY[-lenght_of_current_signs:]
            json_payload = {
                "received_data": GLOBAL_SIGN_ARRAY,
                "status": "success",
            }
            # print(f"Updated GLOBAL_SIGN_ARRAY: {GLOBAL_SIGN_ARRAY}")

            try:
                response = requests.post(
                    f"http://127.0.0.1:{GLOBAL_PORT}/receive_data", json=json_payload
                )
                if response.status_code == 200:
                    print(
                        f"Successfully sent data to Flask endpoint: {response.json()}"
                    )
                else:
                    print(
                        f"Failed to send data to Flask endpoint: {response.status_code}"
                    )
            except requests.RequestException as e:
                print(f"Error sending data to Flask endpoint: {e}")

            print(f"Processed message: DateTime={datetime}, Signs={self.current_signs}")

        except Exception as e:
            print(f"Failed to process message: {e}")
            raise

    def stop_consuming(self):
        self.is_consuming = False
        if self.consumer:
            self.consumer.close()
            print("Kafka consumer stopped.")
        if self.consumer_thread:
            self.consumer_thread.join(timeout=1.0)

camera_manager = CameraManager()

def capture_frames():
    while camera_manager.is_capturing:
        try:
            kafka_producer = KafkaImageProducer()
            camera = camera_manager.get_camera()

            success, frame = camera.read()
            if success:
                swapped_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

                # Compress the frame
                encode_param = [cv2.IMWRITE_JPEG_QUALITY, 80]
                _, compressed_frame = cv2.imencode(".jpg", swapped_frame, encode_param)
                compressed_data = compressed_frame.tobytes()

                kafka_producer.send_frame(compressed_data)

                # timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                # filename = f"captured_frames/frame_{timestamp}.jpg"
                # with open(filename, "wb") as f:
                #    f.write(compressed_data)

                # Update latest frame data
                # camera_manager.latest_frame_data = {
                #    "compressed_frame": compressed_data,
                #    "timestamp": timestamp,
                # }

            time.sleep(2)

        except Exception as e:
            print(f"Error in capture_frames: {str(e)}")
            time.sleep(1)


@app.route("/")
def index():
    return render_template("index.html")

@app.route("/start_capture", methods=["POST"])
def start_capture():
    if not camera_manager.is_capturing:
        try:
            camera_manager.is_capturing = True
            camera_manager.capture_thread = threading.Thread(target=capture_frames)
            camera_manager.capture_thread.daemon = True
            camera_manager.capture_thread.start()

            camera_manager.start_gps_tracking()

            return jsonify(
                {"status": "started", "message": "Capture started successfully"}
            )
        except Exception as e:
            camera_manager.is_capturing = False
            return jsonify({"status": "error", "message": str(e)}), 500

    return jsonify({"status": "running", "message": "Capture already running"})


@app.route("/stop_capture", methods=["POST"])
def stop_capture():
    if camera_manager.is_capturing:
        camera_manager.is_capturing = False
        if camera_manager.capture_thread:
            camera_manager.capture_thread.join(timeout=1.0)
        return jsonify({"status": "stopped", "message": "Capture stopped successfully"})

    return jsonify({"status": "idle", "message": "No capture running"})

# Tu se spreminja
@app.route("/receive_data", methods=["GET", "POST"])
def receive_data():
    global GLOBAL_SIGN_ARRAY
    if request.method == "POST":
        data = request.get_json()  # Parse JSON payload
        if data:
            return jsonify({
                "status": "success",
                "received_data": data.get("received_data", [])
            })
    elif request.method == "GET":
        return jsonify({"status": "success", "received_data": GLOBAL_SIGN_ARRAY, "gps": "True"})
   


@app.route("/receive_gps_data", methods=["GET"])
def send_gps_data():
    return jsonify({"status": "success", "received_data": data_array})

@app.route("/video_feed")
def video_feed():

    def generate_frames():
        while True:
            if camera_manager.is_capturing:
                try:
                    camera = camera_manager.get_camera()
                    success, frame = camera.read()
                    if success:
                        # Convert frame to JPEG format
                        ret, buffer = cv2.imencode(".jpg", frame)
                        if ret:
                            # Convert to bytes
                            frame_bytes = buffer.tobytes()
                            yield (
                                b"--frame\r\n"
                                b"Content-Type: image/jpeg\r\n\r\n"
                                + frame_bytes
                                + b"\r\n"
                            )
                except Exception as e:
                    print(f"Error generating frame: {str(e)}")
                    continue
            time.sleep(0.1)

    return Response(
        generate_frames(), mimetype="multipart/x-mixed-replace; boundary=frame"
    )

if __name__ == "__main__":
    os.makedirs("captured_frames", exist_ok=True)

    consumer = KafkaTrafficSignConsumer()

    try:
        consumer.start_consuming(f"{get_vpn_ip()}")
        # Keep the main thread running

    except:
        consumer.stop_consuming()

    try:
        app.run(debug=True, port=GLOBAL_PORT)
    finally:
        camera_manager.cleanup()
