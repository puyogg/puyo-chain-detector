# Puyo Chain Detector
A real-time overlay for showing trigger points in Puyo chains.

## Building the chain detector from source (Windows)

### 1. Install OpenCV

- Download and extract the prebuilt binaries by following the links on the OpenCV Docs: https://docs.opencv.org/4.5.1/d3/d52/tutorial_windows_install.html#tutorial_windows_install_prebuilt
- Set an environment variable named `OpenCV_DIR` to the extraction path, specifically down to one of the `\lib` directories. For example, i have `OpenCV_DIR` set to `C:\OpenCV\build\x64\vc15\lib`. This ensures that it'll link correctly with CMake.

### 2. Install Boost (with static libs enabled)
I don't even know what I did half the time lol.

- Download the 7z file from this [link](https://www.boost.org/doc/libs/1_75_0/more/getting_started/windows.html) and extract it to a directory, e.g. `C:\boost_1_75_0`
- From the command line, cd to the extracted Boost directory and run `.\BOOTSTRAP.bat` followed by `.\b2` (it takes forever).
- Set these environment variables so CMake finds everything:
    - **BOOST_INCLUDEDIR**: `C:\boost_1_75_0\`
    - **BOOST_LIBRARYDIR**: `C:\boost_1_75_0\stage\lib`
    - **BOOST_ROOT**: `C:\boost_1_75_0\`