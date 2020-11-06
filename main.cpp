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
	if(pillowResize(oriImg, imgRes,"RGB", 200, 200, IMAGING_TRANSFORM_NEAREST) !=0)
		return 0;
	cv::cvtColor(imgRes, imgRes, cv::COLOR_RGB2BGR);
	cv::imwrite("../res.png", imgRes);
	return 0;
}
