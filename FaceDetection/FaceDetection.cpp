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

#include "source_lib/constants.h"	
#include "source_lib/findEyeCenter.h"

using namespace cv;
using namespace std;

#define MAX_FACES			5		// max support detected faces
#define MIN_COUNT			15		// min key points gate (can not too small)

static char WIN_NAME[] = "Face Tracking";
static RNG rng(12345);

// transform gate:
#define TRANS_ENABLE			1

///////////////////////////////////////////////// Help Functions ///////////////////////////////////////////////////

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

// convert points array to rect: 4 points need at least
static void _points_to_rect(vector<Point2f>& corners, Rect& rect)
{
	float x1, x2, y1, y2;
	// most left:
	x1 = corners[0].x;
	x2 = x1;
	y1 = corners[0].y;
	y2 = y1;
	for(size_t i=1; i<corners.size(); ++i)
	{
		if(x1 > corners[i].x) 
			x1 = corners[i].x;
		if(x2 < corners[i].x)
			x2 = corners[i].x;
		if(y1 > corners[i].y)
			y1 = corners[i].y;
		if(y2 < corners[i].y)
			y2 = corners[i].y;
	}
	rect = Rect((int)cvRound(x1), (int)cvRound(y1), (int)cvRound(x2-x1), (int)cvRound(y2-y1)); 
}

// judge two rect overlapped or not:
static bool _rect_overlap(Rect& r1, Rect& r2, int allow=0)
{
	if ((r1.x + allow) > r2.x + r2.width)
		return false;
	if ((r1.y + allow) > r2.y + r2.height)
		return false;
	if (r1.x + r1.width < (r2.x + allow))
		return false;
	if (r1.y + r1.height < (r2.y + allow))
		return false;
	return true;
}

///////////////////////////////////////////////// Face Detection ///////////////////////////////////////////////////

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
		size_t k;
		for(k=0; k<Faces.size(); ++k)
		{
			if(_rect_overlap(Faces_t[i], Faces[k]))
				break;
		}
		if(k < Faces.size())
			continue;

		Faces.push_back(Faces_t[i]);
	}

	if(Faces.size())
		return true;
	else
		return false;
}

// create mask ROIs for detecting keypoints:
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

// detect keypoints:
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

////////////////////////////////////////////////// Generate Normalized Faces ////////////////////////////////////////////////

#define SET_MIN(x, min)		do{if((x) < (min)) (x) = (min);}while(0)
#define SET_MAX(x, max)		do{if((x) > (max)) (x) = (max);}while(0)

const double DESIRED_LEFT_EYE_X = 0.16;
const double DESIRED_LEFT_EYE_Y = 0.14;
const double DESIRED_RIGHT_EYE_X = 1 - 0.16;
const double DESIRED_RIGHT_EYE_Y = 1 - 0.14;
const int DESIRED_FACE_WIDTH = 150;
const int DESIRED_FACE_HEIGHT = 150;
const int MIN_EYES_DISTANCE = 20;

struct norm_face_des {
	Mat _image;
	String _win_name;
};
static vector<norm_face_des> NormFaceInfo[MAX_FACES];

// save and display one normalized face:
static void _push_norm_face(size_t face_index, Mat& frame)
{
	struct norm_face_des norm_face;
	norm_face._image = frame.clone();
	size_t index = NormFaceInfo[face_index].size();
	char name[20];
	sprintf_s(name, "%d-%d", face_index+1, index+1);
	norm_face._win_name = String(name);
	namedWindow(norm_face._win_name);
	moveWindow(norm_face._win_name, index*30, face_index*40+index*30);
	imshow(norm_face._win_name,  norm_face._image);
	NormFaceInfo[face_index].push_back(norm_face);
}

// delete last normalized face:
static void _pop_norm_face(size_t face_index)
{
	size_t size = NormFaceInfo[face_index].size();
	if(size > 0)
	{
		struct norm_face_des norm_face;
		norm_face = NormFaceInfo[face_index][size-1];
		destroyWindow(norm_face._win_name);
		NormFaceInfo[face_index].pop_back();	
	}
}

