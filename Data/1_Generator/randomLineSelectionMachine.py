import cv2
import numpy as np
import random
import os

class RandomLineSelectionMachine:
    """
    A class for selecting random positions of lines in an image based on color criteria.

    Parameters:
    path (str): Path to the image file.
    """
    def __init__(self, path = None):
        if os.path.isfile(path):
            self.bgr_image = cv2.imread(path)

            if self.bgr_image is None:
                raise ValueError(f"Failed to load image from path: {path}")
            else:
                self.img_height, self.img_width, _ = self.bgr_image.shape
                self.image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2RGB)
                self.line_coords = {}
                self.__detect()
        else:
            print(f"{path} is not a valid path.")

    def load_image(self, path = None):
        """
        Load and set the image from the specified path.

        Parameters:
        - path (str, optional): The path to the image file. If None, the image will not be loaded.

        Raises:
        - ValueError: If the image loading fails or the specified path is not a valid file.
        """
        if os.path.isfile(path):
            self.bgr_image = cv2.imread(path)

            if self.bgr_image is None:
                raise ValueError(f"Failed to load image from path: {path}")
            else:
                self.img_height, self.img_width, _ = self.bgr_image.shape
                self.image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2RGB)
                self.line_coords = {}
                self.__detect()
        else:
            print(f"{path} is not a valid path.")

    def __detect(self):
        """
        Detect lines in the image based on color criteria.

        Populates the line_coords dictionary with coordinates of detected lines.
        
        This method iterates over the columns of the image and checks each pixel in the vertical slice 
        to determine if it falls within a specified color range (green color in this case).
        Detected line positions (column and row indices) are stored in the line_coords dictionary.

        Parameters:
        None

        Returns:
        None
        """
        self.line_coords = {}

        for i in range(self.image.shape[1]):
            end_col = i + 1
            slice_image = self.image[:, i:end_col]

            in_range = False
            for j in range(len(slice_image)):
                pixel = slice_image[j][0]
                if (pixel[0] < 60) and (pixel[1] > 200) and (pixel[2] < 60):
                    if not in_range:
                        if i not in self.line_coords:
                            self.line_coords[i] = []
                        self.line_coords[i].append(j)
                        in_range = True
                else:
                    in_range = False

    
    def display(self, show_points = True):
        """
        Display the image with detected line positions marked.

        Parameters:
        show_points (bool, optional): Whether to display points at detected line positions.
            If True (default), circles will be drawn at detected line positions.\n
            If False, only the image will be displayed without any markings.

        Returns:
        None
        """
        temp_mgr_image = self.bgr_image.copy()
        if show_points:
            for i in self.line_coords:
                for j in self.line_coords[i]:
                    x = i * 1
                    y = j
                    cv2.circle(temp_mgr_image, (x, y), radius=3, color=(255, 0, 0), thickness=-1)

        cv2.imshow('Detected Positions', temp_mgr_image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    
    def get_all_positions(self):
        """
        Retrieve all detected line positions as a list of tuples.

        Returns:
        - list of tuples: A list containing all detected line positions.
                          Each tuple represents a line position (column_index, row_index).

        If no line coordinates are available (empty dictionary), returns an empty list [].
        """
        if len(self.line_coords) == 0:
            return []

        all_coords = []
        for key, value in self.line_coords.items():
            for val in value:
                all_coords.append((key, val))
        return all_coords
    
    def get_random_position(self, destroy_surroundings=False, destruction_radius=20):
        """
        Retrieve a random position from the line coordinates.

        Parameters:
        - destroy_surroundings (bool): If True, remove nearby line coordinates within the destruction radius.
        - destruction_radius (int): The radius within which surrounding line coordinates will be destroyed.

        Returns:
        - tuple: A tuple representing the random position, formatted as (key, value).
            If no line coordinates are available (empty dictionary), returns an empty tuple ().
        """
        if len(self.line_coords) == 0:
            return ()

        rand_key = random.choice(list(self.line_coords.keys()))
        random_position = (rand_key, random.choice(self.line_coords[rand_key]))

        if destroy_surroundings:
            to_remove = []
            for key, value in self.line_coords.items():
                if np.abs(key - random_position[0]) < destruction_radius:
                    filtered_values = [val for val in value if np.abs(val - random_position[1]) > destruction_radius]
                    self.line_coords[key] = filtered_values
                    if not filtered_values:
                        to_remove.append(key)

            for key in to_remove:
                del self.line_coords[key]

        return random_position
    
    def get_n_random_positions(self, num_of_points, destroy_surroundings=False, destruction_radius=20):
        """
        This method selects a specified number of random line positions (column and row indices) from the detected coordinates of lines in the image.

        Parameters:
        - num_of_points (int): Number of random line positions to retrieve.
        - destroy_surroundings (bool, optional): If True, nearby line coordinates within the destruction radius will be removed (default: False).
        - destruction_radius (int, optional): Radius around each selected position within which surrounding line coordinates will be destroyed if 'destroy_surroundings' is True (default: 20).

        Returns:
        - List[Tuple[int, int]]: List of tuples representing the randomly selected line positions. Each tuple contains a column index and a row index.
        """
        n_position = []
        for _ in range(num_of_points):
            point = self.get_random_position(destroy_surroundings, destruction_radius)
            if point == ():
                return n_position
            n_position.append(point)
        return n_position
    

if __name__ == "__main__":
    img = RandomLineSelectionMachine(r'C:\Users\steam\OneDrive\Namizje\line_detection\test1.webp')

    print(img.get_random_position(True, 10))
    img.display()

    print(img.get_random_position(True, 10))
    img.display()

    img.load_image(r"C:\Users\steam\OneDrive\Namizje\line_detection\Untitled.jpg")

    print(img.get_random_position(True, 10))
    img.display()

    print(img.get_random_position(True, 10))
    img.display()

    print(img.get_n_random_positions(40, True, 10))
