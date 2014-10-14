#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Constructor that initializes the entire program.
/// The setup process is very lengthy because it requires many custom modifications
/// to the GUI widgets that are not possible to do through the designer. So if something
/// is present here, it's safe to assume it can't be done in the designer.
/// </summary>
Fractorium::Fractorium(QWidget* parent)
	: QMainWindow(parent)
{
	int spinHeight = 20, iconSize = 9;
	size_t i = 0;
	Timing t;
	ui.setupUi(this);
	qRegisterMetaType<QVector<int>>("QVector<int>");//For previews.
	qRegisterMetaType<vector<unsigned char>>("vector<unsigned char>");
	qRegisterMetaType<EmberTreeWidgetItemBase*>("EmberTreeWidgetItemBase*");
	
	m_FontSize = 9;
	m_VarSortMode = 1;//Sort by weight by default.
	m_ColorDialog = new QColorDialog(this);
	m_Settings = new FractoriumSettings(this);

	m_FileDialog = NULL;//Use lazy instantiation upon first use.
	m_FolderDialog = NULL;
	m_FinalRenderDialog = new FractoriumFinalRenderDialog(m_Settings, this);
	m_OptionsDialog = new FractoriumOptionsDialog(m_Settings, this);
	m_AboutDialog = new FractoriumAboutDialog(this);

	//Put the about dialog in the screen center.
	const QRect screen = QApplication::desktop()->screenGeometry();
	m_AboutDialog->move(screen.center() - m_AboutDialog->rect().center());

	//The options dialog should be a fixed size without a size grip, however even if it's here, it still shows up. Perhaps Qt will fix it some day.
	m_OptionsDialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
	m_OptionsDialog->setSizeGripEnabled(false);
	connect(m_ColorDialog, SIGNAL(colorSelected(const QColor&)), this, SLOT(OnColorSelected(const QColor&)), Qt::QueuedConnection);

	m_XformComboColors[i++] = QColor(0XFF, 0X00, 0X00);
	m_XformComboColors[i++] = QColor(0XCC, 0XCC, 0X00);
	m_XformComboColors[i++] = QColor(0X00, 0XCC, 0X00);
	m_XformComboColors[i++] = QColor(0X00, 0XCC, 0XCC);
	m_XformComboColors[i++] = QColor(0X40, 0X40, 0XFF);

	m_XformComboColors[i++] = QColor(0XCC, 0X00, 0XCC);
	m_XformComboColors[i++] = QColor(0XCC, 0X80, 0X00);
	m_XformComboColors[i++] = QColor(0X80, 0X00, 0X4F);
	m_XformComboColors[i++] = QColor(0X80, 0X80, 0X22);
	m_XformComboColors[i++] = QColor(0X60, 0X80, 0X60);

	m_XformComboColors[i++] = QColor(0X50, 0X80, 0X80);
	m_XformComboColors[i++] = QColor(0X4F, 0X4F, 0X80);
	m_XformComboColors[i++] = QColor(0X80, 0X50, 0X80);
	m_XformComboColors[i++] = QColor(0X80, 0X60, 0X22);
	m_FinalXformComboColor  = QColor(0x7F, 0x7F, 0x7F);
	
	for (i = 0; i < XFORM_COLOR_COUNT; i++)
	{
		QPixmap pixmap(iconSize, iconSize);

		pixmap.fill(m_XformComboColors[i]);
		m_XformComboIcons[i] = QIcon(pixmap);
	}

	QPixmap pixmap(iconSize, iconSize);

	pixmap.fill(m_FinalXformComboColor);
	m_FinalXformComboIcon = QIcon(pixmap);
	   
	InitToolbarUI();
	InitParamsUI();
	InitXformsUI();
	InitXformsColorUI();
	InitXformsAffineUI();
	InitXformsVariationsUI();
	InitXformsXaosUI();
	InitPaletteUI();
	InitLibraryUI();
	InitMenusUI();

	//This will init the controller and fill in the variations and palette tables with template specific instances
	//of their respective objects.
#ifdef DO_DOUBLE
	if (m_Settings->Double())
		m_Controller = auto_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<double>(this));
	else