// create normalized face, migarted from Keven's code:
static bool _create_one_norm_face(	size_t face_index, 
									Mat& gray_frame, 
									CascadeClassifier& eye_detector, 
									CascadeClassifier& glasses_detector)
{
	// 1. create face image:
	//cout << "step 1: create face image." << endl;
	Rect face_rect;
	_points_to_rect(CurFaceInfo[face_index]._old_corners, face_rect);
	SET_MIN(face_rect.x, 0);
	SET_MIN(face_rect.y, 0);
	SET_MAX(face_rect.width, (gray_frame.cols - face_rect.x));
	SET_MAX(face_rect.height, (gray_frame.rows - face_rect.y));
	Mat face_image = gray_frame(face_rect); 
	//imshow("Face", face_image);

	// 2. detect eyes using opencv library:
	//cout << "step 2: detect eyes using opencv." << endl;
	vector<Rect> EyeRect;
	eye_detector.detectMultiScale(face_image, EyeRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
	if(EyeRect.size() != 2) 
	{
		EyeRect.clear();
		glasses_detector.detectMultiScale(face_image, EyeRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
		if(EyeRect.size() != 2)
		{
			cout << "Error: EyeRect.size() = " << EyeRect.size() << endl;
			return false;
		}
	}
	if(_rect_overlap(EyeRect[0], EyeRect[1]))
	{
		cout << "Error: two eyes too close." << endl;
		return false;
	}
	//for(size_t i=0; i<EyeRect.size(); ++i)
	//	rectangle(face_image, EyeRect[i], Scalar(255), 2, 8);
	//imshow("opencv eyes", face_image);

	// 3. detect eyes using 3rd-part library:
	//cout << "step 3: detect eyes using 3rd-part lib." << endl;
	Mat show_face = face_image.clone(); // for display only, to be removed...
	Point leftEye = Point(-1,-1);
	Point rightEye = Point(-1,-1);
	//optimal region size for iris detection
	int eye_region_width = (int)(face_image.size().width * (kEyePercentWidth/100.0));	
	int eye_region_height = (int)(face_image.size().height * (kEyePercentHeight/100.0)); 
	int idx0;
	int idx1;
	if(EyeRect[0].x<EyeRect[1].x)
	{
		idx0 = 0;
		idx1 = 1;
	}
	else
	{
		idx0 = 1;
		idx1 = 0;
	}
	int temp = 0;
	// amplify left-eye region:
	leftEye.x = EyeRect[idx0].x + cvRound(EyeRect[idx0].width/2);
	leftEye.y = EyeRect[idx0].y + cvRound(EyeRect[idx0].height/2);
	EyeRect[idx0].x = leftEye.x - cvRound(eye_region_width/2);	
	SET_MIN(EyeRect[idx0].x, 0);
	EyeRect[idx0].y = leftEye.y - cvRound(eye_region_height/2);
	SET_MIN(EyeRect[idx0].y, 0);
	EyeRect[idx0].width = eye_region_width;
	SET_MAX(EyeRect[idx0].width, (face_image.cols - EyeRect[idx0].x));
	EyeRect[idx0].height = eye_region_height;
	SET_MAX(EyeRect[idx0].height, (face_image.rows - EyeRect[idx0].y));
	// amplify right-eye region:
	rightEye.x = EyeRect[idx1].x + cvRound(EyeRect[idx1].width/2);
	rightEye.y = EyeRect[idx1].y + cvRound(EyeRect[idx1].height/2);
	EyeRect[idx1].x = rightEye.x - cvRound(eye_region_width/2);
	SET_MIN(EyeRect[idx1].x, 0);
	EyeRect[idx1].y = rightEye.y - cvRound(eye_region_height/2);
	SET_MIN(EyeRect[idx1].y, 0);
	EyeRect[idx1].width = eye_region_width;
	SET_MAX(EyeRect[idx1].width, (face_image.cols - EyeRect[idx1].x));
	EyeRect[idx1].height = eye_region_height;
	SET_MAX(EyeRect[idx1].height, (face_image.rows - EyeRect[idx1].y));
	if(_rect_overlap(EyeRect[idx0], EyeRect[idx1], (eye_region_width/8 + eye_region_height/8)/2))
	{
		rectangle(show_face, EyeRect[idx0], Scalar(255), 1, 8, 0);
		rectangle(show_face, EyeRect[idx1], Scalar(255), 1, 8, 0);
		imshow("eye1", show_face);
		cout << "Error: two eyes too close - 2." << endl;
		return false;
	}
	Point leftPupil = findEyeCenter(face_image.clone(), EyeRect[idx0], "Left Eye");
	Point rightPupil = findEyeCenter(face_image.clone(), EyeRect[idx1], "Right Eye");
	// coordinate converting:
	Point leftEye_opencv = leftEye;
	Point rightEye_opencv = rightEye;
	leftEye.x = EyeRect[idx0].x + leftPupil.x;
	leftEye.y = EyeRect[idx0].y + leftPupil.y;
	rightEye.x = EyeRect[idx1].x + rightPupil.x;
	rightEye.y = EyeRect[idx1].y + rightPupil.y;
	if(norm(leftEye-leftEye_opencv)>10)  
		leftEye = leftEye_opencv;
	if(norm(rightEye-rightEye_opencv)>10)
		rightEye = rightEye_opencv;
	// show it:
	rectangle(show_face, EyeRect[idx0], Scalar(255), 1, 8, 0);
	rectangle(show_face, EyeRect[idx1], Scalar(255), 1, 8, 0);
	circle(show_face, leftEye, 3, 1234);
	circle(show_face, rightEye, 3, 1234);
	imshow("eye1", show_face);

	// 4. geometric transform
	//cout << "step 4: geometric transform." << endl;
	Point2f eyesCenter;
	eyesCenter.x = (float)(leftEye.x + rightEye.x)/2;
	eyesCenter.y = (float)(leftEye.y + rightEye.y)/2;
	double dy = (rightEye.y - leftEye.y);
	double dx = (rightEye.x - leftEye.x);
	double len = sqrt(dx*dx+dy*dy); 
	double angle = atan2(dy,dx)*180/CV_PI; 
	double desiredLen = (DESIRED_RIGHT_EYE_X - 0.16); 
	double scale = desiredLen*DESIRED_FACE_WIDTH/len; 
	Mat rot_mat = getRotationMatrix2D(eyesCenter,angle,scale);
	double ex = DESIRED_FACE_WIDTH/2 - eyesCenter.x; // 
	double ey = DESIRED_FACE_HEIGHT*DESIRED_LEFT_EYE_Y - eyesCenter.y;
	rot_mat.at<double>(0,2) += ex;
	rot_mat.at<double>(1,2) += ey;
	Mat warped = Mat(DESIRED_FACE_HEIGHT,DESIRED_FACE_WIDTH,CV_8U,Scalar(128));
	warpAffine(face_image,warped,rot_mat,warped.size()); 
	//imshow("warped",warped);

	// 5. equalization
	//cout << "step 5: equalization." << endl;
	int w = warped.cols;
	int h = warped.rows;
	Mat wholeFace;
	equalizeHist(warped,wholeFace);
	int midX = w/2;
	Mat leftSide = warped(Rect(0,0,midX,h));
	Mat rightSide = warped(Rect(midX,0,w-midX,h));
	equalizeHist(leftSide,leftSide); 
	equalizeHist(rightSide,rightSide);
	for(int y=0;y<h;y++) 
	{
		for(int x=0;x<w;x++)
		{
			int v;
			if(x<w/4){
				v = leftSide.at<uchar>(y,x);
			}
			else if(x<w/2){	
				int lv = leftSide.at<uchar>(y,x);
				int wv = wholeFace.at<uchar>(y,x);
				float f = (x-w*1/4)/(float)(w/4);
				v = cvRound((1-f)*lv+f*wv);
			}
			else if(x<w*3/4){
				int rv = rightSide.at<uchar>(y,x-midX);
				int wv = wholeFace.at<uchar>(y,x);
				float f = (x-w*2/4)/(float)(w/4);
				v = cvRound((1-f)*wv+f*rv);
			}
			else{
				v = rightSide.at<uchar>(y,x-midX);
			}
			face_image.at<uchar>(y,x) = v;
		}
	}
	//imshow("Equalzed",wholeFace);

	// 6. elliptical mask
	//cout << "step 6: elliptical mask." << endl;
	Mat mask = Mat(warped.size(),CV_8UC1,Scalar(255));
	double dw = DESIRED_FACE_WIDTH;
	double dh = DESIRED_FACE_HEIGHT;
	Point faceCenter = Point(cvRound(dw/2),cvRound(dh*0.4));
	Size size = Size(cvRound(dw*0.5),cvRound(dh*0.8));	// why 50% x 80% ???
	ellipse(mask,faceCenter,size,0,0,360,Scalar(0),CV_FILLED);
	wholeFace.setTo(Scalar(128),mask);
	//imshow("Processed",wholeFace);

	// OK! we get the normalized face, record it:
	_push_norm_face(face_index, wholeFace);

	return true;
}

static void _create_norm_faces(Mat& gray_frame, CascadeClassifier& eye_detector, CascadeClassifier glasses_detector)
{
	// currently, we only add the first detected face:
	for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
	{
		if(!CurFaceInfo[face_index]._valid)
			continue;
		
		// build one norm face:
		_create_one_norm_face(face_index, gray_frame, eye_detector, glasses_detector);
	}
}

static void _delete_norm_faces(void)
{
	// we only remove the last face:
	for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
	{
		if(!CurFaceInfo[face_index]._valid)
			continue;

		// build one norm face:
		_pop_norm_face(face_index);
	}
}

///////////////////////////////////////////////// Main Entry //////////////////////////////////////////////////////

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
	
	// eye/glasses detector init:
	CascadeClassifier eye_cascade;
	if (!eye_cascade.load("Resource Files/haarcascade_eye.xml"))
	{
		cout << "Cannot load eye cascade file.\n";
		return -1;
	}
	CascadeClassifier glasses_cascade;
	if (!glasses_cascade.load("Resource Files/haarcascade_eye_tree_eyeglasses.xml"))
	{
		cout << "Cannot load glasses cascade file.\n";
		return -1;
	}
	
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
	Mat target_frame, gray_frame, prev_gray_frame, norm_gray_frame;
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
		cvtColor(target_frame, norm_gray_frame, cv::COLOR_RGB2GRAY);
		// smooth it, otherwise a lot of false circles may be detected
		GaussianBlur(norm_gray_frame, gray_frame, Size(9, 9), 0, 0);

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

		switch(waitKey(16))
		{
		case 'a':
		case 'A':
			// add normalized face:
			_create_norm_faces(norm_gray_frame, eye_cascade, glasses_cascade);
			break;
		case 'd':
		case 'D':
			// delete exist normalized face:
			_delete_norm_faces();
			break;
		case ' ':
			// exit program:
			goto EXIT;
		}
	}
EXIT:
	return 0;
}

