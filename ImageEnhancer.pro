#-------------------------------------------------
#
# Project created by QtCreator 2022-11-29T16:40:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageEnhancer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imagedenoizerapi.cpp

HEADERS += \
        mainwindow.h \
    imagedenoizerapi.h

FORMS += \
        mainwindow.ui

LIBS += -LC:/opencv-mingw/x86/mingw/lib/ \
                                -lopencv_core410 \
                                -lopencv_highgui410 \
                                -lopencv_videoio410 \
                                -lopencv_imgcodecs410 \
                                -lopencv_imgproc410 \
                                -lopencv_photo410

INCLUDEPATH +=  C:/opencv-mingw/include/
