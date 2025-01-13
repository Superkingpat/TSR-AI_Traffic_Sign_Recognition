git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

vcpkg install librdkafka
vcpkg integrate install

vcpkg install boost
vcpkg integrate install

https://github.com/opencv/opencv/releases/tag/4.8.1

https://christianjmills.com/posts/opencv-visual-studio-getting-started-tutorial/windows/#configuring-for-release-build-in-visual-studio