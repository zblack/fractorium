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
	QString m_DoAllExt;
	QString m_Prefix;
	QString m_Suffix;
	unsigned int m_PlatformIndex;
	unsigned int m_DeviceIndex;
	unsigned int m_ThreadCount;
	unsigned int m_Width;
	unsigned int m_Height;
	double m_Quality;
	unsigned int m_TemporalSamples;
	unsigned int m_Supersample;
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
	FinalRenderEmberControllerBase(FractoriumFinalRenderDialog* finalRender);
	virtual ~FinalRenderEmberControllerBase() { }

	virtual unsigned __int64 SyncAndComputeMemory() { return 0; }
	virtual QString Name() const { return ""; }
	virtual void ResetProgress(bool total = true) { }
#ifdef DO_DOUBLE
	virtual void SetOriginalEmber(Ember<double>& ember) { }
#else
	virtual void SetOriginalEmber(Ember<float>& ember) { }
#endif
	virtual double OriginalAspect() { return 1; }

	void CancelRender();
	bool CreateRendererFromGUI();

protected:
	bool m_Run;
	bool m_PreviewRun;
	unsigned int m_ImageCount;
	unsigned int m_FinishedImageCount;
	double m_PureIterTime;

	QFuture<void> m_Result;
	QFuture<void> m_PreviewResult;
	std::function<void (void)> m_RenderFunc;
	std::function<void (void)> m_PreviewRenderFunc;
	vector<unsigned char> m_PreviewFinalImage;
	
	FractoriumSettings* m_Settings;
	FractoriumFinalRenderDialog* m_FinalRender;
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

	virtual void SetEmber(const Ember<float>& ember, bool verbatim = false);
	virtual void CopyEmber(Ember<float>& ember);
	virtual void SetEmberFile(const EmberFile<float>& emberFile);
	virtual void CopyEmberFile(EmberFile<float>& emberFile);
#ifdef DO_DOUBLE
	virtual void SetEmber(const Ember<double>& ember, bool verbatim = false);
	virtual void CopyEmber(Ember<double>& ember);
	virtual void SetEmberFile(const EmberFile<double>& emberFile);
	virtual void CopyEmberFile(EmberFile<double>& emberFile);
	virtual void SetOriginalEmber(Ember<double>& ember);
#else
	virtual void SetOriginalEmber(Ember<float>& ember);
#endif
	virtual double OriginalAspect();
	virtual int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs);
	virtual bool Render();
	virtual bool CreateRenderer(eRendererType renderType, unsigned int platform, unsigned int device, bool shared = true);
	virtual unsigned int SizeOfT() { return sizeof(T); }
	virtual unsigned __int64 SyncAndComputeMemory();
	virtual QString Name() const { return QString::fromStdString(m_Ember.m_Name); }
	virtual void ResetProgress(bool total = true);
	void CancelPreviewRender();

protected:
	void Sync(Ember<T>& ember);

	Ember<T> m_Ember;
	Ember<T> m_OriginalEmber;
	Ember<T> m_PreviewEmber;
	EmberFile<T> m_EmberFile;
	EmberToXml<T> m_XmlWriter;
	auto_ptr<EmberNs::Renderer<T, T>> m_PreviewRenderer;
};

template class FinalRenderEmberController<float>;

#ifdef DO_DOUBLE
	template class FinalRenderEmberController<double>;
#endif