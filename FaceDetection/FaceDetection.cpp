// FaceDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << " Usage: display_image ImageToLoadAndDisplay" << endl;
		return -1;
	}

	IplImage* img = cvLoadImage( argv[1] );

	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	cvShowImage("Display window", img);

	while (1)
	{
		if (cvWaitKey(100) == 27) break;
	}

	cvDestroyAllWindows();
	cvReleaseImage(&img);
	return 0;
}

