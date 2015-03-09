CONFIG += warn_off
VERSION = 0.1.4.7

macx {
  LIBS += -framework OpenGL
  LIBS += -framework OpenCL

  # homebrew installs into /usr/local
  LIBS += -L/usr/local/lib

  INCLUDEPATH += /usr/local/include
  INCLUDEPATH += ../../../Deps

  QMAKE_MAC_SDK = macosx10.9
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

  QMAKE_CXXFLAGS += -stdlib=libc++
} else {
  CONFIG += precompile_header
  LIBS += -L/usr/lib -lGL
  LIBS += -L/usr/lib -lOpenCL

  QMAKE_LFLAGS_RELEASE += -s
}

nvidia {
  QMAKE_CXXFLAGS += -DNVIDIA
}

native {
  QMAKE_CXXFLAGS += -march=native
} else {
  QMAKE_CXXFLAGS += -march=k8
}


DESTDIR = $$(HOME)/Dev/fractorium/Bin
debug:DESTDIR = $$(HOME)/Dev/fractorium/Dbg

LIBS += -L/usr/lib -ljpeg
LIBS += -L/usr/lib -lpng
LIBS += -L/usr/lib -ltbb
LIBS += -L/usr/lib/x86_64-linux-gnu -lxml2

CMAKE_CXXFLAGS += -DCL_USE_DEPRECATED_OPENCL_1_1_APIS

INCLUDEPATH += /usr/include/CL
INCLUDEPATH += /usr/include/GL
INCLUDEPATH += /usr/include/glm
INCLUDEPATH += /usr/include/tbb
INCLUDEPATH += /usr/include/libxml2
INCLUDEPATH += ../../../Source/Ember
INCLUDEPATH += ../../../Source/EmberCL
INCLUDEPATH += ../../../Source/EmberCommon

QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS_RELEASE += -DNDEBUG
QMAKE_CXXFLAGS_RELEASE += -fomit-frame-pointer

QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fpermissive
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
QMAKE_CXXFLAGS += -D_CONSOLE
QMAKE_CXXFLAGS += -D_USRDLL

