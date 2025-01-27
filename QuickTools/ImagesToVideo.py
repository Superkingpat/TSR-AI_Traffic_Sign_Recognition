import os
import subprocess

def pngs_to_video(input_dir, output_file, frame_rate=30):
    """
    Scans a directory for PNG files and creates a video using FFmpeg.

    Args:
        input_dir (str): Path to the directory containing PNG files.
        output_file (str): Path to the output video file.
        frame_rate (int): Frame rate for the video. Default is 30.

    Returns:
        None
    """
    # Get all PNG files in the directory
    png_files = [f for f in os.listdir(input_dir) if f.lower().endswith('.png')]

    if not png_files:
        print("No PNG files found in the directory.")
        return

    # Sort files by name
    png_files.sort()

    # Create a temporary text file listing the PNG files in order
    list_file = os.path.join(input_dir, 'file_list.txt')
    with open(list_file, 'w') as f:
        for png in png_files:
            # Each line must have the format: file '<filename>'
            f.write(f"file '{os.path.join(input_dir, png)}'\n")

    # Build FFmpeg command
    command = [
        'ffmpeg',
        '-f', 'concat',
        '-safe', '0',
        '-r', str(frame_rate),
        '-i', list_file,
        '-c:v', 'libx264',
        '-pix_fmt', 'yuv420p',
        output_file
    ]

    # Execute FFmpeg command
    try:
        subprocess.run(command, check=True)
        print(f"Video created successfully: {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Error creating video: {e}")
    finally:
        # Remove the temporary file list
        if os.path.exists(list_file):
            os.remove(list_file)

# Example usage
if __name__ == '__main__':
    input_directory = r"C:\Users\patri\Downloads\Signs"
    output_video = "out5.mp4"
    frame_rate = 30

    pngs_to_video(input_directory, output_video, frame_rate)
