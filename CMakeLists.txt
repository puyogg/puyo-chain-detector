cmake_minimum_required(VERSION 3.17)

project(puyo-chain-detector LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Get the JsonCpp Library
include(FetchContent)
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(jsoncpp)

# Find Qt6
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Find Boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS date_time REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

# Generate library targets: chainsim, detector
# Executable target: PuyoChainDetector
add_subdirectory(src)
