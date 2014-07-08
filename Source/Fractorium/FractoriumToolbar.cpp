#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the toolbar UI.
/// </summary>
void Fractorium::InitToolbarUI()
{
	//These aren't menus but duplicate menu functionality in a pseudo-toolbar.
	connect(ui.SaveCurrentAsXmlButton,		  SIGNAL(clicked(bool)), this, SLOT(OnSaveCurrentAsXmlButtonClicked(bool)),		   Qt::QueuedConnection);
	connect(ui.SaveEntireFileAsXmlButton,	  SIGNAL(clicked(bool)), this, SLOT(OnSaveEntireFileAsXmlButtonClicked(bool)),	   Qt::QueuedConnection);
	connect(ui.SaveCurrentToOpenedFileButton, SIGNAL(clicked(bool)), this, SLOT(OnSaveCurrentToOpenedFileButtonClicked(bool)), Qt::QueuedConnection);
}

/// <summary>
/// Wrappers around calls to menu items.
/// </summary>

void Fractorium::OnSaveCurrentAsXmlButtonClicked(bool checked) { OnActionSaveCurrentAsXml(checked); }
void Fractorium::OnSaveEntireFileAsXmlButtonClicked(bool checked) { OnActionSaveEntireFileAsXml(checked); }
void Fractorium::OnSaveCurrentToOpenedFileButtonClicked(bool checked) { OnActionSaveCurrentToOpenedFile(checked); }
