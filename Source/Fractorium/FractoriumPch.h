#define GL_GLEXT_PROTOTYPES 1
#define XFORM_COLOR_COUNT 14

#include "Renderer.h"
#include "RendererCL.h"
#include "VariationList.h"
#include "OpenCLWrapper.h"
#include "XmlToEmber.h"
#include "EmberToXml.h"
#include "SheepTools.h"
#include "JpegUtils.h"
#include "EmberCommon.h"
#include <deque>

#undef QT_OPENGL_ES_2//Make absolutely sure OpenGL ES is not used.
#define QT_NO_OPENGL_ES_2
#include <QtWidgets>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QColorDialog>
#include <QTreeWidget>
#include <QWheelEvent>
#include <QItemDelegate>
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QGLWidget>
//#include <QOpenGLFunctions_2_0.h>
#include <qopenglfunctions_2_0.h>
#include <QtWidgets/QMainWindow>
#include <QFuture>
#include <QtConcurrentRun>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifndef WIN32
    #undef Bool
#endif

using namespace std;
using namespace EmberNs;
using namespace EmberCLns;
