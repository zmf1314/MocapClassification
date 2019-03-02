#-------------------------------------------------
#
# Project created by QtCreator 2019-02-22T22:24:27
#
#-------------------------------------------------

QT       += widgets

QMAKE_CXXFLAGS += -std=c++17 -pedantic -Wall -Wextra -march=native -mtune=native -ffast-math

TARGET = MDDerivativeDTW
TEMPLATE = lib
CONFIG += plugin

DEFINES += MDDERIVATIVEDTW_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        mdderivativedtw.cpp

HEADERS += \
        mdderivativedtw.h \
        mdderivativedtw_global.h 

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx: LIBS += -L$$OUT_PWD/../ModelBase/ -lModelBase

INCLUDEPATH += $$PWD/../ModelBase
DEPENDPATH += $$PWD/../ModelBase

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../ModelBase/libModelBase.a
