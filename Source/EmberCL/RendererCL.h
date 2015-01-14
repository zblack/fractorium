#pragma once

#include "EmberCLPch.h"
#include "OpenCLWrapper.h"
#include "IterOpenCLKernelCreator.h"
#include "DEOpenCLKernelCreator.h"
#include "FinalAccumOpenCLKernelCreator.h"

/// <summary>
/// RendererCLBase and RendererCL classes.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Serves only as an interface for OpenCL specific rendering functions.
/// </summary>
class EMBERCL_API RendererCLBase
{
public:
	virtual ~RendererCLBase() __TBB_NOEXCEPT(false) { }
	virtual bool ReadFinal(byte* pixels) = 0;
	virtual bool ClearFinal() = 0;
};

/// <summary>
/// RendererCL is a derivation of the basic CPU renderer which
/// overrides various functions to render on the GPU using OpenCL.
/// Since this class derives from EmberReport and also contains an
/// OpenCLWrapper member which also derives from EmberReport, the
/// reporting functions are overridden to aggregate the errors from
/// both sources.
/// It does not support different types for T and bucketT, so it only has one template argument
/// and uses both for the base.
/// </summary>
template <typename T>
class EMBERCL_API RendererCL : public Renderer<T, T>, public RendererCLBase
{
using EmberNs::Renderer<T, T>::RendererBase::Abort;
using EmberNs::Renderer<T, T>::RendererBase::EarlyClip;
using EmberNs::Renderer<T, T>::RendererBase::Transparency;
using EmberNs::Renderer<T, T>::RendererBase::EnterResize;
using EmberNs::Renderer<T, T>::RendererBase::LeaveResize;
using EmberNs::Renderer<T, T>::RendererBase::FinalRasW;
using EmberNs::Renderer<T, T>::RendererBase::FinalRasH;
using EmberNs::Renderer<T, T>::RendererBase::SuperRasW;
using EmberNs::Renderer<T, T>::RendererBase::SuperRasH;
using EmberNs::Renderer<T, T>::RendererBase::SuperSize;
using EmberNs::Renderer<T, T>::RendererBase::BytesPerChannel;
using EmberNs::Renderer<T, T>::RendererBase::TemporalSamples;
using EmberNs::Renderer<T, T>::RendererBase::ItersPerTemporalSample;
using EmberNs::Renderer<T, T>::RendererBase::FuseCount;
using EmberNs::Renderer<T, T>::RendererBase::DensityFilterOffset;
using EmberNs::Renderer<T, T>::RendererBase::m_ProgressParameter;
using EmberNs::Renderer<T, T>::RendererBase::m_YAxisUp;
using EmberNs::Renderer<T, T>::RendererBase::m_LockAccum;
using EmberNs::Renderer<T, T>::RendererBase::m_Abort;
using EmberNs::Renderer<T, T>::RendererBase::m_NumChannels;
using EmberNs::Renderer<T, T>::RendererBase::m_LastIter;
using EmberNs::Renderer<T, T>::RendererBase::m_LastIterPercent;
using EmberNs::Renderer<T, T>::RendererBase::m_Stats;
using EmberNs::Renderer<T, T>::RendererBase::m_Callback;
using EmberNs::Renderer<T, T>::RendererBase::m_Rand;
using EmberNs::Renderer<T, T>::RendererBase::m_RenderTimer;
using EmberNs::Renderer<T, T>::RendererBase::m_IterTimer;
using EmberNs::Renderer<T, T>::RendererBase::m_ProgressTimer;
using EmberNs::Renderer<T, T>::RendererBase::EmberReport::m_ErrorReport;
using EmberNs::Renderer<T, T>::m_RotMat;
using EmberNs::Renderer<T, T>::m_Ember;
using EmberNs::Renderer<T, T>::CenterX;
using EmberNs::Renderer<T, T>::CenterY;
using EmberNs::Renderer<T, T>::K1;
using EmberNs::Renderer<T, T>::K2;
using EmberNs::Renderer<T, T>::Supersample;
using EmberNs::Renderer<T, T>::HighlightPower;
using EmberNs::Renderer<T, T>::HistBuckets;
using EmberNs::Renderer<T, T>::AccumulatorBuckets;
using EmberNs::Renderer<T, T>::GetDensityFilter;
using EmberNs::Renderer<T, T>::GetSpatialFilter;
using EmberNs::Renderer<T, T>::CoordMap;
using EmberNs::Renderer<T, T>::XformDistributions;
using EmberNs::Renderer<T, T>::XformDistributionsSize;

public:
	RendererCL(uint platform = 0, uint device = 0, bool shared = false, GLuint outputTexID = 0);
	~RendererCL() __TBB_NOEXCEPT(false);

	//Non-virtual member functions for OpenCL specific tasks.
	bool Init(uint platform, uint device, bool shared, GLuint outputTexID);
	bool SetOutputTexture(GLuint outputTexID);

	//Iters per kernel/block/grid.
	inline uint IterCountPerKernel() const;
	inline uint IterCountPerBlock() const;
	inline uint IterCountPerGrid() const;

	//Kernels per block.
	inline uint IterBlockKernelWidth() const;
	inline uint IterBlockKernelHeight() const;
	inline uint IterBlockKernelCount() const;

	//Kernels per grid.
	inline uint IterGridKernelWidth() const;
	inline uint IterGridKernelHeight() const;
	inline uint IterGridKernelCount() const;

