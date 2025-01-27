from ultralytics import YOLO
import cv2

def trainModel(model_name, name_m, batch_m, epoch_m, size):
    model = YOLO(model_name)
    model.to('cuda')

    results = model.train(
        data=r"C:\Users\patri\Documents\GitHub\TSR-AI_Traffic_Sign_Recognition\YOLOv8_train_sample\DtSt\data.yaml",
        imgsz= size, #[1280, 720],
        epochs= epoch_m,
        batch= batch_m,
        name= name_m
    )

if __name__ == '__main__':
    """
    trainModel("yolov8m.pt", 'yolov8m_1080', 11, 1000, 1080)
    trainModel("yolov8s.pt", 'yolov8s_1080', 24, 1000, 1080)
    trainModel("yolov8m.pt", 'yolov8m_640', 36, 1000, 640)
    trainModel("yolov8s.pt", 'yolov8s_640', 64, 1000, 640)
    
    trainModel("yolov8n.pt", 'yolov8n_1080', 36, 1000, 1080)
   
    trainModel("yolov8n.pt", 'yolov8n_640', 86, 1000, 640) """
    trainModel("yolov8x.pt", 'yolov8x_640', 18, 1000, 640)
    #trainModel("yolov8x.pt", 'yolov8x_1080', 6, 1000, 1080)