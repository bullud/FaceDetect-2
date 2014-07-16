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

private:
    MODE mode_ = TEMPLATE;
};

#endif // DATACONTEXT_H
