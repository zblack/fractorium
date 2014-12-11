#include "FractoriumPch.h"
#include "AboutDialog.h"

/// <summary>
/// Constructor that takes a parent widget and passes it to the base, then
/// sets up the GUI.
/// </summary>
/// <param name="p">The parent widget. Default: NULL.</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumAboutDialog::FractoriumAboutDialog(QWidget* p, Qt::WindowFlags f)
	: QDialog(p, f)
{
	ui.setupUi(this);
}
