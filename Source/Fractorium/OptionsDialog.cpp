#include "FractoriumPch.h"
#include "OptionsDialog.h"
#include "Fractorium.h"

/// <summary>
/// Constructor that takes a pointer to the settings object and the parent widget.
/// </summary>
/// <param name="settings">A pointer to the settings object to use</param>
/// <param name="p">The parent widget. Default: nullptr.</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumOptionsDialog::FractoriumOptionsDialog(FractoriumSettings* settings, QWidget* p, Qt::WindowFlags f)
	: QDialog(p, f)
{
	int row = 0, spinHeight = 20;
	uint i;

	ui.setupUi(this);
	m_Settings = settings;
	QTableWidget* table = ui.OptionsXmlSavingTable;
	ui.ThreadCountSpin->setRange(1, Timing::ProcessorCount());
	connect(ui.OpenCLCheckBox, SIGNAL(stateChanged(int)),		 this, SLOT(OnOpenCLCheckBoxStateChanged(int)),		  Qt::QueuedConnection);
	connect(ui.PlatformCombo,  SIGNAL(currentIndexChanged(int)), this, SLOT(OnPlatformComboCurrentIndexChanged(int)), Qt::QueuedConnection);

	SetupSpinner<SpinBox, int>(table, this, row, 1, m_XmlTemporalSamplesSpin, spinHeight,  1,	1000, 100, "", "", true, 1000);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_XmlQualitySpin,		  spinHeight,  1, 200000,  50, "", "", true, 1000);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_XmlSupersampleSpin,	  spinHeight,  1,	   4,   1, "", "", true,    2);

	m_IdEdit = new QLineEdit(ui.OptionsIdentityTable);
	ui.OptionsIdentityTable->setCellWidget(0, 1, m_IdEdit);

	m_UrlEdit = new QLineEdit(ui.OptionsIdentityTable);
	ui.OptionsIdentityTable->setCellWidget(1, 1, m_UrlEdit);

	m_NickEdit = new QLineEdit(ui.OptionsIdentityTable);
	ui.OptionsIdentityTable->setCellWidget(2, 1, m_NickEdit);

	m_IdEdit->setText(m_Settings->Id());
	m_UrlEdit->setText(m_Settings->Url());
	m_NickEdit->setText(m_Settings->Nick());

	if (m_Wrapper.CheckOpenCL())
	{
		vector<string> platforms = m_Wrapper.PlatformNames();

		//Populate combo boxes with available OpenCL platforms and devices.
		for (i = 0; i < platforms.size(); i++)
			ui.PlatformCombo->addItem(QString::fromStdString(platforms[i]));

		//If init succeeds, set the selected platform and device combos to match what was saved in the settings.
		if (m_Wrapper.Init(m_Settings->PlatformIndex(), m_Settings->DeviceIndex()))
		{
			ui.OpenCLCheckBox->setChecked(	  m_Settings->OpenCL());
			ui.PlatformCombo->setCurrentIndex(m_Settings->PlatformIndex());
			ui.DeviceCombo->setCurrentIndex(  m_Settings->DeviceIndex());
		}
		else
		{
			OnPlatformComboCurrentIndexChanged(0);
			ui.OpenCLCheckBox->setChecked(false);
		}
	}
	else
	{
		ui.OpenCLCheckBox->setChecked(false);
		ui.OpenCLCheckBox->setEnabled(false);
	}

	ui.EarlyClipCheckBox->setChecked(	   m_Settings->EarlyClip());
	ui.YAxisUpCheckBox->setChecked(		   m_Settings->YAxisUp());
	ui.TransparencyCheckBox->setChecked(   m_Settings->Transparency());
	ui.DoublePrecisionCheckBox->setChecked(m_Settings->Double());
	ui.ShowAllXformsCheckBox->setChecked(  m_Settings->ShowAllXforms());
	ui.ThreadCountSpin->setValue(		   m_Settings->ThreadCount());
	
	if (m_Settings->CpuDEFilter())
		ui.CpuFilteringDERadioButton->setChecked(true);
	else
		ui.CpuFilteringLogRadioButton->setChecked(true);

	if (m_Settings->OpenCLDEFilter())
		ui.OpenCLFilteringDERadioButton->setChecked(true);
	else
		ui.OpenCLFilteringLogRadioButton->setChecked(true);

	ui.CpuSubBatchSpin->setValue(m_Settings->CpuSubBatch());
	ui.OpenCLSubBatchSpin->setValue(m_Settings->OpenCLSubBatch());

	m_XmlTemporalSamplesSpin->setValue(m_Settings->XmlTemporalSamples());
	m_XmlQualitySpin->setValue(m_Settings->XmlQuality());
	m_XmlSupersampleSpin->setValue(m_Settings->XmlSupersample());
	ui.AutoUniqueCheckBox->setChecked(m_Settings->SaveAutoUnique());

	OnOpenCLCheckBoxStateChanged(ui.OpenCLCheckBox->isChecked());
}

/// <summary>
/// GUI settings wrapper functions, getters only.
/// </summary>