	//Blocks per grid.
	inline uint IterGridBlockWidth() const;
	inline uint IterGridBlockHeight() const;
	inline uint IterGridBlockCount() const;

	uint PlatformIndex();
	uint DeviceIndex();
	bool ReadHist();
	bool ReadAccum();
	bool ReadPoints(vector<PointCL<T>>& vec);
	bool ClearHist();
	bool ClearAccum();
	bool WritePoints(vector<PointCL<T>>& vec);
#ifdef TEST_CL
	bool WriteRandomPoints();
#endif
	string IterKernel();

	//Virtual functions overridden from RendererCLBase.
	virtual bool ReadFinal(byte* pixels);
	virtual bool ClearFinal();

	//Public virtual functions overridden from Renderer or RendererBase.
	virtual size_t MemoryAvailable() override;
	virtual bool Ok() const override;
	virtual void NumChannels(size_t numChannels) override;
	virtual void DumpErrorReport() override;
	virtual void ClearErrorReport() override;
	virtual size_t SubBatchSize() const override;
	virtual size_t ThreadCount() const override;
	virtual bool CreateDEFilter(bool& newAlloc) override;
	virtual bool CreateSpatialFilter(bool& newAlloc) override;
	virtual eRendererType RendererType() const override;
	virtual string ErrorReportString() override;
	virtual vector<string> ErrorReport() override;
	virtual bool RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec) override;

#ifndef TEST_CL
protected:
#endif
	//Protected virtual functions overridden from Renderer.
	virtual void MakeDmap(T colorScalar) override;
	virtual bool Alloc() override;
	virtual bool ResetBuckets(bool resetHist = true, bool resetAccum = true) override;
	virtual eRenderStatus LogScaleDensityFilter() override;
	virtual eRenderStatus GaussianDensityFilter() override;
	virtual eRenderStatus AccumulatorToFinalImage(byte* pixels, size_t finalOffset) override;
	virtual EmberStats Iterate(size_t iterCount, size_t temporalSample) override;

private:
	//Private functions for making and running OpenCL programs.
	bool BuildIterProgramForEmber(bool doAccum = true);
	bool RunIter(size_t iterCount, size_t temporalSample, size_t& itersRan);
	eRenderStatus RunLogScaleFilter();
	eRenderStatus RunDensityFilter();
	eRenderStatus RunFinalAccum();
	bool ClearBuffer(const string& bufferName, uint width, uint height, uint elementSize);
	bool RunDensityFilterPrivate(uint kernelIndex, uint gridW, uint gridH, uint blockW, uint blockH, uint chunkSizeW, uint chunkSizeH, uint chunkW, uint chunkH);
	int MakeAndGetDensityFilterProgram(size_t ss, uint filterWidth);
	int MakeAndGetFinalAccumProgram(T& alphaBase, T& alphaScale);
	int MakeAndGetGammaCorrectionProgram();
	void FillSeeds();

	//Private functions passing data to OpenCL programs.
	DensityFilterCL<T> ConvertDensityFilter();
	SpatialFilterCL<T> ConvertSpatialFilter();
	void ConvertEmber(Ember<T>& ember, EmberCL<T>& emberCL, vector<XformCL<T>>& xformsCL);
	static CarToRasCL<T> ConvertCarToRas(const CarToRas<T>& carToRas);

	bool m_Init;
	bool m_NVidia;
	bool m_DoublePrecision;
	uint m_IterCountPerKernel;
	uint m_IterBlocksWide, m_IterBlockWidth;
	uint m_IterBlocksHigh, m_IterBlockHeight;
	uint m_MaxDEBlockSizeW;
	uint m_MaxDEBlockSizeH;
	uint m_WarpSize;
	size_t m_Calls;

	//Buffer names.
	string m_EmberBufferName;
	string m_XformsBufferName;
	string m_ParVarsBufferName;
	string m_SeedsBufferName;
	string m_DistBufferName;
	string m_CarToRasBufferName;
	string m_DEFilterParamsBufferName;
	string m_SpatialFilterParamsBufferName;
	string m_DECoefsBufferName;
	string m_DEWidthsBufferName;
	string m_DECoefIndicesBufferName;
	string m_SpatialFilterCoefsBufferName;
	string m_HistBufferName;
	string m_AccumBufferName;
	string m_FinalImageName;
	string m_PointsBufferName;

	//Kernels.
	string m_IterKernel;

	OpenCLWrapper m_Wrapper;
	cl::ImageFormat m_PaletteFormat;
	cl::ImageFormat m_FinalFormat;
	cl::Image2D m_Palette;
	IMAGEGL2D m_AccumImage;
	GLuint m_OutputTexID;
	EmberCL<T> m_EmberCL;
	vector<XformCL<T>> m_XformsCL;
	vector<glm::highp_uvec2> m_Seeds;
	Palette<float> m_DmapCL;//Used instead of the base class' m_Dmap because OpenCL only supports float textures.
	CarToRasCL<T> m_CarToRasCL;
	DensityFilterCL<T> m_DensityFilterCL;
	SpatialFilterCL<T> m_SpatialFilterCL;
	IterOpenCLKernelCreator<T> m_IterOpenCLKernelCreator;
	DEOpenCLKernelCreator<T> m_DEOpenCLKernelCreator;
	FinalAccumOpenCLKernelCreator<T> m_FinalAccumOpenCLKernelCreator;
	pair<string, vector<T>> m_Params;
	Ember<T> m_LastBuiltEmber;
};
}
