#include "facedetection.h"

// just for debug, to be removed later......
#undef DEBUG_MSG
#define DEBUG_MSG       1
#if(DEBUG_MSG == 1)
#include <iostream>
using std::cout;
using std::endl;
#endif

#define SET_MIN(x, min)		do{if((x) < (min)) (x) = (min);}while(0)
#define SET_MAX(x, max)		do{if((x) > (max)) (x) = (max);}while(0)

FaceDetection::FaceDetection()
    : MAX_FACES(DEF_MAX_FACES)
    , MIN_KP_COUNT(DEF_MIN_KP_COUNT)
    , SIMILAR_GATE(DEF_SIMILAR_GATE)
    , TEMP_SIMILAR_GATE(DEF_TEMP_SIMILAR_GATE)
    , MIN_TEMP_FACES(DEF_MIN_TEMP_FACES)
    , RNG(12345)
    , DESIRED_LEFT_EYE_X(0.16)
    , DESIRED_LEFT_EYE_Y(0.14)
    , DESIRED_RIGHT_EYE_X(1 - 0.16)
    , DESIRED_RIGHT_EYE_Y(1 - 0.14)
    , DESIRED_FACE_WIDTH(150)
    , DESIRED_FACE_HEIGHT(150)
    , MIN_EYES_DISTANCE(20)
    , DIS_IMAGE_SIZE(200, 200)
    , feature_detector(400, 0.01, 5.0, 3, true, 0.04)
    , CurFaceInfo(nullptr)
    , model(nullptr)
    , _database_updated(false)
    , _bcreating_temp(false)
{

}

///////////////////////////////// Public interface //////////////////////////////////////

void FaceDetection::QueryParameters(struct face_parameter* param)
{
    if(!param)
        return;

    param->_max_faces = MAX_FACES;
    param->_min_kp_count = MIN_KP_COUNT;
    param->_min_temp_faces = MIN_TEMP_FACES;
    param->_similar_gate = SIMILAR_GATE;
    param->_temp_similar_gate = TEMP_SIMILAR_GATE;
}

void FaceDetection::ModifyParameters(struct face_parameter* param)
{
    if(!param)
        return;

    // check if parameter changed:
    size_t mask = 0;
    if(MAX_FACES != param->_max_faces)
        mask |= BIT_MAX_FACES;
    if(MIN_KP_COUNT != param->_min_kp_count)
        mask |= BIT_MIN_KP_COUNT;
    if(MIN_TEMP_FACES != param->_min_temp_faces)
        mask |= BIT_MIN_TEMP_FACES;
    if(SIMILAR_GATE != param->_similar_gate)
        mask |= BIT_SIMILAR_GATE;
    if(TEMP_SIMILAR_GATE != param->_temp_similar_gate)
        mask |= BIT_TEMP_SIMILAR_GATE;

    // update:
    MAX_FACES = param->_max_faces;
    MIN_KP_COUNT = param->_min_kp_count;
    MIN_TEMP_FACES = param->_min_temp_faces;
    SIMILAR_GATE = param->_similar_gate;
    TEMP_SIMILAR_GATE = param->_temp_similar_gate;

    // update related thing:
    if(mask)
        _UpdateStatus(mask);
}

