import tkinter as tk
from PIL import Image, ImageTk, ImageTransform
import os
import numpy as np
from settings import *
import DBManager
import pickle
import sqlite3

paths = []
paths_prev = []
points = []
curr_image = ""
img = None
photo = None
rgb = None
checkbox_vars = []
tag_vars = []
rows = None

def generate_cc():
    rgb_matrix_1 = np.zeros((7, 11, 3), dtype=np.uint8)
    rgb_matrix_2 = np.zeros((7, 11, 3), dtype=np.uint8)

    for i in range(11):
        rgb_matrix_1[0][i] = [255, 255 - 25.5*i, 255 - 25.5*i]
        rgb_matrix_1[1][i] = [255 - 25.5*i, 255, 255 - 25.5*i]
        rgb_matrix_1[2][i] = [255 - 25.5*i, 255 - 25.5*i, 255]
        rgb_matrix_1[3][i] = [255 - 25.5*i, 0, 0]
        rgb_matrix_1[4][i] = [0, 255 - 25.5*i, 0]
        rgb_matrix_1[5][i] = [0, 0, 255 - 25.5*i]
        rgb_matrix_1[6][i] = [255 - 25.5*i, 255 - 25.5*i, 255 - 25.5*i]
        rgb_matrix_2[0][i] = [255, 255 - 25.5*i, 255]
        rgb_matrix_2[1][i] = [255, 255, 255 - 25.5*i]
        rgb_matrix_2[2][i] = [255 - 25.5*i, 255, 255]
        rgb_matrix_2[3][i] = [255 - 25.5*i, 0, 255 - 25.5*i]
        rgb_matrix_2[4][i] = [255 - 25.5*i, 255 - 25.5*i, 0]
        rgb_matrix_2[5][i] = [0, 255 - 25.5*i, 255 - 25.5*i]
        rgb_matrix_2[6][i] = [255 - 25.5*i, 255 - 25.5*i, 255 - 25.5*i]

    return rgb_matrix_1, rgb_matrix_2

def sort_points(points):
    pointArr = np.array(points)
    sorted_indices = np.lexsort((pointArr[:, 1], pointArr[:, 0]))
    sorted_points = pointArr[sorted_indices]
    points_left = sorted_points[:2][sorted_points[:2][:, 1].argsort()]
    points_right = sorted_points[2:][sorted_points[2:][:, 1].argsort()]
    return np.array([points_left, points_right])

def get_paths():
    #dir_list = os.listdir(PATH_TO_SIGN_FOLDER)
    global paths
    conn = sqlite3.connect(os.path.join(PATH_TO_ENV_FOLDER,DATABASE_NAME))
    cursor = conn.cursor()
    cursor.execute(f"SELECT fileName FROM EnvConfig")
    files = cursor.fetchall()
    print(files)
    cursor.close()
    conn.close()
    paths = [os.path.join(PATH_TO_ENV_FOLDER, item[0]) for item in files if os.path.isfile(os.path.join(PATH_TO_ENV_FOLDER, item[0]))]

def get_coords(event):
    global img, canvas
    if len(points) < 8:
        canvas.create_oval(event.x - 2, event.y - 2, event.x + 2, event.y + 2, fill='red')
        points.append([event.x, event.y])

def next_image():
    global photo, canvas, paths, paths_prev, curr_image, canvas, points
    points = []
    if len(paths) > 0:
        paths_prev.append(paths.pop())
        curr_image = paths_prev[-1]
        photo = get_image(paths_prev[-1])
        canvas.create_image(0, 0, image=photo, anchor='nw')

def prev_image():
    global photo, canvas, paths, paths_prev, curr_image, canvas, points
    points = []
    if len(paths_prev) > 0:
        paths.append(paths_prev.pop())
        curr_image = paths[-1]
        photo = get_image(paths[-1])
        canvas.create_image(0, 0, image=photo, anchor='nw')

def distance(point1, point2):
    return np.linalg.norm(np.array(point1) - np.array(point2))

