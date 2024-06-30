# Video to Frames Conversion Script

This script converts MP4 videos into frames (.jpg) using ffmpeg, extracting frames at a specified interval.

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

    Create a directory named `data_IN` in the same directory as the script. Place all your `.mp4` video files that you want to convert into frames into this `data_IN` directory.

2. **Run the script**

    Execute the script to start converting videos to frames:

    ```sh
    python main.py
    ```

    The frames will be saved as `.jpg` files in the `data_OUT` folder in the same directory as the script.

## Troubleshooting

- Ensure `data_IN` directory exists and contains `.mp4` video files.
- Verify that `ffmpeg` is correctly installed and accessible from the command line.
- Check file and directory permissions as needed.