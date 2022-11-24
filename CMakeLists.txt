cmake_minimum_required(VERSION 3.24.0)
project(server VERSION 0.1.0)

project(SERVER)

add_executable(${PROJECT_NAME} server.cpp) 

add_subdirectory(ryyisocket)

target_include_directories(${PROJECT_NAME} PUBLIC ryyisocket)
target_link_directories(${PROJECT_NAME} PRIVATE ryyisocket)
target_link_libraries(${PROJECT_NAME} ryyisocket)




set (CMAKE_CXX_STANDARD 20)