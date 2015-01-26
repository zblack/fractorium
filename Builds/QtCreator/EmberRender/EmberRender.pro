TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

include(../shared_settings.pri)

LIBS += -L$$DESTDIR -lEmber
LIBS += -L$$DESTDIR -lEmberCL

!macx:PRECOMPILED_HEADER = ../../../Source/EmberCommon/EmberCommonPch.h

SOURCES += \
    ../../../Source/EmberRender/EmberRender.cpp \
    ../../../Source/EmberCommon/EmberCommonPch.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../../Source/EmberRender/EmberRender.h \
    ../../../Source/EmberCommon/EmberCommon.h \
    ../../../Source/EmberCommon/EmberCommonPch.h \
    ../../../Source/EmberCommon/EmberOptions.h \
    ../../../Source/EmberCommon/JpegUtils.h \
    ../../../Source/EmberCommon/SimpleGlob.h \
    ../../../Source/EmberCommon/SimpleOpt.h