#endif
		m_Controller = auto_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<float>(this));

	if (m_Wrapper.CheckOpenCL() && m_Settings->OpenCL() && m_QualitySpin->value() < 30)
		m_QualitySpin->setValue(30);

	int statusBarHeight = 20;
	ui.statusBar->setMinimumHeight(statusBarHeight);
	ui.statusBar->setMaximumHeight(statusBarHeight);

	m_RenderStatusLabel = new QLabel(this);
	m_RenderStatusLabel->setMinimumWidth(200);
	m_RenderStatusLabel->setAlignment(Qt::AlignRight);
	ui.statusBar->addPermanentWidget(m_RenderStatusLabel);

	m_CoordinateStatusLabel = new QLabel(this);
	m_CoordinateStatusLabel->setMinimumWidth(300);
	m_CoordinateStatusLabel->setMaximumWidth(300);
	m_CoordinateStatusLabel->setAlignment(Qt::AlignLeft);
	ui.statusBar->addWidget(m_CoordinateStatusLabel);
	
	int progressBarHeight = 15;
	int progressBarWidth = 300;
	m_ProgressBar = new QProgressBar(this);
	m_ProgressBar->setRange(0, 100);
	m_ProgressBar->setValue(0);
	m_ProgressBar->setMinimumHeight(progressBarHeight);
	m_ProgressBar->setMaximumHeight(progressBarHeight);
	m_ProgressBar->setMinimumWidth(progressBarWidth);
	m_ProgressBar->setMaximumWidth(progressBarWidth);
	ui.statusBar->addPermanentWidget(m_ProgressBar);

	showMaximized();
	
	connect(ui.DockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
	connect(ui.DockWidget, SIGNAL(topLevelChanged(bool)),                   this, SLOT(OnDockTopLevelChanged(bool)));
	
	//Always ensure the library tab is selected, which will show preview renders.
	ui.ParamsTabWidget->setCurrentIndex(0);
	ui.XformsTabWidget->setCurrentIndex(2);//Make variations tab the currently selected one under the Xforms tab.

	//Setting certain values will completely throw off the GUI, doing everything
	//from setting strange margins, to arbitrarily changing the fonts used.
	//For these cases, the only way to fix the problem is to use style sheets.
	ui.ColorTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.GeometryTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.FilterTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.IterationTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.AffineTab->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.XformWeightNameTable->setStyleSheet("QTableWidget::item { padding: 0px; }");
	ui.XformColorIndexTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.XformColorValuesTable->setStyleSheet("QTableWidget::item { padding: 1px; }");
	ui.XformPaletteRefTable->setStyleSheet("QTableWidget::item { padding: 0px; border: none; margin: 0px; }");
	ui.PaletteAdjustTable->setStyleSheet("QTableWidget::item { padding: 1px; }");//Need this to avoid covering the top border pixel with the spinners.
	ui.statusBar->setStyleSheet("QStatusBar QLabel { padding-left: 2px; padding-right: 2px; }");

	m_PreviousPaletteRow = -1;//Force click handler the first time through.

	//Setup pointer in the GL window to point back to here.
	ui.GLDisplay->SetMainWindow(this);
	SetCoordinateStatus(0, 0, 0, 0);

	SetTabOrders();

	//At this point, everything has been setup except the renderer. Shortly after
	//this constructor exits, GLWidget::initializeGL() will create the initial flock and start the rendering timer
	//which executes whenever the program is idle. Upon starting the timer, the renderer
	//will be initialized.
}

/// <summary>
/// Destructor which saves out the settings file.
/// All other memory is cleared automatically through the use of STL.
/// </summary>
Fractorium::~Fractorium()
{
	m_Settings->sync();
}

