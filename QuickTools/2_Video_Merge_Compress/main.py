from os import listdir, system, makedirs, remove
from os.path import join, abspath, exists, dirname

#####    REQUIRES DATA_IN FOLDER !!!!!    #####
#####    REQUIRES FFMPEG !!!!!    ##### 
# https://univerzamb-my.sharepoint.com/:u:/g/personal/patrik_gobec_student_um_si/EWyX-xlH5o1Ds6qb2Fl_38sB4lSGrW9rPGnt4JGOqJ1q_w?e=mkjKI4

def create_dir(path):
    try:
        if not exists(path):
            makedirs(path)
    except OSError:
        print ('Error: Creating directory. ' +  path)

if __name__ == "__main__":
    data_in = join(dirname(abspath(__file__)), "data_IN")
    data_out = join(dirname(abspath(__file__)), "data_OUT")
    create_dir(data_out)
    videos = [f for f in listdir(data_in) if f.endswith(".mp4") or f.endswith(".MP4")] # Compatible with mp4 (extend code with caution)
    input_string = ""
    for f in videos:
        input_string += f"file '{join(data_in,f)}'\n"

    print(input_string)
    open("a.txt", "w").write(input_string)

    # FFMPEG command to merge compress and mute videos 
    system(f"{dirname(abspath(__file__))}/ffmpeg -f concat -safe 0 -i a.txt -b:v 5M -an {data_out}/out_{videos[0]}")

    remove("a.txt")
    #print(videos)