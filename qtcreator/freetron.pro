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
    ../blobs.cpp \
    ../forms.cpp \
    ../processor.cpp \
    ../website/content.cpp \
    ../website/database.cpp \
    ../website/date.cpp \
    ../website/rpc.cpp \
    ../website/website.cpp

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
    ../blobs.h \
    ../threadqueuevoid.h \
    ../forms.h \
    ../processor.h \
    ../website/content.h \
    ../website/database.h \
    ../website/date.h \
    ../website/options.h \
    ../website/rpc.h \
    ../website/website.h

OTHER_FILES +=

