// FaceDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using cv::Mat;
using cv::waitKey;
using std::wcerr;

int main(int argc, char** argv)
{
	CvCapture* capture = cvCaptureFromCAM(0);
	if (!capture)
	{
		wcerr << L"No camera is detected.\n";
		return 1;
	}

	Mat frame;
	while (true)
	{
		frame = cvQueryFrame(capture);
		if (frame.empty())
		{
			wcerr << L"No captured frame.\n";
			return 2;
		}

		imshow("Face detection", frame);

		int c = waitKey(10);
		if ((char)c == 'c') { return 0; }
	}
	
	return 0;
}

