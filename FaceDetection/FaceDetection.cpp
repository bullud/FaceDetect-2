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
#define MIN_COUNT			15		// min key points gate (can not too small)

static char WIN_NAME[] = "Face Tracking";
static RNG rng(12345);

// transform gate:
#define TRANS_ENABLE			1



// structure to describe face:
struct face_descriptor
{
	bool _valid;
	Rect _face_rect;
	Mat _mask_roi;
	vector<Point2f> _old_points;
	vector<Point2f> _cur_points;
	vector<Point2f> _old_corners;
};

// face desciptors:
static struct face_descriptor CurFaceInfo[MAX_FACES]; 

// invalid all face info:
static void _reset_face_info()
{
	for(size_t i=0; i<MAX_FACES; ++i)
	{
		CurFaceInfo[i]._valid = false;
		CurFaceInfo[i]._old_points.clear();
		CurFaceInfo[i]._cur_points.clear();
		CurFaceInfo[i]._old_corners.clear();
	}
}

// find free face descriptor:
static bool _find_free_node(size_t& index)
{
	for(index=0; index<MAX_FACES; ++index)
	{
		if(!CurFaceInfo[index]._valid)
			return true;
	}
	return false; 
}

// convert rect to points array:
static void _rect_to_points(Rect& rect, vector<Point2f>& points)
{
	Point2f face_corner;
	points.clear(); // important!!!
	face_corner = cvPoint(rect.x, rect.y);
	points.push_back(face_corner);
	face_corner = cvPoint(rect.x + rect.width, rect.y);
	points.push_back(face_corner);
	face_corner = cvPoint(rect.x + rect.width, rect.y + rect.height);
	points.push_back(face_corner);
	face_corner = cvPoint(rect.x, rect.y + rect.height);
	points.push_back(face_corner);
}

// just face rect overlapped with current face info:
static bool _really_new_face(Rect& new_face)
{
	vector<Point2f> face_points;
	Point2f center;
	_rect_to_points(new_face, face_points);

	for(size_t i = 0; i < MAX_FACES; ++i)
	{
		if(!CurFaceInfo[i]._valid)
			continue;

		// old_corners overlapped with new face rect:
		// 1. old_corners in new faces:
		for(size_t k = 0; k < 4; ++k)
		{
			if(cv::pointPolygonTest(face_points, CurFaceInfo[i]._old_corners[k], false) > 0)
			{
				//cout << "[1 failed]" << endl;
				return false;
			}
		}
		// 2. new face corners in old_corners:
		center = Point(new_face.x + new_face.width / 2, new_face.y + new_face.height / 2);
		face_points.push_back(center);
		for(size_t k = 0; k < 5; ++k)
		{
			if(cv::pointPolygonTest(CurFaceInfo[i]._old_corners, face_points[k], false) > 0)
			{
				//cout << "[2 failed]" << endl;
				return false;
			}
		}
	}
	return true;
}

// detect detected faces overlap:
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

	if(Faces.size())
		return true;
	else
		return false;
}