/// <summary>
/// Set the coordinate text in the status bar.
/// </summary>
/// <param name="x">The raster x coordinate</param>
/// <param name="y">The raster y coordinate</param>
/// <param name="worldX">The cartesian world x coordinate</param>
/// <param name="worldY">The cartesian world y coordinate</param>
void Fractorium::SetCoordinateStatus(int x, int y, float worldX, float worldY)
{
	//Use sprintf rather than allocating and concatenating 6 QStrings for efficiency since this is called on every mouse move.
	sprintf_s(m_CoordinateString, 128, "Window: %4d, %4d World: %2.2f, %2.2f", x, y, worldX, worldY);
	m_CoordinateStatusLabel->setText(QString(m_CoordinateString));
}

/// <summary>
/// Apply the settings for saving an ember to an Xml file to an ember (presumably about to be saved).
/// </summary>
/// <param name="ember">The ember to apply the settings to</param>
template <typename T>
void FractoriumEmberController<T>::ApplyXmlSavingTemplate(Ember<T>& ember)
{
	ember.m_Quality         = m_Fractorium->m_Settings->XmlQuality();
	ember.m_Supersample     = m_Fractorium->m_Settings->XmlSupersample();
	ember.m_TemporalSamples = m_Fractorium->m_Settings->XmlTemporalSamples();
}

/// <summary>
/// Return whether the current ember contains a final xform and the GUI is aware of it.
/// </summary>
/// <returns>True if the current ember contains a final xform, else false.</returns>
bool Fractorium::HaveFinal()
{
	QComboBox* combo = ui.CurrentXformCombo;

	return (combo->count() > 0 && combo->itemText(combo->count() - 1) == "Final");
}

/// <summary>
/// Slots.
/// </summary>

/// <summary>
/// Empty placeholder for now.
/// Qt has a severe bug where the dock gets hidden behind the window.
/// Perhaps this will be used in the future if Qt ever fixes that bug.
/// Called when the top level dock is changed.
/// </summary>
/// <param name="topLevel">True if top level, else false.</param>
void Fractorium::OnDockTopLevelChanged(bool topLevel)
{
	//if (topLevel)
	//{
	//	if (ui.DockWidget->y() <= 0)
	//		ui.DockWidget->setGeometry(ui.DockWidget->x(), ui.DockWidget->y() + 100, ui.DockWidget->width(), ui.DockWidget->height());
	//
	//	ui.DockWidget->setFloating(true);
	//}
	//else
	//	ui.DockWidget->setFloating(false);
}

/// <summary>
/// Empty placeholder for now.
/// Qt has a severe bug where the dock gets hidden behind the window.
/// Perhaps this will be used in the future if Qt ever fixes that bug.
/// Called when the dock location is changed.
/// </summary>
/// <param name="area">The dock widget area</param>
void Fractorium::dockLocationChanged(Qt::DockWidgetArea area)
{
	//ui.DockWidget->resize(500, ui.DockWidget->height());
	//ui.DockWidget->update();
	//ui.dockWidget->setFloating(true);
	//ui.dockWidget->setFloating(false);
}

/// <summary>
/// Virtual event overrides.
/// </summary>

/// <summary>
/// Resize event, change width and height double click values to match the window size.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::resizeEvent(QResizeEvent* e)
{
	m_WidthSpin->DoubleClickNonZero(ui.GLParentScrollArea->width());
	m_HeightSpin->DoubleClickNonZero(ui.GLParentScrollArea->height());
	QMainWindow::resizeEvent(e);
}

/// <summary>
/// Stop rendering and block before exiting.
/// Called on program exit.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::closeEvent(QCloseEvent* e)
{
	if (m_Controller.get())
	{
		m_Controller->StopRenderTimer(true);//Will wait until fully exited and stopped.
		m_Controller->StopPreviewRender();
	}

	if (e)
		e->accept();
}

