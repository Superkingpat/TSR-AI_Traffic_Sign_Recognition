from settings import *
from PIL import Image
import numpy as np
from os import makedirs, listdir
from os.path import exists, isfile, basename
from tqdm import tqdm
import multiprocessing
from image_augmentor import augment_images_complete_arg, augment_images_arg

def create_dir(path):
    try:
        if not exists(path):
            makedirs(path)
    except OSError:
        print ('Error: Creating directory. ' +  path)

def validateSettings():
    print_string = "ERROR: Variable is not correct type:"
    if type(ONLY_ENVIRONMENTS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(ONLY_ENVIRONMENTS))
    if type(IGNORE_ENVIRONMENTS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(IGNORE_ENVIRONMENTS))
    """
    if type(IGNORE_TAGS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(IGNORE_TAGS))
    if type(ONLY_TAGS) not in [type(None), type([])]:
        print(print_string, "ONLY_ENVIRONMENTS || ", type(ONLY_TAGS))
    """

def validateDir():
    create_dir(PATH_TO_GEN_FOLDER)

def select_environments(path = None):
    if path == None:
        if ONLY_ENVIRONMENTS != None:
            return ONLY_ENVIRONMENTS
        else:
            envs = [f for f in listdir(PATH_TO_ENV_FOLDER) if (isfile(join(PATH_TO_ENV_FOLDER, f)) and f.endswith(".jpg"))]
        return envs
    else:
        envs = [f for f in listdir(path) if (isfile(join(path, f)) and f.endswith(".jpg"))]
        return envs

def select_signs():
    if ONLY_SIGNS != None:
        return ONLY_SIGNS
    else:
        signs = [join(PATH_TO_SIGN_FOLDER,f) for f in listdir(PATH_TO_SIGN_FOLDER) if (isfile(join(PATH_TO_SIGN_FOLDER, f)) and f.endswith(".png"))]
    return signs

def inBounds(first, sec):
    x1, y1 = first[0]
    x2, y2 = first[1]
    
    x3, y3 = sec[0]
    x4, y4 = sec[1]
    
    if x2 < x3 or x4 < x1 or y2 < y3 or y4 < y1: return False
    else: return True

def load_image(image_path, env = False):
    try:
        image = Image.open(image_path)
        # Fix orientation
        try: 
            exif = image._getexif()
            orientation = exif.get(0x0112)
            if orientation == 3:
                image = image.rotate(180, expand=True)
            elif orientation == 6:
                image = image.rotate(270, expand=True)
            elif orientation == 8:
                image = image.rotate(90, expand=True)
        except (AttributeError, KeyError, IndexError):
            pass
    
        if env:
            width, height = image.size
            if width < height:
                size = (1080, 1920)
            else:
                size = (1920, 1080)
            image = image.resize(size, Image.BICUBIC)
        return image
    except FileNotFoundError:
        print("File not found.")
        return None
    
    except Exception as e:
        print("An error occurred:", e)
        return None

def select_rand_signs(signs):
    rand = np.random.randint(NUM_OF_SIGNS_PER_IMG[0], NUM_OF_SIGNS_PER_IMG[1])
    return np.random.choice(np.array(signs), size=rand, replace=False)

def workload_manager(multiProcess = False):
    global envs
    if not multiProcess:
        for env in tqdm(envs, desc='Generating', ncols=100):
            augment_images_complete_arg((signs, 5, env, (PATH_TO_ENV_FOLDER,PATH_TO_MARKED_ENV_FOLDER), False))
    else:
        args = []
        for env in envs:
            args.append((signs, 5, env, (PATH_TO_ENV_FOLDER,PATH_TO_MARKED_ENV_FOLDER), False))
        pool = multiprocessing.Pool()
        pool.map(augment_images_complete_arg, args)

signs = select_signs()
envs = select_environments()

if __name__ == "__main__":
    validateSettings()
    validateDir()
    workload_manager(True)
