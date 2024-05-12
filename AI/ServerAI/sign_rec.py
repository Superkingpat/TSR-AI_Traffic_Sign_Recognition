import tensorflow as tf
import scipy

data_dir = r"C:\Users\steam\OneDrive\Namizje\Sign_augmentation\gen_imgs"
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
    class_mode='categorical'
)

val_ds = data_gen.flow_from_directory(
    data_dir,
    target_size=(img_height, img_width),
    batch_size=batch_size,
    subset='validation',
    seed=123,
    class_mode='categorical'
)

base_model = tf.keras.applications.DenseNet121(weights='imagenet', include_top=False, input_shape=(224, 224, 3))

x = base_model.output
x = tf.keras.layers.GlobalAveragePooling2D()(x)
x = tf.keras.layers.BatchNormalization()(x)
x = tf.keras.layers.Dropout(0.5)(x)
x = tf.keras.layers.Dense(1024, activation='relu')(x)
x = tf.keras.layers.Dense(512, activation='relu')(x)
x = tf.keras.layers.BatchNormalization()(x)
x = tf.keras.layers.Dropout(0.5)(x)
preds = tf.keras.layers.Dense(244, activation='softmax')(x)

model = tf.keras.models.Model(inputs=base_model.input, outputs=preds)

model.compile(optimizer=tf.keras.optimizers.Adam(lr=0.0001), loss='categorical_crossentropy', metrics=['accuracy'])

tensorboard_callback = tf.keras.callbacks.TensorBoard(log_dir='logs', histogram_freq=1)

early_stopping = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5)

model.fit(train_ds, epochs=100, validation_data=val_ds, callbacks=[early_stopping, tensorboard_callback])
model.save('TSC_full_v4.h5')