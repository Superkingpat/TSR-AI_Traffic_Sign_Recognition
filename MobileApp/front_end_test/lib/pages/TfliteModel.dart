import 'dart:math';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:tflite_flutter/tflite_flutter.dart';
import 'package:image/image.dart' as img;

class TfliteModel extends StatefulWidget {
  const TfliteModel({Key? key}) : super(key: key);

  @override
  _TfliteModelState createState() => _TfliteModelState();
}

class _TfliteModelState extends State<TfliteModel> {
  Interpreter? interpreter;
  String _result = 'No result yet';
  Uint8List? _processedImage;
  Uint8List? _originalImage;

  @override
  void initState() {
    super.initState();
    loadModel();
    classifyDefaultImage();
  }

  Future<void> loadModel() async {
    try {
      interpreter = await Interpreter.fromAsset('assets/best_float32.tflite');
      print('Loaded model successfully');

      // Get input and output shapes
      if (interpreter != null) {
        var inputShape = interpreter!.getInputTensor(0).shape;
        var outputShape = interpreter!.getOutputTensor(0).shape;

        print('Input shape: $inputShape');
        print('Output shape: $outputShape');

        // I/flutter ( 6412): Input shape: [1, 1280, 1280, 3]
        // I/flutter ( 6412): Output shape: [1, 11, 33600]

      }
    } catch (e) {
      print('Failed to load model: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('TFLite Image Processing'),
      ),
      body: Center(
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              _originalImage == null
                  ? Text('No image loaded.')
                  : Image.memory(_originalImage!, height: 1280, width: 1280),
              SizedBox(height: 20),
              _processedImage == null
                ? Text('No output image processed yet.')
                    : Image.memory(_processedImage!, height: 1280, width: 1280),


            ],
          ),
        ),
      ),
    );
  }

  Future<void> classifyDefaultImage() async {
    // Load and process the image
    final ByteData imageBytes = await rootBundle.load('assets/images/0000027.jpg');
    final Uint8List values = imageBytes.buffer.asUint8List();
    final img.Image? originalImage = img.decodeImage(values);
    if (originalImage == null) {
      setState(() => _result = "Could not decode the image.");
      return;
    }

    img.Image resizedImage = img.copyResize(originalImage, width: 1280, height: 1280);
    var input = imageToByteListFloat32(resizedImage, 1280, 0, 255);

    // Set the input tensor data
    int inputIndex = interpreter!.getInputIndex('inputs_0');
    Tensor inputTensor = interpreter!.getInputTensor(inputIndex);
    inputTensor.data.buffer.asFloat32List().setAll(0, input);

    interpreter!.invoke();

    var output = interpreter!.getOutputTensor(0).data;
    Float32List fl = output.buffer.asUint8List().buffer.asFloat32List();


    img.Image? processedImage = await processOutput(fl, values);
    if (processedImage != null) {
      setState(() {
        _processedImage = img.encodePng(processedImage) as Uint8List?;
        _result = "Image processed successfully";
      });
    }

  }

  Float32List imageToByteListFloat32(img.Image image, int inputSize, double mean, double std) {
    var buffer = Float32List(1 * 1280 * 1280 * 3);
    int pixelIndex = 0;
    for (var i = 0; i < inputSize; i++) {
      for (var j = 0; j < inputSize; j++) {
        var pixel = image.getPixel(j, i);
        buffer[pixelIndex++] = (pixel.r - mean) / std;
        buffer[pixelIndex++] = (pixel.g - mean) / std;
        buffer[pixelIndex++] = (pixel.b - mean) / std;
      }
    }
    return buffer;
  }

  Future<img.Image?> processOutput(Float32List output, Uint8List _originalImage) async {
    int totalChannels = 11;
    int numCoords = 4;
    int elementsPerChannel = 33600;
    double threshold = 0.5;

    // Reshape output
    List<List<double>> reshapedOutput = List.generate(totalChannels,
            (i) => List.filled(elementsPerChannel, 0.0, growable: false), growable: false);

    for (int i = 0; i < totalChannels; i++) {
      for (int j = 0; j < elementsPerChannel; j++) {
        reshapedOutput[i][j] = output[i * elementsPerChannel + j];
      }
    }

    List<List<double>> boxes = reshapedOutput.sublist(0, numCoords);
    List<List<double>> probs = reshapedOutput.sublist(numCoords);

    img.Image? processedImage = img.decodeImage(_originalImage);

    if (processedImage == null) {
      return null; // Return null if the image cannot be decoded
    }

    for (int i = 0; i < elementsPerChannel; i++) {
      double maxProb = probs.map((list) => list[i]).reduce(max);
      if (maxProb > threshold) {
        int classId = probs.indexWhere((list) => list[i] == maxProb);
        double xCenter = boxes[0][i] * processedImage.width;
        double yCenter = boxes[1][i] * processedImage.height;
        double width = boxes[2][i] * processedImage.width;
        double height = boxes[3][i] * processedImage.height;
        int xMin = (xCenter - width / 2).round();
        int yMin = (yCenter - height / 2).round();
        int xMax = (xCenter + width / 2).round();
        int yMax = (yCenter + height / 2).round();

        img.Color color = img.ColorFloat32.rgb(11,22,33);

        img.drawRect(processedImage, x1: xMin, y1: yMin, x2: xMax, y2: yMax, color: color, thickness: 2);
      }
    }

    return processedImage;
  }


}
