#pragma once

#include "Ember.h"
#include "Iterator.h"
#include "Utils.h"
#include "SpatialFilter.h"
#include "DensityFilter.h"
#include "TemporalFilter.h"
#include "Interpolate.h"
#include "CarToRas.h"
#include "EmberToXml.h"

/// <summary>
/// Renderer, RenderCallback and EmberStats classes.
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

	uint64_t m_Iters, m_Badvals;
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
public:
	RendererBase() { }
	virtual ~RendererBase() { }
	virtual void SetEmber(Ember<float>& ember, eProcessAction action = FULL_RENDER) { }
	virtual void SetEmber(vector<Ember<float>>& embers) { }
	virtual void SetEmber(Ember<double>& ember, eProcessAction action = FULL_RENDER) { }
	virtual void SetEmber(vector<Ember<double>>& embers) { }
	virtual void Callback(RenderCallback* callback) { }
	virtual bool CreateSpatialFilter(bool& newAlloc) { return false; }
	virtual bool CreateTemporalFilter(bool& newAlloc) { return false; }
	virtual void ComputeBounds() { }
	virtual bool Ok() const { return false; }
	virtual void Reset() { }
	virtual void EnterRender() { }
	virtual void LeaveRender() { }
	virtual void EnterFinalAccum() { }
	virtual void LeaveFinalAccum() { }
	virtual void EnterResize() { }
	virtual void LeaveResize() { }
	virtual void Abort() { }
	virtual bool Aborted() { return false; }
	virtual bool InRender() { return false; }
	virtual bool InFinalAccum() { return false; }
	virtual unsigned int NumChannels() const { return 0; }
	virtual void NumChannels(unsigned int numChannels) { }
	virtual eRendererType RendererType() const { return CPU_RENDERER; }
	virtual void ReclaimOnResize(bool reclaimOnResize) { }
	virtual bool EarlyClip() const { return false; }
	virtual void EarlyClip(bool earlyClip) { }
	virtual bool YAxisUp() const { return false; }
	virtual void YAxisUp(bool yup) { }
	virtual void ThreadCount(unsigned int threads, const char* seedString = NULL) { }
	virtual void Transparency(bool transparency) { }
	virtual void InteractiveFilter(eInteractiveFilter filter) { }
	virtual unsigned int FinalRasW()       const { return 0; }
	virtual unsigned int FinalRasH()       const { return 0; }
	virtual unsigned int SuperRasW()       const { return 0; }
	virtual unsigned int SuperRasH()       const { return 0; }
	virtual unsigned int FinalBufferSize() const { return 0; }
	virtual unsigned int GutterWidth()     const { return 0; }
	virtual double ScaledQuality() const { return 0; }
	virtual double LowerLeftX(bool gutter = true)  const { return 0; }
	virtual double LowerLeftY(bool gutter = true)  const { return 0; }
	virtual double UpperRightX(bool gutter = true) const { return 0; }
	virtual double UpperRightY(bool gutter = true) const { return 0; }
	virtual uint64_t MemoryRequired(bool includeFinal) { return 0; }
	virtual uint64_t MemoryAvailable() { return 0; }
	virtual bool PrepFinalAccumVector(vector<unsigned char>& pixels) { return false; }
	virtual eProcessState  ProcessState()  const { return NONE; }
	virtual eProcessAction ProcessAction() const { return NOTHING; }
	virtual EmberStats     Stats() 		   const { EmberStats stats; return stats; }
	virtual eRenderStatus Run(vector<unsigned char>& finalImage, double time = 0, unsigned int subBatchCountOverride = 0, bool forceOutput = false, size_t finalOffset = 0) { return RENDER_ERROR; }
	virtual EmberImageComments ImageComments(unsigned int printEditDepth = 0, bool intPalette = false, bool hexPalette = true) { EmberImageComments comments; return comments; }
	virtual DensityFilterBase* GetDensityFilter() { return NULL; }
};

