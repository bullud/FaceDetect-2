#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <opencv2/core/core.hpp>

class FaceDetection
{
public:
    // external interface...
    bool Initialize(Size& screen_size, string face_database_folder);
    void Deinitialize(void);
    bool DetectFace(Mat& frame, size_t frame_no);           // just detect face, not to be used in out demo...
    bool CreateFaceTemplate(Mat& frame, size_t frame_no);   // create face template...
    bool RecognizeFace(Mat& frame, size_t frame_no, Mat& recognized_face);  // recognize face...
public:
    // call this to check current face information...
    struct face_descriptor const* GetCurFaceInfo() {
        return CurFaceInfo;
    }
};

#endif // FACEDETECTION_H
