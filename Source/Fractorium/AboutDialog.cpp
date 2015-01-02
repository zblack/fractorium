#include "FractoriumPch.h"
#include "AboutDialog.h"

/// <summary>
/// Constructor that takes a parent widget and passes it to the base, then
/// sets up the GUI.
/// </summary>
/// <param name="p">The parent widget. Default: nullptr.</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumAboutDialog::FractoriumAboutDialog(QWidget* p, Qt::WindowFlags f)
	: QDialog(p, f)
{
	ui.setupUi(this);
	adjustSize();//Must do this to ensure all text is displayed when using different fonts.
	setMinimumHeight(height());//Once properly sized, disallow resizing.
	setMaximumHeight(height());
	setMinimumWidth(width());
	setMaximumWidth(width());
}
