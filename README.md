## EE4208 Edge Detection

This project provides implementation of canny edge detector, LoG edge detector and canny detector with subpixel accuracy[*](https://github.com/fcqing/sub-pixel-edge-detect), in C++.

```
ee4208_edge_detection
  ├── CMakeLists.txt
  ├── README.md
  ├── canny.sh
  ├── LoG.sh
  ├── canny_subpixel.sh
  ├── src
    ├── raw_to_pgm.cpp
    ├── canny.cpp
    ├── LoG.cpp
    ├── canny_devernay_subpixel.cpp
    ├── edge_devernay.cpp
    └── edge_devernay.h
  ├── raw_images
      ├── cana.raw
      ├── fruit.raw
      └── ...
  └── sample_output
      ├── cana_canny.pgm
      ├── cana_LoG.pgm
      ├── cana_subpixel_x1.pgm
      ├── cana_subpixel_x2.pgm
      └── cana_edge_points.txt
```


### Requirements
* cmake
* opencv


### Build the project
* clone the project
```
git clone https://github.com/jhchenjh/ee4208_edge_detection.git
```
* in _CMakeLists.txt_, set _OpenCV_DIR_.
```
SET(OpenCV_DIR /path/to/OpenCV/)
```
* build the project
```
mkdir build
cd build
cmake ..
make
cd ..
```
