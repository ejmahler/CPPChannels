TEMPLATE = app

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

SOURCES += main.cpp

HEADERS += \
    channel.h