/// <summary>
/// Examine the files dragged when it first enters the window area.
/// Ok if at least one file is .flam3, .flam3 or .xml, else not ok.
/// Called when the user first drags files in.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls())
	{
		foreach (QUrl url, e->mimeData()->urls())
		{
			QString localFile = url.toLocalFile();
			QFileInfo fileInfo(localFile);
			QString suf = fileInfo.suffix();

			if (suf == "flam3" || suf == "flame" || suf == "xml")
			{
				e->accept();
				break;
			}
		}
	}
}

/// <summary>
/// Always accept drag when moving, so that the drop event will correctly be called.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dragMoveEvent(QDragMoveEvent* e)
{
	e->accept();
}

/// <summary>
/// Examine and open the dropped files.
/// Called when the user drops a file in.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dropEvent(QDropEvent* e)
{
	QStringList filenames;
	Qt::KeyboardModifiers mod = e->keyboardModifiers();
	bool append = mod.testFlag(Qt::ControlModifier) ? true : false;

	if (e->mimeData()->hasUrls())
	{
		foreach (QUrl url, e->mimeData()->urls())
		{
			QString localFile = url.toLocalFile();
			QFileInfo fileInfo(localFile);
			QString suf = fileInfo.suffix();

			if (suf == "flam3" || suf == "flame" || suf == "xml")
				filenames << localFile;
		}
	}

	if (!filenames.empty())
		m_Controller->OpenAndPrepFiles(filenames, append);
}

/// <summary>
/// Setup a combo box to be placed in a table cell.
/// </summary>
/// <param name="table">The table the combo box belongs to</param>
/// <param name="receiver">The receiver object</param>
/// <param name="row">The row in the table where this combo box resides</param>
/// <param name="col">The col in the table where this combo box resides</param>
/// <param name="comboBox">Double pointer to combo box which will hold the spinner upon exit</param>
/// <param name="vals">The string values to populate the combo box with</param>
/// <param name="signal">The signal the combo box emits</param>
/// <param name="slot">The slot to receive the signal</param>
/// <param name="connectionType">Type of the connection. Default: Qt::QueuedConnection.</param>
void Fractorium::SetupCombo(QTableWidget* table, const QObject* receiver, int& row, int col, StealthComboBox*& comboBox, vector<string>& vals, const char* signal, const char* slot, Qt::ConnectionType connectionType)
{
	comboBox = new StealthComboBox(table);
	ForEach(vals, [&](const string& s) { comboBox->addItem(s.c_str()); });
	table->setCellWidget(row, col, comboBox);
	connect(comboBox, signal, receiver, slot, connectionType);
	row++;
}

/// <summary>
/// Set the header of a table to be fixed.
/// </summary>
/// <param name="header">The header to set</param>
/// <param name="mode">The resizing mode to use. Default: QHeaderView::Fixed.</param>
void Fractorium::SetFixedTableHeader(QHeaderView* header, QHeaderView::ResizeMode mode)
{
	header->setVisible(true);//For some reason, the designer keeps clobbering this value, so force it here.
	header->setSectionsClickable(false);
	header->setSectionResizeMode(mode);
}

/// <summary>
/// Setup and show the open XML dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <returns>The filename selected</returns>
QStringList Fractorium::SetupOpenXmlDialog()
{
	//Lazy instantiate since it takes a long time.
	if (!m_FileDialog)
	{
		m_FileDialog = new QFileDialog(this);
		m_FileDialog->setViewMode(QFileDialog::List);
	}
	
	if (!m_FileDialog)
		return QStringList("");

	QStringList filenames;
	m_FileDialog->disconnect(SIGNAL(filterSelected(const QString&)));
	connect(m_FileDialog, &QFileDialog::filterSelected, [=](const QString &filter) { m_Settings->OpenXmlExt(filter); });

	m_FileDialog->setFileMode(QFileDialog::ExistingFiles);
	m_FileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	m_FileDialog->setNameFilter("Flam3 (*.flam3);;Flame (*.flame);;Xml (*.xml)");
	m_FileDialog->setWindowTitle("Open flame");
	m_FileDialog->setDirectory(m_Settings->OpenFolder());
	m_FileDialog->selectNameFilter(m_Settings->OpenXmlExt());

	if (m_FileDialog->exec() == QDialog::Accepted)
	{
		filenames = m_FileDialog->selectedFiles();

		if (!filenames.empty())
			m_Settings->OpenFolder(QFileInfo(filenames[0]).canonicalPath());
	}

	return filenames;
}

