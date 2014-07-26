#include "FractoriumPch.h"
#include "FinalRenderDialog.h"
#include "Fractorium.h"

/// <summary>
/// Constructor which sets up the GUI for the final rendering dialog.
/// Settings used to populate widgets with initial values.
/// This function contains the render function as a lambda.
/// </summary>
/// <param name="settings">Pointer to the global settings object to use</param>
/// <param name="parent">The parent widget</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumFinalRenderDialog::FractoriumFinalRenderDialog(FractoriumSettings* settings, QWidget* parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{	
	ui.setupUi(this);

	int row = 0, spinHeight = 20;
	unsigned int i;
	double dmax = numeric_limits<double>::max();
	QTableWidget* table = ui.FinalRenderGeometryTable;
	QTableWidgetItem* item = NULL;

	m_Fractorium = (Fractorium*)parent;
	m_Settings = settings;
	ui.FinalRenderThreadCountSpin->setRange(1, Timing::ProcessorCount());
	connect(ui.FinalRenderEarlyClipCheckBox,	   SIGNAL(stateChanged(int)),		 this, SLOT(OnEarlyClipCheckBoxStateChanged(int)),		 Qt::QueuedConnection);
	connect(ui.FinalRenderYAxisUpCheckBox,	       SIGNAL(stateChanged(int)),		 this, SLOT(OnYAxisUpCheckBoxStateChanged(int)),		 Qt::QueuedConnection);
	connect(ui.FinalRenderTransparencyCheckBox,	   SIGNAL(stateChanged(int)),		 this, SLOT(OnTransparencyCheckBoxStateChanged(int)),	 Qt::QueuedConnection);
	connect(ui.FinalRenderOpenCLCheckBox,		   SIGNAL(stateChanged(int)),		 this, SLOT(OnOpenCLCheckBoxStateChanged(int)),		     Qt::QueuedConnection);
	connect(ui.FinalRenderDoublePrecisionCheckBox, SIGNAL(stateChanged(int)),		 this, SLOT(OnDoublePrecisionCheckBoxStateChanged(int)), Qt::QueuedConnection);
	connect(ui.FinalRenderPlatformCombo,		   SIGNAL(currentIndexChanged(int)), this, SLOT(OnPlatformComboCurrentIndexChanged(int)),	 Qt::QueuedConnection);
	connect(ui.FinalRenderDoAllCheckBox,		   SIGNAL(stateChanged(int)),		 this, SLOT(OnDoAllCheckBoxStateChanged(int)),			 Qt::QueuedConnection);
	connect(ui.FinalRenderKeepAspectCheckBox,	   SIGNAL(stateChanged(int)),		 this, SLOT(OnKeepAspectCheckBoxStateChanged(int)),		 Qt::QueuedConnection);
	connect(ui.FinalRenderScaleNoneRadioButton,	   SIGNAL(toggled(bool)),			 this, SLOT(OnScaleRadioButtonChanged(bool)),			 Qt::QueuedConnection);
	connect(ui.FinalRenderScaleWidthRadioButton,   SIGNAL(toggled(bool)),			 this, SLOT(OnScaleRadioButtonChanged(bool)),			 Qt::QueuedConnection);
	connect(ui.FinalRenderScaleHeightRadioButton,  SIGNAL(toggled(bool)),			 this, SLOT(OnScaleRadioButtonChanged(bool)),			 Qt::QueuedConnection);

	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_WidthSpin,		    spinHeight, 10, 100000, 50, SIGNAL(valueChanged(int)),    SLOT(OnWidthChanged(int)),		   true, 1980);
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_HeightSpin,		    spinHeight, 10, 100000, 50, SIGNAL(valueChanged(int)),    SLOT(OnHeightChanged(int)),		   true, 1080);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_QualitySpin,		    spinHeight,  1,   dmax, 50, SIGNAL(valueChanged(double)), SLOT(OnQualityChanged(double)),	   true, 1000);
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_TemporalSamplesSpin, spinHeight,  1,   5000, 50, SIGNAL(valueChanged(int)),    SLOT(OnTemporalSamplesChanged(int)), true, 1000);
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_SupersampleSpin,	    spinHeight,	 1,		 4,  1, SIGNAL(valueChanged(int)),    SLOT(OnSupersampleChanged(int)),	   true,    2);

	row++;//Memory usage.

	TwoButtonWidget* tbw = new TwoButtonWidget("...", "Open", 22, 40, 22, table);
	table->setCellWidget(row, 1, tbw);
	table->item(row++, 1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	connect(tbw->m_Button1, SIGNAL(clicked(bool)), this, SLOT(OnFileButtonClicked(bool)),		Qt::QueuedConnection);
	connect(tbw->m_Button2, SIGNAL(clicked(bool)), this, SLOT(OnShowFolderButtonClicked(bool)), Qt::QueuedConnection);

	m_PrefixEdit = new QLineEdit(table);
	table->setCellWidget(row++, 1, m_PrefixEdit);

	m_SuffixEdit = new QLineEdit(table);
	table->setCellWidget(row++, 1, m_SuffixEdit);

	ui.StartRenderButton->disconnect(SIGNAL(clicked(bool)));
	connect(ui.StartRenderButton, SIGNAL(clicked(bool)), this, SLOT(OnRenderClicked(bool)),		  Qt::QueuedConnection);
	connect(ui.StopRenderButton,  SIGNAL(clicked(bool)), this, SLOT(OnCancelRenderClicked(bool)), Qt::QueuedConnection);

	if (m_Wrapper.CheckOpenCL())
	{
		vector<string> platforms = m_Wrapper.PlatformNames();

		//Populate combo boxes with available OpenCL platforms and devices.
		for (i = 0; i < platforms.size(); i++)
			ui.FinalRenderPlatformCombo->addItem(QString::fromStdString(platforms[i]));

		//If init succeeds, set the selected platform and device combos to match what was saved in the settings.
		if (m_Wrapper.Init(m_Settings->FinalPlatformIndex(), m_Settings->FinalDeviceIndex()))
		{
			ui.FinalRenderOpenCLCheckBox->setChecked(	 m_Settings->FinalOpenCL());
			ui.FinalRenderPlatformCombo->setCurrentIndex(m_Settings->FinalPlatformIndex());
			ui.FinalRenderDeviceCombo->setCurrentIndex(  m_Settings->FinalDeviceIndex());
		}
		else
		{
			OnPlatformComboCurrentIndexChanged(0);
			ui.FinalRenderOpenCLCheckBox->setChecked(false);
		}
	}
	else
	{
		ui.FinalRenderOpenCLCheckBox->setChecked(false);
		ui.FinalRenderOpenCLCheckBox->setEnabled(false);
	}

	ui.FinalRenderEarlyClipCheckBox->setChecked(      m_Settings->FinalEarlyClip());
	ui.FinalRenderYAxisUpCheckBox->setChecked(        m_Settings->FinalYAxisUp());
	ui.FinalRenderTransparencyCheckBox->setChecked(   m_Settings->FinalTransparency());
	ui.FinalRenderDoublePrecisionCheckBox->setChecked(m_Settings->FinalDouble());
	ui.FinalRenderSaveXmlCheckBox->setChecked(		  m_Settings->FinalSaveXml());
	ui.FinalRenderDoAllCheckBox->setChecked(	      m_Settings->FinalDoAll());
	ui.FinalRenderDoSequenceCheckBox->setChecked(     m_Settings->FinalDoSequence());
	ui.FinalRenderKeepAspectCheckBox->setChecked(	  m_Settings->FinalKeepAspect());
	ui.FinalRenderThreadCountSpin->setValue(	      m_Settings->FinalThreadCount());

	m_WidthSpin->setValue(m_Settings->FinalWidth());
	m_HeightSpin->setValue(m_Settings->FinalHeight());
	m_QualitySpin->setValue(m_Settings->FinalQuality());
	m_TemporalSamplesSpin->setValue(m_Settings->FinalTemporalSamples());
	m_SupersampleSpin->setValue(m_Settings->FinalSupersample());

	Scale((eScaleType)m_Settings->FinalScale());

	if (m_Settings->FinalDoAllExt() == "jpg")
		ui.FinalRenderJpgRadioButton->setChecked(true);
	else
		ui.FinalRenderPngRadioButton->setChecked(true);
	
	//Explicitly call these to enable/disable the appropriate controls.
	OnOpenCLCheckBoxStateChanged(ui.FinalRenderOpenCLCheckBox->isChecked());
	OnDoAllCheckBoxStateChanged(ui.FinalRenderDoAllCheckBox->isChecked());

	QSize s = size();
	int desktopHeight = qApp->desktop()->availableGeometry().height();

	s.setHeight(min(s.height(), (int)((double)desktopHeight * 0.90)));
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, s, qApp->desktop()->availableGeometry()));
}

