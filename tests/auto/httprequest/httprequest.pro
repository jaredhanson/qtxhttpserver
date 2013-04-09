TEMPLATE = app
TARGET = tst_httprequest

QT += testlib
macx:CONFIG -= app_bundle

include(../../../mkspecs/test.pri)

HEADERS += tst_httprequest.h
SOURCES += tst_httprequest.cpp
