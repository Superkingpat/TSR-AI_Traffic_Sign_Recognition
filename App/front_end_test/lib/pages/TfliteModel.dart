import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:tflite_flutter/tflite_flutter.dart';
import 'package:image/image.dart' as img;
import 'package:flutter/painting.dart';
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
                    : Image.memory(_processedImage!, height: 256, width: 256),


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

    processOutput(fl);
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

  void processOutput(Float32List output) {
    int totalChannels = 11; 
    int numCoords = 4;
    int numProbs = totalChannels - numCoords;
    int elementsPerChannel = 33600;

    List<List<double>> reshapedOutput = List.generate(totalChannels, (i) => List.filled(elementsPerChannel, 0.0));

    for (int i = 0; i < totalChannels; i++) {
      for (int j = 0; j < elementsPerChannel; j++) {
        reshapedOutput[i][j] = output[i * elementsPerChannel + j];
      }
    }

    List<List<double>> boxes = List.generate(numCoords, (i) => reshapedOutput[i]);
    List<List<double>> probs = List.generate(numProbs, (i) => reshapedOutput[numCoords + i]);

    double threshold = 0.5;
    List<List<bool>> mask = List.generate(numProbs,
            (i) => List.generate(elementsPerChannel, (j) => probs[i][j] > threshold));


    for (int i = 0; i < numProbs; i++) {
      for (int j = 0; j < elementsPerChannel; j++) {
        if (mask[i][j]) {
          print("Prob[${i+numCoords}][${j}] = ${probs[i][j]}");
        }
      }
    }
  }




}
