from os import listdir, makedirs, rename, system, chdir
from os.path import isfile, join, dirname, abspath, exists, isdir
from PIL import Image
from shutil import copy
from fnmatch import fnmatch

system("color a")

wantedSizeX = 1920
wantedSizeY = 1080

resolutionX = 1920
resolutionY = 1080

picsFolderName = "Slike"
indexFolderName = "SlikeIndex"
resizedFolderName = "SlikeResized"
layerPicFolder = "Layers"
clipFolder = "Clips"
videoFolder = "Video"
videoSourceFolder = "Source"
processingProgram = "Program"
processingProgramSource = "data"
processingProgramOutput = "output"
processingProgramLayers = "layers"

pathToFfmpeg = dirname(abspath(__file__)) + "/Ffmpeg.exe"
pathToPicsFolder = dirname(abspath(__file__))+"/data/"+picsFolderName
pathToIndexFolder = dirname(abspath(__file__))+"/data/"+indexFolderName
pathToResizedFolder = dirname(abspath(__file__))+"/data/"+resizedFolderName
pathToLayerFolder = dirname(abspath(__file__))+"/data/"+layerPicFolder
pathToClipFolder = dirname(abspath(__file__))+"/data/"+clipFolder
pathToVideoFolder = dirname(abspath(__file__))+"/data/"+videoFolder
pathToVideoSourceFolder = pathToVideoFolder+"/"+videoSourceFolder
pathToProcessing = dirname(abspath(__file__))+"/"+processingProgram
pathToProcessingSource = pathToProcessing+"/"+processingProgramSource
pathToProcessingOutput = pathToProcessing+"/"+processingProgramOutput
pathToProcessingLayers = pathToProcessing+"/"+processingProgramLayers

allDirList = [pathToPicsFolder, pathToIndexFolder,pathToResizedFolder,pathToLayerFolder, pathToClipFolder, pathToVideoFolder, pathToVideoSourceFolder, pathToProcessing, pathToProcessingSource, pathToProcessingOutput, pathToProcessingLayers]

def CreateAllFolders(dirList):
    for dir in dirList:
        system("cls")
        print(f"Creating dir: {dir}")
        CreateFolder(dir)

def CreateFolder(directory):
    try:
        if not exists(directory):
            makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' +  directory)

def GetListOfPictures(directory):
    picList = [f for f in listdir(directory) if isfile(join(directory, f))]
    return picList

def IndexPictures():
    picList = [f for f in listdir(pathToPicsFolder) if isfile(join(pathToPicsFolder, f))]
    for index in range(len(picList)):
        system("cls")
        print(f"Indexing picture number: {index}")
        nameList = picList[index].split(".")
        newName = str(index)+"."+nameList[-1]
        copy(pathToPicsFolder+"/"+picList[index],pathToIndexFolder)
        rename(pathToIndexFolder+"/"+picList[index], pathToIndexFolder+"/"+newName)

def ResizePics():
    picList = GetListOfPictures(pathToIndexFolder)
    for pic in picList:
        system("cls")
        print(f"Resizing picture: {pic}")
        img = Image.open(pathToIndexFolder+"/"+pic)
        width, height = img.size
        diffrenceImg = height/width
        diffrenceR = resolutionY/resolutionX
        nameList = pic.split(".") 
        if diffrenceImg > diffrenceR:
            k = width/wantedSizeX
            newImg = img.resize((int(wantedSizeX),int(height/k)))
            newImg = newImg.convert("RGB")
            newImg.save(fp=f"{pathToResizedFolder}/{nameList[0]}.jpg")
        elif diffrenceImg < diffrenceR:
            k = height/wantedSizeY
            newImg = img.resize((int(width/k),int(wantedSizeY)))
            newImg = newImg.convert("RGB")
            newImg.save(fp=f"{pathToResizedFolder}/{nameList[0]}.jpg")
        elif diffrenceImg == diffrenceR:
            k = width/wantedSizeX
            newImg = img.resize((int(wantedSizeX),int(height/k)))
            newImg = newImg.convert("RGB")
            newImg.save(fp=f"{pathToResizedFolder}/{nameList[0]}.jpg")

def MovePics():
    picList = GetListOfPictures(pathToResizedFolder)
    for pic in picList:
        system("cls")
        print(f"Moving: {pic}")
        copy(pathToResizedFolder+"/"+pic,pathToProcessingSource)

def DoneProcessing():
    command = ""
    while True:
        command = input("Did you process pictures in Processing?").lower().strip()
        if command == "yes":
            break

