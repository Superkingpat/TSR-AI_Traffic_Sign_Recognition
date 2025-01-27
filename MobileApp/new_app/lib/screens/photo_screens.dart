import 'package:camera/camera.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:gallery_saver/gallery_saver.dart';
import 'package:new_app/main.dart';
import 'package:new_app/screens/video_screens.dart';
import 'package:path/path.dart' as path;
import 'package:path_provider/path_provider.dart' as pathProvider;
import 'package:camera/src/camera_controller.dart';
import 'dart:async';
import 'dart:io'; // Corrected import for File


class PhotoScreen extends StatefulWidget {
  final List<CameraDescription> cameras;

  PhotoScreen(this.cameras);

  @override
  State<PhotoScreen> createState() => _PhotoScreenState();
}

class _PhotoScreenState extends State<PhotoScreen> {
  late CameraController controller;
  bool isCapturing = false;
  int _selectedCameraIndex = 0;
  bool _isFrontCamera = false;
  Offset? _focusPoint;
  double _currentZone = 1.0;
  File? _captureImage;


  @override
  void initState() {
    // TODO: implement initState
    super.initState();
    controller = CameraController(widget.cameras[0], ResolutionPreset.max);
    controller.initialize().then((_){
      if(!mounted){
        return;
      }
      setState(() {

      });
    });
  }

  @override
  void dispose() {
    controller.dispose();
    super.dispose();
  }

  void _switchCamera() async {
    if(controller != null){
      //Dispose current controller to release the camera resource
        await controller.dispose();
    }

    //Increment camera index
    _selectedCameraIndex = (_selectedCameraIndex + 1) % widget.cameras.length;

    _initCamera(_selectedCameraIndex);
  }

  Future<void> _initCamera(int cameraIndex) async {
    controller = CameraController(widget.cameras[cameraIndex], ResolutionPreset.max);

    try{
      await controller.initialize();
      setState(() {
        if(cameraIndex == 0) {
          _isFrontCamera = false;
        }else{
          _isFrontCamera = true;
        }
      });
    }catch(e){
      print("Error message ${e}");
    }

    if(mounted){
      setState(() {

      });
    }
  }

  void capturePhoto() async{
    if (!controller.value.isInitialized) {
      return;
    }

    final Directory appDir = await pathProvider.getApplicationSupportDirectory();
    final String capturePath = path.join(appDir.path,'${DateTime.now()}.jpg');
    if(controller.value.isTakingPicture){
      return;
    }

    try{
      setState(() {
        isCapturing = true;
      });

      final XFile capturedImage = await controller.takePicture();
      String imagePath = capturedImage.path;
      await GallerySaver.saveImage(imagePath);
      print("Photo Capured And Saved ");


      final String filePath = '$capturePath/${DateTime.now().millisecondsSinceEpoch}.jpg';

      _captureImage = File(capturedImage.path);
      _captureImage!.renameSync(filePath);

    }catch(e){
      print("Error message ${e}");
    } finally {
      setState(() {
        isCapturing = false;
      });
    }


  }
  //  Zoom +  Focus
  @override
  Widget build(BuildContext context) {
    // Check if the controller is initialized before using it
    if (controller == null || !controller.value.isInitialized) {
      return Container(
        alignment: Alignment.center,
        child: CircularProgressIndicator(), // Shows a loading spinner until the camera is ready
      );
    }

    return SafeArea(
      child: Scaffold(
        body: LayoutBuilder(
          builder: (BuildContext context, BoxConstraints constraints) {
            return Stack(
              children: [
                Positioned(
                  top: 0,
                  left: 0,
                  right: 0,
                  child: Container(
                    height: 50,
                    decoration: BoxDecoration(
                      color: Colors.black,
                    ),
                  ),
                ),



                Positioned.fill(
                  top: 50,
                  bottom: _isFrontCamera == false ? 0 : 150,
                  child: AspectRatio(
                    aspectRatio: controller.value.aspectRatio, // Now safe to use
                    child: CameraPreview(controller),
                  ),
                ),
                


                Positioned(
                  bottom: 0,
                  left: 0,
                  right: 0,
                  child: Container(
                    height: 120,
                    decoration: BoxDecoration(
                      color: _isFrontCamera == false ? Colors.black87 : Colors.black,
                    ),
                    child: Column(
                      children: [
                        Padding(
                          padding: const EdgeInsets.all(8.0),
                          child: Row(
                            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                            children: [
                              GestureDetector(
                                  onTap: (){
                                    Navigator.push(context, MaterialPageRoute(builder: (c) => VideoScreen(widget.cameras)));
                                    print("Video tapped");
                                  },
                                  child: Text("Video", style: TextStyle(color: Colors.white, fontWeight: FontWeight.bold)),
                              ),
                              GestureDetector(
                                  onTap: (){
                                    print("Photo tapped");
                                  },
                                  child: Text("Photo", style: TextStyle(color: Colors.white, fontWeight: FontWeight.bold))
                              ),
                            ],
                          ),
                        ),
                        Expanded(
                          child: Column(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                              Row(
                                children: [
                                  Expanded(
                                    child: Row(
                                      mainAxisAlignment: MainAxisAlignment.center,
                                      children: [
                                        _captureImage != null
                                      ? Container(
                                          width: 50,
                                          height: 50,
                                          child: Image.file(
                                            _captureImage!,
                                            fit: BoxFit.cover,
                                          )
                                        ) : Container(),
                                        Container(),
                                      ],
                                    ),
                                  ),

                                  Expanded(
                                      child: GestureDetector(
                                        onTap: (){
                                          capturePhoto();
                                        },
                                        child: Center(
                                          child: Container(
                                            height: 70,
                                            width: 70,
                                            decoration: BoxDecoration(
                                                color: Colors.transparent,
                                                borderRadius: BorderRadius.circular(50),
                                                border: Border.all(
                                                  width: 4,
                                                  color: Colors.white,
                                                  style: BorderStyle.solid,
                                                )
                                            ),
                                          ),
                                        ),
                                      )
                                  ),

                                  Expanded(
                                      child: GestureDetector(
                                        onTap: (){
                                          _switchCamera();
                                        },
                                        child: Icon(Icons.cameraswitch_sharp, color: Colors.white,size: 40,),
                                    )
                                  )
                                ],
                              ),

                            ],
                          ),
                        )
                        

                        // Add more widgets or logic as needed
                      ],
                    ),
                  ),
                ),
              ],
            );
          },
        ),
      ),
    );
  }

}
