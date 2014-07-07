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

LIBS+=$(OPENCV_DIR)\x86\vc12\lib\*.lib

SOURCES += main.cpp\
mainwindow.cpp \
datacontext.cpp \
    opencvutil.cpp \
    facedetection.cpp

HEADERS  += mainwindow.h \
datacontext.h \
    opencvutil.h \
    facedetection.h

FORMS    += mainwindow.ui

1.QMAKE_POST_LINK=copy "$(OPENCV_DIR)\x86\vc12\bin\opencv_highgui249.dll" "$(DESTDIR)"
