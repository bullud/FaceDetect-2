#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QString>

enum MODE
{
    TEMPLATE,
    DETECTION,
    RECORD
};

enum VIDEOSOURCE
{
    CAMERA,
    VIDEOFILE
};

class DataContext
{
public:
    MODE GetMode() { return mode_; }
    void SetMode(MODE mode) { mode_ = mode; }

    VIDEOSOURCE GetSource() { return source_; }
    void SetSource(VIDEOSOURCE source) { source_ = source; }

    QString GetVideoFilePath() { return videoFilePath_; }
    void SetVideoFilePath(const QString &videoFilePath) { videoFilePath_ = videoFilePath; }

    bool GetRecordStatus() { return recordStatus_; }
    void SetRecordStatus(bool status) { recordStatus_ = status; }

    bool GetTemplateStatus() { return templateStatus_; }
    void SetTemplateStatus(bool status) { templateStatus_ = status; }

private:
    MODE mode_ = DETECTION;
    VIDEOSOURCE source_ = CAMERA;
    bool recordStatus_ = false;
    bool templateStatus_ = false;
    QString videoFilePath_;
};

#endif // DATACONTEXT_H