static bool _create_mask_rois(vector<Rect>& Faces, Size size, vector<Mat>& mask_roi)
{
	Mat roi_t;
	Point center;
	mask_roi.clear();
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

static bool _detect_keypoints(Mat& target_frame, 
								GoodFeaturesToTrackDetector& feature_detector, 
								Mat& mask_roi_t, 
								Rect& Faces_t,
								vector<Point2f>& cur_points_t,
								vector<Point2f>& old_corners_t)
{
	vector<KeyPoint> keypoints;
	Point2f face_corner;
	feature_detector.detect(target_frame, keypoints, mask_roi_t);
	KeyPoint::convert(keypoints, cur_points_t);
	//draw init rectangle:
	//rectangle(target_frame, Faces[i], Scalar(255, 0, 255), 2, 8);
	//record corners:
	_rect_to_points(Faces_t, old_corners_t);

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
	vector<Rect> NewFaces;
	vector<Mat> mask_roi;
	
	// Keypoint detector init:
	int maxCorners = 400;
	double minDistance = 5.0;
	GoodFeaturesToTrackDetector feature_detector(maxCorners, 0.01, minDistance, 3, true, 0.04); // using GFTT Detector
	
	vector<Point2f> cur_corners(4);
	vector<Point2f> screen_cornners(4);
	// corners should be in image:
	size_t screen_width = (size_t)capture.get(CV_CAP_PROP_FRAME_WIDTH);
	size_t screen_height = (size_t)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	screen_cornners[0] = cvPoint(0, 0);
	screen_cornners[1] = cvPoint(screen_width, 0);
	screen_cornners[2] = cvPoint(screen_width, screen_height);
	screen_cornners[3] = cvPoint(0, screen_height);

	// enter main loop:
	int frame_no = 0, detect_cnt = 0;
	int detect_ratio = 60;
	_reset_face_info();
	cout << "==================== start ====================" << endl;
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

		// decide detect ratio:
		detect_ratio = 1;
		for(size_t i = 0; i < MAX_FACES; ++i)
		{
			// at least one face be tracking...
			if(CurFaceInfo[i]._valid)
			{
				detect_ratio = 60;
				break;
			}
		}

		// every 1 second, we detect faces:
		if((frame_no % detect_ratio) == 0)
		{
			// debug:
			size_t tracking_faces = 0;
			for(size_t i = 0; i < MAX_FACES; ++i)
			{
				if(CurFaceInfo[i]._valid)
					tracking_faces++;
			}

			// decide init ROI:
			if (!_detect_faces(gray_frame, frontal_face_cascade, profile_face_cascade, NewFaces))
			{
				cout << "Status: tracking faces = " << tracking_faces << ", no face detect " << (frame_no % 2 ? "0_o" : "o_0") << endl;
				goto TRACK_FACE;
			}

			// check if overlapped with current tracking face:
			Faces.clear();
			for(size_t i = 0; i < NewFaces.size(); ++i)
			{
				if(!_really_new_face(NewFaces[i]))
					continue;
				
				// really new face:
				Faces.push_back(NewFaces[i]);
			}
			cout << "Status: tracking faces = " << tracking_faces << ", detect -> (new/all) = " << Faces.size() << "/" << NewFaces.size() << endl;

			// if no new face added...
			if(Faces.size() == 0)
			{
				goto TRACK_FACE;
			}

			// update mask roi:
			_create_mask_rois(Faces, target_frame.size(), mask_roi);

			// save info:
			size_t free_index;
			for(size_t i = 0; i < mask_roi.size(); ++i)
			{
				if(!_find_free_node(free_index))
					break;

				CurFaceInfo[free_index]._valid = true;
				CurFaceInfo[free_index]._face_rect = Faces[i];
				CurFaceInfo[free_index]._mask_roi = mask_roi[i];
				CurFaceInfo[free_index]._old_points.clear(); // new-valid node, clear old_points! important!

				// detect keypoints:
				_detect_keypoints(	target_frame, 
									feature_detector,
									CurFaceInfo[free_index]._mask_roi,
									CurFaceInfo[free_index]._face_rect, 
									CurFaceInfo[free_index]._cur_points,
									CurFaceInfo[free_index]._old_corners);
				cout << free_index <<  ": new init keypoints = " << CurFaceInfo[free_index]._cur_points.size() << endl;
			}
		}

TRACK_FACE:
		for (size_t face_index = 0; face_index < MAX_FACES; ++face_index)
		{
			// check if valid:
			if(!CurFaceInfo[face_index]._valid)
				continue;

			// check if old_points valid:
			if (CurFaceInfo[face_index]._old_points.empty())
				continue;
			
			//cout << "track: face_index = " << face_index << endl;

			// track keypoints:
			vector<uchar> status;
			vector<float> err;
			if (prev_gray_frame.empty())
				gray_frame.copyTo(prev_gray_frame);
			CurFaceInfo[face_index]._cur_points.clear();
			cv::calcOpticalFlowPyrLK(prev_gray_frame, 
									 gray_frame, 
									 CurFaceInfo[face_index]._old_points, CurFaceInfo[face_index]._cur_points,
									 status, err, Size(21, 21), 2);
			size_t i, k;
			for (i = k = 0; i < CurFaceInfo[face_index]._cur_points.size(); i++)
			{
				// remove invalid points:
				if (!status[i])
					continue;

				// remove out of range points:
				if (pointPolygonTest(CurFaceInfo[face_index]._old_corners, CurFaceInfo[face_index]._cur_points[i], false) < 0)
					continue;

				CurFaceInfo[face_index]._cur_points[k] = CurFaceInfo[face_index]._cur_points[i];
				CurFaceInfo[face_index]._old_points[k] = CurFaceInfo[face_index]._old_points[i];
				k++;
				circle(target_frame, CurFaceInfo[face_index]._cur_points[i], 3, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
					rng.uniform(0, 255)), 1, 8);
			}
			CurFaceInfo[face_index]._cur_points.resize(k);
			CurFaceInfo[face_index]._old_points.resize(k);
			//cout << "k = " << k << endl;

#if (TRANS_ENABLE == 1)
			// affine transform:
			if (CurFaceInfo[face_index]._cur_points.size() >= MIN_COUNT && CurFaceInfo[face_index]._old_points.size() >= MIN_COUNT)
			{
				// calculate tranform metrix:
				//Mat H = getAffineTransform(old_points_t, cur_points_t);
				Mat H = estimateRigidTransform(CurFaceInfo[face_index]._old_points, CurFaceInfo[face_index]._cur_points, false);
				if(H.rows != 2 || H.cols != 3)
				{
					cout << "error: H.rows = " << H.rows << ", H.cols = " << H.cols << endl;
					break;
				}
				cv::transform(CurFaceInfo[face_index]._old_corners, cur_corners, H);

				// judge if in screen:
				int number_gate = 0;
				for (size_t i = 0; i < 4; ++i)
				{
					if (pointPolygonTest(screen_cornners, cur_corners[i], false) < 0)
						number_gate++;
				}
				if (number_gate > 2) // when more than 2 corners out of screen, re-init:
				{
					cout << "corrner out of range!" << endl;
					CurFaceInfo[face_index]._old_points.clear();
					CurFaceInfo[face_index]._cur_points.clear();
				}
				else
				{
					//-- Draw lines between the corners (the mapped object in the scene - image_2 )
					line(target_frame, cur_corners[0], cur_corners[1], Scalar(0, 255, 0), 2);
					line(target_frame, cur_corners[1], cur_corners[2], Scalar(0, 255, 0), 2);
					line(target_frame, cur_corners[2], cur_corners[3], Scalar(0, 255, 0), 2);
					line(target_frame, cur_corners[3], cur_corners[0], Scalar(0, 255, 0), 2);

					// update previous cornners:
					CurFaceInfo[face_index]._old_corners[0] = cur_corners[0];
					CurFaceInfo[face_index]._old_corners[1] = cur_corners[1];
					CurFaceInfo[face_index]._old_corners[2] = cur_corners[2];
					CurFaceInfo[face_index]._old_corners[3] = cur_corners[3];
				}
			}
#endif // #if (TRANS_ENABLE == 1)
		} // face_index loop

		// update previous frame:
		cv::swap(prev_gray_frame, gray_frame);
		for (size_t face_index = 0; face_index < MAX_FACES; ++face_index)
		{
			if(!CurFaceInfo[face_index]._valid)
				continue;

			// update points:
			if (CurFaceInfo[face_index]._cur_points.size() < MIN_COUNT)
			{
				//cout << face_index << ": keypoints too little - size() = " << CurFaceInfo[face_index]._cur_points.size() << endl;
				CurFaceInfo[face_index]._valid = false; // free node
			}
			else
			{
				std::swap(CurFaceInfo[face_index]._cur_points, CurFaceInfo[face_index]._old_points);
			}
		}	

		// show window:
		imshow(WIN_NAME, target_frame);
		frame_no++;
		//cout << "frame - " << frame_no << endl;

		if (' ' == waitKey(16))
			break;
	}

	return 0;
}

