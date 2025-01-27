import os
import cv2
import numpy as np

def stack_images_in_folder(folder_path, output_path="stacked_image.jpg"):
    # Load all images from the folder
    images = []
    num_images = len(os.listdir(folder_path))
    num_cols = int(np.sqrt(num_images * 16/9))
    num_rows = int(np.ceil(num_images / num_cols))
    
    # Resize images to fit 2K resolution
    resize_width = 2000 // num_cols
    resize_height = 1000 // num_rows
    
    for filename in os.listdir(folder_path):
        filepath = os.path.join(folder_path, filename)
        img = cv2.imread(filepath)
        img = cv2.resize(img, (resize_width, resize_height))
        if img is not None:
            images.append(img)

    if not images:
        print("No images found in the folder.")
        return

    # Get number of images and aspect ratio
    #aspect_ratio = images[0].shape[1] / images[0].shape[0]  # width / height
    
    # Calculate grid dimensions
    resized_images = images
    #resized_images = [cv2.resize(img, (resize_width, resize_height)) for img in images]
    # Stack images into a grid
    stacked_rows = []
    for i in range(num_rows):
        row_images = resized_images[i * num_cols:(i + 1) * num_cols]
        # Fill the row with blank images if it's incomplete
        if len(row_images) < num_cols:
            
            blank_image = np.zeros((resize_height, resize_width, 3), dtype=np.uint8)
            row_images.extend([blank_image] * (num_cols - len(row_images)))  
        stacked_rows.append(np.hstack(row_images))

    # Combine rows into the final grid
    grid_image = np.vstack(stacked_rows)
    
    # Save the resulting image
    cv2.imwrite(output_path, grid_image)
    print(f"Stacked image saved at {output_path}, {len(resized_images)}")

# Usage
folder_path = "data_OUT"
stack_images_in_folder(folder_path)