bool FaceDetection::Initialize(Size& screen, QString face_database_folder)
{
    // malloc face descriptor buffer:
    if(CurFaceInfo)
    {
        delete [] CurFaceInfo;
        CurFaceInfo = nullptr;
    }
    CurFaceInfo = new struct face_descriptor[MAX_FACES];
    if(!CurFaceInfo)
    {
        #if(DEBUG_MSG == 1)
        cout << "Cannot malloc face descriptor buffer.\n";
        #endif
        return false;
    }
    _reset_face_info();

    // normalized faces:
    NormFaceInfo.clear();

    // face detector init:
    if(frontal_face_detector.empty())
    {
        if(!frontal_face_detector.load("xml/haarcascade_frontalface_alt.xml"))
        {
            #if(DEBUG_MSG == 1)
            cout << "Cannot load front face cascade file.\n";
            #endif
            Deinitialize();
            return false;
        };
    }
    if(profile_face_detector.empty())
    {
        if(!profile_face_detector.load("./xml/haarcascade_profileface.xml"))
        {
            #if(DEBUG_MSG == 1)
            cout << "Cannot load profile face cascade file.\n";
            #endif
            Deinitialize();
            return false;
        };

    }

    // eye detector init:
    if(normal_eye_detector.empty())
    {
        if(!normal_eye_detector.load("./xml/haarcascade_eye.xml"))
        {
            #if(DEBUG_MSG == 1)
            cout << "Cannot load eye cascade file.\n";
            #endif
            Deinitialize();
            return false;
        }
    }
    if(glasses_eye_detector.empty())
    {
        if(!glasses_eye_detector.load("./xml/haarcascade_eye_tree_eyeglasses.xml"))
        {
            #if(DEBUG_MSG == 1)
            cout << "Cannot load glasses cascade file.\n";
            #endif
            Deinitialize();
            return false;
        }
    }

    // construct screen rect:
    Point2f p;
    screen_corners.clear();
    p = cvPoint(0, 0);
    screen_corners.push_back(p);
    p = cvPoint(screen.width, 0);
    screen_corners.push_back(p);
    p = cvPoint(screen.width, screen.height);
    screen_corners.push_back(p);
    p = cvPoint(0, screen.height);
    screen_corners.push_back(p);

    // record folder:
    top_folder = face_database_folder;
    QDir t;
    if(!t.exists(top_folder) && !t.mkdir(top_folder))
    {
       Deinitialize();
       return false;
    }
    return true;
}

void FaceDetection::Deinitialize()
{
    if(CurFaceInfo)
    {
        delete [] CurFaceInfo;
        CurFaceInfo = nullptr;
    }
}

bool FaceDetection::DetectFace(Mat& frame, size_t frame_index)
{
    size_t ratio(1);

    // update frame:
    _SetTargetFrame(frame, frame_index);

    // if there is face be tracking, decrease freq:
    if(_cur_face_count() > 0)
        ratio = 10;

    // we detect faces every 10 frames:
    if((frame_no % ratio) == 0)
        _DetectFace();

    // we must track faces every frame:
    _TrackFace();

    // update information:
    _RefreshFaceInfo();

    // check if face be tracking:
    return _cur_face_count() ? true : false;
}

bool FaceDetection::CreateFaceTemplate(Mat& frame, size_t frame_index, bool b_start, size_t* p_created)
{
    // set flag:
    _bcreating_temp = true;

    // we should have a new start:
    if(b_start)
    {
        // remove previous faces:
        NormFaceInfo.clear();
    }

    // there must be faces detected:
    if(!DetectFace(frame, frame_index))
    {
        if(p_created)
            *p_created = NormFaceInfo.size();
        return false;
    }

    // we try to create every 20 frame:
    if((frame_no % 20) == 0)
    {
        // we only extract one face in the frame:
        size_t face_index;
        for(face_index=0; face_index<MAX_FACES; ++face_index)
        {
            if(CurFaceInfo[face_index]._valid)
                break;
        }

        // try to create normalized face:
        Mat whole_face;
        if(_create_one_norm_face(face_index, norm_gray_frame, whole_face))
        {
            if(_valid_norm_face(whole_face))
            {
                NormFaceInfo.push_back(whole_face);
            }
        }

        // check if enough to put into database:
        #if(DEBUG_MSG == 1)
        cout << "Current norm face size = " << NormFaceInfo.size() << endl;
        #endif
        if(NormFaceInfo.size() >= MIN_TEMP_FACES)
        {
            // save it:
            if(_save_norm_faces())
            {
                NormFaceInfo.clear();
                _database_updated = true;   // make face recognizer re-init as database updated....
                if(p_created)
                    *p_created = NormFaceInfo.size();
                return true;
            }
        }
    }
    if(p_created)
        *p_created = NormFaceInfo.size();
    return false;
}

