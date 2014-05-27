// FaceDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

using cv::Mat;
using cv::waitKey;
using cv::CascadeClassifier;
using cv::Rect;
using cv::Size;
using cv::Point;
using cv::Scalar;
using cv::ellipse;
using std::wcerr;
using std::vector;
using std::for_each;

void detect(CascadeClassifier &front_face_cascade, CascadeClassifier &profile_face_cascade, Mat &frame);

int main(int argc, char** argv)
{
	CascadeClassifier front_face_cascade;
	if (!front_face_cascade.load("Resource Files/haarcascade_frontalface_alt.xml"))
	{ 
		wcerr << L"Cannot load front face cascade file.\n"; 
		return -1; 
	};

	CascadeClassifier profile_face_cascade;
	if (!front_face_cascade.load("Resource Files/haarcascade_profileface.xml"))
	{
		wcerr << L"Cannot load profile face cascade file.\n";
		return -1;
	};

	CvCapture* capture = cvCaptureFromCAM(-1);
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

		detect(front_face_cascade, profile_face_cascade, frame);

		//-- Show what you got
		imshow("Face detection", frame);

		int c = waitKey(10);
		if ((char)c == 'c') 
		{ 
			return 0;
		}
	}
	
	return 0;
}

void detect(CascadeClassifier &front_face_cascade, CascadeClassifier &profile_face_cascade, Mat &frame)
{
	Mat frame_gray;
	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	vector<Rect> faces;
	front_face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size{ 30, 30 });

	for_each(faces.begin(), faces.end(), [&frame](const Rect &face)
	{
		Point center{ int(face.x + face.width * 0.5), int(face.y + face.height * 0.5) };
		ellipse(frame, center, Size{ int(face.width * 0.5), int(face.height * 0.5) }, 0, 0, 360, Scalar{ 0, 255, 0 }, 1, 8, 0);
	});

	faces.clear();
	profile_face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size{ 30, 30 });

	for_each(faces.begin(), faces.end(), [&frame](const Rect &face)
	{
		Point center{ int(face.x + face.width * 0.5), int(face.y + face.height * 0.5) };
		ellipse(frame, center, Size{ int(face.width * 0.5), int(face.height * 0.5) }, 0, 0, 360, Scalar{ 0, 255, 0 }, 1, 8, 0);
	});
}


