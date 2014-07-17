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

private:
    MODE mode_ = TEMPLATE;
    bool recordStatus_ = false;
};

#endif // DATACONTEXT_H
