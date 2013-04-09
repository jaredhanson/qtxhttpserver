include(features/qtx_testcase.prf)


QT += network

INCLUDEPATH += ../../../src/httpserver \
               ../../../../qtxmockcore/include

LIBS += -L../../../lib \
        -L../../../../qtxmockcore/lib

LIBS += -lQtxHttpServer -lQtxMockCore
