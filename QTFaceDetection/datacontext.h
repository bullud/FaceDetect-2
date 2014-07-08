#ifndef DATACONTEXT_H
#define DATACONTEXT_H

enum MODE
{
    CREATION,
    DETECTION
};

class DataContext
{
public:
    MODE GetMode() { return mode_; }
    void SetMode(MODE mode) { mode_ = mode; }

private:
    MODE mode_ = CREATION;
};

#endif // DATACONTEXT_H
