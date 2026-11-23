cmake_minimum_required(VERSION 3.16)
project(RainThunderstorm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(scene_core src/Camera.cpp src/Geometry.cpp src/CloudSystem.cpp
    src/RainSystem.cpp src/LightningSystem.cpp src/LightingSystem.cpp
    src/SplashSystem.cpp src/RippleSystem.cpp src/Surface.cpp src/WeatherSystem.cpp)
target_include_directories(scene_core PUBLIC include)
target_link_libraries(scene_core PUBLIC glm::glm)

add_executable(rain_thunderstorm
    src/main.cpp src/Application.cpp src/Renderer.cpp
    src/Shader.cpp src/Mesh.cpp src/Scene.cpp src/DynamicBatch.cpp src/Benchmark.cpp)
target_link_libraries(rain_thunderstorm PRIVATE
    scene_core OpenGL::GL GLUT::GLUT GLEW::GLEW)

# Absolute build-tree path: launching from another working directory is safe.
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")
foreach(shader object.vert object.frag effects.vert effects.frag sky.vert sky.frag)
    configure_file("shaders/${shader}" "${SHADER_OUTPUT_DIR}/${shader}" COPYONLY)
endforeach()
target_compile_definitions(rain_thunderstorm PRIVATE
    RAIN_SHADER_DIR="${SHADER_OUTPUT_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(scene_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(rain_thunderstorm PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(core_tests tests/core_tests.cpp)
    target_link_libraries(core_tests PRIVATE scene_core)
    add_test(NAME geometry_and_camera COMMAND core_tests)
    add_executable(weather_tests tests/weather_tests.cpp)
    target_link_libraries(weather_tests PRIVATE scene_core)
    add_test(NAME weather_simulation COMMAND weather_tests)
    # GUI tests are explicit: ordinary ctest also works without a display.
endif()
