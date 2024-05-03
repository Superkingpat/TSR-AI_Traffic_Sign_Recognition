import tensorflow as tf
import scipy

data_dir = r"C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round"
batch_size = 32
img_height = 224
img_width = 224

data_gen = tf.keras.preprocessing.image.ImageDataGenerator(
    rescale=1./255,
    rotation_range=20,
    width_shift_range=0.2,
    height_shift_range=0.2,
    shear_range=0.2,
    zoom_range=0.2,
    fill_mode='nearest',
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

base_model = tf.keras.applications.MobileNetV2(weights='imagenet', include_top=False, input_shape=(img_height, img_width, 3))

base_model.trainable = False

x = base_model.output
x = tf.keras.layers.GlobalAveragePooling2D()(x)

x = tf.keras.layers.Dense(1024, activation='relu')(x)
x = tf.keras.layers.Dropout(0.5)(x)

predictions = tf.keras.layers.Dense(13, activation='softmax')(x)

model = tf.keras.models.Model(inputs=base_model.input, outputs=predictions)

model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
              loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
              metrics=['accuracy'])

early_stopping = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5)

model.fit(train_ds, epochs=100, validation_data=val_ds, callbacks=[early_stopping])
model.save('speed_classification.h5')
class_names = train_ds.class_indices
print(class_names)