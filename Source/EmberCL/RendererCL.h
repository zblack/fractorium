#pragma once

#include "EmberCLPch.h"
#include "OpenCLWrapper.h"
#include "IterOpenCLKernelCreator.h"
#include "DEOpenCLKernelCreator.h"
#include "FinalAccumOpenCLKernelCreator.h"

/// <summary>
/// RendererCL class.
/// </summary>

namespace EmberCLns
{
class EMBERCL_API RendererCLBase
{
public:
	virtual bool ReadFinal(unsigned char* pixels) { return false; }
	virtual bool ClearFinal() { return false; }
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
class EMBERCL_API RendererCL : public RendererCLBase, public Renderer<T, T>
{
public:
	RendererCL(unsigned int platform = 0, unsigned int device = 0, bool shared = false, GLuint outputTexID = 0);
	~RendererCL();

	//Ordinary member functions for OpenCL specific tasks.
	bool Init(unsigned int platform, unsigned int device, bool shared, GLuint outputTexID);
	inline unsigned int IterBlocksWide();
	inline unsigned int IterBlocksHigh();
	inline unsigned int IterBlockWidth();
	inline unsigned int IterBlockHeight();
	inline unsigned int IterGridWidth();
	inline unsigned int IterGridHeight();
	inline unsigned int TotalIterKernelCount();
	unsigned int PlatformIndex();
	unsigned int DeviceIndex();
	bool ReadHist();
	bool ReadAccum();
	bool ReadPoints(vector<PointCL<T>>& vec);
	virtual bool ReadFinal(unsigned char* pixels);
	virtual bool ClearFinal();
	bool ClearHist();
	bool ClearAccum();
	bool WritePoints(vector<PointCL<T>>& vec);
	string IterKernel();

	//Public virtual functions overriden from Renderer.
	virtual unsigned __int64 MemoryAvailable();
	virtual bool Ok() const;
	virtual void NumChannels(unsigned int numChannels);
	virtual void DumpErrorReport();
	virtual void ClearErrorReport();
	virtual unsigned int SubBatchSize() const;
	virtual unsigned int ThreadCount() const;
	virtual void ThreadCount(unsigned int threads, const char* seedString = NULL);
	virtual bool CreateDEFilter(bool& newAlloc);
	virtual bool CreateSpatialFilter(bool& newAlloc);
	virtual eRendererType RendererType() const;
	virtual string ErrorReportString();
	virtual vector<string> ErrorReport();

#ifndef TEST_CL
protected:
#endif
	//Protected virtual functions overriden from Renderer.
	virtual void MakeDmap(T colorScalar);
	virtual bool Alloc();
	virtual bool ResetBuckets(bool resetHist = true, bool resetAccum = true);
	virtual eRenderStatus LogScaleDensityFilter();
	virtual eRenderStatus GaussianDensityFilter();
	virtual eRenderStatus AccumulatorToFinalImage(unsigned char* pixels, size_t finalOffset);
	virtual EmberStats Iterate(unsigned __int64 iterCount, unsigned int pass, unsigned int temporalSample);

private:
	//Private functions for making and running OpenCL programs.
	bool BuildIterProgramForEmber(bool doAccum = true);
	bool RunIter(unsigned __int64 iterCount, unsigned int pass, unsigned int temporalSample, unsigned __int64& itersRan);
	eRenderStatus RunLogScaleFilter();
	eRenderStatus RunDensityFilter();
	eRenderStatus RunFinalAccum();
	bool ClearBuffer(string bufferName, unsigned int width, unsigned int height, unsigned int elementSize);
	bool RunDensityFilterPrivate(unsigned int kernelIndex, unsigned int gridW, unsigned int gridH, unsigned int blockW, unsigned int blockH, unsigned int chunkSizeW, unsigned int chunkSizeH, unsigned int rowParity, unsigned int colParity);
	int MakeAndGetDensityFilterProgram(unsigned int ss, unsigned int filterWidth);
	int MakeAndGetFinalAccumProgram(T& alphaBase, T& alphaScale);
	int MakeAndGetGammaCorrectionProgram();

	//Private functions passing data to OpenCL programs.
	DensityFilterCL<T> ConvertDensityFilter();
	SpatialFilterCL<T> ConvertSpatialFilter();
	EmberCL<T> ConvertEmber(Ember<T>& ember);
	static CarToRasCL<T> ConvertCarToRas(const CarToRas<T>& carToRas);

	bool m_Init;
	bool m_NVidia;
	bool m_DoublePrecision;
	unsigned int m_IterBlocksWide, m_IterBlockWidth;
	unsigned int m_IterBlocksHigh, m_IterBlockHeight;
	unsigned int m_MaxDEBlockSizeW;
	unsigned int m_MaxDEBlockSizeH;
	unsigned int m_WarpSize;
	unsigned int m_Calls;

	string m_EmberBufferName;
	string m_ParVarsBufferName;
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

	string m_IterKernel;
	
	OpenCLWrapper m_Wrapper;
	cl::ImageFormat m_PaletteFormat;
	cl::ImageFormat m_FinalFormat;
	cl::Image2D m_Palette;
	IMAGEGL2D m_AccumImage;
	GLuint m_OutputTexID;
	EmberCL<T> m_EmberCL;
	Palette<float> m_Dmap;//Used instead of the base class' m_Dmap because OpenCL only supports float textures.
	CarToRasCL<T> m_CarToRasCL;
	DensityFilterCL<T> m_DensityFilterCL;
	SpatialFilterCL<T> m_SpatialFilterCL;
	IterOpenCLKernelCreator<T> m_IterOpenCLKernelCreator;
	DEOpenCLKernelCreator<T> m_DEOpenCLKernelCreator;
	FinalAccumOpenCLKernelCreator<T> m_FinalAccumOpenCLKernelCreator;
	pair<string, vector<T>> m_Params;
	Ember<T> m_LastBuiltEmber;
};

template EMBERCL_API class RendererCL<float>;

#ifdef DO_DOUBLE
	template EMBERCL_API class RendererCL<double>;
#endif
}