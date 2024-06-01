import os
import shutil

def move_files(dir1, dir2, n):
    for root, dirs, files in os.walk(dir1):
        # Get the relative path from dir1 to the current directory
        relative_path = os.path.relpath(root, dir1)
        dest_dir = os.path.join(dir2, relative_path)

        # Create the corresponding directory in dir2 if it doesn't exist
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        # Move n files from the current directory to the destination directory
        count = 0
        for file in files:
            if count < n:
                src_file = os.path.join(root, file)
                dest_file = os.path.join(dest_dir, file)
                shutil.move(src_file, dest_file)
                count += 1
            else:
                break

if __name__ == "__main__":
    dir1 = "./d3"
    dir2 = "./d4"
    n = 2000

    move_files(dir1, dir2, n)
