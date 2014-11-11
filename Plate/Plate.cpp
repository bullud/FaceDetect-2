#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using cv::Mat;
using cv::imread;
using cv::cvtColor;
using cv::namedWindow;
using cv::waitKey;
using cv::WINDOW_AUTOSIZE;
using std::cout;

int main(int argc, char** argv)
{
	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Mat gray;
	cvtColor(image, gray, CV_RGB2GRAY);

	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", gray);                   // Show our image inside it.

	waitKey(0);
	return 0;
}