/// <summary>
/// Setup and show the save XML dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <param name="defaultFilename">The default filename to populate the text box with</param>
/// <returns>The filename selected</returns>
QString Fractorium::SetupSaveXmlDialog(const QString& defaultFilename)
{
	//Lazy instantiate since it takes a long time.
	if (!m_FileDialog)
	{
		m_FileDialog = new QFileDialog(this);
		m_FileDialog->setViewMode(QFileDialog::List);
	}
	
	if (!m_FileDialog)
		return "";

	QString filename;

	m_FileDialog->disconnect(SIGNAL(filterSelected(const QString&)));
	connect(m_FileDialog, &QFileDialog::filterSelected, [=](const QString &filter) { m_Settings->SaveXmlExt(filter); });
	
	//This must come first because it clears various internal states which allow the file text to be properly set.
	//This is most likely a bug in QFileDialog.
	m_FileDialog->setAcceptMode(QFileDialog::AcceptSave);
	m_FileDialog->selectFile(defaultFilename);
	m_FileDialog->setNameFilter("Flam3 (*.flam3);;Flame (*.flame);;Xml (*.xml)");
	m_FileDialog->setWindowTitle("Save flame as xml");
	m_FileDialog->setDirectory(m_Settings->SaveFolder());
	m_FileDialog->selectNameFilter(m_Settings->SaveXmlExt());

	if (m_FileDialog->exec() == QDialog::Accepted)
		filename = m_FileDialog->selectedFiles().value(0);

	return filename;
}

/// <summary>
/// Setup and show the save image dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <param name="defaultFilename">The default filename to populate the text box with</param>
/// <returns>The filename selected</returns>
QString Fractorium::SetupSaveImageDialog(const QString& defaultFilename)
{
	//Lazy instantiate since it takes a long time.
	if (!m_FileDialog)
	{
		m_FileDialog = new QFileDialog(this);
		m_FileDialog->setViewMode(QFileDialog::List);
	}
	
	if (!m_FileDialog)
		return "";
	
	QString filename;
	
	m_FileDialog->disconnect(SIGNAL(filterSelected(const QString&)));
	connect(m_FileDialog, &QFileDialog::filterSelected, [=](const QString &filter) { m_Settings->SaveImageExt(filter); });
	
	//This must come first because it clears various internal states which allow the file text to be properly set.
	//This is most likely a bug in QFileDialog.
	m_FileDialog->setAcceptMode(QFileDialog::AcceptSave);
	m_FileDialog->selectFile(defaultFilename);
	m_FileDialog->setFileMode(QFileDialog::AnyFile);
	m_FileDialog->setOption(QFileDialog::ShowDirsOnly, false);
	m_FileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
	m_FileDialog->setNameFilter("Jpeg (*.jpg);;Png (*.png);;Bmp (*.bmp)");
	m_FileDialog->setWindowTitle("Save image");
	m_FileDialog->setDirectory(m_Settings->SaveFolder());
	m_FileDialog->selectNameFilter(m_Settings->SaveImageExt());

	if (m_FileDialog->exec() == QDialog::Accepted)
		filename = m_FileDialog->selectedFiles().value(0);

	return filename;
}

