import cv2
import os
import imageio.v2 as imageio_v2
from main import augment_images
from tqdm import tqdm
import multiprocessing
import time

data_IN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Signs")
data_OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "d1")

num_of_images = 1000

def augmenting_images(img_path):
    save_dir_path = os.path.join(data_OUT, img_path.replace("\\", "/").split("/")[-1].split(".")[0])

    if not os.path.exists(save_dir_path):
        os.makedirs(save_dir_path)

    imgs = augment_images(img_path, num_of_images)

    t = int(time.time())

    for j, img in enumerate(imgs):
            aug_img_rgb = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
            imageio_v2.imwrite(os.path.join(save_dir_path, f"aug_img{t}_{j}.jpg"), aug_img_rgb)
            #print(f"Created image: aug_img{j}.jpg")
    
def get_filename_without_extension(file_path):
    filename = os.path.basename(file_path)
    name_without_extension, _ = os.path.splitext(filename)
    
    return name_without_extension

def create_dataset(multiProcess = False):
    dirs_to_be = []
    file_paths = []
    for filename in os.listdir(data_IN):
        file_path = os.path.join(data_IN, filename)
        if os.path.isfile(file_path):
            dirs_to_be.append(get_filename_without_extension(file_path))
            file_paths.append(file_path)
    if not multiProcess:
        for file in tqdm(file_paths, desc='Generating', ncols=100):
            augmenting_images(file)
    else:
        pool = multiprocessing.Pool()
        pool.map(augmenting_images, file_paths)

if __name__ == "__main__":
    for i in range(9):
        create_dataset(True)
        print(i)
