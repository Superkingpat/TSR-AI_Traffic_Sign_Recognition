from generate import load_image, create_dir
from os.path import join, basename
from os import walk, remove
from tqdm import tqdm
from settings import DIR, PATH_TO_ENV_FOLDER
import shutil


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
    normalize_path = join(DIR, "Environment/e1")
    fix_path = join(DIR, "Environment/fix")
    create_dir(normalize_path)
    create_dir(fix_path)
    create_dir(PATH_TO_ENV_FOLDER)
    normalize_images(normalize_path, PATH_TO_ENV_FOLDER, fix_path)