def get_color_card():
    global points, img, rgb
    pointArr = sort_points(points[:4])
    pointArr1 = sort_points(points[4:])
    print(pointArr)
    print(pointArr1)

    len_x = distance(pointArr[0][0], pointArr[1][0])
    len_y = distance(pointArr[0][0], pointArr[0][1])

    len_x1 = distance(pointArr1[0][0], pointArr1[1][0])
    len_y1 = distance(pointArr1[0][0], pointArr1[0][1])

    result = img.transform((int(len_x), int(len_y)), ImageTransform.QuadTransform(
                                                                                [pointArr[0][0][0], pointArr[0][0][1],
                                                                                pointArr[0][1][0], pointArr[0][1][1],
                                                                                pointArr[1][1][0], pointArr[1][1][1],
                                                                                pointArr[1][0][0], pointArr[1][0][1]]))
    result1 = img.transform((int(len_x1), int(len_y1)), ImageTransform.QuadTransform(
                                                                                [pointArr1[0][0][0], pointArr1[0][0][1],
                                                                                pointArr1[0][1][0], pointArr1[0][1][1],
                                                                                pointArr1[1][1][0], pointArr1[1][1][1],
                                                                                pointArr1[1][0][0], pointArr1[1][0][1]]))
    
    result = result.convert('RGB')
    result1 = result1.convert('RGB')
    square_width = result.width // 11
    square_height = result.height // 7
    rgb_matrix = np.zeros((7, 11, 3), dtype=np.uint8)
    rgb_matrix1 = np.zeros((7, 11, 3), dtype=np.uint8)

    for i in range(7):
        start_y = square_height / 2 + square_height * i
        for j in range(11):
            start_x = square_width / 2 + square_width * j
            pixel = result.getpixel((int(start_x), int(start_y)))
            pixel1 = result1.getpixel((int(start_x), int(start_y)))
            rgb_matrix[i, j] = pixel
            rgb_matrix1[i, j] = pixel1
            
    
    rgb = { }
    cc_1, cc_2 = generate_cc()

    for i in range(7):
        for j in range(11):
            if tuple(cc_1[i][j]) in rgb:
                rgb[tuple(cc_1[i][j])] = (rgb[tuple(cc_1[i][j])] +  rgb_matrix[i, j]) / 2
            else:
                rgb[tuple(cc_1[i][j])] = rgb_matrix[i, j]
            if tuple(cc_2[i][j]) in rgb:
                rgb[tuple(cc_2[i][j])] = (rgb[tuple(cc_2[i][j])] +  rgb_matrix1[i, j]) / 2
            else:
                rgb[tuple(cc_2[i][j])] = rgb_matrix1[i, j]

    for key in rgb:
        rgb[key] = rgb[key].astype(int)

    rgb_matrix = tuple([rgb_matrix, rgb_matrix1])

    print(rgb)
    clear()
    draw_rectangles(rgb_matrix)

def draw_rectangles(rgb_tuple):
    global canvas_col
    canvas_width = canvas_col.winfo_width()
    canvas_height = canvas_col.winfo_height()
    num_rows = len(rgb_tuple[0])
    num_cols = len(rgb_tuple[0][0])
    part_width = canvas_width / num_cols
    part_height = canvas_height / (2 * num_rows)
    
    for k in range(2):
        for i in range(num_rows):
            for j in range(num_cols):
                x1 = j * part_width
                y1 = (i + k * num_rows) * part_height
                x2 = (j + 1) * part_width
                y2 = (i + 1 + k * num_rows) * part_height
                r, g, b = rgb_tuple[k][i][j]
                color = "#%02x%02x%02x" % (r, g, b)
                canvas_col.create_rectangle(x1, y1, x2, y2, fill=color, outline="")

def save_to_database():
    global points, img, tag_vars, checkbox_vars
    points = []
    
    conn = sqlite3.connect(os.path.join(PATH_TO_ENV_FOLDER,DATABASE_NAME))
    cursor = conn.cursor()
    cursor.execute(f"SELECT idEnv FROM EnvConfig WHERE fileName = '{os.path.basename(curr_image)}'")
    id = cursor.fetchall()[0][0]
    cursor.close()
    conn.close()

    for i in tag_vars:
        DBManager.execute("INSERT INTO EnvTags VALUES (?, ?)",  [id, i])

    DBManager.execute("UPDATE EnvConfig SET color = ? WHERE fileName = ?",  [pickle.dumps(rgb), os.path.basename(curr_image)])    
    tag_vars = []

    for vars in checkbox_vars:
        vars.set(False)

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

def clear():
    global img, canvas, photo, points
    canvas.create_image(0, 0, image=photo, anchor='nw')
    points = []

def checkbox_checked():
    global tag_vars
    for i in range(len(checkbox_vars)):
        if checkbox_vars[i].get() == 1:
            if rows[i][0] not in tag_vars:
                tag_vars.append(rows[i][0])
        else:
            if rows[i][0] in tag_vars:
                tag_vars.remove(rows[i][0])

def init_checkboxes():
    global checkbox_vars, rows, tag_vars
    tag_vars = []
    conn = sqlite3.connect(os.path.join(PATH_TO_ENV_FOLDER,DATABASE_NAME))
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM TagTable")
    rows = cursor.fetchall()

    for i in rows:
        var = tk.IntVar()
        checkbox_vars.append(var)
        checkbox = tk.Checkbutton(root, text=f"{i[1]}", variable=var, command=checkbox_checked)
        checkbox.pack(side='top')

    cursor.close()
    conn.close()

if __name__ == "__main__":
    generate_cc()
    get_paths()
    root = tk.Tk()
    init_checkboxes()
    root.geometry("1920x1080")
    root.title("Color Card Reader")

    paths_prev.append(paths.pop())
    curr_image = paths_prev[-1]
    photo = get_image(paths_prev[-1])

    canvas = tk.Canvas(root, width=photo.width(), height=photo.height())
    canvas.pack(side='left')

    canvas_col = tk.Canvas(root, width=440, height=560,  bg="blue")
    canvas_col.pack(side='right')

    canvas.create_image(0, 0, image=photo, anchor='nw')
    canvas.bind("<Button-1>", get_coords)

    btn_get = tk.Button(root, text='Get CC', bd='5', command=get_color_card)
    btn_get.pack(side='top')

    btn_save = tk.Button(root, text='Save to database', bd='5', command=save_to_database)
    btn_save.pack(side='top')

    btn_prev = tk.Button(root, text='Previous', bd='5', command=prev_image)
    btn_prev.pack(side='top')

    btn_next = tk.Button(root, text='Next', bd='5', command=next_image)
    btn_next.pack(side='top')

    btn_clear = tk.Button(root, text='Clear', bd='5', command=clear)
    btn_clear.pack(side='top')

    root.mainloop()