def AddZerosToFileNames(path):
    file_names = listdir(path)
    frame_file_names = [file_name for file_name in file_names if file_name[:6] == "frame_" and file_name[-4:] == ".png"]
    longest_frame_file_name_length = len(sorted(frame_file_names, key=len, reverse=True)[0])

    for file_name in frame_file_names:
        system("cls")
        print(f"Adding zeros to: {file_name}")
        if len(file_name) < longest_frame_file_name_length:
            new_file_name = "frame_" + "0" * (longest_frame_file_name_length - len(file_name)) + file_name[6:]
            rename(f"{path}/{file_name}", f"{path}/{new_file_name}")

def AddZerosToVideoFileNames(path):
    file_names = listdir(path)
    frame_file_names = [file_name for file_name in file_names if file_name[:4]== "clip" and file_name[-4:] == ".mov"]
    longest_frame_file_name_length = len(sorted(frame_file_names, key=len, reverse=True)[0])

    for file_name in frame_file_names:
        system("cls")
        print(f"Adding zeros to: {file_name}")
        if len(file_name) < longest_frame_file_name_length:
            new_file_name = "clip"+"0" * (longest_frame_file_name_length - len(file_name)) + file_name[5:]
            rename(f"{path}/{file_name}", f"{path}/{new_file_name}")

def MakeClips():
    directory = pathToProcessingOutput
    dirList = [f for f in listdir(directory) if isdir(join(directory, f))]
    for dir in dirList:
        system("cls")
        print(f"Making clip number {dir}")
        workingDir = pathToProcessingOutput+"/"+str(dir)
        copy(pathToFfmpeg, workingDir)
        AddZerosToFileNames(workingDir)
        chdir(workingDir)
        system(f"ffmpeg -i frame_%02d.png -r 60 -vcodec png {dir}.mov")

def GetLayerPics():
    picList = GetListOfPictures(pathToProcessingLayers)
    for pic in picList:
        system("cls")
        print(f"Getting picture: {pic}")
        f = pathToProcessingLayers+"/"+pic
        copy(f, pathToLayerFolder)

def GetClips():
    directory = pathToProcessingOutput
    dirList = [f for f in listdir(directory) if isdir(join(directory, f))] 
    for dir in dirList:
        system("cls")
        print(f"Geting clip: {dir}.mov")
        f = pathToProcessingOutput+"/"+dir+"/"+dir+".mov"
        copy(f, pathToClipFolder)

def InsertedBackground():
    command = ""
    while True:
        command = input("Did you copy background picture in Layer folder its name needs to be  frame_0.png!").lower().strip()
        if command == "yes":
            break

def MergeLayers():
    picList = GetListOfPictures(pathToLayerFolder)
    copy(pathToFfmpeg, pathToLayerFolder)
    oldFile = "frame_0"
    for pic in picList:
        system("cls")
        print(f"Merging layers {oldFile} and {pic}")
        nameList = pic.split(".")
        newFile = "_"+nameList[0]
        chdir(pathToLayerFolder)
        system(f'ffmpeg -i {oldFile}.png -i {nameList[0]}.png -filter_complex "[0:v][1:v] overlay" {newFile}.png')

def MakeVideo():
    clipList = [f for f in listdir(pathToClipFolder) if isfile(join(pathToClipFolder, f))]
    for index in range(len(clipList)):
        system("cls")
        print(f"Overlaying clip number: {index}")
        layer = pathToLayerFolder+"/_frame_"+str(index-1)+".png"
        clip = pathToClipFolder+"/"+str(index)+".mov"
        newFile = "clip"+str(index)+".mov"
        copy(pathToFfmpeg, pathToVideoSourceFolder)
        chdir(pathToVideoSourceFolder)
        system(f'ffmpeg -i {layer} -i {clip} -filter_complex "overlay" {newFile}')

def FinishVideo():
    system("cls")
    print("Finishing video!")
    chdir(pathToVideoSourceFolder)
    clipList = [f for f in listdir(pathToVideoSourceFolder) if isfile(join(pathToVideoSourceFolder, f))]
    f = open("text.txt","w")
    for index in range(len(clipList)):
        if fnmatch(clipList[index], ".exe"): 
            clipList.remove(clipList[index])
        else:
            f.write(f"file 'clip{index+1}.mov'\n")
    f.close()  
    system(f'ffmpeg -f concat -safe 0 -i text.txt -c copy Video.mov')
#Made by Superkingpat#