/// <summary>
/// Setup and show the save folder dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <returns>The folder selected, with '/' appended to the end</returns>
QString Fractorium::SetupSaveFolderDialog()
{
	//Lazy instantiate since it takes a long time.
	if (!m_FolderDialog)
	{
		m_FolderDialog = new QFileDialog(this);
		m_FolderDialog->setViewMode(QFileDialog::List);
	}
	
	if (!m_FolderDialog)
		return "";
	
	QString filename;
	
	//This must come first because it clears various internal states which allow the file text to be properly set.
	//This is most likely a bug in QFileDialog.
	m_FolderDialog->setAcceptMode(QFileDialog::AcceptSave);
	m_FolderDialog->setFileMode(QFileDialog::Directory);
	m_FolderDialog->setOption(QFileDialog::ShowDirsOnly, true);
	m_FolderDialog->setOption(QFileDialog::DontUseNativeDialog, true);
	m_FolderDialog->selectFile("");
	m_FolderDialog->setWindowTitle("Save to folder");
	m_FolderDialog->setDirectory(m_Settings->SaveFolder());

	if (m_FolderDialog->exec() == QDialog::Accepted)
	{
		filename = MakeEnd(m_FolderDialog->selectedFiles().value(0), '/');
	}

	return filename;
}

/// <summary>
/// Thin wrapper around QMessageBox::critical() to allow it to be invoked from another thread.
/// </summary>
void Fractorium::ShowCritical(const QString& title, const QString& text, bool invokeRequired)
{
	if (!invokeRequired)
	{
		QMessageBox::critical(this, title, text);
	}
	else
	{
		QMetaObject::invokeMethod(this, "ShowCritical", Qt::QueuedConnection, Q_ARG(const QString&, title), Q_ARG(const QString&, text), Q_ARG(bool, false));
	}
}

