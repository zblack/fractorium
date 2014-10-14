#pragma once

#include "ui_FinalRenderDialog.h"
#include "SpinBox.h"
#include "DoubleSpinBox.h"
#include "TwoButtonComboWidget.h"
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
	bool YAxisUp();
	bool Transparency();
	bool OpenCL();
	bool Double();
	bool SaveXml();
	bool DoAll();
	bool DoSequence();
	bool KeepAspect();
	bool ApplyToAll();
	eScaleType Scale();
	void Scale(eScaleType scale);
	QString Ext();
	QString Path();
	void Path(const QString& s);
	QString Prefix();
	QString Suffix();
	unsigned int Current();
	unsigned int PlatformIndex();
	unsigned int DeviceIndex();
	unsigned int ThreadCount();
	double WidthScale();
	double HeightScale();
	double Quality();
	unsigned int TemporalSamples();
	unsigned int Supersample();
	unsigned int Strips();
	FinalRenderGuiState State();

public Q_SLOTS:
	void MoveCursorToEnd();
	void OnEarlyClipCheckBoxStateChanged(int state);
	void OnYAxisUpCheckBoxStateChanged(int state);
	void OnTransparencyCheckBoxStateChanged(int state);
	void OnOpenCLCheckBoxStateChanged(int state);
	void OnDoublePrecisionCheckBoxStateChanged(int state);
	void OnDoAllCheckBoxStateChanged(int state);
	void OnDoSequenceCheckBoxStateChanged(int state);
	void OnFinalRenderCurrentSpinChanged(int d);
	void OnPlatformComboCurrentIndexChanged(int index);
	void OnApplyAllCheckBoxStateChanged(int state);
	void OnFinalRenderWidthScaleChanged(double d);
	void OnFinalRenderHeightScaleChanged(double d);
	void OnKeepAspectCheckBoxStateChanged(int state);
	void OnScaleRadioButtonChanged(bool checked);
	void OnQualityChanged(double d);
	void OnTemporalSamplesChanged(int d);
	void OnSupersampleChanged(int d);
	void OnStripsChanged(int d);
	void OnFileButtonClicked(bool checked);
	void OnShowFolderButtonClicked(bool checked);
	void OnFinalRenderExtIndexChanged(int d);
	void OnFinalRenderPrefixChanged(const QString& s);
	void OnFinalRenderSuffixChanged(const QString& s);
	void OnRenderClicked(bool checked);
	void OnCancelRenderClicked(bool checked);

protected:
	virtual void reject();
	virtual void showEvent(QShowEvent* e);

private:
	bool CreateControllerFromGUI(bool createRenderer);
	bool SetMemory();
	
	int m_MemoryCellIndex;
	int m_PathCellIndex;
	OpenCLWrapper m_Wrapper;
	Timing m_RenderTimer;
	DoubleSpinBox* m_WidthScaleSpin;
	DoubleSpinBox* m_HeightScaleSpin;
	DoubleSpinBox* m_QualitySpin;
	SpinBox* m_TemporalSamplesSpin;
	SpinBox* m_SupersampleSpin;
	SpinBox* m_StripsSpin;
	TwoButtonComboWidget* m_Tbcw;
	QLineEdit* m_PrefixEdit;
	QLineEdit* m_SuffixEdit;
	FractoriumSettings* m_Settings;
	Fractorium* m_Fractorium;
	auto_ptr<FinalRenderEmberControllerBase> m_Controller;
	Ui::FinalRenderDialog ui;
};