bool FaceDetection::RecognizeFace(Mat& frame, size_t frame_index)
{
    // set flag:
    _bcreating_temp = false;

    // if there is no face detected, do nothing:
    if(!DetectFace(frame, frame_index))
        return false;

    // we recognize every 10 frame:
    if((frame_no % 10) == 0)
    {
        // check if need to init face recognizer:
        if(model.empty() || _database_updated)
        {
            if(!_init_face_recognizer())
                return false;
        }
        _database_updated = false;

        // ok, let's perdict:
        for(size_t face_index=0; face_index<MAX_FACES; ++face_index)
        {
            // must be detected:
            if(!CurFaceInfo[face_index]._valid)
                continue;

            // check if recognized already:
            if(CurFaceInfo[face_index]._recognized)
                continue;

            // build one normalized face:
            Mat whole_face;
            if(!_create_one_norm_face(face_index, norm_gray_frame, whole_face))
            {
                // can not create normalized face, go to next...
                #if(DEBUG_MSG == 1)
                cout << "failed to create normalized face." << endl;
                #endif
                continue;
            }

            // OK, let's predict:
            int label = -1;
            if(_recognize_one_face(whole_face, label))
            {
                CurFaceInfo[face_index]._recognized = true;
                CurFaceInfo[face_index]._label = label;
            }
        }
    }

    // once face recognized, we're success!
    size_t recognized_faces = _rec_face_count();
    //cout << "Status: recognized face size = " << recognized_faces << endl;
    return recognized_faces ? true : false;
}

///////////////////////////////// middle -level function ////////////////////////////////

void FaceDetection::_UpdateStatus(size_t mask)
{
    // judge which parameter changed:
    if(mask & BIT_MAX_FACES)
    {
        // we have to re-alloc buffers:
        Deinitialize();
        // malloc face descriptor buffer:
        CurFaceInfo = new struct face_descriptor[MAX_FACES];
        if(!CurFaceInfo)
        {
            #if(DEBUG_MSG == 1)
            cout << "[_UpdateStatus]: Cannot malloc face descriptor buffer.\n";
            #endif
            return;
        }
        _reset_face_info();

        // norm faces:
        NormFaceInfo.clear();
    }
    else if(mask & BIT_MIN_KP_COUNT)
    {
        // nothing to do...
    }
    else if(mask & BIT_MIN_TEMP_FACES)
    {
        // nothing to do...
    }
    else if(mask & BIT_SIMILAR_GATE)
    {
        // we have to re-recognize faces, so...
        size_t face_index;
        for(face_index = 0; face_index < MAX_FACES; ++face_index)
        {
            if(!CurFaceInfo[face_index]._valid)
                continue;
            // indicate not recognized...
            CurFaceInfo[face_index]._recognized = false;
            CurFaceInfo[face_index]._label = -1;
        }
    }
    else if(mask & BIT_TEMP_SIMILAR_GATE)
    {
        // we have to remove all created templates as gate changed...
        size_t face_index;
        for(face_index = 0; face_index < MAX_FACES; ++face_index)
        {
            NormFaceInfo.clear();
        }
    }
    else
    {
        // can not come here....
    }
}

void FaceDetection::_SetTargetFrame(Mat& frame, size_t frame_index)
{
    target_frame = frame; // attention: here copy data not occur actually, so...
    frame_no = frame_index; // frame number for trace...
    // conver to gray image:
    cv::cvtColor(target_frame, norm_gray_frame, cv::COLOR_RGB2GRAY);
    // smooth it, otherwise a lot of false circles may be detected
    cv::GaussianBlur(norm_gray_frame, gray_frame, Size(9, 9), 0, 0);
}