/// <summary>
/// Explicitly set the tab orders for the entire program.
/// Qt has a facility to do this, but it fails when using custom widgets in
/// tables, so it must be done manually here.
/// This list must be kept in sync with any UI changes.
/// </summary>
void Fractorium::SetTabOrders()
{
	QWidget* w = SetTabOrder(this, ui.ColorTable, m_BrightnessSpin);//Flame.
	
	w = SetTabOrder(this, w, m_GammaSpin);
	w = SetTabOrder(this, w, m_GammaThresholdSpin);
	w = SetTabOrder(this, w, m_VibrancySpin);
	w = SetTabOrder(this, w, m_HighlightSpin);
	w = SetTabOrder(this, w, m_BackgroundColorButton);
	w = SetTabOrder(this, w, m_PaletteModeCombo);
	
	w = SetTabOrder(this, w, m_CenterXSpin);
	w = SetTabOrder(this, w, m_CenterYSpin);
	w = SetTabOrder(this, w, m_ScaleSpin);
	w = SetTabOrder(this, w, m_ZoomSpin);
	w = SetTabOrder(this, w, m_RotateSpin);
	w = SetTabOrder(this, w, m_ZPosSpin);
	w = SetTabOrder(this, w, m_PerspectiveSpin);
	w = SetTabOrder(this, w, m_PitchSpin);
	w = SetTabOrder(this, w, m_YawSpin);
	w = SetTabOrder(this, w, m_DepthBlurSpin);
	
	w = SetTabOrder(this, w, m_SpatialFilterWidthSpin);
	w = SetTabOrder(this, w, m_SpatialFilterTypeCombo);
	w = SetTabOrder(this, w, m_TemporalFilterTypeCombo);
	w = SetTabOrder(this, w, m_DEFilterMinRadiusSpin);
	w = SetTabOrder(this, w, m_DEFilterMaxRadiusSpin);
	w = SetTabOrder(this, w, m_DECurveSpin);
	
	w = SetTabOrder(this, w, m_PassesSpin);
	w = SetTabOrder(this, w, m_TemporalSamplesSpin);
	w = SetTabOrder(this, w, m_QualitySpin);
	w = SetTabOrder(this, w, m_SupersampleSpin);
	w = SetTabOrder(this, w, m_AffineInterpTypeCombo);
	w = SetTabOrder(this, w, m_InterpTypeCombo);
	
	w = SetTabOrder(this, ui.CurrentXformCombo, ui.AddXformButton);//Xforms.
	w = SetTabOrder(this, w, ui.DuplicateXformButton);
	w = SetTabOrder(this, w, ui.ClearXformButton);
	w = SetTabOrder(this, w, ui.DeleteXformButton);
	w = SetTabOrder(this, w, ui.AddFinalXformButton);
	w = SetTabOrder(this, w, m_XformWeightSpin);
	w = SetTabOrder(this, w, m_XformWeightSpinnerButtonWidget->m_Button);

	w = SetTabOrder(this, m_XformColorIndexSpin, ui.XformColorScroll);//Xforms color.
	w = SetTabOrder(this, w, m_XformColorSpeedSpin);
	w = SetTabOrder(this, w, m_XformOpacitySpin);
	w = SetTabOrder(this, w, m_XformDirectColorSpin);
	w = SetTabOrder(this, w, ui.SoloXformCheckBox);

	w = SetTabOrder(this, ui.PreAffineGroupBox, m_PreX1Spin);//Xforms affine.
	w = SetTabOrder(this, w, m_PreX2Spin);
	w = SetTabOrder(this, w, m_PreY1Spin);
	w = SetTabOrder(this, w, m_PreY2Spin);
	w = SetTabOrder(this, w, m_PreO1Spin);
	w = SetTabOrder(this, w, m_PreO2Spin);
	w = SetTabOrder(this, w, ui.PreFlipVerticalButton);
	w = SetTabOrder(this, w, ui.PreResetButton);
	w = SetTabOrder(this, w, ui.PreFlipHorizontalButton);
	w = SetTabOrder(this, w, ui.PreRotate90CcButton);
	w = SetTabOrder(this, w, ui.PreRotateCcButton);
	w = SetTabOrder(this, w, ui.PreRotateCombo);
	w = SetTabOrder(this, w, ui.PreRotateCButton);
	w = SetTabOrder(this, w, ui.PreRotate90CButton);
	w = SetTabOrder(this, w, ui.PreMoveUpButton);
	w = SetTabOrder(this, w, ui.PreMoveDownButton);
	w = SetTabOrder(this, w, ui.PreMoveCombo);
	w = SetTabOrder(this, w, ui.PreMoveLeftButton);
	w = SetTabOrder(this, w, ui.PreMoveRightButton);
	w = SetTabOrder(this, w, ui.PreScaleUpButton);
	w = SetTabOrder(this, w, ui.PreScaleCombo);
	w = SetTabOrder(this, w, ui.PreScaleDownButton);
	w = SetTabOrder(this, w, ui.ShowPreAffineCurrentRadio);
	w = SetTabOrder(this, w, ui.ShowPreAffineAllRadio);
	w = SetTabOrder(this, w, ui.PostAffineGroupBox);
	w = SetTabOrder(this, w, m_PostX1Spin);
	w = SetTabOrder(this, w, m_PostX2Spin);
	w = SetTabOrder(this, w, m_PostY1Spin);
	w = SetTabOrder(this, w, m_PostY2Spin);
	w = SetTabOrder(this, w, m_PostO1Spin);
	w = SetTabOrder(this, w, m_PostO2Spin);
	w = SetTabOrder(this, w, ui.PostFlipVerticalButton);
	w = SetTabOrder(this, w, ui.PostResetButton);
	w = SetTabOrder(this, w, ui.PostFlipHorizontalButton);
	w = SetTabOrder(this, w, ui.PostRotate90CcButton);
	w = SetTabOrder(this, w, ui.PostRotateCcButton);
	w = SetTabOrder(this, w, ui.PostRotateCombo);
	w = SetTabOrder(this, w, ui.PostRotateCButton);
	w = SetTabOrder(this, w, ui.PostRotate90CButton);
	w = SetTabOrder(this, w, ui.PostMoveUpButton);
	w = SetTabOrder(this, w, ui.PostMoveDownButton);
	w = SetTabOrder(this, w, ui.PostMoveCombo);
	w = SetTabOrder(this, w, ui.PostMoveLeftButton);
	w = SetTabOrder(this, w, ui.PostMoveRightButton);
	w = SetTabOrder(this, w, ui.PostScaleUpButton);
	w = SetTabOrder(this, w, ui.PostScaleCombo);
	w = SetTabOrder(this, w, ui.PostScaleDownButton);
	w = SetTabOrder(this, w, ui.ShowPostAffineCurrentRadio);
	w = SetTabOrder(this, w, ui.ShowPostAffineAllRadio);
	w = SetTabOrder(this, w, ui.LocalPivotRadio);
	w = SetTabOrder(this, w, ui.WorldPivotRadio);

	w = SetTabOrder(this, ui.VariationsFilterLineEdit, ui.VariationsFilterClearButton);//Xforms variation.
	w = SetTabOrder(this, w, ui.VariationsTree);
	
	//Xforms xaos is done dynamically every time.

	w = SetTabOrder(this, m_PaletteHueSpin, m_PaletteContrastSpin);//Palette.
	w = SetTabOrder(this, w, m_PaletteSaturationSpin);
	w = SetTabOrder(this, w, m_PaletteBlurSpin);
	w = SetTabOrder(this, w, m_PaletteBrightnessSpin);
	w = SetTabOrder(this, w, m_PaletteFrequencySpin);
	w = SetTabOrder(this, w, ui.PaletteListTable);

	w = SetTabOrder(this, ui.InfoBoundsGroupBox, ui.InfoBoundsFrame);//Info.
	w = SetTabOrder(this, w, ui.InfoBoundsTable);
	w = SetTabOrder(this, w, ui.InfoFileOpeningGroupBox);
	w = SetTabOrder(this, w, ui.InfoFileOpeningTextEdit);
	w = SetTabOrder(this, w, ui.InfoRenderingGroupBox);
	w = SetTabOrder(this, w, ui.InfoRenderingTextEdit);
}

