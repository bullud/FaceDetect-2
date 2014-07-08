#include "stdafx.h"
#include "windows.h"
#include "atlstr.h"
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
#include "opencv2/contrib/contrib.hpp"

#include "source_lib/constants.h"	
#include "source_lib/findEyeCenter.h"

using namespace cv;
using namespace std;

#define MAX_FACES			5		// max support detected faces
#define MIN_COUNT			15		// min key points gate (can not too small)
#define SIMILAR_GATE		0.3		// max similar level (can not too large)

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
static bool _rect_overlap(Rect& r1, Rect& r2, double allow = 0)
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

// calculate two points center:
static void _cal_line_center(Point2f& p1, Point2f& p2, Point2f& p)
{
	p.x = (p1.x + p2.x) / 2;
	p.y = (p1.y + p2.y) / 2;
}

// calculate center point of 4-point polygon, we assume the polygon is symmetric...
static void _cal_polygon_center(vector<Point2f>& points, Point2f& p)
{
	Point2f p1, p2;
	_cal_line_center(points[0], points[2], p1);
	_cal_line_center(points[1], points[3], p2);
	_cal_line_center(p1, p2, p);
}

// calculate the rect center:
static void _cal_rect_center(Rect& rect, Point2f& p)
{
	p.x = (float)((float)rect.x + (float)rect.width/2.0);
	p.y = (float)((float)rect.y + (float)rect.height/2.0);
}

// calculate line length:
static double _cal_line_length(Point2f& p1, Point2f& p2)
{
	double dx = (p1.x > p2.x) ? (p1.x - p2.x) : (p2.x - p1.x);
	double dy = (p1.y > p2.y) ? (p1.y - p2.y) : (p2.y - p1.y);
	return (double)sqrt(dx*dx+dy*dy);  
}

// create directory:
static bool _create_directory(const CString& dir_path)
{
	WIN32_FIND_DATA wfd; 
	if(FindFirstFile(dir_path, &wfd) == INVALID_HANDLE_VALUE || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))  
	{  
		CreateDirectory((LPCTSTR)dir_path, NULL);
		return true;
	}  
	return false;
}

// judge if the target file exist or not:
static bool _file_exist(const CString& file_path)
{
	WIN32_FIND_DATA wfd; 
	if(FindFirstFile(file_path, &wfd) != INVALID_HANDLE_VALUE && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))  
	{  
		return true;
	}  
	return false;
}

///////////////////////////////////////////////// Face Detection/Tracking ///////////////////////////////////////////////////

