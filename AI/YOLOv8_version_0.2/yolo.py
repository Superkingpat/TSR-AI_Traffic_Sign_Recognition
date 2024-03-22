from ultralytics import YOLO
import cv2

model = YOLO('yolov8s.pt')

results = model.train(
    data='DtSt\data.yaml',
    imgsz=[1280, 720],
    epochs=25,
    batch=16,
    name='yolov8n_tsd',
    rect=True
)