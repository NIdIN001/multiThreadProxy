cmake_minimum_required(VERSION 3.17)
project(mtpr)

set(CMAKE_CXX_STANDARD 20)

add_executable(mtpr Cache.cpp
        CacheReader.cpp
        ClientSocketHandler.cpp
        HostSocketHandler.cpp
        HttpProxy.cpp
        main.cpp
        ServerSocket.cpp
        TcpSocket.cpp)