/// <summary>
/// Renderer is the main class where all of the execution takes place.
/// It is intended that the program have one instance of it that it
/// keeps around for its duration. After a user sets up an ember, it's passed
/// in to be rendered.
/// This class derives from EmberReport, so the caller is able
/// to retrieve a text dump of error information if any errors occur.
/// The final image output vector is also passed in because the calling code has more
/// use for it than this class does.
/// Several functions are made virtual and have a default CPU-based implementation
/// that roughly matches what flam3 did. However they can be overriden in derived classes
/// to provide alternative rendering implementations, such as using the GPU.
/// Since this is a templated class, it's supposed to be entirely implemented in this .h file.
/// However, VC++ 2010 has very crippled support for lambdas, which Renderer makes use of.
/// If too many lambdas are used in a .h file, it will crash the compiler when another library
/// tries to link to it. To work around the bug, only declarations are here and all implementations
/// are in the .cpp file. It's unclear at what point it starts/stops working. But it seems that once
/// enough code is placed in the .h file, the compiler crashes. So for the sake of consistency, everything
/// is moved to the .cpp, even simple getters. One drawback however, is that the class must be
/// explicitly exported at the bottom of the file.
/// Also, despite explicitly doing this, the compiler throws a C4661 warning
/// for every single function in this class, saying it can't find the implementation. This warning
/// can be safely ignored.
/// Template argument T expected to be float or double.
/// Template argument bucketT was originally used to experiment with different types for the histogram, however
/// the only types that work are float and double, so it's useless and should always match what T is.
/// Mismatched types between T and bucketT are undefined.
/// </summary>
template <typename T, typename bucketT>
class EMBER_API Renderer : public RendererBase
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void ComputeBounds();
	void ComputeCamera();
	void ChangeVal(std::function<void (void)> func, eProcessAction action);
	virtual void SetEmber(Ember<T>& ember, eProcessAction action = FULL_RENDER);
	virtual void SetEmber(vector<Ember<T>>& embers);
	void AddEmber(Ember<T>& ember);
	bool CreateTemporalFilter(bool& newAlloc);
	bool AssignIterator();
	virtual bool PrepFinalAccumVector(vector<unsigned char>& pixels);
	virtual eRenderStatus Run(vector<unsigned char>& finalImage, double time = 0, unsigned int subBatchCountOverride = 0, bool forceOutput = false, size_t finalOffset = 0);
	virtual EmberImageComments ImageComments(unsigned int printEditDepth = 0, bool intPalette = false, bool hexPalette = true);
	virtual uint64_t MemoryRequired(bool includeFinal);

	//Virtual functions to be overriden in derived renderers that use the GPU.
	virtual unsigned __int64 MemoryAvailable();
	virtual void Reset();
	virtual bool Ok() const;
	virtual bool CreateDEFilter(bool& newAlloc);
	virtual bool CreateSpatialFilter(bool& newAlloc);
	virtual unsigned int SubBatchSize() const;
	virtual void SubBatchSize(unsigned int sbs);
	virtual unsigned int NumChannels() const;
	virtual void NumChannels(unsigned int numChannels);
	virtual eRendererType RendererType() const;
	virtual unsigned int ThreadCount() const;
	virtual void ThreadCount(unsigned int threads, const char* seedString = NULL);
	virtual void Callback(RenderCallback* callback);

protected:
	//Virtual functions to be overriden in derived renderers that use the GPU, but not accessed outside.
	virtual void MakeDmap(T colorScalar);
	virtual bool Alloc();
	virtual bool ResetBuckets(bool resetHist = true, bool resetAccum = true);
	virtual eRenderStatus LogScaleDensityFilter();
	virtual eRenderStatus GaussianDensityFilter();
	virtual eRenderStatus AccumulatorToFinalImage(vector<unsigned char>& pixels, size_t finalOffset);
	virtual eRenderStatus AccumulatorToFinalImage(unsigned char* pixels, size_t finalOffset);
	virtual EmberStats Iterate(uint64_t iterCount, unsigned int pass, unsigned int temporalSample);

