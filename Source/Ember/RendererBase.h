#pragma once

#include "Utils.h"
#include "Ember.h"
#include "DensityFilter.h"

/// <summary>
/// RendererBase, RenderCallback and EmberStats classes.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Function pointers present a major restriction when dealing
/// with member functions, and that is they can only point to
/// static ones. So instead of a straight function pointer, use
/// a callback class with a single virtual callback
/// member function.
/// Template argument expected to be float or double.
/// </summary>
class EMBER_API RenderCallback
{
public:
	/// <summary>
	/// Virtual destructor to ensure anything declared in derived classes gets cleaned up.
	/// </summary>
	virtual ~RenderCallback() { }

	/// <summary>
	/// Empty progress function to be implemented in derived classes to take action on progress updates.
	/// </summary>
	/// <param name="ember">The ember currently being rendered</param>
	/// <param name="foo">An extra dummy parameter</param>
	/// <param name="fraction">The progress fraction from 0-100</param>
	/// <param name="stage">The stage of iteration. 1 is iterating, 2 is density filtering, 2 is final accumulation.</param>
	/// <param name="etaMs">The estimated milliseconds to completion of the current stage</param>
	/// <returns>Override should return 0 if an abort is requested, else 1 to continue rendering</returns>
	virtual int ProgressFunc(Ember<float>& ember, void* foo, double fraction, int stage, double etaMs) { return 0; }
	virtual int ProgressFunc(Ember<double>& ember, void* foo, double fraction, int stage, double etaMs) { return 0; }
};

/// <summary>
/// Render statistics for the number of iterations ran,
/// number of bad values calculated during iteration, and
/// the total time for the entire render from the start of
/// iteration to the end of final accumulation.
/// </summary>
class EMBER_API EmberStats
{
public:
	/// <summary>
	/// Constructor which sets all values to 0.
	/// </summary>
	EmberStats()
	{
		Clear();
	}

	void Clear()
	{
		m_Iters = 0;
		m_Badvals = 0;
		m_IterMs = 0;
		m_RenderMs = 0;
	}

	EmberStats& operator += (const EmberStats& stats)
	{
		m_Iters += stats.m_Iters;
		m_Badvals += stats.m_Badvals;
		m_IterMs += stats.m_IterMs;
		m_RenderMs += stats.m_RenderMs;
		return *this;
	}

	size_t m_Iters, m_Badvals;
	double m_IterMs, m_RenderMs;
};

/// <summary>
/// The types of available renderers.
/// Add more in the future as different rendering methods are experimented with.
/// Possible values might be: CPU+OpenGL, Particle, Inverse.
/// </summary>
enum eRendererType { CPU_RENDERER, OPENCL_RENDERER };

/// <summary>
/// A base class with virtual functions to allow both templating and polymorphism to work together.
/// Derived classes will implement all of these functions.
/// Note that functions which return a decimal number use the most precise type, double.
/// </summary>
class EMBER_API RendererBase : public EmberReport
{
//using EmberReport::m_ErrorReport;
public:
	RendererBase();
	virtual ~RendererBase() { }

	//Non-virtual processing functions.
	void ChangeVal(std::function<void(void)> func, eProcessAction action);
	size_t HistMemoryRequired(size_t strips);
	pair<size_t, size_t> MemoryRequired(size_t strips, bool includeFinal, bool threadedWrite);
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> RandVec();
	bool PrepFinalAccumVector(vector<byte>& pixels);

