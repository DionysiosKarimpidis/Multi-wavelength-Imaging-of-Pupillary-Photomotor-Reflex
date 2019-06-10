#-------------------------------------------------
#
# Project created by QtCreator 2016-08-27T13:27:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = Client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    jpegdecoder.cpp \
    x264decoder.cpp \
    x264decodert.cpp \
    fsmode.cpp \
    h264config.cpp \
    imageregistrationpanel.cpp \
    imageprocessthread.cpp \
    customview.cpp \
    eyetestthread.cpp \
    pupilmeasurethread.cpp

HEADERS  += mainwindow.h \
    jpegdecoder.h \
    x264decoder.h \
    x264decodert.h \
    fsmode.h \
    h264config.h \
    imageregistrationpanel.h \
    imageprocessthread.h \
    definitions.h \
    customview.h \
    eyetestthread.h \
    pupilmeasurethread.h

FORMS    += mainwindow.ui \
    fsmode.ui \
    h264config.ui \
    imageregistrationpanel.ui

RESOURCES += \
    resources.qrc

include(QtXlsxWriter-master/src/xlsx/qtxlsx.pri)

INCLUDEPATH += $$PWD/libjpeg-turbo/include
LIBS += -L$$PWD/libjpeg-turbo/bin
LIBS += -L$$PWD/libjpeg-turbo/lib
LIBS += -lturbojpeg -ljpeg

INCLUDEPATH += $$PWD/ffmpeg/include
LIBS += -L$$PWD/ffmpeg/bin
LIBS += -L$$PWD/ffmpeg/lib
LIBS += -lavformat -lavcodec -lavutil -lswscale -lswresample -lpostproc

INCLUDEPATH += $$PWD/opencv_x64_OPENMP_mingw/include
LIBS += -L$$PWD/opencv_x64_OPENMP_mingw/bin
LIBS += -L$$PWD/opencv_x64_OPENMP_mingw/lib
LIBS += -lopencv_core300 -lopencv_imgproc300 -lopencv_imgcodecs300 -lopencv_photo300 -lgomp
