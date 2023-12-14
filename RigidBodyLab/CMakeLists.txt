cmake_minimum_required(VERSION 3.13.4)
project(RigidBodyLab)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the runtime library for MSVC
if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

# Paths to GLFW, ImGui, Google Test, and GLM relative to this CMakeLists.txt
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/glfw")
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/imgui")
set(GOOGLETEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/googletest")
set(GLM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/glm")
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/stb")
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/glad")

# Add the subdirectories for dependencies
add_subdirectory(${GLFW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glfw)
add_subdirectory(${GOOGLETEST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/googletest)

# Manually include ImGui files
file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp" "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp" "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
file(GLOB IMGUI_HEADERS "${IMGUI_DIR}/*.h" "${IMGUI_DIR}/backends/imgui_impl_glfw.h" "${IMGUI_DIR}/backends/imgui_impl_opengl3.h")
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

# Include directories for the main project
include_directories(${GLFW_DIR}/include)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${GLM_DIR})
include_directories(${STB_DIR})
include_directories(${GLAD_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../extern/freeimage)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/include)

# Add source files for the main project
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/include/*.h")

# Group source files for Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Define the FreeImage library
add_library(FreeImage STATIC IMPORTED)
set_property(TARGET FreeImage PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib/freeimage/FreeImage.lib)

# Define the executable for the main project
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/src/glad.c")

# Link libraries with the main project
target_link_libraries(${PROJECT_NAME} glfw imgui opengl32 FreeImage)

# Copy the FreeImage.dll to the build output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/../lib/freeimage/FreeImage.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Define the test project
set(TEST_PROJECT_NAME ${PROJECT_NAME}_Test)

# Include directories for the test project
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/include")
include_directories(${GLM_DIR})

# Add source files for the test project
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/tests/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/RigidBodyLab/tests/*.h")

# Group source files for Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES} ${TEST_HEADERS})

# Define the executable for the test project
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES} ${TEST_HEADERS})

# Link libraries with the test project
target_link_libraries(${TEST_PROJECT_NAME} gtest gtest_main glfw imgui opengl32)

# Enable testing
enable_testing()

# Add the tests to be run
add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

# Set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Create solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Organize projects into folders
set_property(TARGET gtest        PROPERTY FOLDER "Dependencies")
set_property(TARGET gtest_main   PROPERTY FOLDER "Dependencies")
set_property(TARGET gmock        PROPERTY FOLDER "Dependencies")
set_property(TARGET gmock_main   PROPERTY FOLDER "Dependencies")
set_property(TARGET imgui        PROPERTY FOLDER "Dependencies")
