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

// structure to describe the detect/track/recognize patameters:
struct face_parameter{
    size_t _max_faces;              // max faces to be detect/track/recognized
    size_t _min_kp_count;           // min keypoints count needed for tracking
    size_t _min_temp_faces;         // min face template the database needed for one face
    double _similar_gate;           // for judging two faces are for same persion or not
    double _temp_similar_gate;      // for create face template, remove too-similar faces
};
// parameter bit mask:
#define BIT_MAX_FACES               (1 << 0)
#define BIT_MIN_KP_COUNT            (1 << 1)
#define BIT_MIN_TEMP_FACES          (1 << 2)
#define BIT_SIMILAR_GATE            (1 << 3)
#define BIT_TEMP_SIMILAR_GATE       (1 << 4)
// default parameter values:
#define DEF_MAX_FACES               (5)
#define DEF_MIN_KP_COUNT            (16)
#define DEF_MIN_TEMP_FACES          (10)
#define DEF_SIMILAR_GATE            (0.4)
#define DEF_TEMP_SIMILAR_GATE       (0.3)

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
    bool CreateFaceTemplate(Mat& frame, size_t frame_index, bool b_start = false, size_t* p_created = nullptr);
    /**
     * @brief RecognizeFace: recognize faces in frame
     * @param frame: in which to recognize faces
     * @param frame_index: index of frame
     * @return true: face(s) have been recognized, otherwise false
     * @note when return true, call GetCurFaceInfo() to check which face(s) been recognized.
     */
    bool RecognizeFace(Mat& frame, size_t frame_index);
    /**
     * @brief GetCurFaceInfo: query current information, check struct face_descriptor for details
     * @return pointer to struct face_descriptor array.
     */
    struct face_descriptor const* GetCurFaceInfo() {
        return CurFaceInfo;
    }
    /**
     * @brief QueryParameters/ModifyParameters: check/change parameters for face detect/track/recognize
     * @param param: check struct face_paramter for details
     * @return none
     */
    void QueryParameters(struct face_parameter* param);
    void ModifyParameters(struct face_parameter* param);

protected:
    // middle-level functions:
    void _SetTargetFrame(Mat& frame, size_t frame_index); // set target frame to be processed
    void _DetectFace(); // detect face, call GetCurFaceInfo() for result...
    void _TrackFace(); // track face, call GetCurFaceInfo() for result...
    void _RefreshFaceInfo(); // refresh CurFaceInfo...
    void _UpdateStatus(size_t mask); // when parameters updated, we have to do corresponding action...

private:
    size_t frame_no;
    Mat target_frame;
    Mat gray_frame;
    Mat prev_gray_frame;
    Mat norm_gray_frame;
    vector<Point2f> screen_corners;
    QString top_folder;
    CascadeClassifier frontal_face_detector;
    CascadeClassifier profile_face_detector;
    GoodFeaturesToTrackDetector feature_detector;
    CascadeClassifier normal_eye_detector;
    CascadeClassifier glasses_eye_detector;
    Ptr<FaceRecognizer> model;

protected:
    // helpers:
    void _rect_to_points(Rect& rect, vector<Point2f>& points);
    void _points_to_rect(vector<Point2f>& corners, Rect& rect);
    bool _rect_overlap(Rect& r1, Rect& r2, double allow=0);
    void _cal_line_center(Point2f& p1, Point2f& p2, Point2f& p);
    void _cal_polygon_center(vector<Point2f>& points, Point2f& p);
    void _cal_rect_center(Rect& rect, Point2f& p);
    double _cal_line_length(Point2f& p1, Point2f& p2);

protected:
    // face detection/tracking:
    void _build_display_image(Mat& frame, Mat& face, Rect face_rect);
    bool _really_new_face(Rect& new_face);
    bool _detect_faces(Mat& gray_frame, vector<Rect>& Faces);
    bool _create_mask_rois(vector<Rect>& Faces, Size size, vector<Mat>& mask_roi);
    bool _detect_keypoints(Mat& target_frame,
                           Mat& mask_roi_t,
                           Rect& Faces_t,
                           vector<Point2f>& cur_points_t,
                           vector<Point2f>& old_corners_t);
protected:
    // face normalization:
    bool _database_updated;
    bool _bcreating_temp;
    bool _really_eyes(vector<Rect>& eye_rects, Rect& face_rect, vector<Point2f>& face_corner);
    bool _create_one_norm_face(size_t face_index,
                               Mat& gray_frame,
                               Mat& wholeFace);
    bool _save_norm_faces();
    bool _valid_norm_face(Mat& new_face);

protected:
    // face recognize:
    bool _init_face_recognizer();
    Mat _reconstruct_face(const Mat preprocessedFace);
    double _get_similarity(const Mat A, const Mat B);
    bool _recognize_one_face(Mat& target_face, int& label);

private:
    struct face_descriptor *CurFaceInfo;
    vector<Mat> NormFaceInfo;
    void _reset_face_info(); // reset CurFaceInfo
    bool _find_free_node(size_t& index);
    size_t _cur_face_count();
    size_t _rec_face_count();

private:
    // from macro:
    size_t MIN_KP_COUNT; // minimal keyponits count
    size_t MAX_FACES;
    double SIMILAR_GATE;
    double TEMP_SIMILAR_GATE; // for creating face templates
    RNG RNG;
    double DESIRED_LEFT_EYE_X;
    double DESIRED_LEFT_EYE_Y;
    double DESIRED_RIGHT_EYE_X;
    double DESIRED_RIGHT_EYE_Y;
    int DESIRED_FACE_WIDTH; // normalized face width
    int DESIRED_FACE_HEIGHT;
    int MIN_EYES_DISTANCE;
    size_t MIN_TEMP_FACES;
    Size DIS_IMAGE_SIZE;

};

#endif // FACEDETECTION_H