/// <summary>
/// GUI settings wrapper functions, getters only.
/// </summary>

bool FractoriumFinalRenderDialog::EarlyClip() { return ui.FinalRenderEarlyClipCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::YAxisUp() { return ui.FinalRenderYAxisUpCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::Transparency() { return ui.FinalRenderTransparencyCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::OpenCL() { return ui.FinalRenderOpenCLCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::Double() { return ui.FinalRenderDoublePrecisionCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::SaveXml() { return ui.FinalRenderSaveXmlCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::DoAll() { return ui.FinalRenderDoAllCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::DoSequence() { return ui.FinalRenderDoSequenceCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::KeepAspect() { return ui.FinalRenderKeepAspectCheckBox->isChecked(); }
QString FractoriumFinalRenderDialog::DoAllExt() { return ui.FinalRenderJpgRadioButton->isChecked() ? "jpg" : "png"; }
QString FractoriumFinalRenderDialog::Path() { return ui.FinalRenderGeometryTable->item(6, 1)->text(); }
void FractoriumFinalRenderDialog::Path(QString s) { ui.FinalRenderGeometryTable->item(6, 1)->setText(s); }
QString FractoriumFinalRenderDialog::Prefix() { return m_PrefixEdit->text(); }
QString FractoriumFinalRenderDialog::Suffix() { return m_SuffixEdit->text(); }
unsigned int FractoriumFinalRenderDialog::PlatformIndex() { return ui.FinalRenderPlatformCombo->currentIndex(); }
unsigned int FractoriumFinalRenderDialog::DeviceIndex() { return ui.FinalRenderDeviceCombo->currentIndex(); }
unsigned int FractoriumFinalRenderDialog::ThreadCount() { return ui.FinalRenderThreadCountSpin->value(); }
unsigned int FractoriumFinalRenderDialog::Width() { return m_WidthSpin->value(); }
unsigned int FractoriumFinalRenderDialog::Height() { return m_HeightSpin->value(); }
double FractoriumFinalRenderDialog::Quality() { return m_QualitySpin->value(); }
unsigned int FractoriumFinalRenderDialog::TemporalSamples() { return m_TemporalSamplesSpin->value(); }
unsigned int FractoriumFinalRenderDialog::Supersample() { return m_SupersampleSpin->value(); }

/// <summary>
/// Capture the current state of the Gui.
/// Used to hold options for performing the final render.
/// </summary>
/// <returns>The state of the Gui as a struct</returns>
FinalRenderGuiState FractoriumFinalRenderDialog::State()
{
	FinalRenderGuiState state;

	state.m_EarlyClip = EarlyClip();
	state.m_YAxisUp = YAxisUp();
	state.m_Transparency = Transparency();
	state.m_OpenCL = OpenCL();
	state.m_Double = Double();
	state.m_SaveXml = SaveXml();
	state.m_DoAll = DoAll();
	state.m_DoSequence = DoSequence();
	state.m_KeepAspect = KeepAspect();
	state.m_Scale = Scale();
	state.m_Path = Path();
	state.m_DoAllExt = DoAllExt();
	state.m_Prefix = Prefix();
	state.m_Suffix = Suffix();
	state.m_PlatformIndex = PlatformIndex();
	state.m_DeviceIndex = DeviceIndex();
	state.m_ThreadCount = ThreadCount();
	state.m_Width = Width();
	state.m_Height = Height();
	state.m_Quality = Quality();
	state.m_TemporalSamples = TemporalSamples();
	state.m_Supersample = Supersample();

	return state;
}

/// <summary>
/// Return the type of scaling desired based on what radio button has been selected.
/// </summary>
/// <returns>The type of scaling as an eScaleType enum</returns>
eScaleType FractoriumFinalRenderDialog::Scale()
{
	if (ui.FinalRenderScaleNoneRadioButton->isChecked())
		return SCALE_NONE;
	else if (ui.FinalRenderScaleWidthRadioButton->isChecked())
		return SCALE_WIDTH;
	else if (ui.FinalRenderScaleHeightRadioButton->isChecked())
		return SCALE_HEIGHT;
	else
		return SCALE_NONE;
}

/// <summary>
/// Set the type of scaling desired which will select the corresponding radio button.
/// </summary>
/// <param name="scale">The type of scaling to use</param>
void FractoriumFinalRenderDialog::Scale(eScaleType scale)
{
	if (scale == SCALE_NONE)
		ui.FinalRenderScaleNoneRadioButton->setChecked(true);
	else if (scale == SCALE_WIDTH)
		ui.FinalRenderScaleWidthRadioButton->setChecked(true);
	else if (scale == SCALE_HEIGHT)
		ui.FinalRenderScaleHeightRadioButton->setChecked(true);
	else
		ui.FinalRenderScaleNoneRadioButton->setChecked(true);
}

/// <summary>
/// Simple wrapper to put moving the cursor to the end in a signal
/// so it can be called from a thread.
/// </summary>
void FractoriumFinalRenderDialog::MoveCursorToEnd()
{
	ui.FinalRenderTextOutput->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

/// <summary>
/// Whether to use early clipping before spatial filtering.
/// </summary>
/// <param name="index">True to early clip, else don't.</param>
void FractoriumFinalRenderDialog::OnEarlyClipCheckBoxStateChanged(int state)
{
	SetMemory();
}

/// <summary>
/// Whether the positive Y axis of the final output image is up.
/// </summary>
/// <param name="yup">True if the positive y axis is up, else false.</param>
void FractoriumFinalRenderDialog::OnYAxisUpCheckBoxStateChanged(int state)
{
	SetMemory();
}

/// <summary>
/// Whether to use transparency in png images.
/// </summary>
/// <param name="index">True to use transparency, else don't.</param>
void FractoriumFinalRenderDialog::OnTransparencyCheckBoxStateChanged(int state)
{
	SetMemory();
}

/// <summary>
/// Set whether to use OpenCL in the rendering process or not.
/// </summary>
/// <param name="state">Use OpenCL if state == Qt::Checked, else don't.</param>
void FractoriumFinalRenderDialog::OnOpenCLCheckBoxStateChanged(int state)
{
	bool checked = state == Qt::Checked;

	ui.FinalRenderPlatformCombo->setEnabled(checked);
	ui.FinalRenderDeviceCombo->setEnabled(checked);
	ui.FinalRenderThreadCountSpin->setEnabled(!checked);
	SetMemory();
}

/// <summary>
/// Set whether to use double or single precision in the rendering process or not.
/// This will recreate the entire controller.
/// </summary>
/// <param name="state">Use double if state == Qt::Checked, else float.</param>
void FractoriumFinalRenderDialog::OnDoublePrecisionCheckBoxStateChanged(int state)
{
	SetMemory();
}

/// <summary>
/// Populate the the device combo box with all available
/// OpenCL devices for the selected platform.
/// Called when the platform combo box index changes.
/// </summary>
/// <param name="index">The selected index of the combo box</param>
void FractoriumFinalRenderDialog::OnPlatformComboCurrentIndexChanged(int index)
{
	vector<string> devices = m_Wrapper.DeviceNames(index);

	ui.FinalRenderDeviceCombo->clear();

	for (size_t i = 0; i < devices.size(); i++)
		ui.FinalRenderDeviceCombo->addItem(QString::fromStdString(devices[i]));
}

/// <summary>
/// The do all checkbox was changed.
/// If checked, render all embers available in the currently opened file, else
/// only render the current ember.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnDoAllCheckBoxStateChanged(int state)
{
	ui.FinalRenderDoSequenceCheckBox->setEnabled(ui.FinalRenderDoAllCheckBox->isChecked());
	ui.FinalRenderExtensionGroupBox->setEnabled(ui.FinalRenderDoAllCheckBox->isChecked());
}

/// <summary>
/// Whether to keep the aspect ratio of the desired width and height the same
/// as that of the original width and height.
/// </summary>
/// <param name="checked">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnKeepAspectCheckBoxStateChanged(int state)
{
	if (state && m_Controller.get())
		m_HeightSpin->SetValueStealth(m_WidthSpin->value() / m_Controller->OriginalAspect());

	SetMemory();
}

/// <summary>
/// The scaling method radio button selection was changed.
/// </summary>
/// <param name="checked">The state of the radio button</param>
void FractoriumFinalRenderDialog::OnScaleRadioButtonChanged(bool checked)
{
	if (checked)
		SetMemory();
}

/// <summary>
/// The width spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the height spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnWidthChanged(int d)
{
	if (ui.FinalRenderKeepAspectCheckBox->isChecked() && m_Controller.get())
		m_HeightSpin->SetValueStealth(m_WidthSpin->value() / m_Controller->OriginalAspect());

	SetMemory();
}

/// <summary>
/// The height spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the width spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnHeightChanged(int d)
{
	if (ui.FinalRenderKeepAspectCheckBox->isChecked() && m_Controller.get())
		m_WidthSpin->SetValueStealth(m_HeightSpin->value() * m_Controller->OriginalAspect());

	SetMemory();
}

/// <summary>
/// The quality spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnQualityChanged(double d)
{
	SetMemory();
}

/// <summary>
/// The temporal samples spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnTemporalSamplesChanged(int d)
{
	SetMemory();
}

/// <summary>
/// The supersample spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnSupersampleChanged(int d)
{
	SetMemory();
}

/// <summary>
/// If a single ember is being rendered, show the save file dialog.
/// If a more than one is being rendered, show the save folder dialog.
/// Called when the ... file button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnFileButtonClicked(bool checked)
{
	bool doAll = ui.FinalRenderDoAllCheckBox->isChecked();
	QString filename;
	
	if (doAll)
		filename = m_Fractorium->SetupSaveFolderDialog();
	else
		filename = m_Fractorium->SetupSaveImageDialog(m_Controller->Name());

	if (filename != "")
	{
		if (doAll)
		{
			if (!filename.endsWith(QDir::separator()))
				filename += "/";
		}

		QFileInfo fileInfo(filename);
		QString path = fileInfo.absolutePath();

		m_Settings->SaveFolder(path);//Any time they exit the box with a valid value, preserve it in the settings.
		Path(filename);
		SetMemory();
	}
}

/// <summary>
/// Show the folder where the last rendered image was saved to.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnShowFolderButtonClicked(bool checked)
{
	QString text = Path();
	
	if (text != "")
	{
		QFileInfo fileInfo(text);
		QString path = fileInfo.absolutePath();
		QDir dir(path);

		if (dir.exists())
			QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
}

/// <summary>
/// Start the render process.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnRenderClicked(bool checked)
{
	if (CreateControllerFromGUI(true))
		m_Controller->Render();
}

/// <summary>
/// Cancel the render.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnCancelRenderClicked(bool checked)
{
	if (m_Controller.get())
		m_Controller->CancelRender();
}

/// <summary>
/// Uses the options and the ember to populate widgets.
/// Called when the dialog is initially shown.
/// </summary>
/// <param name="e">The event</param>
void FractoriumFinalRenderDialog::showEvent(QShowEvent* e)
{
#ifdef DO_DOUBLE
	Ember<double> ed;
#else
	Ember<float> ed;
#endif

	if (CreateControllerFromGUI(true))
	{
		m_Fractorium->m_Controller->CopyEmber(ed);//Copy the current ember from the main window out in to a temp.
		m_Controller->SetEmber(ed);//Copy the temp into the final render controller.
		m_Controller->SetOriginalEmber(ed);
		SetMemory();
		m_Controller->ResetProgress();
	}
	
	QDir dir(m_Settings->SaveFolder());
	QString name = m_Controller->Name();

	if (dir.exists() && name != "")
		Path(dir.absolutePath() + "/" + name + ".png");

	ui.FinalRenderTextOutput->clear();
	QDialog::showEvent(e);
}

/// <summary>
/// Close the dialog without running, or if running, cancel and exit.
/// Settings will not be saved.
/// Control will be returned to Fractorium::OnActionFinalRender().
/// </summary>
void FractoriumFinalRenderDialog::reject()
{
	if (m_Controller.get())
	{
		m_Controller->CancelRender();
		m_Controller->DeleteRenderer();
	}

	QDialog::reject();
}

/// <summary>
/// Create the controller from the options and optionally its underlying renderer.
/// </summary>
/// <returns>True if successful, else false.</returns>
bool FractoriumFinalRenderDialog::CreateControllerFromGUI(bool createRenderer)
{
	bool ok = true;
#ifdef DO_DOUBLE
	size_t size = Double() ? sizeof(double) : sizeof(float);
	Ember<double> ed;
	Ember<double> orig;
	EmberFile<double> efd;
#else
	size_t size = sizeof(float);
	Ember<float> ed;
	Ember<float> orig;
	EmberFile<float> efd;
#endif

	if (!m_Controller.get() || (m_Controller->SizeOfT() != size))
	{
		//First check if a controller has already been created, and if so, save its embers and gracefully shut it down.
		if (m_Controller.get())
		{		
			m_Controller->CopyEmber(ed);//Convert float to double or save double verbatim;
			m_Controller->CopyEmberFile(efd);
			m_Controller->Shutdown();
		}

		//Create a float or double controller based on the GUI.
#ifdef DO_DOUBLE
		if (Double())
			m_Controller = auto_ptr<FinalRenderEmberControllerBase>(new FinalRenderEmberController<double>(this));
		else
#endif
			m_Controller = auto_ptr<FinalRenderEmberControllerBase>(new FinalRenderEmberController<float>(this));

		//Restore the ember and ember file.
		if (m_Controller.get())
		{
			m_Controller->SetEmber(ed);//Convert float to double or set double verbatim;
			m_Controller->SetEmberFile(efd);	
			m_Fractorium->m_Controller->CopyEmber(orig);//Copy the current ember from the main window out in to a temp.
			m_Controller->SetOriginalEmber(orig);
		}
	}

	if (m_Controller.get())
	{
		if (createRenderer)
			return m_Controller->CreateRendererFromGUI();
		else
			return true;
	}
	else
		return false;
}

/// <summary>
/// Compute the amount of memory needed via call to SyncAndComputeMemory(), then
/// assign the result to the table cell as text.
/// </summary>
void FractoriumFinalRenderDialog::SetMemory()
{
	if (isVisible() && CreateControllerFromGUI(true))
		ui.FinalRenderGeometryTable->item(5, 1)->setText(QLocale(QLocale::English).toString(m_Controller->SyncAndComputeMemory()));
}
