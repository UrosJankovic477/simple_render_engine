cmake_minimum_required(VERSION 3.7.0)
project(simple_render_engine VERSION 0.1.0)

include(CTest)
enable_testing()

file(GLOB_RECURSE SOURCE_FILES 
${CMAKE_SOURCE_DIR}/src/*.c
${CMAKE_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE HEADER_FILES 
${CMAKE_SOURCE_DIR}/src/*.h
${CMAKE_SOURCE_DIR}/src/*.hpp
${CMAKE_SOURCE_DIR}/include/*.h
${CMAKE_SOURCE_DIR}/include/*.hpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(OpenGL REQUIRED)

find_library(SDL2 SDL2 HINTS ${CMAKE_SOURCE_DIR}/lib)
find_library(SDL2MAIN SDL2main HINTS ${CMAKE_SOURCE_DIR}/lib)
find_library(CGLM HINTS ${CMAKE_SOURCE_DIR}/lib)

add_library(GLAD "./src/glad.c" "include")
target_include_directories(GLAD PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
set(LIBS opengl32 mingw32 SDL2MAIN SDL2 GLAD CGLM)
target_link_libraries(${PROJECT_NAME} ${LIBS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,20000000")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)