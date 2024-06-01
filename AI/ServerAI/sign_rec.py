import tensorflow as tf
import scipy
import numpy as np
import os

data_dir = r"C:\Users\steam\OneDrive\Namizje\Sign_augmentation\gen_imgs"
batch_size = 32
img_height = 224
img_width = 224

'''data_gen = tf.keras.preprocessing.image.ImageDataGenerator(
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
)'''

def generate_class_mapping(n, k):
    k = max(k, 1)
    result_array = [num for num in range(1, k + 1) for _ in range(n)]
    return result_array

directory_path = ""
num_of_imgs = 0

img_array = None
classes = [os.path.splitext(file)[0] for file in os.listdir(directory_path) if os.path.isfile(os.path.join(directory_path, file))]
class_mapping = generate_class_mapping(num_of_imgs, 244)
img_array_tensor = tf.convert_to_tensor(img_array, dtype=tf.float32)
classes_tensor = tf.convert_to_tensor(classes, dtype=tf.int32)
class_mapping_tensor = tf.convert_to_tensor(class_mapping, dtype=tf.int32)
dataset = tf.data.Dataset.from_tensor_slices((img_array_tensor, classes_tensor, class_mapping_tensor))
dataset = dataset.shuffle(buffer_size=1024)
total_size = tf.data.experimental.cardinality(dataset).numpy()
train_size = int(total_size * 0.8)
val_size = total_size - train_size
train_dataset = dataset.take(train_size)
val_dataset = dataset.skip(train_size).take(val_size)

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

model.fit(train_dataset, epochs=100, validation_data=val_dataset, callbacks=[early_stopping, tensorboard_callback])
model.save('TSC_full_v4.h5')