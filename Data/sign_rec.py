import tensorflow as tf

def start_train():
    data_dir = r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\Data\5_Image_Augmentor\Data"
    batch_size = 32
    img_height = 224
    img_width = 224

    data_gen = tf.keras.preprocessing.image.ImageDataGenerator(
        rescale=1./255,
        validation_split=0.2
    )

    train_ds = data_gen.flow_from_directory(
        data_dir,
        target_size=(img_height, img_width),
        batch_size=batch_size,
        subset='training',
        seed=123,
        class_mode='sparse'
    )

    val_ds = data_gen.flow_from_directory(
        data_dir,
        target_size=(img_height, img_width),
        batch_size=batch_size,
        subset='validation',
        seed=123,
        class_mode='sparse'
    )

    base_model = tf.keras.applications.ResNet152V2(
        include_top=False,
        weights="imagenet",
        input_tensor=None,
        input_shape=None,
        pooling=None,
        classes=244,
        classifier_activation="softmax",
    )

    base_model.trainable = False

    x = base_model.output
    x = tf.keras.layers.GlobalAveragePooling2D()(x)

    x = tf.keras.layers.Dense(1024, activation='relu')(x)
    x = tf.keras.layers.Dropout(0.5)(x)

    predictions = tf.keras.layers.Dense(244, activation='softmax')(x)

    model = tf.keras.models.Model(inputs=base_model.input, outputs=predictions)

    model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
                loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=False),#from_logits=True
                metrics=['accuracy'])

    tensorboard_callback = tf.keras.callbacks.TensorBoard(log_dir='logs', histogram_freq=1)

    early_stopping = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5)

    model.fit(train_ds, epochs=10, validation_data=val_ds, callbacks=[early_stopping, tensorboard_callback])
    model.save('TSC_full.h5')

def continiue_training():
    model = tf.keras.models.load_model('TSC_full.h5')
    epoch = 0

    data_dir = r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\Data\5_Image_Augmentor\d3"
    batch_size = 32
    img_height = 224
    img_width = 224

    data_gen = tf.keras.preprocessing.image.ImageDataGenerator(
        rescale=1./255,
        validation_split=0.2
    )

    train_ds = data_gen.flow_from_directory(
        data_dir,
        target_size=(img_height, img_width),
        batch_size=batch_size,
        subset='training',
        seed=123,
        class_mode='sparse'
    )

    val_ds = data_gen.flow_from_directory(
        data_dir,
        target_size=(img_height, img_width),
        batch_size=batch_size,
        subset='validation',
        seed=123,
        class_mode='sparse'
    )

    model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
                    loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=False),
                    metrics=['accuracy'])

    tensorboard_callback = tf.keras.callbacks.TensorBoard(log_dir='logs', histogram_freq=1)
    early_stopping = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5)
    
    while True:
        model.fit(train_ds, epochs=1, validation_data=val_ds, callbacks=[early_stopping, tensorboard_callback])

        model.save('TSC_full.h5')
        if epoch % 5 == 0:
            model.save(f'TSC_full_{epoch}.h5')
            
continiue_training()