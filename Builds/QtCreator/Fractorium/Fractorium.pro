#-------------------------------------------------
#
# Project created by QtCreator 2014-12-09T21:18:06
#
#-------------------------------------------------

QT       += core gui opengl concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Fractorium
TEMPLATE = app
!macx:CONFIG += precompile_header
VERSION = 0.1.4.7

DESTDIR = $$(HOME)/Dev/fractorium/Bin

!macx:LIBS += -L/usr/lib -lOpenCL
macx:LIBS += -framework OpenCL

!macx:LIBS += -L/usr/lib -lGL
macx:LIBS += -framework OpenGL

LIBS += -L/usr/lib -ljpeg
LIBS += -L/usr/lib -lpng
LIBS += -L/usr/lib -ltbb
LIBS += -L/usr/lib/x86_64-linux-gnu -lxml2
LIBS += -L$$DESTDIR -lEmber
LIBS += -L$$DESTDIR -lEmberCL

INCLUDEPATH += /usr/include/CL
INCLUDEPATH += /usr/include/GL
INCLUDEPATH += /usr/include/glm
INCLUDEPATH += /usr/include/tbb
INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += ../../../Source/Ember
INCLUDEPATH += ../../../Source/EmberCL
INCLUDEPATH += ../../../Source/EmberCommon
INCLUDEPATH += ../../../Source/Fractorium

# homebrew installs into /usr/local
macx:LIBS += -L/usr/local/lib

macx:INCLUDEPATH += /usr/local/include
macx:INCLUDEPATH += ../../../Deps

macx:QMAKE_MAC_SDK = macosx10.9
macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

QMAKE_CXXFLAGS_RELEASE += -O2
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
#QMAKE_CXXFLAGS += -Wredundant-decls
QMAKE_CXXFLAGS += -Wcast-align
#QMAKE_CXXFLAGS += -Winline
QMAKE_CXXFLAGS += -Wunreachable-code
QMAKE_CXXFLAGS += -Wmissing-include-dirs
#QMAKE_CXXFLAGS += -Wswitch-enum
#QMAKE_CXXFLAGS += -Wswitch-default
QMAKE_CXXFLAGS += -Wmain
#QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
#QMAKE_CXXFLAGS += -Wfatal-errors
QMAKE_CXXFLAGS += -Wall -fpermissive
QMAKE_CXXFLAGS += -Wold-style-cast
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-function
QMAKE_CXXFLAGS += -Wold-style-cast
QMAKE_CXXFLAGS += -D_M_X64
QMAKE_CXXFLAGS += -D_USRDLL 
QMAKE_CXXFLAGS += -D_CONSOLE

!macx:QMAKE_LFLAGS_RELEASE += -s

!macx:PRECOMPILED_HEADER = ../../../Source/Fractorium/FractoriumPch.h

SOURCES += \
    ../../../Source/Fractorium/AboutDialog.cpp \
    ../../../Source/Fractorium/DoubleSpinBox.cpp \
    ../../../Source/Fractorium/FinalRenderDialog.cpp \
    ../../../Source/Fractorium/FinalRenderEmberController.cpp \
    ../../../Source/Fractorium/Fractorium.cpp \
    ../../../Source/Fractorium/FractoriumEmberController.cpp \
    ../../../Source/Fractorium/FractoriumInfo.cpp \
    ../../../Source/Fractorium/FractoriumLibrary.cpp \
    ../../../Source/Fractorium/FractoriumMenus.cpp \
    ../../../Source/Fractorium/FractoriumPalette.cpp \
    ../../../Source/Fractorium/FractoriumParams.cpp \
    ../../../Source/Fractorium/FractoriumPch.cpp \
    ../../../Source/Fractorium/FractoriumRender.cpp \
    ../../../Source/Fractorium/FractoriumSettings.cpp \
    ../../../Source/Fractorium/FractoriumToolbar.cpp \
    ../../../Source/Fractorium/FractoriumXforms.cpp \
    ../../../Source/Fractorium/FractoriumXformsAffine.cpp \
    ../../../Source/Fractorium/FractoriumXformsColor.cpp \
    ../../../Source/Fractorium/FractoriumXformsVariations.cpp \
    ../../../Source/Fractorium/FractoriumXformsXaos.cpp \
    ../../../Source/Fractorium/GLEmberController.cpp \
    ../../../Source/Fractorium/GLWidget.cpp \
    ../../../Source/Fractorium/Main.cpp \
    ../../../Source/Fractorium/OptionsDialog.cpp \
    ../../../Source/Fractorium/SpinBox.cpp

HEADERS  += \
    ../../../Source/Fractorium/AboutDialog.h \
    ../../../Source/Fractorium/DoubleSpinBox.h \
    ../../../Source/Fractorium/EmberFile.h \
    ../../../Source/Fractorium/EmberTreeWidgetItem.h \
    ../../../Source/Fractorium/FinalRenderDialog.h \
    ../../../Source/Fractorium/FinalRenderEmberController.h \
    ../../../Source/Fractorium/Fractorium.h \
    ../../../Source/Fractorium/FractoriumEmberController.h \
    ../../../Source/Fractorium/FractoriumPch.h \
    ../../../Source/Fractorium/FractoriumSettings.h \
    ../../../Source/Fractorium/GLEmberController.h \
    ../../../Source/Fractorium/GLWidget.h \
    ../../../Source/Fractorium/OptionsDialog.h \
    ../../../Source/Fractorium/resource.h \
    ../../../Source/Fractorium/SpinBox.h \
    ../../../Source/Fractorium/StealthComboBox.h \
    ../../../Source/Fractorium/TableWidget.h \
    ../../../Source/Fractorium/TwoButtonComboWidget.h \
    ../../../Source/Fractorium/VariationTreeWidgetItem.h \
    ../../../Source/EmberCommon/EmberCommon.h \
    ../../../Source/EmberCommon/JpegUtils.h \
    ../../../Source/EmberCommon/EmberCommonPch.h \
    ../../../Source/Fractorium/FractoriumCommon.h

FORMS    += \
    ../../../Source/Fractorium/AboutDialog.ui \
    ../../../Source/Fractorium/FinalRenderDialog.ui \
    ../../../Source/Fractorium/Fractorium.ui \
    ../../../Source/Fractorium/OptionsDialog.ui

OTHER_FILES += \
    ../../../Source/Fractorium/Fractorium.aps \
    ../../../Source/Fractorium/Fractorium.rc

RESOURCES += \
    ../../../Source/Fractorium/Fractorium.qrc
