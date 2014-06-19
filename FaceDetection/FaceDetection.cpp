#include "stdafx.h"
#include <iostream>

#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"

using namespace cv;
using namespace std;

#define MAX_FACES			3		// max support detected faces
#define MIN_COUNT			20		// min key points gate (can not too small)

static char WIN_NAME[] = "Face Tracking";
static RNG rng(12345);

// transform gate:
#define TRANS_ENABLE			1

// judge if two rect overlapped:
static bool _faces_overlap(Rect& rect, vector<Rect>& target)
{
	for (size_t i = 0; i < target.size(); ++i)
	{
		if (rect.x > target[i].x + target[i].width)
			continue;
		if (rect.y > target[i].y + target[i].height)
			continue;
		if (rect.x + rect.width < target[i].x)
			continue;
		if (rect.y + rect.height < target[i].y)
			continue;
		return true;
	}
	return false;
}

static bool _detect_faces(	Mat& gray_frame, 
							CascadeClassifier& frontal_face_detector, 
							CascadeClassifier& profile_face_detector,
							vector<Rect>& Faces) 
{
	// check:
	if(gray_frame.empty())
		return false;

	Mat equalized_frame;
	equalizeHist(gray_frame, equalized_frame);

	// front face:
	Faces.clear();
	frontal_face_detector.detectMultiScale(equalized_frame, Faces, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(100, 100));
	if(Faces.size() >= MAX_FACES)
		return true;

	// profile face:
	vector<Rect> Faces_t;
	profile_face_detector.detectMultiScale(equalized_frame, Faces_t, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(100, 100));

	for(size_t i=0; i<Faces_t.size(); ++i)
	{
		if(Faces.size() >= MAX_FACES)
			break;
			
		// check if overlap:
		if(_faces_overlap(Faces_t[i], Faces))
			continue;
			
		Faces.push_back(Faces_t[i]);
	}

	// debug:
	cout << "Faces.size() = " << Faces.size() << endl;

	if(Faces.size())
		return true;
	else
		return false;
}

static bool _update_mask_rois(vector<Rect>& Faces, Size size, vector<Mat>& mask_roi)
{
	Mat roi_t;
	Point center;
	for (size_t i = 0; i < Faces.size(); ++i)
	{	
		// calculate the center: we only support one face currently...	
		roi_t = Mat::zeros(size, CV_8U);
		center = Point(Faces[i].x + Faces[i].width / 2, Faces[i].y + Faces[i].height / 2);
		ellipse(roi_t, center, Size(Faces[i].width / 3, Faces[i].height / 3), 0, 0, 360, Scalar(255, 0, 255), -1, 8, 0);
		mask_roi.push_back(roi_t);
	}

	return true;
}

