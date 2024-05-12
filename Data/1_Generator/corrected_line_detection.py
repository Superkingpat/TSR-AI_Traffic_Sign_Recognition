import cv2
import numpy as np
import random
import os
from numba import njit
import matplotlib.pyplot as plt

line_coords = None


def load_image(path = None):
    global line_coords
    if os.path.isfile(path):
        bgr_image = cv2.imread(path)

        if bgr_image is None:
            raise ValueError(f"Failed to load image from path: {path}")
        else:
            img_height, img_width, _ = bgr_image.shape
            image = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2RGB)
            line_coords = detect(image)
    else:
        print(f"{path} is not a valid path.")


@njit
def detect(image):
    width = image.shape[1]
    line_coords = []

    for i in range(width):
        end_col = i + 1
        slice_image = image[:, i:end_col]

        in_range = False
        for j in range(len(slice_image)):
            pixel = slice_image[j][0]
            if (pixel[0] < 60) and (pixel[1] > 200) and (pixel[2] < 60):
                if not in_range:
                    line_coords.append(np.array([j, i]))
                    in_range = True
            else:
                in_range = False
                
    return line_coords
    
def get_random_position(destroy_surroundings=False, destruction_radius=20):
    global line_coords
    if len(line_coords) == 0:
        return ()

    rand_index = random.randint(0, len(line_coords) - 1)
    random_position = tuple(line_coords[rand_index])

    if destroy_surroundings:
        to_remove = []
        new_line_coords = []
        for coord in line_coords:
            if np.abs(coord[0] - random_position[0]) > destruction_radius and np.abs(coord[1] - random_position[1]) > destruction_radius:
                new_line_coords.append(coord)
            else:
                to_remove.append(coord)

        line_coords = np.array(new_line_coords)

    return random_position

def get_n_random_positions(num_of_points, destroy_surroundings=False, destruction_radius=20):
    n_position = []
    for _ in range(num_of_points):
        point = get_random_position(destroy_surroundings, destruction_radius)
        if point == ():
            return n_position
        n_position.append(point)
    return n_position


load_image(r"C:\Users\steam\OneDrive\Namizje\line_detection\test1.webp")
print(get_n_random_positions(20, True))

image = cv2.imread(r"C:\Users\steam\OneDrive\Namizje\line_detection\test1.webp")
for point in line_coords:
    cv2.circle(image, (int(point[1]), int(point[0])), 3, (255, 0, 0), -1)

image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

plt.imshow(image_rgb)
for point in line_coords:
    plt.plot(point[1], point[0], 'bo')
plt.title("Image with Points")
plt.show()