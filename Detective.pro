#-------------------------------------------------
#
# Project created by QtCreator 2020-02-13T11:56:15
#
#-------------------------------------------------

QT       += core gui multimedia 

greaterThan(QT_MAJOR_VERSION, 4): QT += multimediawidgets

TARGET = Detective
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += OPENCV_DATA_DIR=\\\"/usr/local/share/opencv4/\\\"
unix {
    INCLUDEPATH += /usr/local/include/opencv4
    #INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /root/opencv_build/opencv/modules
    INCLUDEPATH += /root/opencv_build/opencv_contrib/modules
    target.path = /usr/lib
    INSTALLS += target
    LIBS += -L/usr/local/lib -lopencv_imgproc -lopencv_core -lopencv_video -lopencv_videoio -lopencv_imgcodecs -lopencv_objdetect -lopencv_dnn
    CONFIG += link_pkgconfig\
           plugin
    PKGCONFIG += opencv4

}


SOURCES += main.cpp\
        mainwindow.cpp \
    capture_thread.cpp \
    utilities.cpp

HEADERS  += mainwindow.h \
    capture_thread.h \
    utilities.h
RESOURCES += images.qrc

