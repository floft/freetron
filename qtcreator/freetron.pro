TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../rotate.cpp \
    ../read.cpp \
    ../pixels.cpp \
    ../outline.cpp \
    ../math.cpp \
    ../log.cpp \
    ../histogram.cpp \
    ../freetron.cpp \
    ../extract.cpp \
    ../data.cpp \
    ../cores.cpp \
    ../boxes.cpp \
    ../box.cpp \
    ../blobs.cpp

HEADERS += \
    ../threadqueue.h \
    ../rotate.h \
    ../read.h \
    ../pixels.h \
    ../outline.h \
    ../options.h \
    ../math.h \
    ../maputils.h \
    ../log.h \
    ../histogram.h \
    ../extract.h \
    ../data.h \
    ../cores.h \
    ../boxes.h \
    ../box.h \
    ../blobs.h

OTHER_FILES +=

