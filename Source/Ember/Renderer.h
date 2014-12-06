#pragma once

#include "RendererBase.h"
#include "Iterator.h"
#include "SpatialFilter.h"
#include "TemporalFilter.h"
#include "Interpolate.h"
#include "CarToRas.h"
#include "EmberToXml.h"

/// <summary>
/// Renderer.
/// </summary>

namespace EmberNs
{
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
/// that roughly matches what flam3 did. However they can be overridden in derived classes
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
//using EmberReport::m_ErrorReport;
public:
	Renderer();
	virtual ~Renderer();

	//Non-virtual processing functions.
	void AddEmber(Ember<T>& ember);
	bool AssignIterator();

	//Virtual processing functions overriden from RendererBase.
	virtual void ComputeBounds() override;
	virtual void ComputeCamera() override;
	virtual void SetEmber(Ember<T>& ember, eProcessAction action = FULL_RENDER) override;
	virtual void SetEmber(vector<Ember<T>>& embers) override;
	virtual bool CreateDEFilter(bool& newAlloc) override;
	virtual bool CreateSpatialFilter(bool& newAlloc) override;
	virtual bool CreateTemporalFilter(bool& newAlloc) override;
	virtual size_t HistBucketSize() const override { return sizeof(glm::detail::tvec4<bucketT, glm::defaultp>); }
	virtual eRenderStatus Run(vector<byte>& finalImage, double time = 0, size_t subBatchCountOverride = 0, bool forceOutput = false, size_t finalOffset = 0) override;
	virtual EmberImageComments ImageComments(EmberStats& stats, size_t printEditDepth = 0, bool intPalette = false, bool hexPalette = true) override;

protected:
	//New virtual functions to be overridden in derived renderers that use the GPU, but not accessed outside.
	virtual void MakeDmap(T colorScalar);
	virtual bool Alloc();
	virtual bool ResetBuckets(bool resetHist = true, bool resetAccum = true);
	virtual eRenderStatus LogScaleDensityFilter();
	virtual eRenderStatus GaussianDensityFilter();
	virtual eRenderStatus AccumulatorToFinalImage(vector<byte>& pixels, size_t finalOffset);
	virtual eRenderStatus AccumulatorToFinalImage(byte* pixels, size_t finalOffset);
	virtual EmberStats Iterate(size_t iterCount, size_t temporalSample);

public:
	//Non-virtual render properties, getters and setters.
	inline T PixelAspectRatio() const;
	void PixelAspectRatio(T pixelAspectRatio);

	//Non-virtual renderer properties, getters only.
	inline T                            Scale()                         const;
	inline T                            PixelsPerUnitX()                const;
	inline T                            PixelsPerUnitY()                const;
	inline T                            K1()                            const;
	inline T                            K2()                            const;
	inline const CarToRas<T>*           CoordMap()                      const;
	inline glm::detail::tvec4<bucketT, glm::defaultp>* HistBuckets();
	inline glm::detail::tvec4<bucketT, glm::defaultp>* AccumulatorBuckets();
	inline SpatialFilter<T>*            GetSpatialFilter();
	inline TemporalFilter<T>*           GetTemporalFilter();

	//Virtual renderer properties overridden from RendererBase, getters only.
	virtual double ScaledQuality()				   const override;
	virtual double LowerLeftX(bool  gutter = true) const override;
	virtual double LowerLeftY(bool  gutter = true) const override;
	virtual double UpperRightX(bool gutter = true) const override;
	virtual double UpperRightY(bool gutter = true) const override;
	virtual DensityFilterBase* GetDensityFilter()        override;

	//Non-virtual ember wrappers, getters only.
	inline bool              XaosPresent()		   const;
	inline size_t			 Supersample()         const;
	inline size_t			 PaletteIndex()        const;
	inline T                 Time()                const;
	inline T                 Quality()             const;
	inline T                 SpatialFilterRadius() const;
	inline T                 PixelsPerUnit()       const;
	inline T                 Zoom()                const;
	inline T                 CenterX()             const;
	inline T                 CenterY()             const;
	inline T                 Rotate()              const;
	inline T                 Hue()                 const;
	inline T                 Brightness()          const;
	inline T                 Contrast()            const;
	inline T                 Gamma()               const;
	inline T                 Vibrancy()            const;
	inline T                 GammaThresh()         const;
	inline T                 HighlightPower()      const;
	inline Color<T>			 Background()          const;
	inline const Xform<T>*   Xforms()              const;
	inline Xform<T>*         NonConstXforms();
	inline size_t			 XformCount()          const;
	inline const Xform<T>*   FinalXform()          const;
	inline Xform<T>*         NonConstFinalXform();
	inline bool              UseFinalXform()       const;
	inline const Palette<T>* GetPalette()          const;
	inline ePaletteMode      PaletteMode()         const;

	//Virtual ember wrappers overridden from RendererBase, getters only.
	virtual size_t TemporalSamples() const override;
	virtual size_t FinalRasW()       const override;
	virtual size_t FinalRasH()       const override;
	virtual size_t SubBatchSize()    const override;
	virtual size_t FuseCount()		 const override;

	//Non-virtual iterator wrappers.
	const byte* XformDistributions()		 const;
	const size_t		 XformDistributionsSize()    const;
	Point<T>*			 Samples(size_t threadIndex) const;

protected:
	//Non-virtual functions that might be needed by a derived class.
	void PrepFinalAccumVals(Color<T>& background, T& g, T& linRange, T& vibrancy);

	private:
	//Miscellaneous non-virtual functions used only in this class.
	void Accumulate(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, Point<T>* samples, size_t sampleCount, const Palette<bucketT>* palette);
	/*inline*/ void AddToAccum(const glm::detail::tvec4<bucketT, glm::defaultp>& bucket, intmax_t i, intmax_t ii, intmax_t j, intmax_t jj);
	template <typename accumT> void GammaCorrection(glm::detail::tvec4<bucketT, glm::defaultp>& bucket, Color<T>& background, T g, T linRange, T vibrancy, bool doAlpha, bool scale, accumT* correctedChannels);

protected:
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
	T m_ScaledQuality;
	Color<T> m_Background;
	Affine2D<T> m_RotMat;
	Ember<T> m_Ember;
	Ember<T> m_TempEmber;
	Ember<T> m_LastEmber;
	vector<Ember<T>> m_Embers;
	CarToRas<T> m_CarToRas;
	Iterator<T>* m_Iterator;
	unique_ptr<StandardIterator<T>> m_StandardIterator;
	unique_ptr<XaosIterator<T>> m_XaosIterator;
	Palette<bucketT> m_Dmap;
	vector<glm::detail::tvec4<bucketT, glm::defaultp>> m_HistBuckets;
	vector<glm::detail::tvec4<bucketT, glm::defaultp>> m_AccumulatorBuckets;
	unique_ptr<SpatialFilter<T>> m_SpatialFilter;
	unique_ptr<TemporalFilter<T>> m_TemporalFilter;
	unique_ptr<DensityFilter<T>> m_DensityFilter;
	vector<vector<Point<T>>> m_Samples;
	EmberToXml<T> m_EmberToXml;
};

//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
//template EMBER_API class Renderer<float, float>;

//#ifdef DO_DOUBLE
//	template EMBER_API class Renderer<double, double>;
//#endif
}
