import serial

def initialize_device(port="COM3", baudrate=115200, timeout=1):
    try:
        return serial.Serial(port, baudrate=baudrate, timeout=timeout)
    except serial.SerialException as e:
        print(f"Error: {e}")
        exit()

import re

def decode_gps_data(data):
    # Split the data into lines
    lines = data.split("¥¥¥¥¥¥¥¥¥¥")
    
    # Define a function to parse NMEA sentences
    def parse_nmea_sentence(sentence):
        if sentence.startswith("$GPGLL"):
            return parse_gpgll(sentence)
        elif sentence.startswith("$GPRMC"):
            return parse_gprmc(sentence)
        elif sentence.startswith("$GPVTG"):
            return parse_gpvtg(sentence)
        elif sentence.startswith("$GPGSA"):
            return parse_gpgsa(sentence)
        elif sentence.startswith("$GPGGA"):
            return parse_gpgga(sentence)
        elif sentence.startswith("$GPGSV"):
            return parse_gpgsv(sentence)
        else:
            return None
    
    def parse_gpgll(sentence):
        parts = sentence.split(',')
        if len(parts) > 7:
            return {
                'type': 'GPGLL',
                'latitude': parts[1],
                'longitude': parts[3],
                'time': parts[5],
                'status': parts[6],
                'mode': parts[7].split('*')[0],
            }
        return None

    def parse_gprmc(sentence):
        parts = sentence.split(',')
        if len(parts) > 11:
            return {
                'type': 'GPRMC',
                'time': parts[1],
                'status': parts[2],
                'latitude': parts[3],
                'longitude': parts[5],
                'speed': parts[7],
                'course': parts[8],
                'date': parts[9],
            }
        return None

    def parse_gpvtg(sentence):
        parts = sentence.split(',')
        if len(parts) > 8:
            return {
                'type': 'GPVTG',
                'track_degrees_true': parts[1],
                'track_degrees_magnetic': parts[3],
                'speed_knots': parts[5],
                'speed_kmh': parts[7],
                'mode': parts[9].split('*')[0],
            }
        return None

    def parse_gpgsa(sentence):
        parts = sentence.split(',')
        if len(parts) > 17:
            return {
                'type': 'GPGSA',
                'mode': parts[1],
                'fix_type': parts[2],
                'satellites': parts[3:15],
                'pdop': parts[15],
                'hdop': parts[16],
                'vdop': parts[17].split('*')[0],
            }
        return None

    def parse_gpgga(sentence):
        parts = sentence.split(',')
        if len(parts) > 14:
            return {
                'type': 'GPGGA',
                'time': parts[1],
                'latitude': parts[2],
                'longitude': parts[4],
                'fix_quality': parts[6],
                'num_satellites': parts[7],
                'hdop': parts[8],
                'altitude': parts[9],
                'geoid_separation': parts[11],
            }
        return None

    def parse_gpgsv(sentence):
        parts = sentence.split(',')
        if len(parts) > 4:
            return {
                'type': 'GPGSV',
                'total_msgs': parts[1],
                'msg_number': parts[2],
                'satellites_in_view': parts[3],
                'satellite_data': parts[4:],
            }
        return None

    # Process each line and parse valid NMEA sentences
    decoded_data = []
    for line in lines:
        match = re.match(r"\$.*?\*", line)
        if match:
            nmea_sentence = match.group(0)
            parsed_data = parse_nmea_sentence(nmea_sentence)
            if parsed_data:
                decoded_data.append(parsed_data)
    
    return decoded_data

if __name__ == "__main__":
    device = initialize_device()
    while True:

        if device.in_waiting:
            size_byte = device.read(1)
            size = int.from_bytes(size_byte, byteorder='big')

            while device.in_waiting < size: pass

            raw_data = device.read(size)
            decoded_data = raw_data.decode('ascii')  # Decode as needed

            print(size, decoded_data)

    device.close()