// structure to describe face:
struct face_descriptor
{
	bool _valid;
	Rect _face_rect;
	Mat _mask_roi;
	vector<Point2f> _old_points;
	vector<Point2f> _cur_points;
	vector<Point2f> _old_corners;
	bool _recognized;
	int _label;
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
		CurFaceInfo[i]._recognized = false;
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

// improved new face filtering: when the center of new face not in previous face rect, we think it's new...
static bool _really_new_face(Rect& new_face)
{
	Point2f center;
	_cal_rect_center(new_face, center);

	for(size_t i = 0; i < MAX_FACES; ++i)
	{
		if(!CurFaceInfo[i]._valid)
			continue;

		if(cv::pointPolygonTest(CurFaceInfo[i]._old_corners, center, false) > 0)
		{
			//cout << "not new face." << endl;
			return false;
		}	
	}
	return true;
}

// detect face:
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

// judge if the eye_rects is valid:
static bool _really_eyes(vector<Rect>& eye_rects, Rect& face_rect, vector<Point2f>& face_corner)
{
	// should more than 2 eyes:
	if(eye_rects.size() < 2)
	{
		return false;
	}

	vector<Point2f> eye_range, left_range, right_range;
	Point2f p;
	// eyes range:
	eye_range.push_back(face_corner[0]);
	eye_range.push_back(face_corner[1]);
	_cal_line_center(face_corner[1], face_corner[2], p);
	eye_range.push_back(p);
	_cal_line_center(face_corner[0], face_corner[3], p);
	eye_range.push_back(p);
	// left range:
	left_range.push_back(eye_range[0]);
	_cal_line_center(eye_range[0], eye_range[1], p);
	left_range.push_back(p);
	_cal_line_center(eye_range[2], eye_range[3], p);
	left_range.push_back(p);
	left_range.push_back(eye_range[3]);
	// right range:
	right_range.push_back(left_range[1]);
	right_range.push_back(eye_range[1]);
	right_range.push_back(eye_range[2]);
	right_range.push_back(left_range[2]);

	// prepare parametes:
	double len;
	// for left eye:
	Point2f left_center;
	double left_max_len;
	_cal_polygon_center(left_range, left_center);
	// calculate min length required:
	left_max_len = _cal_line_length(left_center, left_range[0]);
	for(size_t i=1; i<4; ++i)
	{
		len = _cal_line_length(left_center, left_range[i]);
		if(left_max_len > len) 
			left_max_len = len;
	}
	left_max_len = left_max_len * 0.65; // 65% of center to closest corner..
	// for right eye:
	Point2f right_center;
	double right_max_len;
	_cal_polygon_center(right_range, right_center);
	right_max_len = _cal_line_length(right_center, right_range[0]);
	for(size_t i=1; i<4; ++i)
	{
		len = _cal_line_length(right_center, right_range[i]);
		if(right_max_len > len) 
			right_max_len = len;
	}
	right_max_len = right_max_len * 0.65;

	// classify left eye and right eye:
	vector<Rect> left_eye_rect, right_eye_rect;
	vector<Point2f> left_eye_center, right_eye_center;
	for(size_t i=0; i<eye_rects.size(); ++i)
	{
		// calcualte eye rect center:
		Rect _Rect = eye_rects[i];
		_Rect.x += face_rect.x;	// coordinate convert
		_Rect.y += face_rect.y; // coordinate convert
		_cal_rect_center(_Rect, p);

		// judge in left range
		if(cv::pointPolygonTest(left_range, p, false) > 0) 
		{	
			// when in range, the center must be close enough to range center:
			len = _cal_line_length(left_center, p);
			if(len <= left_max_len)
			{
				left_eye_rect.push_back(eye_rects[i]);
				left_eye_center.push_back(p);
			}
		}
		if(cv::pointPolygonTest(right_range, p, false) > 0)
		{
			// when in range, the center must be close enough to range center:
			len = _cal_line_length(right_center, p);
			if(len <= right_max_len)
			{
				right_eye_rect.push_back(eye_rects[i]);
				right_eye_center.push_back(p);
			}
		}
	}
	//cout << "left eye size = " << left_eye_center.size() << endl;
	//cout << "right eye size = " << right_eye_center.size() << endl;

	// check:
	if(left_eye_center.size() < 1)
	{
		cout << "Error: no left eye." << endl;
		return false;
	}
	if(right_eye_center.size() < 1)
	{
		cout << "Error: no right eye." << endl;
		return false;
	}
	size_t valid_left_index = 0;
	size_t valid_right_index = 0;
	if(left_eye_center.size() > 1) // we have to remove the wrong one:
	{
		// calculate left range center:
		double min_len = _cal_line_length(left_center, left_eye_center[valid_left_index]);
		for(size_t i=1; i<left_eye_center.size(); ++i)
		{
			len = _cal_line_length(left_center, left_eye_center[i]);
			if(len < min_len)
			{
				min_len = len;
				valid_left_index = i;
			}
		}
		//cout << "valid left index = " << valid_left_index << endl;
	}
	if(right_eye_center.size() > 1)  // we have to remove the wrong one:
	{
		// calculate right range center:
		double min_len = _cal_line_length(right_center, right_eye_center[valid_right_index]);
		for(size_t i=1; i<right_eye_center.size(); ++i)
		{
			len = _cal_line_length(right_center, right_eye_center[i]);
			if(len < min_len)
			{
				min_len = len;
				valid_right_index = i;
			}
		}
		//cout << "valid right index = " << valid_right_index << endl;
	}

	// here, left have one, right have one, which we want...
	eye_rects.clear();
	eye_rects.push_back(left_eye_rect[0]);
	eye_rects.push_back(right_eye_rect[0]);

	return true;
}

// create normalized face, migarted from Keven's code:
static bool _create_one_norm_face(	size_t face_index, 
									Mat& gray_frame, 
									CascadeClassifier& eye_detector, 
									CascadeClassifier& glasses_detector,
									Mat& wholeFace)
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
	//cout << "Eye size = " << EyeRect.size() << endl;
	if(!_really_eyes(EyeRect, face_rect, CurFaceInfo[face_index]._old_corners))
	{
		// detect glasses:
		EyeRect.clear();
		glasses_detector.detectMultiScale(face_image, EyeRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
		//cout << "Glasses eye size = " << EyeRect.size() << endl;
		if(!_really_eyes(EyeRect, face_rect, CurFaceInfo[face_index]._old_corners))
		{
			cout << "Error: not really eyes detected." << endl;
			return false;
		}
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
	if(_rect_overlap(EyeRect[idx0], EyeRect[idx1], (eye_region_width/2.1 + eye_region_height/2.1)/2.0))
	{
		rectangle(show_face, EyeRect[idx0], Scalar(255), 1, 8, 0);
		rectangle(show_face, EyeRect[idx1], Scalar(255), 1, 8, 0);
		imshow("eye1", show_face);
		cout << "Error: two eyes too close." << endl;
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
	//imshow("eye1", show_face);

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
	equalizeHist(warped,wholeFace);
	/*
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
	*/
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
	
	cout << "create one normalized face success!" << endl;
	return true;
}

static void _create_norm_faces(Mat& gray_frame, CascadeClassifier& eye_detector, CascadeClassifier& glasses_detector)
{
	// currently, we only add the first detected face:
	for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
	{
		if(!CurFaceInfo[face_index]._valid)
			continue;
		
		cout << "No." << face_index+1 << " Face -----> detect eye:" << endl;
		// build one norm face:
		Mat whole_face;
		if(_create_one_norm_face(face_index, gray_frame, eye_detector, glasses_detector, whole_face))
		{
			// OK! we get the normalized face, record it:
			_push_norm_face(face_index, whole_face);
		}
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

static void _save_norm_faces(const CString& top_folder)
{
	CString sub_folder, file_name;
	cout << "save captured norm faces into file..." << endl;
	USES_CONVERSION;
	int lib_index = 1;
	for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
	{
		if(NormFaceInfo[face_index].empty())
			continue;

		// create top folder:
		while(1)
		{
			sub_folder.Format(_T("%s/s%d"), top_folder, lib_index);
			if(_create_directory(sub_folder))
				break;
			lib_index++;
		}

		for(size_t i=0; i<NormFaceInfo[face_index].size(); ++i)
		{
			file_name.Format(_T("%s/%d.pgm"), sub_folder, i+1);
			imwrite(W2A(file_name), NormFaceInfo[face_index][i]._image);
		}
	}
}

////////////////////////////////////////////// Face Recognize /////////////////////////////////////////////////////

static Mat _reconstruct_face(const Ptr<FaceRecognizer> model, const Mat preprocessedFace)
{
	// Since we can only reconstruct the face for some types of FaceRecognizer models (ie: Eigenfaces or Fisherfaces),
	// we should surround the OpenCV calls by a try/catch block so we don't crash for other models.
	try {

		// Get some required data from the FaceRecognizer model.
		Mat eigenvectors = model->get<Mat>("eigenvectors");
		Mat averageFaceRow = model->get<Mat>("mean");

		int faceHeight = preprocessedFace.rows;

		// Project the input image onto the PCA subspace.
		Mat projection = subspaceProject(eigenvectors, averageFaceRow, preprocessedFace.reshape(1,1));
		//printMatInfo(projection, "projection");

		// Generate the reconstructed face back from the PCA subspace.
		Mat reconstructionRow = subspaceReconstruct(eigenvectors, averageFaceRow, projection);
		//printMatInfo(reconstructionRow, "reconstructionRow");

		// Convert the float row matrix to a regular 8-bit image. Note that we
		// shouldn't use "getImageFrom1DFloatMat()" because we don't want to normalize
		// the data since it is already at the perfect scale.

		// Make it a rectangular shaped image instead of a single row.
		Mat reconstructionMat = reconstructionRow.reshape(1, faceHeight);
		// Convert the floating-point pixels to regular 8-bit uchar pixels.
		Mat reconstructedFace = Mat(reconstructionMat.size(), CV_8U);
		reconstructionMat.convertTo(reconstructedFace, CV_8U, 1, 0);
		//printMatInfo(reconstructedFace, "reconstructedFace");

		return reconstructedFace;

	} catch (cv::Exception e) {
		//cout << "WARNING: Missing FaceRecognizer properties." << endl;
		return Mat();
	}
}

static double _get_similarity(const Mat A, const Mat B)
{
	if (A.rows > 0 && A.rows == B.rows && A.cols > 0 && A.cols == B.cols) {
		// Calculate the L2 relative error between the 2 images.
		double errorL2 = norm(A, B, CV_L2);
		// Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
		double similarity = errorL2 / (double)(A.rows * A.cols);
		return similarity;
	}
	else {
		//cout << "WARNING: Images have a different size in 'getSimilarity()'." << endl;
		return 100000000.0;  // Return a bad value
	}
}

static bool _recognize_one_face(Ptr<FaceRecognizer>& model, Mat& target_face, CString& top_folder, int& label)
{
	//recognize:
	int identity = model->predict(target_face);
	cout << "identity = " << identity << endl;
	//validation:
	Mat reconstructedFace = _reconstruct_face(model, target_face); 
	//imshow("reconstructed", reconstructedFace);
	double gate = _get_similarity(target_face, reconstructedFace);
	cout<< "similarity = " << gate << " (gate = " << SIMILAR_GATE << ")" << endl;
	if(gate < SIMILAR_GATE)
	{
		label = identity;
		return true;
	}
	else
	{
		return false;
	}
}

static bool _recognize_faces(Mat& gray_frame, 
							 CascadeClassifier& eye_detector, 
							 CascadeClassifier& glasses_detector, 
							 CString& top_folder, 
							 Ptr<FaceRecognizer>& model)
{
	if(model.empty())
	{
		cout << "model empty, need init" << endl;
		vector<Mat> faces;
		vector<int> labels;
		bool no_file_flag = false;
		USES_CONVERSION;
		for(int k=1; true; k++){
			string pathname = W2A(top_folder + _T("/s"));
			char tmp[10];
			_itoa_s(k,tmp,10);
			pathname = pathname + tmp + "/";
			int i;
			for(i=1; true; i++){
				char s[10];
				_itoa_s(i,s,10);
				string filename = pathname + s + ".pgm";
				cout << filename.c_str() << endl;
				if(!_file_exist(A2W(filename.c_str())))
					break;
				Mat face = imread(filename,CV_LOAD_IMAGE_GRAYSCALE);
				faces.push_back(face);
				labels.push_back(k);
			}
			if(i==1)
				break;
		}
		if(faces.size() <= 1)
		{
			cout << "Error: faces not enough! - size = " << faces.size() << endl;
			return false;
		}	
		cout << "faces for recognize = " << faces.size() << endl;

		// init module:
		bool haveContribModule = initModule_contrib();
		if (!haveContribModule) 
		{
			cout << "Error: The 'contrib' module is needed for FaceRecognizer but has not been loaded into OpenCV!" << endl;
			return false;
		}

		string facerecAlgorithm = "FaceRecognizer.Eigenfaces";
		model = Algorithm::create<FaceRecognizer>(facerecAlgorithm);
		if(model.empty())
		{
			cout << "Error : no such algorithm" << endl;
			return false;
		}
		model->train(faces,labels);
	}

	// ok, let's perdict:
	int recognized = 0;
	for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
	{
		if(!CurFaceInfo[face_index]._valid)
			continue;
		if(CurFaceInfo[face_index]._recognized)
		{
			recognized++;
			continue;
		}
		
		// build one norm face:
		Mat whole_face;
		if(!_create_one_norm_face(face_index, gray_frame, eye_detector, glasses_detector, whole_face))
		{
			// can not create normalized face, go to next...
			cout << "failed to create normalized face." << endl;
			continue;
		}

		// OK, let's predict:
		int label = -1;
		if(_recognize_one_face(model, whole_face, top_folder, label))
		{
			CurFaceInfo[face_index]._recognized = true;
			CurFaceInfo[face_index]._label = label;
			recognized++;
		}
	}

	cout << "Status: recognized face size = " << recognized << endl;

	return true;
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
	
	// face recognize init:
	Ptr<FaceRecognizer> model;
	vector<Mat> rec_faces;
	vector<int> rec_labels;
	CString top_folder;
	bool do_recognize = false;
	top_folder.Format(_T("./face_database"));
	_create_directory(top_folder);
	
	// open camera video:
	//VideoCapture capture("test_video.avi"); // when test local video file...
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
	int detect_ratio = 1; 
	int recognize_ratio = 5;
	Scalar color(0, 255, 0);
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
				detect_ratio = 10; // we accelerate detect frequecy....
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
				CurFaceInfo[free_index]._recognized = false; // clear flag!!!

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
					// check if need recognize:
					color = CurFaceInfo[face_index]._recognized ? Scalar(0, 0, 255) : Scalar(0, 255, 0);
					
					//-- Draw lines between the corners (the mapped object in the scene - image_2 )
					line(target_frame, cur_corners[0], cur_corners[1], color, 2);
					line(target_frame, cur_corners[1], cur_corners[2], color, 2);
					line(target_frame, cur_corners[2], cur_corners[3], color, 2);
					line(target_frame, cur_corners[3], cur_corners[0], color, 2);

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
				CurFaceInfo[face_index]._recognized = false;
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
		
		// check if need auto-recognize:
		if(do_recognize)
		{
			recognize_ratio = 10;
			if((frame_no % recognize_ratio) == 0)
				_recognize_faces(norm_gray_frame, eye_cascade, glasses_cascade, top_folder, model);
		}
		else
		{
			// clear flag:
			for(size_t i=0; i<MAX_FACES; ++i)
				CurFaceInfo[i]._recognized = false;
		}

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
		case 's':
		case 'S':
			// save the detected normalized face into database:
			_save_norm_faces(top_folder);
			break;
		case 'r':
		case 'R':
			// start recognizing...
			do_recognize = true;
			break;
		case 'e':
		case 'E':
			do_recognize = false;
			break;
		case ' ':
			// exit program:
			goto EXIT;
		}
	}
EXIT:
	return 0;
}