public:
	//Accessors for render properties.
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> RandVec();
	bool RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec);

	inline bool LockAccum() const;
	void LockAccum(bool lockAccum);

	virtual bool EarlyClip() const;
	virtual void EarlyClip(bool earlyClip);

	virtual bool YAxisUp() const;
	virtual void YAxisUp(bool yup);

	inline bool InsertPalette() const;
	void InsertPalette(bool insertPalette);

	inline bool ReclaimOnResize() const;
	virtual void ReclaimOnResize(bool reclaimOnResize);

	inline bool Transparency() const;
	virtual void Transparency(bool transparency);

	inline unsigned int BytesPerChannel() const; 
	void BytesPerChannel(unsigned int bytesPerChannel);

	inline T PixelAspectRatio() const;
	void PixelAspectRatio(T pixelAspectRatio);

	inline eInteractiveFilter InteractiveFilter() const;
	virtual void InteractiveFilter(eInteractiveFilter filter);
	
	//Threading control.
	virtual void EnterRender();
	virtual void LeaveRender();
	virtual void EnterFinalAccum();
	virtual void LeaveFinalAccum();
	virtual void EnterResize();
	virtual void LeaveResize();
	virtual void Abort();
	virtual bool Aborted();
	virtual bool InRender();
	virtual bool InFinalAccum();

	//Renderer properties, getters only.
	virtual unsigned int                SuperRasW()                     const;
	virtual unsigned int                SuperRasH()                     const;
	inline unsigned int                 SuperSize()                     const;
	virtual unsigned int                FinalBufferSize()               const;
	inline unsigned int                 FinalRowSize()                  const;
	unsigned int                 		FinalDimensions()               const;
	inline unsigned int                 PixelSize()                     const;
	virtual unsigned int                GutterWidth()                   const;
	inline unsigned int                 DensityFilterOffset()           const;
	virtual double                      ScaledQuality()                 const;
	inline T                            Scale()                         const;
	inline T                            PixelsPerUnitX()                const;
	inline T                            PixelsPerUnitY()                const;
	virtual double                      LowerLeftX(bool gutter = true)  const;
	virtual double                      LowerLeftY(bool gutter = true)  const;
	virtual double                      UpperRightX(bool gutter = true) const;
	virtual double                      UpperRightY(bool gutter = true) const;
	inline T                            K1()                            const;
	inline T                            K2()                            const;
	inline uint64_t             		TotalIterCount()	            const;
	inline uint64_t	            		ItersPerTemporalSample()        const;
	virtual eProcessState               ProcessState()			        const;
	virtual eProcessAction              ProcessAction()			        const;
	virtual EmberStats                  Stats() 				        const;
	inline const CarToRas<T>*           CoordMap()                      const;
	inline glm::detail::tvec4<bucketT, glm::defaultp>* HistBuckets();
	inline glm::detail::tvec4<bucketT, glm::defaultp>* AccumulatorBuckets();
	inline SpatialFilter<T>*            GetSpatialFilter();
	inline TemporalFilter<T>*           GetTemporalFilter();
	virtual DensityFilter<T>*           GetDensityFilter();
	
	//Ember wrappers, getters only.
	inline bool                 XaosPresent();
	unsigned int 				FinalRasW()           const;
	unsigned int 				FinalRasH()           const;
	inline unsigned int         Supersample()         const;
	inline unsigned int         Passes()              const;
	inline unsigned int         TemporalSamples()     const;
	inline unsigned int         PaletteIndex()        const;
	inline T                    Time()                const;
	inline T                    Quality()             const;
	inline T                    SpatialFilterRadius() const;
	inline T                    PixelsPerUnit()       const;
	inline T                    Zoom()                const;
	inline T                    CenterX()             const;
	inline T                    CenterY()             const;
	inline T                    Rotate()              const;
	inline T                    Hue()                 const;
	inline T                    Brightness()          const;
	inline T                    Contrast()            const;
	inline T                    Gamma()               const;
	inline T                    Vibrancy()            const;
	inline T                    GammaThresh()         const;
	inline T                    HighlightPower()      const;
	inline Color<T>				Background()          const;
	inline const Xform<T>*      Xforms()              const;
	inline Xform<T>*            NonConstXforms();
	inline unsigned int         XformCount()          const;
	inline const Xform<T>*      FinalXform()          const;
	inline Xform<T>*            NonConstFinalXform();
	inline bool                 UseFinalXform()       const;
	inline const Palette<T>*    GetPalette()          const;
	inline ePaletteMode         PaletteMode()         const;
	
	//Iterator wrappers.
	const unsigned char* XformDistributions()			   const;
	const unsigned int   XformDistributionsSize()		   const;
	Point<T>*			 Samples(unsigned int threadIndex) const;

	void* m_ProgressParameter;

