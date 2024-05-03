import tensorflow as tf
import numpy as np
import scipy

data_dir = r"C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round"
batch_size = 32
img_height = 224
img_width = 224

model = tf.keras.models.load_model('speed_classification.h5')

def load_and_preprocess_image(img_path):
    img = tf.keras.preprocessing.image.load_img(img_path, target_size=(img_height, img_width))
    img_array = tf.keras.preprocessing.image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0)
    img_array /= 255.
    return img_array

class_ind = {'100': 0, '130': 1, '30': 2, '40': 3, '40-': 4, '50': 5, '50-': 6, '60': 7, '60-': 8, '70': 9, '70-': 10, '80': 11, '90': 12}
class_ind = {v: k for k, v in class_ind.items()}
img_path = r'C:\Users\steam\OneDrive\Namizje\TF_Signs\Dataset\Round\40\new_40s.jpg'
img = load_and_preprocess_image(img_path)
predictions = model.predict(img)
predicted_class = np.argmax(predictions[0])
print(f"The predicted class is: {class_ind[predicted_class]}")