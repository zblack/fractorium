#-------------------------------------------------
#
# Project created by QtCreator 2014-12-09T21:18:06
#
#-------------------------------------------------

QT       += core gui opengl concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Fractorium
TEMPLATE = app

include(../shared_settings.pri)

LIBS += -L$$DESTDIR -lEmber
LIBS += -L$$DESTDIR -lEmberCL

INCLUDEPATH += ../../../Source/Fractorium

!macx:PRECOMPILED_HEADER = ../../../Source/Fractorium/FractoriumPch.h

SOURCES += \
    ../../../Source/Fractorium/AboutDialog.cpp \
    ../../../Source/Fractorium/CurvesGraphicsView.cpp \
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
    ../../../Source/Fractorium/FractoriumXaos.cpp \
    ../../../Source/Fractorium/FractoriumXforms.cpp \
    ../../../Source/Fractorium/FractoriumXformsAffine.cpp \
    ../../../Source/Fractorium/FractoriumXformsColor.cpp \
    ../../../Source/Fractorium/FractoriumXformsVariations.cpp \
    ../../../Source/Fractorium/GLEmberController.cpp \
    ../../../Source/Fractorium/GLWidget.cpp \
    ../../../Source/Fractorium/Main.cpp \
    ../../../Source/Fractorium/OptionsDialog.cpp \
    ../../../Source/Fractorium/SpinBox.cpp

HEADERS  += \
    ../../../Source/Fractorium/AboutDialog.h \
    ../../../Source/Fractorium/CurvesGraphicsView.h \
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
