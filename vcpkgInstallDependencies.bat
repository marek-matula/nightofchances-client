git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
vcpkg install jsoncpp
vcpkg install protobuf

vcpkg install jsoncpp:x64-windows
vcpkg install protobuf:x64-windows