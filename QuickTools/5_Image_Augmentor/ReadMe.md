# Image Augmentation Script

This script applies various augmentations to an input image using `imgaug` and `opencv-python`, producing multiple augmented images.

## Requirements

- Python 3.x
- `numpy`
- `imgaug`
- `opencv-python`

## Setup

1. **Download the script**

    Download the script file and place it in your desired directory.

2. **Install required Python libraries**

    Install the necessary Python libraries using pip:

    ```sh
    pip install numpy imgaug opencv-python
    ```

## Usage

1. **Prepare your input image**

    - Specify the path to the input image (`image_path`).
    - Determine the number of augmented images to generate (`num_of_images`).

2. **Run the script**

    Execute the script to generate augmented images:

    ```sh
    python augment_images.py
    ```

    - Adjust parameters within the script to customize augmentations or save images to disk.

## Augmentation Pipeline

The augmentation pipeline includes various transformations:

- **Color Adjustments**: Modifies saturation, brightness, hue, and contrast of images.
- **Obstructions**: Introduces cutout obstructions to images.
- **Blur**: Applies Gaussian blur, motion blur, or leaves images unaltered.
- **Transformations**: Includes scaling, perspective transform, shearing, rotation, and cropping.
- **Segmentations**: Utilizes techniques like Voronoi diagrams or average pooling.
- **Weather Simulation**: Simulates weather effects such as clouds, snowflakes, fog, and rain.

## Troubleshooting

- Verify that the input image path (`image_path`) is correct and accessible.
- Ensure Python libraries (`numpy`, `imgaug`, `opencv-python`) are installed correctly.
- Check file and directory permissions as needed.
