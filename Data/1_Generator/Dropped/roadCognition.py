import numpy as np
import cv2 as cv

video_path = r"C:\Users\patri\Downloads\OneDrive_2024-04-21\tmp_cesta\out1.mp4"
cap = cv.VideoCapture(video_path)

new_width, new_height = 600, 850

paused = False
frame_index = 0

while True:
    if not paused or cv.waitKey(0) == ord("n"):
        ret, frame = cap.read()

        if not ret:
            print("1")
            break
    
        frame = cv.resize(frame, (new_width, new_height))

        gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

        edges = cv.Canny(gray, 350 ,450)

        mask = np.zeros_like(edges)
        mask[new_height // 2 + 120:,:]+255

        edges_masked = cv.bitwise_and(edges, mask)

        contour, _ = cv.findContours(edges_masked, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)

        cv.drawContours(frame, contour, 1, (0,0,255), 2)

        cv.imshow("1", gray)
        cv.imshow("2 ", frame)
        cv.imshow("3", mask)
        cv.imshow("Edge Detection", edges_masked)

        key = cv.waitKey(30)
        if key == ord('q'):
            break
        elif key == ord('p'):
            paused = not paused
        elif key == ord('n'):
            frame_index += 1
            cap.set(cv.CAP_PROP_POS_FRAMES,frame_index)
    else:
        key = cv.waitKey(0)
        if key == ord('q'):
            break
        elif key == ord('p'):
            paused = not paused

cap.release()
cv.destroyAllWindows()