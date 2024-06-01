import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import 'package:geolocator/geolocator.dart';
class MapPage extends StatefulWidget {
  const MapPage({super.key});

  @override
  State<MapPage> createState() => _MapPageState();
}

class _MapPageState extends State<MapPage> {
  double? lat;
  double? lang;

  void initState() {
    super.initState();
    _getCurrentLocation();
  }

  Future<void> _getCurrentLocation() async {
    bool serviceEnabled = await Geolocator.isLocationServiceEnabled();
    if (!serviceEnabled) {
      return Future.error("Location Disabled");
    }

    LocationPermission permission = await Geolocator.checkPermission();
    if (permission == LocationPermission.denied) {
      permission = await Geolocator.requestPermission();
      if (permission == LocationPermission.denied) {
        return Future.error("Permission denied");
      }
    }

    Position position = await Geolocator.getCurrentPosition();
    setState(() {
      lat = position.latitude;
      lang = position.longitude;
    });
  }

  void _liveLocation(){
    LocationSettings locationSettings = const LocationSettings(
      accuracy: LocationAccuracy.high,
      distanceFilter: 5,
    );

    Geolocator.getPositionStream(locationSettings: locationSettings).listen((Position position) {
      lat = position.latitude;
      lang = position.longitude;
    });

    setState(() {

    });
  }

  @override
  Widget build(BuildContext context) {
    return FlutterMap(options:
      MapOptions(
        initialCenter: LatLng(lat!, lang!),
        initialZoom: 15,
        interactionOptions: const InteractionOptions(flags: InteractiveFlag.doubleTapZoom),
      ),
      children: [
        openStreetMapTileLater,
        MarkerLayer(markers: [
          Marker(
              point: LatLng(lat!, lang!),
              width: 60,
              height: 60,
              alignment: Alignment.centerLeft,
              child: Icon(Icons.location_pin, size:60, color: Colors.red,)
            )
          ]
        )
      ],
    );
  }
}

TileLayer get openStreetMapTileLater => TileLayer(
  urlTemplate: "https://tile.openstreetmap.org/{z}/{x}/{y}.png",
  userAgentPackageName: "dev.fleaflet.flutter_map.example",
  );

