import os
import time
import cv2
import random

TRAIN_DIR = r"dataset\train"
VALIDATION_DIR = r"dataset\valid"
TEST_DIR = r"dataset\test"
IMAGE_DIR = "images"
LABEL_DIR = "labels"

def save_labels(save_dir, img_name, x_centers_array, y_centers_array, widths, heigths):
    file = open(os.path.join(save_dir, f"labels/{img_name}.txt"), "w")
    for i in range(len(x_centers_array)):
        file.write(f"0 {x_centers_array[i]} {y_centers_array[i]} {widths[i]} {heigths[i]}\n")
    file.close()

def create_dir(path):
    try:
        if not os.path.exists(path):
            os.makedirs(path)
    except OSError:
        print ('Error: Creating directory. ' +  path)

def create_dataset(images, xy_pos, width_height):
    """
    - images - an array of multiple images
    - sign_x_centers - an array of arrays. The sub arrays are for each inwidual image
    - sign_y_centers - the same as for sign_x_centers bu for y
    - sign_widths - array of arrays for the widths of the signs
    - sign_heights - same as for sign_widths but for heights
    """
    create_dir(os.path.join(TRAIN_DIR, IMAGE_DIR))
    create_dir(os.path.join(TRAIN_DIR, LABEL_DIR))
    create_dir(os.path.join(VALIDATION_DIR, IMAGE_DIR))
    create_dir(os.path.join(VALIDATION_DIR, LABEL_DIR))
    create_dir(os.path.join(TEST_DIR, IMAGE_DIR))
    create_dir(os.path.join(TEST_DIR, LABEL_DIR))

    time_now = time.time()
    save_dir = ""
    for i in range(len(images)):
        rand_save = random.randint(1, 10)
        if rand_save == 1 or rand_save == 3:
            save_dir = TEST_DIR
        elif rand_save == 2 or rand_save == 4:
            save_dir = VALIDATION_DIR
        else:
            save_dir = TRAIN_DIR

        img_name = f"{time_now}_{i}.jpg"
        cv2.imwrite(os.path.join(os.path.join(save_dir, IMAGE_DIR), img_name), images[i])
        save_labels(save_dir=save_dir, img_name=img_name, x_centers_array=xy_pos[i][:,0]/images[i].shape[0], y_centers_array=xy_pos[i][:,1]/images[i].shape[1], widths=width_height[i][:,0]/images[i].shape[0], heigths=width_height[i][:,1]/images[i].shape[1])