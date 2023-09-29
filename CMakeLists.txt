cmake_minimum_required(VERSION 3.25)
project(fivefour)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/raylib)
add_executable(fivefour main.cpp)

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY --preload-file ../resources")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

target_include_directories(fivefour PUBLIC external/raylib)
target_link_libraries(fivefour PUBLIC raylib)