void FaceDetection::_DetectFace()
{
    vector<Rect> Faces;
    vector<Rect> NewFaces;
    vector<Mat> mask_roi;

    // check how many faces be tracking:
    #if(DEBUG_MSG == 1)
    size_t tracking_faces = _cur_face_count();
    #endif

    // decide init ROI:
    if(!_detect_faces(gray_frame, NewFaces))
    {
        #if(DEBUG_MSG == 1)
        cout << "Status: tracking faces = " << tracking_faces << ", no face detect " << (frame_no % 2 ? "0_o" : "o_0") << endl;
        #endif
        return;
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
    #if(DEBUG_MSG == 1)
    cout << "Status: tracking faces = " << tracking_faces << ", detect -> (new/all) = " << Faces.size() << "/" << NewFaces.size() << endl;
    #endif

    // if no new face added...
    if(Faces.size() == 0)
    {
        return;
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
        _build_display_image(target_frame, CurFaceInfo[free_index]._image, Faces[i]);

        // detect keypoints:
        _detect_keypoints(target_frame,
                          CurFaceInfo[free_index]._mask_roi,
                          CurFaceInfo[free_index]._face_rect,
                          CurFaceInfo[free_index]._cur_points,
                          CurFaceInfo[free_index]._old_corners);
        #if(DEBUG_MSG == 1)
        cout << free_index <<  ": new init keypoints = " << CurFaceInfo[free_index]._cur_points.size() << endl;
        #endif
    }
}

void FaceDetection::_TrackFace()
{
    vector<Point2f> cur_corners(4);
    Scalar color;

    // track each face detected:
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
            if (cv::pointPolygonTest(CurFaceInfo[face_index]._old_corners, CurFaceInfo[face_index]._cur_points[i], false) < 0)
                continue;

            CurFaceInfo[face_index]._cur_points[k] = CurFaceInfo[face_index]._cur_points[i];
            CurFaceInfo[face_index]._old_points[k] = CurFaceInfo[face_index]._old_points[i];
            k++;
            // draw the keypoints ???
            //cv::circle(target_frame, CurFaceInfo[face_index]._cur_points[i], 3, Scalar(RNG.uniform(0, 255), RNG.uniform(0, 255),
            //    RNG.uniform(0, 255)), 1, 8);
        }
        CurFaceInfo[face_index]._cur_points.resize(k);
        CurFaceInfo[face_index]._old_points.resize(k);
        //cout << "k = " << k << endl;

        // affine transform:
        if (CurFaceInfo[face_index]._cur_points.size() >= MIN_KP_COUNT && CurFaceInfo[face_index]._old_points.size() >= MIN_KP_COUNT)
        {
            // calculate tranform metrix:
            //Mat H = getAffineTransform(old_points_t, cur_points_t);
            Mat H = cv::estimateRigidTransform(CurFaceInfo[face_index]._old_points, CurFaceInfo[face_index]._cur_points, false);
            if(H.rows != 2 || H.cols != 3)
            {
                #if(DEBUG_MSG == 1)
                cout << "error: H.rows = " << H.rows << ", H.cols = " << H.cols << endl;
                #endif
                break;
            }
            cv::transform(CurFaceInfo[face_index]._old_corners, cur_corners, H);

            // judge if in screen:
            int number_gate = 0;
            for (size_t i = 0; i < 4; ++i)
            {
                if (cv::pointPolygonTest(screen_corners, cur_corners[i], false) < 0)
                    number_gate++;
            }
            if (number_gate > 2) // when more than 2 corners out of screen, re-init:
            {
                #if(DEBUG_MSG == 1)
                cout << "corrner out of range!" << endl;
                #endif
                CurFaceInfo[face_index]._old_points.clear();
                CurFaceInfo[face_index]._cur_points.clear();
            }
            else
            {
                // check if need recognize:
                if(_bcreating_temp)
                    color = Scalar(255, 0, 0);
                else
                    color = CurFaceInfo[face_index]._recognized ? Scalar(0, 0, 255) : Scalar(0, 255, 0);

                //-- Draw lines between the corners (the mapped object in the scene - image_2 )
                cv::line(target_frame, cur_corners[0], cur_corners[1], color, 2);
                cv::line(target_frame, cur_corners[1], cur_corners[2], color, 2);
                cv::line(target_frame, cur_corners[2], cur_corners[3], color, 2);
                cv::line(target_frame, cur_corners[3], cur_corners[0], color, 2);

                // update previous cornners:
                CurFaceInfo[face_index]._old_corners[0] = cur_corners[0];
                CurFaceInfo[face_index]._old_corners[1] = cur_corners[1];
                CurFaceInfo[face_index]._old_corners[2] = cur_corners[2];
                CurFaceInfo[face_index]._old_corners[3] = cur_corners[3];
            }
        }
    }
}

