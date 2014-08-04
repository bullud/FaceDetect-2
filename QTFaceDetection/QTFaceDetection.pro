#-------------------------------------------------
#
# Project created by QtCreator 2014-07-01T23:20:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTFaceDetection
TEMPLATE = app

INCLUDEPATH+=$(OPENCV_DIR)\include

LIBS+=$(OPENCV_DIR)\x86\vc12\lib\*d.lib
#LIBS+=$(OPENCV_DIR)\x86\vc12\lib\opencv_core249d.lib \
#$(OPENCV_DIR)\x86\vc12\lib\opencv_highgui249d.lib \
#$(OPENCV_DIR)\x86\vc12\lib\opencv_imgproc249d.lib

SOURCES += main.cpp \
mainwindow.cpp \
datacontext.cpp \
    opencvutil.cpp \
    facedetection.cpp \
    source_lib/findEyeCenter.cpp \
    source_lib/findEyeCorner.cpp \
    source_lib/helpers.cpp \
    dialogparam.cpp \
    dialogvideosource.cpp

HEADERS  += mainwindow.h \
datacontext.h \
    opencvutil.h \
    facedetection.h \
    source_lib/constants.h \
    source_lib/findEyeCenter.h \
    source_lib/findEyeCorner.h \
    source_lib/helpers.h \
    dialogparam.h \
    dialogvideosource.h

FORMS    += mainwindow.ui \
    dialogParam.ui \
    dialogvideosource.ui

RESOURCES +=

