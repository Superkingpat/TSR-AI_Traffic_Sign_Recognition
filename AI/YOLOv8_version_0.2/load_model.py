from ultralytics import YOLO
import cv2

if __name__ == '__main__':
    model = YOLO(r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\runs\detect\yolov8n_tsd12\weights\best.pt")

    result = model.predict(r"C:\Users\patri\Downloads\20240321_135051.jpg")

    result[0].show()

