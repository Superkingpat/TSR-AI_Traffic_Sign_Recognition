#WARNING program will only work for formats (jpg, tif, png)
import os
from os import listdir, remove, makedirs
from os.path import dirname, abspath, exists, join, isdir
from time import monotonic
from PIL import Image
from datetime import timedelta
from Settings import *
#Install "pip install pillow", "pip install ExiFread",'pip install datetime'

def CreateFolder(directory):
    try:
        if not exists(directory):
            makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' +  directory)

def ResizePics():
    index = 0
    oldList = GetListOfFiles(picDirectory)

    oldDateTime = ""
    nonIdentifiedList = []
    startTime = monotonic()
    sorted = False
    lOldList = len(oldList)
    settings = False
    i = 0
    try:
        textFile = open(join(dirname(abspath(__file__))+nameOfTimeSortedFile), "r")
        if len(textFile.readlines()) == len(oldList):
            sorted = True
        helpFile = open(join(dirname(abspath(__file__))+nameOfHelpFile), "r")
        if len(helpFile.readlines()) != 0:
            settings = True
    except:
        pass
    if settings == True:
        helpFile = open(join(dirname(abspath(__file__))+nameOfHelpFile), "r")
        i = int(helpFile.readline().replace("\n","").strip())
        oldDateTime = str(helpFile.readline().replace("\n","").strip())
        oldList = []
        for f in helpFile.readlines():
            oldList.append(f.strip().replace("\n",""))
        try:
            helpFile2 = open(join(dirname(abspath(__file__))+nameOfHelpFile2), "r")
            for f in helpFile2.readlines():
                nonIdentifiedList.append(f.strip().replace("\n",""))
            lNonIdentifiedList = len(nonIdentifiedList)
        except:
            pass
    if sorted == False:
        while len(oldList) > 0:
            textFile = open(join(dirname(abspath(__file__))+nameOfTimeSortedFile), "a")
            try:
                for f in oldList:
                    with open(f"{picOutputDirectory}/{f}", 'rb') as fh:
                        tags = exifread.process_file(fh, stop_tag="EXIF DateTimeOriginal")
                        fh.close()
                        dateTaken = str(tags["EXIF DateTimeOriginal"])
                        date, time = dateTaken.split(' ')
                        year, month, day = date.split(":")
                        hour, min, sec = time.split(":")
                        dateTime = year + month + day + hour + min + sec
                        if oldDateTime == "":
                            oldDateTime = dateTime
                        for index in range(0,len(dateTime)):
                            if dateTime[index] > oldDateTime[index]:
                                oldDateTime = dateTime
                                pic = f
            except:
                nonIdentifiedList.append(f)
                helpFile2 = open(join(dirname(abspath(__file__))+nameOfHelpFile2), "w")
                for f in nonIdentifiedList:
                    helpFile2.write(f"{f}\n")
                helpFile2.close()
                oldList.remove(f)
            try:
                oldList.remove(pic)
                textFile.write(f"{pic}\n")
                textFile.close()
                ModifyPics(pic)
            except:
                pass
            lNonIdentifiedList = len(nonIdentifiedList)
            i += 1
            lapTime = monotonic()
            print(f"{timedelta(seconds=((lapTime - startTime)/i+1)*(len(oldList))+(len(nonIdentifiedList))*5)} left!")
            print(str(i)+"/"+str(lOldList))

            helpFile = open(join(dirname(abspath(__file__))+nameOfHelpFile), "w")
            helpFile.write(f"{i}\n{oldDateTime}\n")

            for f in oldList:
                helpFile.write(f"{f}\n")
            helpFile.close()

            if i == 10:
                StartShowingPictures()
        if len(oldList) == 0:
            while len(nonIdentifiedList) > 0:
                pic = nonIdentifiedList[0]
                ModifyPics(pic)
                nonIdentifiedList.remove(pic)
                i += 1
                lapTime = monotonic()
                print(f"{timedelta(seconds=((lapTime - startTime)/(len(nonIdentifiedList)+1))*(len(nonIdentifiedList))+(len(nonIdentifiedList))*0.2)} left!")
                print(str(i)+"/"+str(lOldList+lNonIdentifiedList))
                
                if len(nonIdentifiedList)+10 == lNonIdentifiedList:
                    StartShowingPictures()

                helpFile2 = open(join(dirname(abspath(__file__))+nameOfHelpFile2), "w")
                for f in nonIdentifiedList:
                    helpFile2.write(f"{f}\n")
                helpFile2.close()

                helpFile = open(join(dirname(abspath(__file__))+nameOfHelpFile), "w")
                helpFile.write(f"{i}\n\n\n")
                helpFile.close()

                textFile = open(join(dirname(abspath(__file__))+nameOfTimeSortedFile), "a")
                textFile.write(f"{pic}\n")
                textFile.close()

    if len(nonIdentifiedList) == 0 and len(oldList) == 0:
        remove(dirname(abspath(__file__))+nameOfHelpFile)
        remove(dirname(abspath(__file__))+nameOfHelpFile2)

def GetListOfFiles(directory):
    oldList = list()
    listOfFile = listdir(directory)
    for entry in listOfFile:
        fullPath = join(directory, entry)
        if isdir(fullPath):
            oldList = oldList + GetListOfFiles(fullPath)
        else:
            if fullPath.endswith(".jpg") or fullPath.endswith(".png") or fullPath.endswith(".tif"):
                pic = fullPath.replace(picDirectory, "").replace("\\","/")
                oldList.append(pic)
    return oldList

def ModifyPics(pic):
    index = 0
    bkg = Image.open(dirname(abspath(__file__))+"/__"+color+"__.png")
    img = Image.open(picDirectory+pic)
    width, height = img.size
    diffrenceImg = height/width
    diffrenceR = wantedSizeY/wantedSizeX
    pic = pic.split("/")
    nameList = pic[-1].split(".")

    if diffrenceImg < diffrenceR:
        k = width/wantedSizeX
        img = img.resize((int(wantedSizeX),int(height/k)))
    elif diffrenceImg > diffrenceR:
        k = height/wantedSizeY
        img = img.resize((int(width/k),int(wantedSizeY)))  
    elif diffrenceImg == diffrenceR:
        k = width/wantedSizeX
        img = img.resize((int(wantedSizeX),int(height/k)))

    img = img.convert("RGBA")
    bkg = bkg.convert("RGBA")

    width, height = img.size

    bkg.paste(img,(int((wantedSizeX-width)/2),int((wantedSizeY-height)/2)))

    bkg = bkg.convert("L")
    bkg.save(fp=f"{picOutputDirectory}/{nameList[0]+pictureFormat}")
    index = index + 1
    img.close()
    bkg.close()
    if overWrite == True:
        remove(picDirectory+pic)

def StartShowingPictures():
    try:
        Popen.terminate()
    except:
        pass
    Popen(['nohup', 'python', f'{dirname(abspath(__file__))}/ShowPics.py'], shell=True, stdout=None, stderr=None, preexec_fn=os.setpgrp )

if picDirectory == "":
    picDirectory = dirname(abspath(__file__))  
if picOutputDirectory == "":
    picOutputDirectory = dirname(abspath(__file__))
if overWrite == True:
    picOutputDirectory = picDirectory
else:
    CreateFolder(picOutputDirectory)

ResizePics()
try:
    Popen.terminate()
except:
    pass
ShowPics()
#Made by Superkingpat#