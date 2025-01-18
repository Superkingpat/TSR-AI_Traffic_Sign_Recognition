from time import sleep
import serial
import serial.tools.list_ports
import argparse
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

    def __init__(
        self, port="/dev/tty.usbmodem2071358550471", baudrate=115200, callback=None
    ):
        self.callback = callback
        try:
            self.ser = serial.Serial(port, baudrate=baudrate, timeout=0.5)
            sleep(2)
        except serial.SerialException as e:
            print(f"Error initializing serial port: {e}")
            raise

    def parse_gpgll(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 7:
            return {
                "type": "GPGLL",
                "latitude": parts[1],
                "longitude": parts[3],
                "time": parts[5],
                "status": parts[6],
                "mode": parts[7].split("*")[0],
            }
        return None

    def parse_gprmc(self, sentence):
        parts = sentence.split(",")
        if len(parts) > 11:
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
                size_byte = self.ser.read(1)
                size = int.from_bytes(size_byte, byteorder="big")

                # Wait for complete data
                timeout_counter = 0
                while self.ser.in_waiting < size:
                    sleep(0.1)
                    timeout_counter += 1
                    if timeout_counter > 10:  # 1 second timeout
                        print("Timeout waiting for data")
                        return None

                raw_data = self.ser.read(size)
                decoded_data = raw_data.decode("ascii")
                return self.decode_gps_data(decoded_data)

            except (ValueError, UnicodeDecodeError) as e:
                print(f"Error processing data: {e}")
                return None
        return None

    def run_continuous(self):
        try:
            while True:
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
