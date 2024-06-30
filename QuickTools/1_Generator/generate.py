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

def select_signs(path=None):
    if path == None:
        if ONLY_SIGNS != None:
            return ONLY_SIGNS
        else:
            signs = [join(PATH_TO_SIGN_FOLDER,f) for f in listdir(PATH_TO_SIGN_FOLDER) if (isfile(join(PATH_TO_SIGN_FOLDER, f)) and f.endswith(".png"))]
        return signs
    else:
        signs = [join(PATH_TO_SIGN_FOLDER,f) for f in listdir(PATH_TO_SIGN_FOLDER) if (isfile(join(PATH_TO_SIGN_FOLDER, f)) and f.endswith(".png"))]
        return signs

def select_rand_elements(arr, rand_num=None):
    if rand_num == None:
        rand_num = np.random.randint(0, len(arr))
    return np.random.choice(np.array(arr), size=rand_num, replace=False)

def workload_manager_recognition(multiProcess=False, 
                                 envs=select_environments(), 
                                 signs=select_signs(), 
                                 max_signs_per_image=5, 
                                 path_to_env_folder=PATH_TO_ENV_FOLDER, 
                                 path_to_marked_env_folder=PATH_TO_MARKED_ENV_FOLDER):
    if not multiProcess:
        for env in tqdm(envs, desc='Generating', ncols=100):
            augment_images_complete_arg((signs, max_signs_per_image, env, (path_to_env_folder, path_to_marked_env_folder), False))
    else:
        args = []
        for env in envs:
            args.append((signs, max_signs_per_image, env, (path_to_env_folder, path_to_marked_env_folder), False))
        pool = multiprocessing.Pool()
        pool.map(augment_images_complete_arg, args)

def workload_manager_classification(multiProcess=False,
                                    envs=select_environments(), 
                                    signs=select_signs(), 
                                    num_of_images_per_sign=1000, 
                                    path_to_env_folder=PATH_TO_ENV_FOLDER, 
                                    path_to_marked_env_folder=PATH_TO_MARKED_ENV_FOLDER):
    if not multiProcess:
        for sig in tqdm(signs, desc='Generating', ncols=100):
            augment_images_arg((sig, num_of_images_per_sign, envs, (path_to_env_folder,path_to_marked_env_folder), False))
    else:
        args = []
        for sig in signs:
            args.append((sig, num_of_images_per_sign, envs, (path_to_env_folder,path_to_marked_env_folder), False))
        pool = multiprocessing.Pool()
        pool.map(augment_images_arg, args)


if __name__ == "__main__":
    validateSettings()
    validateDir()
    for i in range (20):
        print(i)
        workload_manager_recognition(False)
