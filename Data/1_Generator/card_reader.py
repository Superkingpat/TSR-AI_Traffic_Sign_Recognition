import tkinter as tk
from PIL import Image, ImageTk
import os
from settings import *

paths = []
paths_prev = []
points = []
curr_image = ""
img = None
photo = None

def get_paths():
    dir_list = os.listdir(PATH_TO_SIGN_FOLDER)
    global paths
    paths = [os.path.join(PATH_TO_SIGN_FOLDER, item) for item in dir_list if os.path.isfile(os.path.join(PATH_TO_SIGN_FOLDER, item))]

def get_coords(event):
    global img, canvas
    print(f"X: {event.x}, Y: {event.y}")
    print(img.getpixel([event.x, event.y]))
    if len(points) < 4:
        canvas.create_oval(event.x - 2, event.y - 2, event.x + 2, event.y + 2, fill='red')
        points.append([event.x, event.y])

def next_image():
    global photo, canvas, paths, paths_prev, curr_image, canvas

    if len(paths) > 0:
        paths_prev.append(paths.pop())
        curr_image = paths_prev[-1]
        photo = get_image(paths_prev[-1])
        canvas.create_image(0, 0, image=photo, anchor='nw')

def prev_image():
    global photo, canvas, paths, paths_prev, curr_image, canvas

    if len(paths_prev) > 0:
        paths.append(paths_prev.pop())
        curr_image = paths[-1]
        photo = get_image(paths[-1])
        canvas.create_image(0, 0, image=photo, anchor='nw')

def save_to_database():
    global points
    print(points)
    points = []
    print("Saved the image data")

def get_image(image_path):
    global img
    img = Image.open(image_path)

    base_width = 1000
    wpercent = (base_width / float(img.size[0]))
    hsize = int((float(img.size[1]) * float(wpercent)))
    new_size = (base_width, hsize)
    img = img.resize(new_size, Image.Resampling.LANCZOS)
    photo = ImageTk.PhotoImage(img)
    return photo

if __name__ == "__main__":
    get_paths()
    root = tk.Tk()
    root.geometry("1920x1080")
    root.title("Color Card Reader")

    paths_prev.append(paths.pop())
    curr_image = paths_prev[-1]
    photo = get_image(paths_prev[-1])

    canvas = tk.Canvas(root, width=photo.width(), height=photo.height())
    canvas.pack()

    canvas.create_image(0, 0, image=photo, anchor='nw')
    canvas.bind("<Button-1>", get_coords)

    btn_save = tk.Button(root, text='Save to database', bd='5', command=save_to_database)
    btn_save.pack(side='top')

    btn_prev = tk.Button(root, text='Previous', bd='5', command=prev_image)
    btn_prev.pack(side='top')

    btn_next = tk.Button(root, text='Next', bd='5', command=next_image)
    btn_next.pack(side='top')

    root.mainloop()
