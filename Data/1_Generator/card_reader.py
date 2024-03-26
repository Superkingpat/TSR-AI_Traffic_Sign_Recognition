import tkinter as tk
from PIL import Image, ImageTk
from settings import *

def print_coordinates(event):
    print(f"X: {event.x}, Y: {event.y}")

root = tk.Tk()
root.geometry("1920x1080")
root.title("Color Card Reader")

img = Image.open(r"C:\Data\lableimg\JPEGImages\JPEGImages\0000012.jpg")

base_width = 800
wpercent = (base_width / float(img.size[0]))
hsize = int((float(img.size[1]) * float(wpercent)))
new_size = (base_width, hsize)
img = img.resize(new_size, Image.Resampling.LANCZOS)

canvas = tk.Canvas(root, width=img.width, height=img.height)
canvas.pack()

photo = ImageTk.PhotoImage(img)

canvas.create_image(0, 0, image=photo, anchor='nw')
canvas.bind("<Button-1>", print_coordinates)

root.mainloop()