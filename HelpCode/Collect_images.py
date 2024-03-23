from ultralytics import YOLO
import os

#Trenutek bo prišel, ko bom tole zagnal nad direktorijem ki ima 16k slik. Ne danes, mogeče ne jutri, ampak enkrat. Wish me luck when the time comes

if __name__ == "__main__":
    #Path do datoteke kje so shranjene slike
    img_dir = r'C:\Users\steam\OneDrive\Namizje\YOLOv8_version_0.2\DtSt\test\images'
    #Path do modela za analizo slik
    model_dir = r'C:\Users\steam\OneDrive\Namizje\YOLOv8_version_0.2\runs\detect\yolov8n_tsd\weights\best.pt'
    #Path do direktorija kje hočemo posamezne slike znakov shraniti
    save_dir = r'C:\Data\lableimg\TF_images'

    all_files = []

    model = YOLO(model_dir)

    for root, directories, files in os.walk(img_dir):
        for filename in files:
            all_files.append(filename)
            print(filename)

    for file in files:
        model.predict(os.path.join(img_dir, file), save_crop=True, project=save_dir, exist_ok=True)