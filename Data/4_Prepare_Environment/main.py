from os.path import join, basename
from os import walk, remove
from tqdm import tqdm
from PIL import Image
import shutil
from os import makedirs, remove
from os.path import join, abspath, exists, dirname

#####    REQUIRES DATA_IN FOLDER !!!!!    #####

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

def create_dir(path):
    try:
        if not exists(path):
            makedirs(path)
    except OSError:
        print ('Error: Creating directory. ' +  path)

def normalize_images(pathIN, pathOUT, pathFIX):
    imgs = [join(r,i) for r,_,f in walk(pathIN) for i in f]
    for i in tqdm(imgs, desc='Normalizing', ncols=100):
        img = load_image(i, True)
        if img == None: 
            print("Skiping:",i)
            shutil.move(i, join(pathFIX, basename(i)))
            continue
        img.save(join(pathOUT, f"{basename(i).split(".", -1)[0]}.jpg"))
        remove(i)

if __name__ == "__main__":
    data_in = join(dirname(abspath(__file__)), "data_IN")
    data_out = join(dirname(abspath(__file__)), "data_OUT")
    data_fix = join(dirname(abspath(__file__)), "data_FIX")
    create_dir(data_in)
    create_dir(data_fix)
    create_dir(data_out)
    normalize_images(data_in, data_out, data_fix)