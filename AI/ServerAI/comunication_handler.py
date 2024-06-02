from confluent_kafka import Producer, Consumer, error
from confluent_kafka.admin import AdminClient, NewTopic

class comunicationHandler:
    """
    The CommunicationHandler class is designed to facilitate communication with a Kafka server,\n
    providing functionalities for creating topics, subscribing to topics, producing messages, consuming messages,\n
    and closing the consumer. It utilizes the confluent-kafka-python library to interact with Kafka brokers.

    Parameters:
    -server_address: The address of the Kafka server.
    -consumer_group_name: The name of the consumer group.

    Errors: Raises an exeption if the Consumer, Producer or AdminClient could not connect to Kafka
    """
    def __init__(self, server_address, consumer_group_name):
        self.server_address = server_address
        self.consumer_group_name = consumer_group_name

        try:
            self.admin_client = AdminClient({'bootstrap.servers': self.server_address})
        except error.KafkaError as e:
            raise(f"An error occured while the AdminClient tried to connect to Kafka. Error: {str(e)}")

        try:
            self.consumer = Consumer({
                'bootstrap.servers': server_address,
                'group.id': consumer_group_name,
                'auto.offset.reset': 'earliest'
            })
        except error.KafkaError as e:
            raise(f"An error occured while the Consumer tried to connect to Kafka. Error: {str(e)}")

        try:
            self.producer = Producer({'bootstrap.servers': server_address})
        except error.KafkaError as e:
            raise(f"An error occured while the Producer tried to connect to Kafka. Error: {str(e)}")


    def create_topic(self, topic_name, num_partitions, replication_factor):
        """
        Creates a new Kafka topic with specified partitions and replication factor.

        Parameters:
        -topic_name: The name of the topic to create.
        -num_partitions: The number of partitions for the topic.
        -replication_factor: The replication factor for the topic.
        
        Returns: None

        Errors: Raises an exception if topic creation fails.
        """
        topic_conf = NewTopic(topic_name, num_partitions, replication_factor)
        futures = self.admin_client.create_topics([topic_conf])

        for future in futures:
            try:
                future.result()
                print(f"Topic {future.topic} created with replication factor {future.replication_factor}")
            except Exception as e:
                raise(f"Failed to create topic {future.topic}: {e}")
            
    def __delivery_report(err, msg):
        """
        Callback function for message delivery reports. Prints a message delivery status.
        
        Parameters:
        -err: An error object if the delivery failed.
        -msg: The message object.
        
        Returns: None
        
        Errors: None
        """
        if err is not None:
            print('Message delivery failed: {}'.format(err))
        
    def set_consumer_topic_subscribtion(self, topic, auto_topic_creation=True, num_partitions=1, replication_factor=1):
        """
        Subscribes the consumer to a topic. Optionally creates the topic if it doesn't exist.
        
        Parameters:
        -topic: The topic to subscribe to.
        -auto_topic_creation: A boolean indicating whether to automatically create the topic if it doesn't exist.
        -num_partitions: The number of partitions for the topic.
        -replication_factor: The replication factor for the topic.
        
        Returns: None
        
        Errors: Raises an exception if an error occurs during subscription or topic creation.
        """
        try:
            self.consumer.subscribe([topic])
        except error.KafkaException as e:
            if 'Unknown topic or partition' in str(e):
                if auto_topic_creation:
                    self.create_topic(topic, num_partitions, replication_factor)
                    self.consumer.subscribe([topic])
                else:
                    raise(f"Topic {topic} does not exist. auto_topic_creation set to false.")
            else:
                raise(f"An error occures. Error: {str(e)}")
    
    def produce(self, topic, massage):
        """
        Produces a message to a Kafka topic.

        Parameters:
        -topic: The topic to produce the message to.
        -message: The message to produce.
        
        Returns: None
        
        Errors: Raises a Kafka error if an error occurs during message production.
        """
        try:
            self.producer.produce(topic, value=massage, callback=self.__delivery_report)
            #self.producer.flush()
        except error.KafkaError as e:
            print(f"Kafka error occured: {str(e)}")
    
    def consume(self, timeout_len=1.0):
        """
        Consumes messages from a Kafka topic.
        
        Parameters:
        -timeout_len: The timeout length in seconds for the poll operation.
        -Returns: The consumed message or None if a timeout occurs.
        
        Errors: Raises a Kafka error if an error occurs during consumption.
        """
        consumer_msg = None
        try:
            consumer_msg = self.consumer.poll(timeout_len)
            if consumer_msg is None:
                print(f"Timeout reached. Timeout was set to {timeout_len} seconds.")
            else:
                return consumer_msg
        except error.KafkaError as e:
            raise(f"Kafka error occured: {str(e)}")
    
    def end_handler(self):
        """
        Closes the consumer.
        
        Parameters: None
        
        Returns: None
        
        Errors: None
        """
        self.consume.close()