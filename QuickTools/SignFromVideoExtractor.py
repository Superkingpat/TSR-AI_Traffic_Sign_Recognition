import os
import cv2
from ultralytics import YOLO  # Ensure you have the ultralytics package installed

def process_videos(input_folder, output_folder, model_path):
    # Load the YOLOv8 model
    model = YOLO(model_path)
    model.to("cuda")

    # Create the output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Iterate through each file in the input folder
    for video_file in os.listdir(input_folder):
        video_path = os.path.join(input_folder, video_file)
        
        # Check if the file is a video
        if not video_file.lower().endswith((".mp4", ".avi", ".mov", ".mkv")):
            print(f"Skipping non-video file: {video_file}")
            continue

        # Create a folder for the video output
        video_output_folder = os.path.join(output_folder, os.path.splitext(video_file)[0])
        os.makedirs(video_output_folder, exist_ok=True)

        # Open the video file
        cap = cv2.VideoCapture(video_path)
        frame_count = 0

        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break  # End of video

            frame_count += 1

            # Run detection on the frame
            results = model(frame)

            # Save detected objects
            for idx, result in enumerate(results[0].boxes.xyxy):
                x1, y1, x2, y2 = map(int, result)  # Bounding box coordinates
                detected_object = frame[y1:y2, x1:x2]

                # Save the detected object to the output folder
                object_filename = os.path.join(video_output_folder, f"frame{frame_count}_object{idx}.jpg")
                cv2.imwrite(object_filename, detected_object)

        cap.release()
        print(f"Processed video: {video_file}, saved results in {video_output_folder}")

    print("Processing completed.")

# Example usage
if __name__ == "__main__":
    input_folder = "in"
    output_folder = "o1"
    model_path = "best1080.pt"  # Replace with your YOLOv8 model path

    process_videos(input_folder, output_folder, model_path)
