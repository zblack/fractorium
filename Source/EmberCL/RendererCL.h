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
	virtual bool ReadFinal(unsigned char* pixels) = 0;
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
class EMBERCL_API RendererCL : public RendererCLBase, public Renderer<T, T>
{
public:
	RendererCL(unsigned int platform = 0, unsigned int device = 0, bool shared = false, GLuint outputTexID = 0);
	~RendererCL();

	//Non-virtual member functions for OpenCL specific tasks.
	bool Init(unsigned int platform, unsigned int device, bool shared, GLuint outputTexID);
	bool SetOutputTexture(GLuint outputTexID);
	inline unsigned int IterCountPerKernel();
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
	bool ClearHist();
	bool ClearAccum();
	bool WritePoints(vector<PointCL<T>>& vec);
	string IterKernel();

	//Virtual functions overridden from RendererCLBase.
	virtual bool ReadFinal(unsigned char* pixels);
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

#ifndef TEST_CL
protected:
#endif
	//Protected virtual functions overridden from Renderer.
	virtual void MakeDmap(T colorScalar) override;
	virtual bool Alloc() override;
	virtual bool ResetBuckets(bool resetHist = true, bool resetAccum = true) override;
	virtual eRenderStatus LogScaleDensityFilter() override;
	virtual eRenderStatus GaussianDensityFilter() override;
	virtual eRenderStatus AccumulatorToFinalImage(unsigned char* pixels, size_t finalOffset) override;
	virtual EmberStats Iterate(size_t iterCount, size_t pass, size_t temporalSample) override;

private:
	//Private functions for making and running OpenCL programs.
	bool BuildIterProgramForEmber(bool doAccum = true);
	bool RunIter(size_t iterCount, size_t pass, size_t temporalSample, size_t& itersRan);
	eRenderStatus RunLogScaleFilter();
	eRenderStatus RunDensityFilter();
	eRenderStatus RunFinalAccum();
	bool ClearBuffer(const string& bufferName, unsigned int width, unsigned int height, unsigned int elementSize);
	bool RunDensityFilterPrivate(unsigned int kernelIndex, unsigned int gridW, unsigned int gridH, unsigned int blockW, unsigned int blockH, unsigned int chunkSizeW, unsigned int chunkSizeH, unsigned int rowParity, unsigned int colParity);
	int MakeAndGetDensityFilterProgram(size_t ss, unsigned int filterWidth);
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
	unsigned int m_IterCountPerKernel;
	unsigned int m_IterBlocksWide, m_IterBlockWidth;
	unsigned int m_IterBlocksHigh, m_IterBlockHeight;
	unsigned int m_MaxDEBlockSizeW;
	unsigned int m_MaxDEBlockSizeH;
	unsigned int m_WarpSize;
	size_t m_Calls;

	//Buffer names.
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

	//Kernels.
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