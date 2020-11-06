## Pillow resize function in C++

Usually you find that results from openCV resize and Pillow(Python) resize function are different. Especially when one model was trained using Pillow(Python) and some deep learning framework, and you want to accelerate it using C++ and openCV. When you need Pillow(Python) resize function in C++, you can simply using the following code to implement it. 

If this give you help, please give me a star, thank you.

### Supported mode
Codes are on Pillow-7.2.0.

| mode | support|
| ---- | ---- |
| "L" | YES |
|"RGB" | YES |
| "RGBA" | 0.4 difference average value |

Now only mode "L" and "RGB" were same with Python-Pillow. Result of "RGBA" mode is a little different now.

Other modes will find there ways.

## Version

V0.1. 

C++11 above.

Only tested on Ubuntu18.04, and it should work on most linux.

## Usage

Pillow(Python) resize filters: 

| value | macro |
| ---- | ---- |
| 0 | IMAGING_TRANSFORM_NEAREST | 
| 1 | IMAGING_TRANSFORM_LANCZOS |
| 2 | IMAGING_TRANSFORM_BILINEAR| 
| 3 | IMAGING_TRANSFORM_BICUBIC |
| 4 | IMAGING_TRANSFORM_BOX | 
| 5 | IMAGING_TRANSFORM_HAMMING |


Example code:
main.cpp
```C++
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "ImPlatformSim.h"
#include "ImagingSim.h"
int main(){
	cv::Mat oriImg = cv::imread("../liziqi.jpg");
	cv::cvtColor(oriImg, oriImg, cv::COLOR_BGR2RGB);
	cv::Mat imgRes;
	if(pillowResize(oriImg, imgRes, "RGB", 200, 200, IMAGING_TRANSFORM_NEAREST) !=0)
		return 0;
	cv::cvtColor(imgRes, imgRes, cv::COLOR_RGB2BGR);
	cv::imwrite("../res.png", imgRes);
	return 0;
}
```
Obviously you need install openCV. And after installing it, just run code below.

```Bash
cd build
cmake ..
make
./app
```