void FaceDetection::_RefreshFaceInfo()
{
    // update previous frame:
    cv::swap(prev_gray_frame, gray_frame);
    for (size_t face_index = 0; face_index < MAX_FACES; ++face_index)
    {
        if(!CurFaceInfo[face_index]._valid)
            continue;

        // update points:
        if (CurFaceInfo[face_index]._cur_points.size() < MIN_KP_COUNT)
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
}

////////////////////////////////// helper function //////////////////////////////////////

// convert rect to points array:
void FaceDetection::_rect_to_points(Rect& rect, vector<Point2f>& points)
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
void FaceDetection::_points_to_rect(vector<Point2f>& corners, Rect& rect)
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
bool FaceDetection::_rect_overlap(Rect& r1, Rect& r2, double allow)
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

// calculate the center point of one line:
void FaceDetection::_cal_line_center(Point2f& p1, Point2f& p2, Point2f& p)
{
    p.x = (p1.x + p2.x) / 2;
    p.y = (p1.y + p2.y) / 2;
}

// calculate the center point of one polygon, which is not very accurate actually:
void FaceDetection::_cal_polygon_center(vector<Point2f>& points, Point2f& p)
{
    Point2f p1, p2;
    _cal_line_center(points[0], points[2], p1);
    _cal_line_center(points[1], points[3], p2);
    _cal_line_center(p1, p2, p);
}

// calculate the center point of one rect:
void FaceDetection::_cal_rect_center(Rect& rect, Point2f& p)
{
    p.x = (float)((float)rect.x + (float)rect.width/2.0);
    p.y = (float)((float)rect.y + (float)rect.height/2.0);
}

// calculate the length of one line:
double FaceDetection::_cal_line_length(Point2f& p1, Point2f& p2)
{
    double dx = (p1.x > p2.x) ? (p1.x - p2.x) : (p2.x - p1.x);
    double dy = (p1.y > p2.y) ? (p1.y - p2.y) : (p2.y - p1.y);
    return (double)sqrt(dx*dx+dy*dy);
}

//////////////////////////////////////// Face Detect/Tracking ///////////////////////////////////////////

void FaceDetection::_build_display_image(Mat& frame, Mat& face, Rect face_rect)
{
    Size screen_size;
    screen_size.width = screen_corners[2].x - screen_corners[0].x;
    screen_size.height = screen_corners[2].y - screen_corners[0].y;
    int dx = face_rect.width / 8;
    int dy = face_rect.height / 8;
    face_rect.x -= dx;
    face_rect.y -= dy;
    face_rect.width += dx * 2;
    face_rect.height += dy * 2;
    SET_MIN(face_rect.x, 0);
    SET_MIN(face_rect.y, 0);
    SET_MAX(face_rect.width, screen_size.width-face_rect.x);
    SET_MAX(face_rect.height, screen_size.height-face_rect.y);
    Mat temp = frame(face_rect);
    cv::resize(temp, face, DIS_IMAGE_SIZE);
}

// invalid all face info:
void FaceDetection::_reset_face_info()
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
bool FaceDetection::_find_free_node(size_t& index)
{
    for(index=0; index<MAX_FACES; ++index)
    {
        if(!CurFaceInfo[index]._valid)
            return true;
    }
    return false;

}

size_t FaceDetection::_cur_face_count()
{
    size_t face_count = 0;
    for(size_t i=0; i<MAX_FACES; ++i)
    {
        if(!CurFaceInfo[i]._valid)
            continue;
        face_count++;
    }
    return face_count;
}

size_t FaceDetection::_rec_face_count()
{
    size_t rec_count = 0;
    for(size_t i=0; i<MAX_FACES; ++i)
    {
        if(!CurFaceInfo[i]._valid)
            continue;
        if(!CurFaceInfo[i]._recognized)
            continue;
        rec_count++;
    }
    return rec_count;
}

bool FaceDetection::_really_new_face(Rect& new_face)
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

bool FaceDetection::_detect_faces(Mat& gray_frame, vector<Rect>& Faces)
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
            if(_rect_overlap(Faces_t[i], Faces[k], 0))
                break;
        }
        if(k < Faces.size())
            continue;

        // really new face, record it:
        Faces.push_back(Faces_t[i]);
    }

    return Faces.size() ? true : false;
}

