from settings import *
from PIL import Image
from tensorflow.keras.preprocessing.image import ImageDataGenerator
import numpy as np
from os import makedirs, listdir
from os.path import exists, isfile, basename
from tqdm import tqdm
import multiprocessing

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

def augment_image(image):
    factor = 2
    width, height = image.size
    new_width = int(width * factor)
    new_height = int(height * factor)
    upscaled_image = image.resize((new_width, new_height), Image.BICUBIC)

    image_array = np.array(upscaled_image) # Convert PIL image to numpy array

    # Reshape the image array to add batch dimension
    image_array = image_array.reshape((1,) + image_array.shape)

    # Augmentation
    datagen = ImageDataGenerator(
        rotation_range=10,  # Simulate slight rotation due to perspective changes
        width_shift_range=0.1,  # Simulate slight horizontal displacement
        height_shift_range=0.1,  # Simulate slight vertical displacement
        shear_range=5,  # Simulate shear due to viewing angle changes
        zoom_range=[1.5, 3.],  # Allow scaling up to 300% of original size
        brightness_range=[0.8, 1.2],  # Simulate changes in lighting conditions
        fill_mode='constant',
        cval=0)  # Value to fill with transparent

    # Generate augmented image
    augmented_image_array = next(datagen.flow(image_array, batch_size=1))[0].astype(np.uint8)

    # Convert back to PIL image
    augmented_image = Image.fromarray(augmented_image_array)
    return augmented_image

def select_environments(path = None):
    if path == None:
        if ONLY_ENVIRONMENTS != None:
            return ONLY_ENVIRONMENTS
        else:
            envs = [join(PATH_TO_ENV_FOLDER,f) for f in listdir(PATH_TO_ENV_FOLDER) if (isfile(join(PATH_TO_ENV_FOLDER, f)) and f.endswith(".jpg"))]
        return envs
    else:
        envs = [join(path,f) for f in listdir(path) if (isfile(join(path, f)) and f.endswith(".jpg"))]
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

def insert_signs_in_env(env_img, signs_imgs):
    env_img = env_img.convert("RGBA")
    result = Image.new("RGBA", env_img.size, (255, 255, 255, 0))

    covered = []

    for sign_img in signs_imgs:
        max_x = env_img.width - sign_img.width/2
        max_y = env_img.height - sign_img.height/2
        index = 0
        border = []
        
        while border == []:
            index += 1
            x = np.random.randint(sign_img.width/2, max_x)
            y = np.random.randint(sign_img.height/2, max_y)
            
            border = [(x-sign_img.width/2, y-sign_img.height/2), (x+sign_img.width/2, y+sign_img.height/2)]
            if not IMAGE_OVERLAY and index < 50000:
                for c in covered:
                    if inBounds(c, border):
                        border = []
                        break

        covered.append(border)

        factor = 1 + (abs(x - env_img.width / 2)/ (env_img.width / 2))

        scaled_sign_img = sign_img.resize((int(sign_img.width * factor), int(sign_img.height * factor)), Image.BICUBIC)

        try:
            result.paste(scaled_sign_img, (x-int(sign_img.width * factor/2), y-int(sign_img.height * factor/2)), scaled_sign_img)
        except ValueError:
            result.paste(scaled_sign_img, (x-int(sign_img.width * factor/2), y-int(sign_img.height * factor/2)), None)

    final_image = Image.alpha_composite(env_img, result)
    
    return final_image.convert("RGB")

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

def generate_images(env):
    global signs
    env_img = load_image(env, env = True)
    if env_img == None: 
            print("Skiping:",i)
            return
    name = basename(env).split(".", -1)[0]
    for i in range(NUM_OF_GEN_PHOTOS_PER_ENV):
        selected_signs = select_rand_signs(signs)
        signs_imgs = []
        for sign in selected_signs:
            sign_img = load_image(sign)
            signs_imgs.append(augment_image(sign_img))
        img = insert_signs_in_env(env_img, signs_imgs)
        img.save(join(PATH_TO_GEN_FOLDER,f"{name}_{i}.jpg"))

def workload_manager(multiProcess = False):
    global envs
    if not multiProcess:
        for env in tqdm(envs, desc='Generating', ncols=100):
            generate_images(env)
    else:
        pool = multiprocessing.Pool()
        pool.map(generate_images, envs)

signs = select_signs()
envs = select_environments()

if __name__ == "__main__":
    validateSettings()
    validateDir()
    workload_manager(True)
