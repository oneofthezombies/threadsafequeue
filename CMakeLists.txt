cmake_minimum_required(VERSION 3.0.0)
project(threadsafequeue VERSION 0.1.0)

add_executable(${PROJECT_NAME} 
    main.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
