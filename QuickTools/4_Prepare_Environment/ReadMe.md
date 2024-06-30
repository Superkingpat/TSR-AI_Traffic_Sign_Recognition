# Image Normalization Script

This script normalizes images (resizes them) in a specified input directory (`data_IN`) and saves the normalized images as JPEG files in an output directory (`data_OUT`). Images that cannot be processed are moved to a separate directory (`data_FIX`).

## Requirements

- Python 3.x
- `PIL` (Python Imaging Library)
- `tqdm` (for progress visualization)

## Installation

1. **Download the script**

    Download the script file and place it in your desired directory.

2. **Install required Python libraries**

    Install the necessary Python libraries using pip:

    ```sh
    pip install Pillow tqdm
    ```

## Usage

1. **Prepare your input directory**

    Create a directory named `data_IN` in the same directory as the script. Place all your images that you want to normalize into this `data_IN` directory.

2. **Run the script**

    Execute the script to start normalizing images:

    ```sh
    python main.py
    ```

    - Normalized images will be saved as `.jpg` files in the `data_OUT` folder.
    - Images that cannot be processed (due to errors) will be moved to the `data_FIX` folder.

## Troubleshooting

- Ensure `data_IN` directory exists and contains valid image files.
- Check that Python libraries (`PIL` and `tqdm`) are correctly installed.
- Verify file and directory permissions as necessary.
- Images that are in data_FIX folder can usualy be saved by opening it in paint.net program and resaving it to remove coruption. 
