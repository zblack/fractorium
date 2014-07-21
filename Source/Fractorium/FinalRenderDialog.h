#pragma once

#include "ui_FinalRenderDialog.h"
#include "SpinBox.h"
#include "DoubleSpinBox.h"
#include "TwoButtonWidget.h"
#include "FractoriumSettings.h"
#include "FinalRenderEmberController.h"

/// <summary>
/// FractoriumFinalRenderDialog class.
/// </summary>

class Fractorium;//Forward declaration since Fractorium uses this dialog.

/// <summary>
/// The final render dialog is for when the user is satisfied with the parameters they've
/// set and they want to do a final render at a higher quality and at a specific resolution
/// and save it out to a file.
/// It supports rendering either the current ember, or all of them in the opened file.
/// If a single ember is rendered, it will be saved to a single output file.
/// If multiple embers are rendered, they will all be saved to a specified directory using
/// default names.
/// The user can optionally save the Xml file with each ember.
/// They can be treated as individual images, or as an animation sequence in which case
/// motion blurring with temporal samples will be applied.
/// It keeps a pointer to the main window and the global settings object for convenience.
/// The settings used here are saved to the /finalrender portion of the settings file.
/// It has its own OpenCLWrapper member for populating the combo boxes.
/// Upon running, it will delete the main window's renderer to save memory/GPU resources and restore it to its
/// original state upon exiting.
/// This class uses a controller-based design similar to the main window.
/// </summary>
class FractoriumFinalRenderDialog : public QDialog
{
	Q_OBJECT

	friend Fractorium;
	friend FinalRenderEmberControllerBase;
	friend FinalRenderEmberController<float>;
#ifdef DO_DOUBLE
	friend FinalRenderEmberController<double>;
#endif

public:
	FractoriumFinalRenderDialog(FractoriumSettings* settings, QWidget* parent, Qt::WindowFlags f = 0);
	bool EarlyClip();
	bool Transparency();
	bool OpenCL();
	bool Double();
	bool SaveXml();
	bool DoAll();
	bool DoSequence();
	bool KeepAspect();
	eScaleType Scale();
	void Scale(eScaleType scale);
	QString DoAllExt();
	QString Path();
	void Path(QString s);
	QString Prefix();
	QString Suffix();
	unsigned int PlatformIndex();
	unsigned int DeviceIndex();
	unsigned int ThreadCount();
	unsigned int Width();
	unsigned int Height();
	double Quality();
	unsigned int TemporalSamples();
	unsigned int Supersample();
	FinalRenderGuiState State();

public Q_SLOTS:
	void MoveCursorToEnd();
	void OnEarlyClipCheckBoxStateChanged(int state);
	void OnTransparencyCheckBoxStateChanged(int state);
	void OnOpenCLCheckBoxStateChanged(int state);
	void OnDoublePrecisionCheckBoxStateChanged(int state);
	void OnPlatformComboCurrentIndexChanged(int index);
	void OnDoAllCheckBoxStateChanged(int state);
	void OnKeepAspectCheckBoxStateChanged(int state);
	void OnScaleRadioButtonChanged(bool checked);
	void OnWidthChanged(int d);
	void OnHeightChanged(int d);
	void OnQualityChanged(double d);
	void OnTemporalSamplesChanged(int d);
	void OnSupersampleChanged(int d);
	void OnFileButtonClicked(bool checked);
	void OnShowFolderButtonClicked(bool checked);
	void OnRenderClicked(bool checked);
	void OnCancelRenderClicked(bool checked);

protected:
	virtual void reject();
	virtual void showEvent(QShowEvent* e);

private:
	bool CreateControllerFromGUI(bool createRenderer);
	void SetMemory();
	
	OpenCLWrapper m_Wrapper;
	Timing m_RenderTimer;
	SpinBox* m_WidthSpin;
	SpinBox* m_HeightSpin;
	DoubleSpinBox* m_QualitySpin;
	SpinBox* m_TemporalSamplesSpin;
	SpinBox* m_SupersampleSpin;
	QLineEdit* m_PrefixEdit;
	QLineEdit* m_SuffixEdit;
	FractoriumSettings* m_Settings;
	Fractorium* m_Fractorium;
	auto_ptr<FinalRenderEmberControllerBase> m_Controller;
	Ui::FinalRenderDialog ui;
};
