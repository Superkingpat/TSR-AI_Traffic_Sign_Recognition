from ultralytics import YOLO

model = YOLO(r'runs\detect\yolov8n_tsd\weights\best.pt')

result = model(r'C:\Data\lableimg\JPEGImages\JPEGImages\0000654.jpg')

result[0].show()