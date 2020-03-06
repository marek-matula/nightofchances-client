# nightofchances-client
Client application for attendees of our workshop on Night of Chances.

### The vcpkg dependency installation
Download and install JsonCpp using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install jsoncpp
	
### Protocolbuffer usage
Generating classes for C++

    .\protoc.exe --cpp_out=. game.proto