int main(int argc, char** argv)
{
	// face detector init:
	CascadeClassifier frontal_face_cascade;
	if (!frontal_face_cascade.load("Resource Files/haarcascade_frontalface_alt.xml"))
	{ 
		cout << "Cannot load front face cascade file.\n"; 
		return -1; 
	};
	CascadeClassifier profile_face_cascade;
	if (!profile_face_cascade.load("Resource Files/haarcascade_profileface.xml"))
	{
		cout << "Cannot load profile face cascade file.\n";
		return -1;
	};
	
	// open camera video:
	VideoCapture capture(0); 
	if (!capture.isOpened())
	{
		cout << "can not open camera!" << endl;
		return -1;
	}

	// create window:
	namedWindow(WIN_NAME, CV_WINDOW_AUTOSIZE);

	// wait for camera video really come in:
	Mat t;
	do{
		capture >> t;
	} while (t.empty());

	// OK, let's show forever...
	Mat target_frame, gray_frame, prev_gray_frame;
	vector<Rect> Faces;
	vector<Mat> mask_roi;
	// Keypoint detect init:
	int maxCorners = 400;
	double minDistance = 5.0;
	GoodFeaturesToTrackDetector feature_detector(maxCorners, 0.01, minDistance, 3, true, 0.04); // using GFTT Detector
	vector<KeyPoint> keypoints;
	vector<Point2f> old_points[MAX_FACES];
	vector<Point2f> cur_points[MAX_FACES];
	bool needToInit = false;
	int init_count = 0;
	vector<Point2f> old_corners[MAX_FACES];
	vector<Point2f> cur_corners(4);
	Point2f face_corner;
	vector<Point2f> screen_cornners(4);
	// corners should be in image:
	size_t screen_width = (size_t)capture.get(CV_CAP_PROP_FRAME_WIDTH);
	size_t screen_height = (size_t)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	screen_cornners[0] = cvPoint(0, 0);
	screen_cornners[1] = cvPoint(screen_width, 0);
	screen_cornners[2] = cvPoint(screen_width, screen_height);
	screen_cornners[3] = cvPoint(0, screen_height);

	// enter main loop:
	for (;;)
	{
		// get raw image:
		capture >> target_frame;
		if (target_frame.empty())
		{
			cout << "no camera video." << endl;
			break;
		}
		// conver to gray image:
		cvtColor(target_frame, gray_frame, cv::COLOR_RGB2GRAY);
		// smooth it, otherwise a lot of false circles may be detected
		GaussianBlur(gray_frame, gray_frame, Size(9, 9), 0, 0);

		// Detect Keypoints:
		if (needToInit) 
		{
			cout << "init times = " << ++init_count << endl;

			// decide init ROI:
			if (!_detect_faces(gray_frame, frontal_face_cascade, profile_face_cascade, Faces))
			{
				cout << "can not detect faces." << endl;
				goto SHOW_FRAME;
			}

			// update mask roi:
			mask_roi.clear();
			_update_mask_rois(Faces, target_frame.size(), mask_roi);

			// detect keypoints:
			for (size_t i = 0; i < mask_roi.size(); ++i)
			{
				feature_detector.detect(target_frame, keypoints, mask_roi[i]);
				KeyPoint::convert(keypoints, cur_points[i]);
				cout << i <<  ": init keypoints = " << cur_points[i].size() << endl;
				//draw init rectangle:
				//rectangle(target_frame, Faces[i], Scalar(255, 0, 255), 2, 8);
				//record corners:
				old_corners[i].clear(); // important!!!
				face_corner = cvPoint(Faces[i].x, Faces[i].y);
				old_corners[i].push_back(face_corner);
				face_corner = cvPoint(Faces[i].x + Faces[i].width, Faces[i].y);
				old_corners[i].push_back(face_corner);
				face_corner = cvPoint(Faces[i].x + Faces[i].width, Faces[i].y + Faces[i].height);
				old_corners[i].push_back(face_corner);
				face_corner = cvPoint(Faces[i].x, Faces[i].y + Faces[i].height);
				old_corners[i].push_back(face_corner);
			}

			// only tracking...
			needToInit = false;
		}
		else // start tracking...
		{
			for (size_t face_index = 0; face_index < mask_roi.size(); ++face_index)
			{
				if (old_points[face_index].empty())
					continue;

				// track keypoints:
				vector<uchar> status;
				vector<float> err;
				if (prev_gray_frame.empty())
					gray_frame.copyTo(prev_gray_frame);
				cur_points[face_index].clear();
				cv::calcOpticalFlowPyrLK(prev_gray_frame, gray_frame, old_points[face_index], cur_points[face_index], status, err, Size(21, 21), 2);
				size_t i, k;
				for (i = k = 0; i < cur_points[face_index].size(); i++)
				{
					// remove invalid points:
					if (!status[i])
						continue;

					// remove out of range points:
					if (cv::pointPolygonTest(old_corners[face_index], cur_points[face_index][i], false) < 0)
						continue;

					cur_points[face_index][k] = cur_points[face_index][i];
					old_points[face_index][k] = old_points[face_index][i];
					k++;
					circle(target_frame, cur_points[face_index][i], 3, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
						rng.uniform(0, 255)), 1, 8);
				}
				cur_points[face_index].resize(k);
				old_points[face_index].resize(k);
				//cout << "size = " << k << endl;

#if (TRANS_ENABLE == 1)
				// affine transform:
				if (old_points[face_index].size() >= MIN_COUNT && cur_points[face_index].size() >= MIN_COUNT)
				{
					int key_size = min(old_points[face_index].size(), cur_points[face_index].size());
					std::vector<Point2f> old_points_t;
					std::vector<Point2f> cur_points_t;
					for (size_t i = 0; i < key_size; ++i)
					{
						old_points_t.push_back(old_points[face_index][i]);
						cur_points_t.push_back(cur_points[face_index][i]);
					}
					// calculate tranform metrix:
					//Mat H = cv::getAffineTransform(old_points_t, cur_points_t);
					Mat H = cv::estimateRigidTransform(old_points_t, cur_points_t, false);
					if(H.rows != 2 || H.cols != 3) // need 2x3 metrix!!!
					{
						cout << "transform metrix invalid: rows = " << H.rows << ", cols = " << H.cols << endl;
						break;
					}
					cv::transform(old_corners[face_index], cur_corners, H);

					// judge if in screen:
					int number_gate = 0;
					for (size_t i = 0; i < 4; ++i)
					{
						if (cv::pointPolygonTest(screen_cornners, cur_corners[i], false) < 0)
							number_gate++;
					}
					if (number_gate > 2) // when more than 2 corners out of screen, re-init:
					{
						cout << "corrner out of range!" << endl;
						old_points[face_index].clear();
						cur_points[face_index].clear();
						needToInit = true;
					}
					else
					{
						//-- Draw lines between the corners (the mapped object in the scene - image_2 )
						line(target_frame, cur_corners[0], cur_corners[1], Scalar(0, 255, 0), 2);
						line(target_frame, cur_corners[1], cur_corners[2], Scalar(0, 255, 0), 2);
						line(target_frame, cur_corners[2], cur_corners[3], Scalar(0, 255, 0), 2);
						line(target_frame, cur_corners[3], cur_corners[0], Scalar(0, 255, 0), 2);

						// update previous cornners:
						old_corners[face_index][0] = cur_corners[0];
						old_corners[face_index][1] = cur_corners[1];
						old_corners[face_index][2] = cur_corners[2];
						old_corners[face_index][3] = cur_corners[3];
					}
				}
#endif // #if (TRANS_ENABLE == 1)
			} // face_index loop
		} // if (needToInit)

		// update previous frame:
		cv::swap(prev_gray_frame, gray_frame);
		if (mask_roi.size() == 0)
		{
			needToInit = true;
		}
		if (!needToInit)
		{
			for (size_t face_index = 0; face_index < mask_roi.size(); ++face_index)
			{
				if (cur_points[face_index].size() < MIN_COUNT)
				{
					cout << face_index << ": keypoints too little - size() = " << cur_points[face_index].size() << endl;
					needToInit = true;
					old_points[face_index].clear(); // clear old key points...
				}
				else
				{
					std::swap(cur_points[face_index], old_points[face_index]);
				}
			}
		}
		
SHOW_FRAME:
		// show window:
		imshow(WIN_NAME, target_frame);

		if (' ' == waitKey(16))
			break;
	}

	return 0;
}

