TEMPLATE = lib
CONFIG += shared
CONFIG += warn_off
CONFIG += precompile_header
CONFIG -= app_bundle
CONFIG -= qt
VERSION = 0.1.4.7

DESTDIR = ../../../Bin

LIBS += -L/usr/lib -ltbb
LIBS += -L/usr/lib/x86_64-linux-gnu -lxml2

INCLUDEPATH += /usr/include/glm
INCLUDEPATH += /usr/include/tbb
INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += ../../../Source/Ember

QMAKE_CXXFLAGS += -O2
QMAKE_CXXFLAGS += -march=k8
QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CXXFLAGS += -fomit-frame-pointer
QMAKE_CXXFLAGS += -pedantic
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wnon-virtual-dtor
QMAKE_CXXFLAGS += -Wshadow
QMAKE_CXXFLAGS += -Winit-self
QMAKE_CXXFLAGS += -Wredundant-decls
QMAKE_CXXFLAGS += -Wcast-align
QMAKE_CXXFLAGS += -Winline
QMAKE_CXXFLAGS += -Wunreachable-code
QMAKE_CXXFLAGS += -Wmissing-include-dirs
QMAKE_CXXFLAGS += -Wswitch-enum
QMAKE_CXXFLAGS += -Wswitch-default
QMAKE_CXXFLAGS += -Wmain
QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
QMAKE_CXXFLAGS += -Wfatal-errors
QMAKE_CXXFLAGS += -Wall -fpermissive
QMAKE_CXXFLAGS += -Wold-style-cast
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-function
QMAKE_CXXFLAGS += -Wold-style-cast
QMAKE_CXXFLAGS += -D_M_X64
QMAKE_CXXFLAGS += -D_USRDLL 
QMAKE_CXXFLAGS += -DNDEBUG
QMAKE_CXXFLAGS += -D_CONSOLE
QMAKE_CXXFLAGS += -DBUILDING_EMBER

QMAKE_LFLAGS += -s

PRECOMPILED_HEADER = ../../../Source/Ember/EmberPch.h

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

