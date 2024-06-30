from os import listdir, system, makedirs, remove
from os.path import join, abspath, exists, dirname

#####    REQUIRES DATA_IN FOLDER !!!!!    #####
#####    REQUIRES FFMPEG !!!!!    ##### 

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

    for video in videos:
        system(f"{dirname(abspath(__file__))}/ffmpeg -i {join(data_in,video)} -vf fps=1/10 {data_out}/{video.split(".")[0]}_%d.jpg")
