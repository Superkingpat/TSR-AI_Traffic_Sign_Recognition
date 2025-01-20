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

        self.traffic_signs = {'10': 0, '100': 1, '120': 2, '20': 3, '30': 4, '30-': 5, '40': 6, '40-': 7, '50': 8, '50-': 9, '60': 10, '60-': 11, '70': 12, '70-': 13, '80': 14, '80-': 15, 'delo_na_cestiscu': 16, 'kolesarji_na_cestiscu': 17, 'konec_omejitev': 18, 'odvzem_prednosti': 19, 'otroci_na_cestiscu': 20, 'prednost': 21, 'prehod_za_pesce': 22, 'stop': 23, 'unknown': 24}
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

            time.sleep(1)

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

# Trenutno zakomentiram vse -


# TODO: Aplikacija -> Trackanje lokacije ✅
# TODO: https://reincubate.com/camo/about/ -> Problem pri svojem telegonu
# TODO: Dodaj threde, ki bodo delale na test.py -> in pridobivale GPS ✅
"""
void _startKafkaConsumer() async {
    if (session == null) {
      _showError("Kafka session is not initialized.");
      return;
    }

    String? vpnAddress = await getConnectedVpnAddresses();
    if (vpnAddress == null) {
      _showError("Failed to get VPN IP address.");
      return;
    }

    ipAddress = vpnAddress;
    topicNameConsumer = ipAddress!;

    var group = ConsumerGroup(session!, 'myConsumerGroup');
    var topics = {topicNameConsumer: {0}};

    try {
      consumer = Consumer(session!, group, topics, 100, 1);
      _consumerSubscription = consumer!.consume().listen((MessageEnvelope envelope) {
        try {
          var jsonString = String.fromCharCodes(envelope.message.value);

          var data = jsonDecode(jsonString);

          String datetime = data['DateTime'];
          String result = data['Result'];

          setState(() {
            // Ensure result is treated as a list
            List<String> results = result.contains(',') ? result.split(',') : [result];
            trafficSignsToShow = results.map((r) {
              return trafficSigns.keys.firstWhere(
                    (key) => trafficSigns[key].toString() == r.trim(),
                orElse: () => "unknown",
              );
            }).toList();
          });

          _updateTrafficSignMarkers();

          envelope.commit('metadata');
        } catch (e) {
          _showError("Error processing message: $e");
          print("Error processing message: $e");
        }
      }, onError: (error) {
        _showError("Error consuming message: $error");
        print("Error consuming message: $error");
      });
    } catch (e) {
      _showError("Failed to start Kafka consumer: $e");
      print("Failed to start Kafka consumer: $e");
    }
  }"""
