TEMPLATE = lib
CONFIG += shared
CONFIG -= app_bundle
CONFIG -= qt

include(../shared_settings.pri)

!macx:PRECOMPILED_HEADER = ../../../Source/Ember/EmberPch.h

QMAKE_CXXFLAGS += -D_USRDLL 
QMAKE_CXXFLAGS += -D_CONSOLE
QMAKE_CXXFLAGS += -BUILDING_EMBERCL

SOURCES += \
    ../../../Source/Ember/Affine2D.cpp \
    ../../../Source/Ember/DllMain.cpp \
    ../../../Source/Ember/Ember.cpp \
    ../../../Source/Ember/EmberPch.cpp \
    ../../../Source/Ember/Renderer.cpp \
    ../../../Source/Ember/RendererBase.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../../Source/Ember/Affine2D.h \
    ../../../Source/Ember/CarToRas.h \
    ../../../Source/Ember/DensityFilter.h \
    ../../../Source/Ember/Ember.h \
    ../../../Source/Ember/EmberDefines.h \
    ../../../Source/Ember/EmberPch.h \
    ../../../Source/Ember/EmberToXml.h \
    ../../../Source/Ember/Interpolate.h \
    ../../../Source/Ember/Isaac.h \
    ../../../Source/Ember/Iterator.h \
    ../../../Source/Ember/Palette.h \
    ../../../Source/Ember/PaletteList.h \
    ../../../Source/Ember/Point.h \
    ../../../Source/Ember/Renderer.h \
    ../../../Source/Ember/RendererBase.h \
    ../../../Source/Ember/SheepTools.h \
    ../../../Source/Ember/SpatialFilter.h \
    ../../../Source/Ember/TemporalFilter.h \
    ../../../Source/Ember/Timing.h \
    ../../../Source/Ember/Utils.h \
    ../../../Source/Ember/Variation.h \
    ../../../Source/Ember/VariationList.h \
    ../../../Source/Ember/Variations01.h \
    ../../../Source/Ember/Variations02.h \
    ../../../Source/Ember/Variations03.h \
    ../../../Source/Ember/Variations04.h \
    ../../../Source/Ember/Variations05.h \
    ../../../Source/Ember/VariationsDC.h \
    ../../../Source/Ember/Xform.h \
    ../../../Source/Ember/XmlToEmber.h

