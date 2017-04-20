#-------------------------------------------------
#
# Project created by QtCreator 2015-12-25T10:58:18
#
#-------------------------------------------------

QT       += core gui \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = atcdtdemo

TEMPLATE = app

CONFIG += unversioned_libname

FORMS    += dtdemo.ui \
    directorywidget.ui \
    videoplay.ui

RESOURCES += \
    videopic.qrc

SOURCES += main.cpp\
        dtdemo.cpp \
        load_data.cpp \
    widget_directory.cpp \
    dtdemo_videoplay.cpp

HEADERS  += dtdemo.h \
         load_data.h \
        widget_directory.h \
    dtdemo_videoplay.h
    
    
# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += ${DA_TOP}/lib/include \
			   ${DA_TOP}/multimedia/render/include \
			   ${DA_SYSROOT}/usr/lib/glib-2.0/include \
			   ${DA_SYSROOT}/usr/include/glib-2.0

# INCLUDEPATH += ${DA_TOP}/lib/include/breakpad



MY_BUILD_SYSTEM=$$(BUILD_SYSTEM)
equals(MY_BUILD_SYSTEM, atc) {                
LIBS += -lsurface_atc -ldirectrender -lglib-2.0 -lgstatcutils-1.0 -L${DA_TOP}/lib/lib
} else {
LIBS += -lsurface_atc -ldirectrender -lglib-2.0 -lgstatcutils-1.0 -L${DA_TOP}/application/lib -L${DA_TOP}/lib/lib
}




