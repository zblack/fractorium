#pragma once

#include "FractoriumSettings.h"
#include "FractoriumEmberController.h"

/// <summary>
/// FinalRenderEmberControllerBase and FinalRenderEmberController<T> classes.
/// </summary>

/// <summary>
/// FractoriumEmberController and Fractorium need each other, but each can't include the other.
/// So Fractorium includes this file, and Fractorium is declared as a forward declaration here.
/// </summary>
class Fractorium;
class FractoriumFinalRenderDialog;

/// <summary>
/// Used to hold the options specified in the current state of the Gui for performing the final render.
/// </summary>
struct FinalRenderGuiState
{
	bool m_EarlyClip;
	bool m_YAxisUp;
	bool m_AlphaChannel;
	bool m_Transparency;
	bool m_OpenCL;
	bool m_Double;
	bool m_SaveXml;
	bool m_DoAll;
	bool m_DoSequence;
	bool m_KeepAspect;
	eScaleType m_Scale;
	QString m_Path;
	QString m_Ext;
	QString m_Prefix;
	QString m_Suffix;
	uint m_PlatformIndex;
	uint m_DeviceIndex;
	uint m_ThreadCount;
	double m_WidthScale;
	double m_HeightScale;
	double m_Quality;
	uint m_TemporalSamples;
	uint m_Supersample;
	uint m_Strips;
};

/// <summary>
/// FinalRenderEmberControllerBase serves as a non-templated base class with virtual
/// functions which will be overridden in a derived class that takes a template parameter.
/// Although not meant to be used as an interactive renderer, it derives from FractoriumEmberControllerBase
/// to access a few of its members to avoid having to redefine them here.
/// </summary>
class FinalRenderEmberControllerBase : public FractoriumEmberControllerBase
{
	friend FractoriumFinalRenderDialog;

public:
	FinalRenderEmberControllerBase(FractoriumFinalRenderDialog* finalRenderDialog);
	virtual ~FinalRenderEmberControllerBase() { }

	virtual void SyncCurrentToGui() { }
	virtual void SyncGuiToEmbers(size_t widthOverride = 0, size_t heightOverride = 0) { }
	virtual void SyncCurrentToSizeSpinners(bool scale, bool size) { }
	virtual void ResetProgress(bool total = true) { }
	virtual pair<size_t, size_t> SyncAndComputeMemory() { return pair<size_t, size_t>(0, 0); }
	virtual double OriginalAspect() { return 1; }
	virtual QString ComposePath(const QString& name) { return ""; }

	void CancelRender();
	bool CreateRendererFromGUI();
	void Output(const QString& s);

protected:
	bool m_Run;
	bool m_PreviewRun;
	uint m_ImageCount;
	uint m_FinishedImageCount;

	QFuture<void> m_Result;
	QFuture<void> m_FinalPreviewResult;
	std::function<void (void)> m_FinalRenderFunc;
	std::function<void (void)> m_FinalPreviewRenderFunc;
	
	FractoriumSettings* m_Settings;
	FractoriumFinalRenderDialog* m_FinalRenderDialog;
	FinalRenderGuiState m_GuiState;
	OpenCLWrapper m_Wrapper;
	CriticalSection m_PreviewCs;
	Timing m_RenderTimer;
	Timing m_TotalTimer;
};

/// <summary>
/// Templated derived class which implements all interaction functionality between the embers
/// of a specific template type and the final render dialog;
/// </summary>
template<typename T>
class FinalRenderEmberController : public FinalRenderEmberControllerBase
{
public:
	FinalRenderEmberController(FractoriumFinalRenderDialog* finalRender);
	virtual ~FinalRenderEmberController() { }

	//Virtual functions overridden from FractoriumEmberControllerBase.
	virtual void SetEmberFile(const EmberFile<float>& emberFile) override;
	virtual void CopyEmberFile(EmberFile<float>& emberFile, std::function<void(Ember<float>& ember)> perEmberOperation = [&](Ember<float>& ember) { }) override;
#ifdef DO_DOUBLE
	virtual void SetEmberFile(const EmberFile<double>& emberFile) override;
	virtual void CopyEmberFile(EmberFile<double>& emberFile, std::function<void(Ember<double>& ember)> perEmberOperation = [&](Ember<double>& ember) { }) override;
#endif
	virtual void SetEmber(size_t index) override;
	virtual bool Render() override;
	virtual bool CreateRenderer(eRendererType renderType, uint platform, uint device, bool shared = true) override;
	virtual int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs) override;
	virtual size_t Index() const override { return m_Ember->m_Index; }
	virtual uint SizeOfT() const override { return sizeof(T); }

	//Virtual functions overridden from FinalRenderEmberControllerBase.
	virtual void SyncCurrentToGui() override;
	virtual void SyncGuiToEmbers(size_t widthOverride = 0, size_t heightOverride = 0) override;
	virtual void SyncCurrentToSizeSpinners(bool scale, bool size) override;
	virtual void ResetProgress(bool total = true)  override;
	virtual pair<size_t, size_t> SyncAndComputeMemory() override;
	virtual double OriginalAspect() override { return double(m_Ember->m_OrigFinalRasW) / m_Ember->m_OrigFinalRasH; }
	virtual QString Name() const override { return QString::fromStdString(m_Ember->m_Name); }
	virtual QString ComposePath(const QString& name) override;
	
protected:
	void CancelPreviewRender();
	void RenderComplete(Ember<T>& ember);
	void SyncGuiToEmber(Ember<T>& ember, size_t widthOverride = 0, size_t heightOverride = 0);

	Ember<T>* m_Ember;
	Ember<T> m_PreviewEmber;
	EmberFile<T> m_EmberFile;
	EmberToXml<T> m_XmlWriter;
	unique_ptr<EmberNs::Renderer<T, T>> m_FinalPreviewRenderer;
};

template class FinalRenderEmberController<float>;

#ifdef DO_DOUBLE
	template class FinalRenderEmberController<double>;
#endif