#include "FractoriumPch.h"
#include "Fractorium.h"
#include <QtWidgets/QApplication>

/// <summary>
/// Main program entry point for Fractorium.exe.
/// </summary>
/// <param name="argc">The number of command line arguments passed</param>
/// <param name="argv">The command line arguments passed</param>
/// <returns>0 if successful, else 1.</returns>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef TEST_CL
	QMessageBox::critical(QApplication::desktop(), "Error", "Fractorium cannot be run in test mode, undefine TEST_CL first.");
	return 1;
#endif

#ifdef ISAAC_FLAM3_DEBUG
	QMessageBox::critical(QApplication::desktop(), "Error", "Fractorium cannot be run in test mode, undefine ISAAC_FLAM3_DEBUG first.");
	return 1;
#endif

	//Required for large allocs, else GPU memory usage will be severely limited to small sizes.
	//This must be done in the application and not in the EmberCL DLL.
#ifdef WIN32
	_putenv_s("GPU_MAX_ALLOC_PERCENT", "100");
#else
	putenv(const_cast<char*>("GPU_MAX_ALLOC_PERCENT=100"));
#endif
	
#ifndef WIN32
	a.setStyleSheet("QGroupBox { border: 1px solid gray; border-radius: 3px; margin-top: 1.1em; background-color: transparent; } \n"
	"QTabBar::tab { height: 2.8ex; } \n"
	"QGroupBox::title "
	"{"
	 "  background-color: transparent;"
	 "  subcontrol-origin: margin; "
	 //"  left: 3px; "
	 "  subcontrol-position: top left;"
	 "  padding: 0 3px 0 3px;"
	 //"    padding: 2px;"
	 "}" );
#endif

	Fractorium w;
	w.show();
	a.installEventFilter(&w);
	return a.exec();
}

