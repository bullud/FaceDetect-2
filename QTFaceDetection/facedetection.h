#ifndef FACEDETECTION_H
#define FACEDETECTION_H

/**
 * Migrate description:
 *
 * 1. make all static functions to be member function.
 * 2. function parameters be kept except detectors been removed from parameter list.
 */

#include <vector>
#include <string>
#include <QString>
#include <QDir>
#include <QFile>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "./source_lib/constants.h"
#include "./source_lib/findEyeCenter.h"

// namespace declaration:
using std::vector;
using std::string;
using cv::Rect;
using cv::Point2f;
using cv::Mat;
using cv::Size;
using cv::KeyPoint;
using cv::Point;
using cv::RNG;
using cv::Scalar;
using cv::Ptr;
using cv::CascadeClassifier;
using cv::GoodFeaturesToTrackDetector;
using cv::FaceRecognizer;

// structure to describe faces being detected/tracked/recognized:
struct face_descriptor
{
    bool _valid;                    // flag to indicate the struct valid or not
    Rect _face_rect;                // internal use: face position
    Mat _mask_roi;                  // internal use: mask ROI for track face
    vector<Point2f> _old_points;    // internal use: previous keypoints
    vector<Point2f> _cur_points;    // internal use: current keypoints
    vector<Point2f> _old_corners;   // internal use:
    bool _recognized;               // flag to indicate if this face has been recognized or not
    int _label;                     // label of this face in face database if recognized
    Mat _image;                     // the face image to be displayed to user
};

class FaceDetection
{
public:
    FaceDetection();

public:
    /**
     * @brief Initialize: to be called before using FaceDetection class.
     * @param screen_size: the size of screen to render the frame.
     * @param face_database_folder: the path where the face template database has been/to be placed.
     * @return true mean success, false mean failed.
     * @note when return false, FaceDetection class can not be used.
     */
    bool Initialize(Size& screen_size, QString face_database_folder);
    /**
     * @brief Deinitialize: to be called after using FaceDetection class.
     * @param none
     * @return none
     */
    void Deinitialize(void);
    /**
     * @brief DetectFace: detect and track faces in frame
     * @param frame: in which to detect and track face
     * @param frame_index: index of frame
     * @return true: there's face detected, otherwise false
     */
    bool DetectFace(Mat& frame, size_t frame_index);
    /**
     * @brief CreateFaceTemplate: create normalized face from frame
     * @param frame: in which to extrace normalized face
     * @param frame_index: index of frame
     * @param b_start: flag indicate frame is the first frame to create face template
     * @return true: face templated created and saved, otherwise false.
     * @note face templates will saved in face_database_folder, the parameter of Initialize()
     */
    bool CreateFaceTemplate(Mat& frame, size_t frame_index, bool b_start = false);
    /**
     * @brief RecognizeFace: recognize faces in frame
     * @param frame: in which to recognize faces
     * @param frame_index: index of frame
     * @return true: face(s) have been recognized, otherwise false
     * @note when return true, call GetCurFaceInfo() to check which face(s) been recognized.
     */
    bool RecognizeFace(Mat& frame, size_t frame_index);        // recognize face...
    /**
     * @brief GetCurFaceInfo: query current information, check struct face_descriptor for details
     * @return pointer to struct face_descriptor array.
     */
    struct face_descriptor const* GetCurFaceInfo() {
        return CurFaceInfo;
    }
	
	// ..........
	
}

#endif // FACEDETECTION_H