protected:
	//Non-virtual functions that might be needed by a derived class.
	void PrepFinalAccumVals(Color<T>& background, T& g, T& linRange, T& vibrancy);

private:
	//Miscellaneous functions used only in this class.
	void Accumulate(Point<T>* samples, unsigned int sampleCount, const Palette<bucketT>* palette);
	inline void AddToAccum(const glm::detail::tvec4<bucketT, glm::defaultp>& bucket, int i, int ii, int j, int jj);
	template <typename accumT> void GammaCorrection(glm::detail::tvec4<bucketT, glm::defaultp>& bucket, Color<T>& background, T g, T linRange, T vibrancy, bool doAlpha, bool scale, accumT* correctedChannels);

protected:
	bool m_EarlyClip;
	bool m_YAxisUp;
	bool m_Transparency;
	unsigned int m_SuperRasW;
	unsigned int m_SuperRasH;
	unsigned int m_SuperSize;
	unsigned int m_GutterWidth;
	unsigned int m_DensityFilterOffset;
	unsigned int m_NumChannels;
	unsigned int m_BytesPerChannel;
	unsigned int m_SubBatchSize;
	unsigned int m_ThreadsToUse;
	T m_ScaledQuality;
	T m_Scale;
	T m_PixelsPerUnitX;
	T m_PixelsPerUnitY;
	T m_PixelAspectRatio;
	T m_LowerLeftX;
	T m_LowerLeftY;
	T m_UpperRightX;
	T m_UpperRightY;
	T m_K1;
	T m_K2;
	T m_Vibrancy;//Accumulate these after each temporal sample.
	T m_Gamma;
	Color<T> m_Background;
	Affine2D<T> m_RotMat;

	volatile bool m_Abort;
	bool m_LockAccum;
	bool m_InRender;
	bool m_InFinalAccum;
	bool m_InsertPalette;
	bool m_ReclaimOnResize;
	unsigned int m_VibGamCount;
	unsigned int m_LastPass;
	unsigned int m_LastTemporalSample;
	uint64_t m_LastIter;
	double m_LastIterPercent;
	eProcessAction m_ProcessAction;
	eProcessState m_ProcessState;
	eInteractiveFilter m_InteractiveFilter;
	EmberStats m_Stats;
	Ember<T> m_Ember;
	Ember<T> m_TempEmber;
	Ember<T> m_LastEmber;
	vector<Ember<T>> m_Embers;
	CarToRas<T> m_CarToRas;
	RenderCallback* m_Callback;
	Iterator<T>* m_Iterator;
	auto_ptr<StandardIterator<T>> m_StandardIterator;
	auto_ptr<XaosIterator<T>> m_XaosIterator;
	Palette<bucketT> m_Dmap;
	vector<glm::detail::tvec4<bucketT, glm::defaultp>> m_HistBuckets;
	vector<glm::detail::tvec4<bucketT, glm::defaultp>> m_AccumulatorBuckets;
	auto_ptr<SpatialFilter<T>> m_SpatialFilter;
	auto_ptr<TemporalFilter<T>> m_TemporalFilter;
	auto_ptr<DensityFilter<T>> m_DensityFilter;
	vector<vector<Point<T>>> m_Samples;
	vector<uint64_t> m_SubBatch;
	vector<uint64_t> m_BadVals;
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> m_Rand;
	tbb::task_group m_TaskGroup;
	CriticalSection m_RenderingCs, m_AccumCs, m_FinalAccumCs, m_ResizeCs;
	Timing m_RenderTimer, m_IterTimer, m_ProgressTimer;
	EmberToXml<T> m_EmberToXml;
};

//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
template EMBER_API class Renderer<float, float>;

#ifdef DO_DOUBLE
	template EMBER_API class Renderer<double, double>;
#endif
}