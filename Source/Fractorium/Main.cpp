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
	//Required for large allocs, else GPU memory usage will be severely limited to small sizes.
	//This must be done in the application and not in the EmberCL DLL.
	_putenv_s("GPU_MAX_ALLOC_PERCENT", "100");
	Fractorium w;
	w.show();
	return a.exec();
}
