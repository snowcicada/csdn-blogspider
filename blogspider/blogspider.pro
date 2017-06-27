#-------------------------------------------------
#
# Project created by QtCreator 2012-03-20T22:52:10
#
#-------------------------------------------------

QT       += core gui network sql phonon

TARGET = blogspider
TEMPLATE = app


SOURCES += main.cpp\
        blogspider.cpp \
    dealthread.cpp

HEADERS  += blogspider.h \
    dealthread.h \
    common.h

FORMS    += blogspider.ui

RESOURCES += \
    images/images.qrc

RC_FILE = images/spider.rc