bool FractoriumOptionsDialog::EarlyClip() { return ui.EarlyClipCheckBox->isChecked(); }
bool FractoriumOptionsDialog::YAxisUp() { return ui.YAxisUpCheckBox->isChecked(); }
bool FractoriumOptionsDialog::Transparency() { return ui.TransparencyCheckBox->isChecked(); }
bool FractoriumOptionsDialog::OpenCL() { return ui.OpenCLCheckBox->isChecked(); }
bool FractoriumOptionsDialog::Double() { return ui.DoublePrecisionCheckBox->isChecked(); }
bool FractoriumOptionsDialog::ShowAllXforms() { return ui.ShowAllXformsCheckBox->isChecked(); }
bool FractoriumOptionsDialog::AutoUnique() { return ui.AutoUniqueCheckBox->isChecked(); }
uint FractoriumOptionsDialog::PlatformIndex() { return ui.PlatformCombo->currentIndex(); }
uint FractoriumOptionsDialog::DeviceIndex() { return ui.DeviceCombo->currentIndex(); }
uint FractoriumOptionsDialog::ThreadCount() { return ui.ThreadCountSpin->value(); }

/// <summary>
/// Disable or enable the OpenCL related controls based on the state passed in.
/// Called when the state of the OpenCL checkbox is changed.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumOptionsDialog::OnOpenCLCheckBoxStateChanged(int state)
{
	bool checked = state == Qt::Checked;

	ui.PlatformCombo->setEnabled(checked);
	ui.DeviceCombo->setEnabled(checked);
	ui.ThreadCountSpin->setEnabled(!checked);
}

/// <summary>
/// Populate the the device combo box with all available
/// OpenCL devices for the selected platform.
/// Called when the platform combo box index changes.
/// </summary>
/// <param name="index">The selected index of the combo box</param>
void FractoriumOptionsDialog::OnPlatformComboCurrentIndexChanged(int index)
{
	vector<string> devices = m_Wrapper.DeviceNames(index);

	ui.DeviceCombo->clear();

	for (size_t i = 0; i < devices.size(); i++)
		ui.DeviceCombo->addItem(QString::fromStdString(devices[i]));

	if (ui.PlatformCombo->currentIndex() == m_Settings->PlatformIndex())
		ui.DeviceCombo->setCurrentIndex(m_Settings->DeviceIndex());
}

/// <summary>
/// Save all settings on the GUI to the settings object.
/// Called when the user clicks ok.
/// Not called if cancelled or closed with the X.
/// </summary>
void FractoriumOptionsDialog::accept()
{
	//Interactive rendering.
	m_Settings->EarlyClip(EarlyClip());
	m_Settings->YAxisUp(YAxisUp());
	m_Settings->Transparency(Transparency());
	m_Settings->OpenCL(OpenCL());
	m_Settings->Double(Double());
	m_Settings->ShowAllXforms(ShowAllXforms());
	m_Settings->PlatformIndex(PlatformIndex());
	m_Settings->DeviceIndex(DeviceIndex());
	m_Settings->ThreadCount(ThreadCount());
	m_Settings->CpuSubBatch(ui.CpuSubBatchSpin->value());
	m_Settings->OpenCLSubBatch(ui.OpenCLSubBatchSpin->value());
	m_Settings->CpuDEFilter(ui.CpuFilteringDERadioButton->isChecked());
	m_Settings->OpenCLDEFilter(ui.OpenCLFilteringDERadioButton->isChecked());

	//Xml saving.
	m_Settings->XmlTemporalSamples(m_XmlTemporalSamplesSpin->value());
	m_Settings->XmlQuality(m_XmlQualitySpin->value());
	m_Settings->XmlSupersample(m_XmlSupersampleSpin->value());
	m_Settings->SaveAutoUnique(AutoUnique());

	//Identity.
	m_Settings->Id(m_IdEdit->text());
	m_Settings->Url(m_UrlEdit->text());
	m_Settings->Nick(m_NickEdit->text());

	QDialog::accept();
}

/// <summary>
/// Restore all GUI items to what was originally in the settings object.
/// Called when the user clicks cancel or closes with the X.
/// </summary>
void FractoriumOptionsDialog::reject()
{
	//Interactive rendering.
	ui.EarlyClipCheckBox->setChecked(m_Settings->EarlyClip());
	ui.YAxisUpCheckBox->setChecked(m_Settings->YAxisUp());
	ui.TransparencyCheckBox->setChecked(m_Settings->Transparency());
	ui.OpenCLCheckBox->setChecked(m_Settings->OpenCL());
	ui.DoublePrecisionCheckBox->setChecked(m_Settings->Double());
	ui.ShowAllXformsCheckBox->setChecked(m_Settings->ShowAllXforms());
	ui.PlatformCombo->setCurrentIndex(m_Settings->PlatformIndex());
	ui.DeviceCombo->setCurrentIndex(m_Settings->DeviceIndex());
	ui.ThreadCountSpin->setValue(m_Settings->ThreadCount());
	ui.CpuSubBatchSpin->setValue(m_Settings->CpuSubBatch());
	ui.OpenCLSubBatchSpin->setValue(m_Settings->OpenCLSubBatch());
	ui.CpuFilteringDERadioButton->setChecked(m_Settings->CpuDEFilter());
	ui.OpenCLFilteringDERadioButton->setChecked(m_Settings->OpenCLDEFilter());

	//Xml saving.
	m_XmlTemporalSamplesSpin->setValue(m_Settings->XmlTemporalSamples());
	m_XmlQualitySpin->setValue(m_Settings->XmlQuality());
	m_XmlSupersampleSpin->setValue(m_Settings->XmlSupersample());
	ui.AutoUniqueCheckBox->setChecked(m_Settings->SaveAutoUnique());

	//Identity.
	m_IdEdit->setText(m_Settings->Id());
	m_UrlEdit->setText(m_Settings->Url());
	m_NickEdit->setText(m_Settings->Nick());

	QDialog::reject();
}
