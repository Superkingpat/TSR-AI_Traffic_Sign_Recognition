import cv2
import os
import imageio.v2 as imageio_v2
from main import augment_images

def augmenting_images(img_path, save_dir_path, num_of_images):
    if not os.path.exists(save_dir_path):
        os.makedirs(save_dir_path)

    imgs = augment_images(img_path, num_of_images)

    for j, img in enumerate(imgs):
            aug_img_rgb = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
            imageio_v2.imwrite(os.path.join(save_dir_path, f"aug_img{j}.jpg"), aug_img_rgb)
            print(f"Created image: aug_img{j}.jpg")
    
    
def get_filename_without_extension(file_path):
    filename = os.path.basename(file_path)
    name_without_extension, _ = os.path.splitext(filename)
    
    return name_without_extension

def create_dataset(dataset_dir, save_dir, num_of_images):
    dirs_to_be = []
    file_paths = []
    for filename in os.listdir(dataset_dir):
        file_path = os.path.join(dataset_dir, filename)
        if os.path.isfile(file_path):
            dirs_to_be.append(get_filename_without_extension(file_path))
            file_paths.append(file_path)
    
    for i in range(len(file_paths)):
        augmenting_images(file_paths[i], os.path.join(save_dir, dirs_to_be[i]), num_of_images)

create_dataset(r"C:\Projects\AI_TSR\TSR-AI_Traffic_Sign_Recognition\Data\1_Generator\Signs", r"C:\Users\steam\OneDrive\Namizje\Sign_augmentation\gen_imgs", 5)