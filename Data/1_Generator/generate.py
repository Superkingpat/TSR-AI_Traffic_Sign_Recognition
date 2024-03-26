from settings import *
from PIL import Image

def load_environment_image(image_path):
    try:
        image = Image.open(image_path)   
        return image
    except FileNotFoundError:
        print("File not found.")
    except Exception as e:
        print("An error occurred:", e)

def generate_images():
    pass

def workload_manager():
    pass

if __name__ == "__main__":
    load_environment_image(DIRECTORY+"/test_env.jpg")