/// <summary>
/// This is no longer needed and was used to compensate for a different bug
/// however the code is interesting, so keep it around for possible future use.
/// This was used to correct a rotation bug where matrix rotation comes out in the wrong direction
/// if x1, y1 (a & d) are on the left side of the line from 0,0 to 
/// x2, y2 (b, e). In that case, the angle must be flipped. In order
/// to determine which side of the line it's on, create a mat2
/// and find its determinant. Values > 0 are on the left side of the line.
/// </summary>
/// <param name="affine">The affine.</param>
/// <returns></returns>
int Fractorium::FlipDet(Affine2D<float>& affine)
{
	float x1 = affine.A();
	float y1 = affine.D();
	float x2 = affine.B();
	float y2 = affine.E();

	//Just make the other end of the line be the center of the circle.
	glm::mat2 mat( 0 - x1,  0 - y1,//Col 0.
				  x2 - x1, y2 - y1);//Col 1.

	return (glm::determinant(mat) > 0) ? -1 : 1;
}

//template<typename spinType, typename valType>//See note at the end of Fractorium.h
//void Fractorium::SetupSpinner(QTableWidget* table, const QObject* receiver, int& row, int col, spinType*& spinBox, int height, valType min, valType max, valType step, const char* signal, const char* slot, bool incRow, valType val, valType doubleClickZero, valType doubleClickNonZero)
//{
//	spinBox = new spinType(table, height, step);
//	spinBox->setRange(min, max);
//	spinBox->setValue(val);
//	table->setCellWidget(row, col, spinBox);
//
//	if (string(signal) != "" && string(slot) != "")
//		connect(spinBox, signal, receiver, slot, connectionType);
//
//	if (doubleClickNonZero != -999 && doubleClickZero != -999)
//	{
//		spinBox->DoubleClick(true);
//		spinBox->DoubleClickZero((valType)doubleClickZero);
//		spinBox->DoubleClickNonZero((valType)doubleClickNonZero);
//	}
//
//	if (incRow)
//		row++;
//}
