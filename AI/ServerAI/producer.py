from confluent_kafka import Producer
import time

ip = '10.8.2.3:9092'

p = Producer({'bootstrap.servers': ip})

def delivery_report(err, msg):
    if err is not None:
        print('Message delivery failed: {}'.format(err))
    else:
        print('Message delivered to {} [{}]'.format(msg.topic(), msg.partition()))

i = 0

while True:
    p.poll(0)
    i += 1
    p.produce('mytopic', str(i).encode('utf-8'), callback=delivery_report)
    time.sleep(2)

p.flush()