# Image Downloader

This script downloads images from URLs listed in a text file and saves them to a specified folder.

## Requirements

- Python 3.x
- `requests` library

## Installation

1. **Download the script**

    Or simply download the `linkLoader.py` file and place it in your desired directory.

2. **Install required Python libraries**

    Ensure you have `requests` installed. You can install it using pip:

    ```sh
    pip install requests
    ```

## Usage

1. **Prepare your links file**

    Create a text file named `links.txt` in the same directory as the script. Each line in this file should contain a URL to an image you want to download. For example:

    ```
    https://example.com/image1.png
    https://example.com/image2.jpg
    ```

2. **Run the script**

    Execute the script to start downloading images:

    ```sh
    python linkLoader.py
    ```

    The images will be downloaded to the `pics` folder in the same directory as the script.

## Troubleshooting

- Ensure `links.txt` is in the correct location and contains `links to image` files.
- Check your file and directory permissions.