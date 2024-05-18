from ultralytics import YOLO
import cv2
if __name__ == '__main__':
    model = YOLO(r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\runs\detect\yolov8l_tsd_100K\weights\best.pt")
    model.to('cuda')

    results = model.train(
        data=r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\dataset\data.yaml",
        #imgsz= 1080, #[1280, 720],
        epochs=1000,
        batch=16,
        name='yolov8l_tsd_100K'
    )