bool FaceDetection::_create_mask_rois(vector<Rect>& Faces, Size size, vector<Mat>& mask_roi)
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

bool FaceDetection::_detect_keypoints(Mat& target_frame,
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

/////////////////////////////////////////// Face Normalization ///////////////////////////////////////////////

// judge if the eye_rects is valid:
bool FaceDetection::_really_eyes(vector<Rect>& eye_rects, Rect& face_rect, vector<Point2f>& face_corner)
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
        #if(DEBUG_MSG == 1)
        cout << "Error: no left eye." << endl;
        #endif
        return false;
    }
    if(right_eye_center.size() < 1)
    {
        #if(DEBUG_MSG == 1)
        cout << "Error: no right eye." << endl;
        #endif
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
    eye_rects.push_back(left_eye_rect[valid_left_index]);
    eye_rects.push_back(right_eye_rect[valid_right_index]);

    return true;
}

// create normalized face, migarted from Keven's code:
bool FaceDetection::_create_one_norm_face(size_t face_index,
                                          Mat& gray_frame,
                                          Mat& whole_face)
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
    normal_eye_detector.detectMultiScale(face_image, EyeRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
    //cout << "Eye size = " << EyeRect.size() << endl;
    if(!_really_eyes(EyeRect, face_rect, CurFaceInfo[face_index]._old_corners))
    {
        // detect glasses:
        EyeRect.clear();
        glasses_eye_detector.detectMultiScale(face_image, EyeRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
        //cout << "Glasses eye size = " << EyeRect.size() << endl;
        if(!_really_eyes(EyeRect, face_rect, CurFaceInfo[face_index]._old_corners))
        {
            #if(DEBUG_MSG == 1)
            cout << "Error: not really eyes detected." << endl;
            #endif
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
    //int temp = 0;
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
        #if(DEBUG_MSG == 1)
        cout << "Error: two eyes too close." << endl;
        #endif
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
    //int w = warped.cols;
    //int h = warped.rows;
    equalizeHist(warped, whole_face);
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
    whole_face.setTo(Scalar(128),mask);
    //imshow("Processed",wholeFace);

    #if(DEBUG_MSG == 1)
    cout << "create one normalized face success!" << endl;
    #endif
    return true;
}

bool FaceDetection::_valid_norm_face(Mat& new_face)
{
    double similar;
    for(size_t i=0; i<NormFaceInfo.size(); ++i)
    {
        similar = _get_similarity(NormFaceInfo[i], new_face);
        #if(DEBUG_MSG == 1)
        cout << "similarity = " << similar << endl;
        #endif
        if(similar < TEMP_SIMILAR_GATE)
            return false;
    }
    return true;
}

bool FaceDetection::_save_norm_faces()
{
    QString sub_folder, file_name;
    #if(DEBUG_MSG == 1)
    cout << "save captured norm faces into file..." << endl;
    #endif
    int lib_index = 1;

    if(NormFaceInfo.empty())
        return false;

    // create top folder:
    QDir t;
    while(1)
    {
        sub_folder.sprintf("%s/s%d", top_folder.toStdString().c_str(), lib_index);
        if(t.exists(sub_folder))
        {
            lib_index++;
        }
        else
        {
            if(t.mkdir(sub_folder))
                break;
            else
                return false;
        }
    }

    for(size_t i=0; i<NormFaceInfo.size(); ++i)
    {
        file_name.sprintf("%s/%d.pgm", sub_folder.toStdString().c_str(), i+1);
        cv::imwrite(file_name.toStdString(), NormFaceInfo[i]);
    }
    return true;
}

/////////////////////////////////////////// Face Recognization /////////////////////////////////////////////

bool FaceDetection::_init_face_recognizer()
{
    #if(DEBUG_MSG == 1)
    cout << "init recognizer model..." << endl;
    #endif
    if(model)
    {
        // re-init as the database updated....
        model.release();
        model = nullptr;
    }

    // let's init the recongnize model:
    vector<Mat> faces;
    vector<int> labels;
    for(int k=1; true; k++){
        QString pathname("./face_database/s");
        char tmp[10];
        _itoa_s(k,tmp,10);
        pathname = pathname + tmp + "/";
        int i;
        for(i=1; true; i++){
            char s[10];
            _itoa_s(i,s,10);
            QString filename = pathname + QString(s) + QString(".pgm");
            #if(DEBUG_MSG == 1)
            cout << filename.toStdString().c_str() << endl;
            #endif
            QFile t;
            if(!t.exists(filename))
                break;
            Mat face = cv::imread(filename.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);
            faces.push_back(face);
            labels.push_back(k);
        }
        if(i==1)
            break;
    }
    if(faces.size() <= 1)
    {
        #if(DEBUG_MSG == 1)
        cout << "Error: faces not enough! - size = " << faces.size() << endl;
        #endif
        return false;
    }
    #if(DEBUG_MSG == 1)
    cout << "faces for recognize = " << faces.size() << endl;
    #endif

    // init module:
    bool haveContribModule = cv::initModule_contrib();
    if (!haveContribModule)
    {
        #if(DEBUG_MSG == 1)
        cout << "Error: The 'contrib' module is needed for FaceRecognizer but has not been loaded into OpenCV!" << endl;
        #endif
        return false;
    }

    string facerecAlgorithm = "FaceRecognizer.Eigenfaces";
    model = cv::Algorithm::create<FaceRecognizer>(facerecAlgorithm);
    if(model.empty())
    {
        #if(DEBUG_MSG == 1)
        cout << "Error : no such algorithm" << endl;
        #endif
        return false;
    }
    model->train(faces,labels);

    return true;
}

Mat FaceDetection::_reconstruct_face(const Mat preprocessedFace)
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

double FaceDetection::_get_similarity(const Mat A, const Mat B)
{
    if (A.rows > 0 && A.rows == B.rows && A.cols > 0 && A.cols == B.cols) {
        // Calculate the L2 relative error between the 2 images.
        double errorL2 = cv::norm(A, B, CV_L2);
        // Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
        double similarity = errorL2 / (double)(A.rows * A.cols);
        return similarity;
    }
    else {
        //cout << "WARNING: Images have a different size in 'getSimilarity()'." << endl;
        return 100000000.0;  // Return a bad value
    }
}

bool FaceDetection::_recognize_one_face(Mat& target_face, int& label)
{
    //validation:
    Mat reconstructedFace = _reconstruct_face(target_face);
    //imshow("reconstructed", reconstructedFace);
    double gate = _get_similarity(target_face, reconstructedFace);
    #if(DEBUG_MSG == 1)
    cout<< "similarity = " << gate << " (gate = " << (double)SIMILAR_GATE << ")" << endl;
    #endif
    if(gate < SIMILAR_GATE)
    {
         //recognize:
        label = model->predict(target_face);
        #if(DEBUG_MSG == 1)
        cout << "identity = " << label << endl;
        #endif
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////// END of FILE//////////////////////////////////////////



