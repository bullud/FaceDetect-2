#ifndef DATACONTEXT_H
#define DATACONTEXT_H

enum MODE
{
    TEMPLATE,
    DETECTION,
    RECORD
};

class DataContext
{
public:
    MODE GetMode() { return mode_; }
    void SetMode(MODE mode) { mode_ = mode; }

    bool GetRecordStatus() { return recordStatus_; }
    void SetRecordStatus(bool status) { recordStatus_ = status; }

    bool GetTemplateStatus() { return templateStatus_; }
    void SetTemplateStatus(bool status) { templateStatus_ = status; }

private:
    MODE mode_ = DETECTION;
    bool recordStatus_ = false;
    bool templateStatus_ = false;
};

#endif // DATACONTEXT_H
