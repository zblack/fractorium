TEMPLATE = lib
CONFIG += shared
CONFIG += warn_off
CONFIG += precompile_header
CONFIG -= app_bundle
CONFIG -= qt
VERSION = 0.1.4.7

DESTDIR = $$(HOME)/Dev/fractorium/Bin

LIBS += -L/usr/lib -lOpenCL
LIBS += -L/usr/lib -lGL

INCLUDEPATH += /usr/include/CL
INCLUDEPATH += /usr/include/GL
INCLUDEPATH += /usr/include/glm
INCLUDEPATH += /usr/include/tbb
INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += ../../../Source/Ember

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

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
QMAKE_CXXFLAGS += -D_CONSOLE
QMAKE_CXXFLAGS += -BUILDING_EMBERCL

QMAKE_LFLAGS_RELEASE += -s

PRECOMPILED_HEADER = ../../../Source/EmberCL/EmberCLPch.h

SOURCES += \
    ../../../Source/EmberCL/DllMain.cpp \
    ../../../Source/EmberCL/FinalAccumOpenCLKernelCreator.cpp \
    ../../../Source/EmberCL/IterOpenCLKernelCreator.cpp \
    ../../../Source/EmberCL/OpenCLWrapper.cpp \
    ../../../Source/EmberCL/RendererCL.cpp \
    ../../../Source/EmberCL/DEOpenCLKernelCreator.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../../Source/EmberCL/DEOpenCLKernelCreator.h \
    ../../../Source/EmberCL/EmberCLFunctions.h \
    ../../../Source/EmberCL/EmberCLPch.h \
    ../../../Source/EmberCL/EmberCLStructs.h \
    ../../../Source/EmberCL/FinalAccumOpenCLKernelCreator.h \
    ../../../Source/EmberCL/IterOpenCLKernelCreator.h \
    ../../../Source/EmberCL/OpenCLWrapper.h \
    ../../../Source/EmberCL/RendererCL.h

