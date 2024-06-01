import 'package:flutter/material.dart';
import 'package:front_end_test/pages/secondPage.dart';
import 'package:front_end_test/pages/mapPage.dart';
import 'package:front_end_test/pages/TfliteModel.dart';


class MapView extends StatefulWidget {
  const MapView({super.key});

  @override
  State<MapView> createState() => _MapViewState();
}

class _MapViewState extends State<MapView> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: PageView.builder(
        itemCount: 3, // The total number of pages
        itemBuilder: (BuildContext context, int index) {
          // Here we decide which widget to display based on the index
          if (index == 0) {
            return const MapPage();
          } else if (index == 1) {
            return const SecondPage();
          }else if (index == 2) {
            return const TfliteModel();
          } else {
            // Optionally, handle other indices if you have more pages
            return Container(); // Empty container for indexes that aren't handled
          }
        },
      ),
    );
  }
}
