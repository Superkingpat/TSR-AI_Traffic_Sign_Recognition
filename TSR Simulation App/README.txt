git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

vcpkg install librdkafka
vcpkg integrate install

vcpkg install boost
vcpkg integrate install

vcpkg install opencv
vcpkg integrate install

correct path in under includes Properties/VC++ Directories/Include Directories

.\vcpkg\packages\opencv4_x64-windows\include\opencv4