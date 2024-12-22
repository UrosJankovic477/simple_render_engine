set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.7.0)
project(simple_render_engine VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(-lpthread -O3 -msse2 -mfpmath=sse -ftree-vectorizer-verbose=5)

file(GLOB_RECURSE SOURCE_FILES
${CMAKE_SOURCE_DIR}/src/*.c
${CMAKE_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE HEADER_FILES
${CMAKE_SOURCE_DIR}/src/*.h
${CMAKE_SOURCE_DIR}/src/*.hpp
${CMAKE_SOURCE_DIR}/include/*.h
${CMAKE_SOURCE_DIR}/include/*.hpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)


#if(WIN32)
#find_package(OpenGL REQUIRED)
#elseif(UNIX)
#find_package(GL REQUIRED)
#endif(WIN32)

if(WIN32)
find_library(SDL2 SDL2 HINTS ${CMAKE_SOURCE_DIR}/lib)
find_library(SDL2MAIN SDL2main HINTS ${CMAKE_SOURCE_DIR}/lib)
endif(WIN32)

find_library(CGLM cglm HINTS ${CMAKE_SOURCE_DIR}/lib)
find_library(MATH_LIBRARY m)
find_library(DL_LIBRARY, dl)

add_library(GLAD "./src/glad.c" "include")
target_include_directories(GLAD PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
set(GL_LIB opengl32)
elseif(UNIX)
set(GL_LIB GL)
endif(WIN32)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
set(LIBS SDL2main SDL2 GLAD ${GL_LIB} cglm ${MATH_LIBRARY} ${DL_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${LIBS})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})



include(CPack)
