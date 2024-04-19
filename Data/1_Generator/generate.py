from settings import *
from PIL import Image
from DBManager import execute
import Augmentor as ag
import numpy as np

def validateSettings():
    print_string = "ERROR: Variable is not correct type:"
    if type(ONLY_ENVIRONMENTS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(ONLY_ENVIRONMENTS))
    if type(IGNORE_ENVIRONMENTS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(IGNORE_ENVIRONMENTS))
    if type(IGNORE_TAGS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(IGNORE_TAGS))
    if type(ONLY_TAGS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(ONLY_TAGS))

def augmentImage(image):
    image_directory = r"C:\Data\lableimg\TF_images\predict\crops\Stop"
    output_directory = r"C:\Data\lableimg\TF_images\predict\crops\aug_Stop"
    aug = ag.Pipeline(image_directory, output_directory=output_directory, save_format="JPEG")
    aug.skew(probability=0.7, magnitude=0.7)
    aug.random_distortion(probability=0.6, magnitude=2, grid_width=6, grid_height=6)
    aug.rotate(probability=0.6, max_left_rotation=25, max_right_rotation=25)
    aug.resize(probability=1, width=140, height=140)
    aug.sample(100)

def augment_image(image):
    
    return image


def select_environments():
    if ONLY_ENVIRONMENTS != None:
        return ONLY_ENVIRONMENTS
    else:
        dict = execute(":out := SELECT fileName FROM EnvConfig")
        items = np.array(dict[":out"]).flatten()
    sql_script = ""

def select_signs():
    pass

def load_environment_image(image_path):
    try:
        image = Image.open(image_path)   
        return image
    except FileNotFoundError:
        print("File not found.")
    except Exception as e:
        print("An error occurred:", e)

def generate_images(signs, env_img):

    pass

def workload_manager():
    #image = load_environment_image()
    pass 

if __name__ == "__main__":
    validateSettings()
    select_environments()
    #workload_manager()
