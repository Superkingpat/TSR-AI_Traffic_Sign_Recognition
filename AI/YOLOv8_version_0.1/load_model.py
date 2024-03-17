from ultralytics import YOLO

#Tukaj se naložijo uteži modela. Model za katerega so uteži je najlažji model
#Model si je trebo prvo naložiti z driva
model = YOLO(r'runs\detect\yolov8s_tsd\weights\best.pt')

#Tukaj model oceni sliko in vrne rezultat
result = model(r'DtSt\test\images\0000023_jpg.rf.ed57c231721110bcce7bef842a17920f.jpg')

#Tukaj se prikaže slika z označenimi prometnimi znaki
result[0].show()

#Za treniranje tega modela je bilo uporabljenih somo 100 slik, ki so bile tudi augmentirane zato performanca modela še ni zelo dobra