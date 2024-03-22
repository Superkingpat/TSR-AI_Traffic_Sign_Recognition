from ultralytics import YOLO
import cv2
if __name__ == '__main__':
    model = YOLO('yolov8m.pt')

    results = model.train(
        data=r'C:\Users\patri\OneDrive\Dokumenti\GitHub\TSR-AI_Traffic_Sign_Recognition\AI\YOLOv8_version_0.2\DtSt\data.yaml',
        imgsz=[1280, 720],
        epochs=25,
        batch=16,
        name='yolov8n_tsd',
        rect=True
    )