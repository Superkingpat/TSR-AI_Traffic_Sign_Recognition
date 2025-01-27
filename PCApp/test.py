from time import sleep
import serial
import serial.tools.list_ports
import re

class Controller:
    def list_serial_ports(self):
        ports = serial.tools.list_ports.comports()

        if not ports:
            print("No serial ports found!")
            return

        print("\nAvailable serial ports:")
        for port in ports:
            print(f"- Port: {port.device}")
            print(f"  Description: {port.description}")
            print(f"  Hardware ID: {port.hwid}\n")

    def validate_data(self,lat,long,status):
        if status != "A":
            return False
        
        if lat == "" or long == "":
            return False
        
        try:
            lat = float(lat)
            long = float(long)

            if lat == 0 or long == 0:
                return False

            print("The data has been validated")
            return True
        
        except (ValueError, UnicodeDecodeError) as e:
            print(f"The error is {e}")
            return False

    def __init__(self, port="COM15", baudrate=115200, callback=None):
        self.callback = callback
        try:
            self.ser = serial.Serial(port, baudrate=baudrate, timeout=0.5)
            sleep(2)
            self.send_data(" T.S.R. External Device", "center")
        except serial.SerialException as e:
            print(f"Error initializing serial port: {e}")
            raise

    def parse_gpgll(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 7:
            if self.validate_data(parts[1],parts[3],parts[6]):
                return {
                    "type": "GPGLL",
                    "latitude": parts[1],
                    "longitude": parts[3],
                    "time": parts[5],
                    "status": parts[6],
                    "mode": parts[7].split("*")[0],
                }
            else:
                return None

    def parse_gprmc(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 11:

            if self.validate_data(parts[3],parts[5],parts[2]):
                return {
                "type": "GPRMC",
                "time": parts[1],
                "status": parts[2],
                "latitude": parts[3],
                "longitude": parts[5],
                "speed": parts[7],
                "course": parts[8],
                "date": parts[9],
            }
            else:
                return None
             
    def parse_gpvtg(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 8:
            return {
                "type": "GPVTG",
                "track_degrees_true": parts[1],
                "track_degrees_magnetic": parts[3],
                "speed_knots": parts[5],
                "speed_kmh": parts[7],
                "mode": parts[9].split("*")[0],
            }
        return None

    def parse_gpgsa(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 17:
            return {
                "type": "GPGSA",
                "mode": parts[1],
                "fix_type": parts[2],
                "satellites": parts[3:15],
                "pdop": parts[15],
                "hdop": parts[16],
                "vdop": parts[17].split("*")[0],
            }
        return None

    def parse_gpgga(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 14:
            return {
                "type": "GPGGA",
                "time": parts[1],
                "latitude": parts[2],
                "longitude": parts[4],
                "fix_quality": parts[6],
                "num_satellites": parts[7],
                "hdop": parts[8],
                "altitude": parts[9],
                "geoid_separation": parts[11],
            }
        return None

    def parse_gpgsv(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 4:
            return {
                "type": "GPGSV",
                "total_msgs": parts[1],
                "msg_number": parts[2],
                "satellites_in_view": parts[3],
                "satellite_data": parts[4:],
            }
        return None

    def parse_nmea_sentence(self, sentence):
        try:
            if sentence.startswith("$GPGLL"):
                return self.parse_gpgll(sentence)
            elif sentence.startswith("$GPRMC"):
                return self.parse_gprmc(sentence)
            elif sentence.startswith("$GPVTG"):
                return self.parse_gpvtg(sentence)
            elif sentence.startswith("$GPGSA"):
                return self.parse_gpgsa(sentence)
            elif sentence.startswith("$GPGGA"):
                return self.parse_gpgga(sentence)
            elif sentence.startswith("$GPGSV"):
                return self.parse_gpgsv(sentence)
            else:
                return None
        except Exception as e:
            print(f"Something went wrong! ({e})")

    def decode_gps_data(self, data):
        lines = data.split("\n")  # To ne delaj - dodaj newline
        decoded_data = []

        for line in lines:
            match = re.match(r"\$.*?\*", line)
            if match:
                nmea_sentence = match.group(0)
                parsed_data = self.parse_nmea_sentence(nmea_sentence)
                if parsed_data:
                    decoded_data.append(parsed_data)

        return decoded_data

    def receive_data(self):
        if self.ser.in_waiting:
            try:
                buffer = self.ser.read(200).upper()
                buffer = buffer.replace(b'\x00', b'').replace(b'\n', b'').replace(b'\r', b'').replace(b'\xFF', b'').strip()
                decoded_data = buffer.decode('ascii', errors='ignore')
                #print(decoded_data, self.decode_gps_data(decoded_data))
                return self.decode_gps_data(decoded_data)
            except (ValueError, UnicodeDecodeError) as e:
                print(f"Error processing data: {e}")
                return None
        return None
    
    def send_data(self, data, alignment):
        # Helper function for aligning text
        def align_text(text, width, align_type):
            if align_type == 'left':
                return text.ljust(width)
            elif align_type == 'right':
                return text.rjust(width)
            elif align_type == 'center':
                return text.center(width)
            else:
                raise ValueError("Invalid alignment type. Choose 'left', 'right', or 'center'.")

        # Split the data into words
        words = data.split()
        line1, line2 = "", ""
        in_l2 = False

        # Construct the lines with word wrapping
        for word in words:
            if len(line1) + len(word) + 1 <= 16 and not in_l2:  # +1 accounts for the space between words
                if line1:
                    line1 += " "
                line1 += word
            elif len(line2) + len(word) + 1 <= 16:
                in_l2 = True
                if line2:
                    line2 += " "
                line2 += word
            else:
                size = len(word)
                s1 = int(size/2)
                s2 = size - s1
                line1 = word[:s1+1] + "-"
                line2 = word[s2:]

        # Align each line
        line1 = align_text(line1, 16, alignment)
        line2 = align_text(line2, 16, alignment)

        # Combine lines and pad to 32 characters (16x2)
        lcd_data = line1 + line2
        while len(data) < 32: data += " "
        if len(data) > 32: data = data[:32]
        data = data.upper()
        self.ser.write(lcd_data.encode('utf-8'))

    def run_continuous(self, shared_queue):
        try:
            while True:
                data = ""
                if not shared_queue.empty():
                    data = shared_queue.get()
                    if len(data) != 0:
                        data = " ".join(map(str, data))
                        data = "Prepoznani znaki: " + data
                        print(data)
                        self.send_data(data, "center")
                gps_data = self.receive_data()
                if gps_data:
                    for sentence in gps_data:
                        if sentence["type"] in ["GPGLL", "GPRMC"]:
                            gps_info = {
                                "time": sentence.get("time", 0),
                                "latitude": sentence.get("latitude", 0),
                                "longitude": sentence.get("longitude", 0),
                            }
                            if self.callback:
                                self.callback(gps_info)
                sleep(0.1)
        except KeyboardInterrupt:
            print("\nStopping GPS monitoring...")
        except Exception as e:
            print(f"Error was {e}")

"""
a = Controller()
a.send_data("Prepoznani znaki 70", "center")
#a.run_continuous(None)
while True:
    a.receive_data()


import cv2

cap = cv2.VideoCapture(1)  # Replace 0 with the correct index for the Camo app
if not cap.isOpened():
    print("Error: Could not open camera.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to retrieve frame.")
        break

    # Show the frame
    cv2.imshow("Camera Feed", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):  # Press 'q' to quit
        break

cap.release()
cv2.destroyAllWindows()
"""

# def main():
#    parser = argparse.ArgumentParser(description="GPS Data Monitor")
#    parser.add_argument(
#        "--port", default="/dev/tty.usbmodem2071358550471", help="Serial port"
#    )
#    parser.add_argument("--baudrate", type=int, default=115200, help="Baud rate")
#    parser.add_argument(
#        "--list-ports", action="store_true", help="List available serial ports"
#    )
#
#    args = parser.parse_args()
#
#    try:
#        controller = Controller(args.port, args.baudrate)
#
#        if args.list_ports:
#            controller.list_serial_ports()
#            return
#
#        controller.run_continuous()
#
#    except Exception as e:
#        print(f"Error: {e}")
#    finally:
#        if "controller" in locals():
#            controller.ser.close()
#
#
# if __name__ == "__main__":
#    main()
