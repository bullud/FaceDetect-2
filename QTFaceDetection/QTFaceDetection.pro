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
    facedetection.cpp

HEADERS  += mainwindow.h \
datacontext.h \
    opencvutil.h \
    facedetection.h

FORMS    += mainwindow.ui

