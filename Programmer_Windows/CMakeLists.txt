cmake_minimum_required(VERSION 3.10)

# set the project name
project(Arduino_PIC18f1220-PIC18f1320_Programmer)

# add the executable
add_executable(Arduino_PIC18f1220-PIC18f1320_Programmer main.c HEX.c HEX.h rs232.c rs232.h)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()