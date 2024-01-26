cmake_minimum_required(VERSION 3.22)
project(clientServer)

set (CMAKE_CXX_STANDARD 23)

set (SRC_SERVER
    "src/server.cpp"
    "src/synchroFile.cpp"
    "include/synchroFile.h"
)

set (SRC_CLIENT
    "src/client.cpp"
)


add_executable(server ${SRC_SERVER})
add_executable(client ${SRC_CLIENT})