	//Virtual processing functions.
	virtual bool Ok() const;
	virtual size_t MemoryAvailable();
	virtual void SetEmber(Ember<float>& ember, eProcessAction action = FULL_RENDER) { }
	virtual void SetEmber(vector<Ember<float>>& embers) { }
	virtual void SetEmber(Ember<double>& ember, eProcessAction action = FULL_RENDER) { }
	virtual void SetEmber(vector<Ember<double>>& embers) { }
	virtual bool RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec);

	//Abstract processing functions.
	virtual bool CreateDEFilter(bool& newAlloc) = 0;
	virtual bool CreateSpatialFilter(bool& newAlloc) = 0;
	virtual bool CreateTemporalFilter(bool& newAlloc) = 0;
	virtual void ComputeBounds() = 0;
	virtual void ComputeCamera() = 0;
	virtual eRenderStatus Run(vector<byte>& finalImage, double time = 0, size_t subBatchCountOverride = 0, bool forceOutput = false, size_t finalOffset = 0) = 0;
	virtual EmberImageComments ImageComments(EmberStats& stats, size_t printEditDepth = 0, bool intPalette = false, bool hexPalette = true) = 0;
	virtual DensityFilterBase* GetDensityFilter() = 0;

	//Non-virtual renderer properties, getters only.
	size_t		   SuperRasW()					 const;
	size_t		   SuperRasH()					 const;
	size_t		   SuperSize()					 const;
	size_t		   FinalRowSize()				 const;
	size_t		   FinalDimensions()			 const;
	size_t		   FinalBufferSize()			 const;
	size_t		   PixelSize()					 const;
	size_t		   GutterWidth()				 const;
	size_t		   DensityFilterOffset()		 const;
	size_t		   TotalIterCount(size_t strips) const;
	size_t		   ItersPerTemporalSample()		 const;
	eProcessState  ProcessState()				 const;
	eProcessAction ProcessAction()				 const;
	EmberStats     Stats() 						 const;

	//Non-virtual render getters and setters.
	bool LockAccum() const;
	void LockAccum(bool lockAccum);
	bool EarlyClip() const;
	void EarlyClip(bool earlyClip);
	bool YAxisUp() const;
	void YAxisUp(bool yup);
	bool InsertPalette() const;
	void InsertPalette(bool insertPalette);
	bool ReclaimOnResize() const;
	void ReclaimOnResize(bool reclaimOnResize);
	bool Transparency() const;
	void Transparency(bool transparency);
	void Callback(RenderCallback* callback);
	void ThreadCount(size_t threads, const char* seedString = nullptr);
	size_t BytesPerChannel() const;
	void BytesPerChannel(size_t bytesPerChannel);
	size_t NumChannels() const;
	eInteractiveFilter InteractiveFilter() const;
	void InteractiveFilter(eInteractiveFilter filter);

	//Virtual render properties, getters and setters.
	virtual void NumChannels(size_t numChannels);
	virtual size_t ThreadCount()   const;
	virtual eRendererType RendererType() const;

	//Abstract render properties, getters only.
	virtual size_t TemporalSamples()			   const = 0;
	virtual size_t HistBucketSize()				   const = 0;
	virtual size_t FinalRasW()		               const = 0;
	virtual size_t FinalRasH()					   const = 0;
	virtual size_t SubBatchSize()				   const = 0;
	virtual size_t FuseCount()					   const = 0;
	virtual double ScaledQuality()                 const = 0;
	virtual double LowerLeftX(bool  gutter = true) const = 0;
	virtual double LowerLeftY(bool  gutter = true) const = 0;
	virtual double UpperRightX(bool gutter = true) const = 0;
	virtual double UpperRightY(bool gutter = true) const = 0;

	//Non-virtual threading control.
	void Reset();
	void EnterRender();
	void LeaveRender();
	void EnterFinalAccum();
	void LeaveFinalAccum();
	void EnterResize();
	void LeaveResize();
	void Abort();
	bool Aborted();
	bool InRender();
	bool InFinalAccum();

	void* m_ProgressParameter;

protected:
	bool m_EarlyClip;
	bool m_YAxisUp;
	bool m_Transparency;
	bool m_LockAccum;
	bool m_InRender;
	bool m_InFinalAccum;
	bool m_InsertPalette;
	bool m_ReclaimOnResize;
	volatile bool m_Abort;
	size_t m_SuperRasW;
	size_t m_SuperRasH;
	size_t m_SuperSize;
	size_t m_GutterWidth;
	size_t m_DensityFilterOffset;
	size_t m_NumChannels;
	size_t m_BytesPerChannel;
	size_t m_ThreadsToUse;
	size_t m_VibGamCount;
	size_t m_LastTemporalSample;
	size_t m_LastIter;
	double m_LastIterPercent;
	eProcessAction m_ProcessAction;
	eProcessState m_ProcessState;
	eInteractiveFilter m_InteractiveFilter;
	EmberStats m_Stats;
	RenderCallback* m_Callback;
	vector<size_t> m_SubBatch;
	vector<size_t> m_BadVals;
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> m_Rand;
	auto_ptr<tbb::task_group> m_TaskGroup;
	CriticalSection m_RenderingCs, m_AccumCs, m_FinalAccumCs, m_ResizeCs;
	Timing m_RenderTimer, m_IterTimer, m_ProgressTimer;
};
}
