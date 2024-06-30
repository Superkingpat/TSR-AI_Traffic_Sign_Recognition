# Video Merger and Compressor

This script merges, compresses, and mutes MP4 videos located in a specific input directory and outputs the processed video to a specified output directory.

## Requirements

- Python 3.x
- `ffmpeg`

## Installation

1. **Download the script**

    Download the script file and place it in your desired directory.

2. **Install required Python libraries**

    Ensure you have the `ffmpeg` installed. You can install it using your package manager:

    For **Ubuntu/Debian**:
    ```sh
    sudo apt update
    sudo apt install ffmpeg
    ```

    For **MacOS** (using Homebrew):
    ```sh
    brew install ffmpeg
    ```

    For **Windows**:
    Download the executable from the [official site](https://ffmpeg.org/download.html) and follow the installation instructions.

## Usage

1. **Prepare your input directory**

    Create a directory named `data_IN` in the same directory as the script. Place all your `.mp4` video files that you want to merge and compress into this `data_IN` directory.

2. **Run the script**

    Execute the script to start processing the videos:

    ```sh
    python main.py
    ```

    The processed video will be saved in the `data_OUT` folder in the same directory as the script.

## Troubleshooting

- Ensure `data_IN` is in the correct location and contains `.mp4` files.
- Make sure `ffmpeg` is installed and accessible from the command line.
- Check your file and directory permissions.