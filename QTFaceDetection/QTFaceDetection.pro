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

CONFIG(debug, debug|release) {
    LIBS+=$(OPENCV_DIR)\x86\vc12\lib\opencv_core249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_highgui249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_imgproc249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_objdetect249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_features2d249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_contrib249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_calib3d249d.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_video249d.lib
} else {
    LIBS+=$(OPENCV_DIR)\x86\vc12\lib\opencv_core249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_highgui249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_imgproc249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_objdetect249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_features2d249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_contrib249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_calib3d249.lib \
    $(OPENCV_DIR)\x86\vc12\lib\opencv_video249.lib
}

SOURCES += main.cpp \
    mainwindow.cpp \
    datacontext.cpp \
    opencvutil.cpp \
    facedetection.cpp \
    source_lib/findEyeCenter.cpp \
    source_lib/findEyeCorner.cpp \
    source_lib/helpers.cpp \
    dialogparam.cpp \
    dialogvideosource.cpp \
    dialogabout.cpp

HEADERS  += mainwindow.h \
    datacontext.h \
    opencvutil.h \
    facedetection.h \
    source_lib/constants.h \
    source_lib/findEyeCenter.h \
    source_lib/findEyeCorner.h \
    source_lib/helpers.h \
    dialogparam.h \
    dialogvideosource.h \
    dialogabout.h

FORMS    += mainwindow.ui \
    dialogParam.ui \
    dialogvideosource.ui \
    dialogabout.ui

#QMAKE_POST_LINK += $(COPY_DIR) $$quote(xml) $$quote($${DESTDIR})

RC_ICONS += QTFaceDetection.ico

RESOURCES += \
    QTFaceDetection.qrc

