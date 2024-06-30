from ultralytics import YOLO
import cv2

if __name__ == '__main__':
    model = YOLO(r"C:\Users\patri\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\runs\detect\yolov8n_tsd12\weights\best.pt")

    result = model.predict(r"C:\Projects\AI_TSR\TSR-AI_Traffic_Sign_Recognition\AI\ServerAI\1716116788.0306077.jpg")

    result[0].show()

