#include "FractoriumPch.h"
#include "AboutDialog.h"

/// <summary>
/// Constructor that takes a parent widget and passes it to the base, then
/// sets up the GUI.
/// </summary>
/// <param name="parent">The parent widget. Default: NULL.</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumAboutDialog::FractoriumAboutDialog(QWidget* parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	ui.setupUi(this);
}
