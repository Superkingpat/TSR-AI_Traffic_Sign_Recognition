import 'package:flutter/material.dart';
import 'package:camera/camera.dart';
import 'package:front_end_test/pages/camera/photo_screen.dart';

class SecondPage extends StatefulWidget {
  const SecondPage({Key? key}) : super(key: key);

  @override
  _SecondPageState createState() => _SecondPageState();
}

class _SecondPageState extends State<SecondPage> {
  List<CameraDescription> cameras = [];
  bool _isCameraInitialized = false;

  @override
  void initState() {
    super.initState();
    _initCameras();
  }

  Future<void> _initCameras() async {
    try {
      cameras = await availableCameras();
      setState(() {
        _isCameraInitialized = true;
      });
    } on CameraException catch (e) {
      print('Error initializing cameras: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    // If cameras are not initialized, show a progress indicator
    if (!_isCameraInitialized) {
      return const Scaffold(
        body: Center(
          child: CircularProgressIndicator(),
        ),
      );
    }

    // If cameras are initialized, return the PhotoScreen
    return PhotoScreen(cameras);
  }
}

