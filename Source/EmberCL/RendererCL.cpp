#include "EmberCLPch.h"
#include "RendererCL.h"

namespace EmberCLns
{
/// <summary>
/// Constructor that inintializes various buffer names, block dimensions, image formats
/// and finally initializes OpenCL using the passed in parameters.
/// </summary>
/// <param name="platform">The index platform of the platform to use. Default: 0.</param>
/// <param name="device">The index device of the device to use. Default: 0.</param>
/// <param name="shared">True if shared with OpenGL, else false. Default: false.</param>
/// <param name="outputTexID">The texture ID of the shared OpenGL texture if shared. Default: 0.</param>
template <typename T>
RendererCL<T>::RendererCL(uint platform, uint device, bool shared, GLuint outputTexID)
{
	m_Init = false;
	m_NVidia = false;
	m_DoublePrecision = typeid(T) == typeid(double);
	m_NumChannels = 4;
	m_Calls = 0;

	//Buffer names.
	m_EmberBufferName               = "Ember";
	m_XformsBufferName				= "Xforms";
	m_ParVarsBufferName             = "ParVars";
	m_SeedsBufferName				= "Seeds";
	m_DistBufferName                = "Dist";
	m_CarToRasBufferName            = "CarToRas";
	m_DEFilterParamsBufferName      = "DEFilterParams";
	m_SpatialFilterParamsBufferName = "SpatialFilterParams";
	m_DECoefsBufferName             = "DECoefs";
	m_DEWidthsBufferName            = "DEWidths";
	m_DECoefIndicesBufferName		= "DECoefIndices";
	m_SpatialFilterCoefsBufferName  = "SpatialFilterCoefs";
	m_HistBufferName                = "Hist";
	m_AccumBufferName               = "Accum";
	m_FinalImageName                = "Final";
	m_PointsBufferName              = "Points";

	//It's critical that these numbers never change. They are
	//based on the cuburn model of each kernel launch containing
	//256 threads. 32 wide by 8 high. Everything done in the OpenCL
	//iteraion kernel depends on these dimensions.
	m_IterCountPerKernel = 256;
	m_IterBlockWidth = 32;
	m_IterBlockHeight = 8;
	m_IterBlocksWide = 64;
	m_IterBlocksHigh = 2;

	m_PaletteFormat.image_channel_order = CL_RGBA;
	m_PaletteFormat.image_channel_data_type = CL_FLOAT;
	m_FinalFormat.image_channel_order = CL_RGBA;
	m_FinalFormat.image_channel_data_type = CL_UNORM_INT8;//Change if this ever supports 2BPC outputs for PNG.

	FillSeeds();
	Init(platform, device, shared, outputTexID);//Init OpenCL upon construction and create programs that will not change.
}

/// <summary>
/// Virtual destructor.
/// </summary>
template <typename T>
RendererCL<T>::~RendererCL()
{
}

/// <summary>
/// Non-virtual member functions for OpenCL specific tasks.
/// </summary>

/// <summary>
/// Initialize OpenCL.
/// In addition to initializing, this function will create the zeroization program,
/// as well as the basic log scale filtering programs. This is done to ensure basic
/// compilation works. Further compilation will be done later for iteration, density filtering,
/// and final accumulation.
/// </summary>
/// <param name="platform">The index platform of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <param name="outputTexID">The texture ID of the shared OpenGL texture if shared</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::Init(uint platform, uint device, bool shared, GLuint outputTexID)
{
	//Timing t;
	bool b = true;
	m_OutputTexID = outputTexID;
	const char* loc = __FUNCTION__;

	if (!m_Wrapper.Ok() || PlatformIndex() != platform || DeviceIndex() != device)
	{
		m_Init = false;
		b = m_Wrapper.Init(platform, device, shared);
	}

	if (b && m_Wrapper.Ok() && !m_Init)
	{
		m_NVidia = ToLower(m_Wrapper.DeviceAndPlatformNames()).find_first_of("nvidia") != string::npos && m_Wrapper.LocalMemSize() > (32 * 1024);
		m_WarpSize = m_NVidia ? 32 : 64;
		m_IterOpenCLKernelCreator = IterOpenCLKernelCreator<T>(m_NVidia);
		m_DEOpenCLKernelCreator = DEOpenCLKernelCreator<T>(m_NVidia);

		string zeroizeProgram = m_IterOpenCLKernelCreator.ZeroizeKernel();
		string logAssignProgram = m_DEOpenCLKernelCreator.LogScaleAssignDEKernel();//Build a couple of simple programs to ensure OpenCL is working right.

		if (b && !(b = m_Wrapper.AddProgram(m_IterOpenCLKernelCreator.ZeroizeEntryPoint(),		  zeroizeProgram,	m_IterOpenCLKernelCreator.ZeroizeEntryPoint(),        m_DoublePrecision))) { m_ErrorReport.push_back(loc); }
		if (b && !(b = m_Wrapper.AddProgram(m_DEOpenCLKernelCreator.LogScaleAssignDEEntryPoint(), logAssignProgram, m_DEOpenCLKernelCreator.LogScaleAssignDEEntryPoint(), m_DoublePrecision))) { m_ErrorReport.push_back(loc); }
		if (b && !(b = m_Wrapper.AddAndWriteImage("Palette", CL_MEM_READ_ONLY, m_PaletteFormat, 256, 1, 0, nullptr))) { m_ErrorReport.push_back(loc); }
		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_SeedsBufferName, reinterpret_cast<void*>(m_Seeds.data()), SizeOf(m_Seeds)))) { m_ErrorReport.push_back(loc); }

		//This is the maximum box dimension for density filtering which consists of (blockSize  * blockSize) + (2 * filterWidth).
		//These blocks must be square, and ideally, 32x32.
		//Sadly, at the moment, Fermi runs out of resources at that block size because the DE filter function is so complex.
		//The next best block size seems to be 24x24.
		//AMD is further limited because of less local memory so these have to be 16 on AMD.
		m_MaxDEBlockSizeW = m_NVidia ? 32 : 16;//These *must* both be divisible by 16 or else pixels will go missing.
		m_MaxDEBlockSizeH = m_NVidia ? 32 : 16;
		m_Init = true;
		//t.Toc(loc);
	}

	return b;
}

/// <summary>
/// Set the shared output texture where final accumulation will be written to.
/// </summary>
/// <param name="outputTexID">The texture ID of the shared OpenGL texture if shared</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::SetOutputTexture(GLuint outputTexID)
{
	bool success = true;
	const char* loc = __FUNCTION__;

	if (!m_Wrapper.Ok())
		return false;

	m_OutputTexID = outputTexID;
	EnterResize();

	if (!m_Wrapper.AddAndWriteImage(m_FinalImageName, CL_MEM_WRITE_ONLY, m_FinalFormat, FinalRasW(), FinalRasH(), 0, nullptr, m_Wrapper.Shared(), m_OutputTexID))
	{
		m_ErrorReport.push_back(loc);
		success = false;
	}

	LeaveResize();
	return success;
}

/// <summary>
/// OpenCL property accessors, getters only.
/// </summary>

//Iters per kernel/block/grid.
template <typename T> uint RendererCL<T>::IterCountPerKernel() const { return m_IterCountPerKernel; }
template <typename T> uint RendererCL<T>::IterCountPerBlock()  const { return IterCountPerKernel() * IterBlockKernelCount(); }
template <typename T> uint RendererCL<T>::IterCountPerGrid()   const { return IterCountPerKernel() * IterGridKernelCount();  }

//Kernels per block.
template <typename T> uint RendererCL<T>::IterBlockKernelWidth()  const { return m_IterBlockWidth;								 }
template <typename T> uint RendererCL<T>::IterBlockKernelHeight() const { return m_IterBlockHeight;								 }
template <typename T> uint RendererCL<T>::IterBlockKernelCount()  const { return IterBlockKernelWidth() * IterBlockKernelHeight(); }

//Kernels per grid.
template <typename T> uint RendererCL<T>::IterGridKernelWidth()  const { return IterGridBlockWidth() * IterBlockKernelWidth();   }
template <typename T> uint RendererCL<T>::IterGridKernelHeight() const { return IterGridBlockHeight() * IterBlockKernelHeight(); }
template <typename T> uint RendererCL<T>::IterGridKernelCount()	 const { return IterGridKernelWidth() * IterGridKernelHeight();  }

//Blocks per grid.
template <typename T> uint RendererCL<T>::IterGridBlockWidth()  const { return m_IterBlocksWide;							   }
template <typename T> uint RendererCL<T>::IterGridBlockHeight() const { return m_IterBlocksHigh;							   }
template <typename T> uint RendererCL<T>::IterGridBlockCount()  const { return IterGridBlockWidth() * IterGridBlockHeight(); }

template <typename T> uint RendererCL<T>::PlatformIndex() { return m_Wrapper.PlatformIndex(); }
template <typename T> uint RendererCL<T>::DeviceIndex()   { return m_Wrapper.DeviceIndex();   }

/// <summary>
/// Read the histogram into the host side CPU buffer.
/// Used for debugging.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ReadHist()
{
	if (Renderer<T, T>::Alloc())//Allocate the memory to read into.
		return m_Wrapper.ReadBuffer(m_HistBufferName, reinterpret_cast<void*>(HistBuckets()), SuperSize() * sizeof(v4T));

	return false;
}

/// <summary>
/// Read the density filtering buffer into the host side CPU buffer.
/// Used for debugging.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ReadAccum()
{
	if (Renderer<T, T>::Alloc())//Allocate the memory to read into.
		return m_Wrapper.ReadBuffer(m_AccumBufferName, reinterpret_cast<void*>(AccumulatorBuckets()), SuperSize() * sizeof(v4T));

	return false;
}

/// <summary>
/// Read the temporary points buffer into a host side CPU buffer.
/// Used for debugging.
/// </summary>
/// <param name="vec">The host side buffer to read into</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ReadPoints(vector<PointCL<T>>& vec)
{
	vec.resize(IterGridKernelCount());//Allocate the memory to read into.

	if (vec.size() >= IterGridKernelCount())
		return m_Wrapper.ReadBuffer(m_PointsBufferName, reinterpret_cast<void*>(vec.data()), IterGridKernelCount() * sizeof(PointCL<T>));

	return false;
}

/// <summary>
/// Clear the histogram buffer with all zeroes.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ClearHist()
{
	return ClearBuffer(m_HistBufferName, uint(SuperRasW()), uint(SuperRasH()), sizeof(v4T));
}

/// <summary>
/// Clear the desnity filtering buffer with all zeroes.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ClearAccum()
{
	return ClearBuffer(m_AccumBufferName, uint(SuperRasW()), uint(SuperRasH()), sizeof(v4T));
}

/// <summary>
/// Write values from a host side CPU buffer into the temporary points buffer.
/// Used for debugging.
/// </summary>
/// <param name="vec">The host side buffer whose values to write</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::WritePoints(vector<PointCL<T>>& vec)
{
	return m_Wrapper.WriteBuffer(m_PointsBufferName, reinterpret_cast<void*>(vec.data()), SizeOf(vec));
}

#ifdef TEST_CL
template <typename T>
bool RendererCL<T>::WriteRandomPoints()
{
	size_t size = IterGridKernelCount();
	vector<PointCL<T>> vec(size);

	for (int i = 0; i < size; i++)
	{
		vec[i].m_X = m_Rand[0].Frand11<T>();
		vec[i].m_Y = m_Rand[0].Frand11<T>();
		vec[i].m_Z = 0;
		vec[i].m_ColorX = m_Rand[0].Frand01<T>();
		vec[i].m_LastXfUsed = 0;
	}

	return WritePoints(vec);
}
#endif

/// <summary>
/// Get the kernel string for the last built iter program.
/// </summary>
/// <returns>The string representation of the kernel for the last built iter program.</returns>
template <typename T>
string RendererCL<T>::IterKernel() { return m_IterKernel; }

/// <summary>
/// Virtual functions overridden from RendererCLBase.
/// </summary>

/// <summary>
/// Read the final image buffer buffer into the host side CPU buffer.
/// This must be called before saving the final output image to file.
/// </summary>
/// <param name="pixels">The host side buffer to read into</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ReadFinal(byte* pixels)
{
	if (pixels)
		return m_Wrapper.ReadImage(m_FinalImageName, FinalRasW(), FinalRasH(), 0, m_Wrapper.Shared(), pixels);

	return false;
}

/// <summary>
/// Clear the final image output buffer with all zeroes by copying a host side buffer.
/// Slow, but never used because the final output image is always completely overwritten.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ClearFinal()
{
	vector<byte> v;
	uint index = m_Wrapper.FindImageIndex(m_FinalImageName, m_Wrapper.Shared());

	if (this->PrepFinalAccumVector(v))
	{
		bool b = m_Wrapper.WriteImage2D(index, m_Wrapper.Shared(), FinalRasW(), FinalRasH(), 0, v.data());

		if (!b)
			m_ErrorReport.push_back(__FUNCTION__);

		return b;
	}
	else
		return false;
}

/// <summary>
/// Public virtual functions overridden from Renderer or RendererBase.
/// </summary>

/// <summary>
/// The amount of video RAM available on the GPU to render with.
/// </summary>
/// <returns>An unsigned 64-bit integer specifying how much video memory is available</returns>
template <typename T>
size_t RendererCL<T>::MemoryAvailable()
{
	return Ok() ? m_Wrapper.GlobalMemSize() : 0ULL;
}

/// <summary>
/// Return whether OpenCL has been properly initialized.
/// </summary>
/// <returns>True if OpenCL has been properly initialized, else false.</returns>
template <typename T>
bool RendererCL<T>::Ok() const
{
	return m_Init;
}

/// <summary>
/// Override to force num channels to be 4 because RGBA is always used for OpenCL
/// since the output is actually an image rather than just a buffer.
/// </summary>
/// <param name="numChannels">The number of channels, ignored.</param>
template <typename T>
void RendererCL<T>::NumChannels(size_t numChannels)
{
	m_NumChannels = 4;
}

/// <summary>
/// Dump the error report for this class as well as the OpenCLWrapper member.
/// </summary>
template <typename T>
void RendererCL<T>::DumpErrorReport()
{
	EmberReport::DumpErrorReport();
	m_Wrapper.DumpErrorReport();
}

/// <summary>
/// Clear the error report for this class as well as the OpenCLWrapper member.
/// </summary>
template <typename T>
void RendererCL<T>::ClearErrorReport()
{
	EmberReport::ClearErrorReport();
	m_Wrapper.ClearErrorReport();
}

/// <summary>
/// The sub batch size for OpenCL will always be how many
/// iterations are ran per kernel call. The caller can't
/// change this.
/// </summary>
/// <returns>The number of iterations ran in a single kernel call</returns>
template <typename T>
size_t RendererCL<T>::SubBatchSize() const
{
	return IterCountPerGrid();
}

/// <summary>
/// The thread count for OpenCL is always considered to be 1, however
/// the kernel internally runs many threads.
/// </summary>
/// <returns>1</returns>
template <typename T>
size_t RendererCL<T>::ThreadCount() const
{
	return 1;
}

/// <summary>
/// Create the density filter in the base class and copy the filter values
/// to the corresponding OpenCL buffers.
/// </summary>
/// <param name="newAlloc">True if a new filter instance was created, else false.</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::CreateDEFilter(bool& newAlloc)
{
	bool b = true;

	if (Renderer<T, T>::CreateDEFilter(newAlloc))
	{
		//Copy coefs and widths here. Convert and copy the other filter params right before calling the filtering kernel.
		if (newAlloc)
		{
			const char* loc = __FUNCTION__;
			DensityFilter<T>* filter = dynamic_cast<DensityFilter<T>*>(GetDensityFilter());

			if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DECoefsBufferName, reinterpret_cast<void*>(const_cast<T*>(filter->Coefs())), filter->CoefsSizeBytes())))					   { m_ErrorReport.push_back(loc); }
			if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DEWidthsBufferName, reinterpret_cast<void*>(const_cast<T*>(filter->Widths())), filter->WidthsSizeBytes())))				   { m_ErrorReport.push_back(loc); }
			if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DECoefIndicesBufferName, reinterpret_cast<void*>(const_cast<uint*>(filter->CoefIndices())), filter->CoefsIndicesSizeBytes()))) { m_ErrorReport.push_back(loc); }
		}
	}
	else
		b = false;

	return b;
}

/// <summary>
/// Create the spatial filter in the base class and copy the filter values
/// to the corresponding OpenCL buffers.
/// </summary>
/// <param name="newAlloc">True if a new filter instance was created, else false.</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::CreateSpatialFilter(bool& newAlloc)
{
	bool b = true;

	if (Renderer<T, T>::CreateSpatialFilter(newAlloc))
	{
		if (newAlloc)
			if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_SpatialFilterCoefsBufferName, reinterpret_cast<void*>(GetSpatialFilter()->Filter()), GetSpatialFilter()->BufferSizeBytes()))) { m_ErrorReport.push_back(__FUNCTION__); }

	}
	else
		b = false;

	return b;
}

/// <summary>
/// Get the renderer type enum.
/// </summary>
/// <returns>OPENCL_RENDERER</returns>
template <typename T>
eRendererType RendererCL<T>::RendererType() const
{
	return OPENCL_RENDERER;
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// OpenCLWrapper member as a single string.
/// </summary>
/// <returns>The concatenated error report string</returns>
template <typename T>
string RendererCL<T>::ErrorReportString()
{
	return EmberReport::ErrorReportString() + m_Wrapper.ErrorReportString();
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// OpenCLWrapper member as a vector of strings.
/// </summary>
/// <returns>The concatenated error report vector of strings</returns>
template <typename T>
vector<string> RendererCL<T>::ErrorReport()
{
	vector<string> ours = EmberReport::ErrorReport();
	vector<string> wrappers = m_Wrapper.ErrorReport();

	ours.insert(ours.end(), wrappers.begin(), wrappers.end());
	return ours;
}

/// <summary>
/// Set the vector of random contexts.
/// Call the base, and reset the seeds vector.
/// </summary>
/// <param name="randVec">The vector of random contexts to assign</param>
/// <returns>True if the size of the vector matched the number of threads used for rendering and writing seeds to OpenCL succeeded, else false.</returns>
template <typename T>
bool RendererCL<T>::RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec)
{
	bool b = Renderer<T, T>::RandVec(randVec);
	const char* loc = __FUNCTION__;

	if (m_Wrapper.Ok())
	{
		FillSeeds();
		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_SeedsBufferName, reinterpret_cast<void*>(m_Seeds.data()), SizeOf(m_Seeds)))) { m_ErrorReport.push_back(loc); }
	}

	return b;
}

/// <summary>
/// Protected virtual functions overridden from Renderer.
/// </summary>

/// <summary>
/// Make the final palette used for iteration.
/// This override differs from the base in that it does not use
/// bucketT as the output palette type. This is because OpenCL
/// only supports floats for texture images.
/// </summary>
/// <param name="colorScalar">The color scalar to multiply the ember's palette by</param>
template <typename T>
void RendererCL<T>::MakeDmap(T colorScalar)
{
	//m_Ember.m_Palette.MakeDmap<float>(m_DmapCL, colorScalar);
	m_Ember.m_Palette.MakeDmap(m_DmapCL, colorScalar);
}


/// <summary>
/// Allocate all buffers required for running as well as the final
/// 2D image.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::Alloc()
{
	if (!m_Wrapper.Ok())
		return false;

	EnterResize();
	m_XformsCL.resize(m_Ember.TotalXformCount());

	bool b = true;
	size_t histLength = SuperSize() * sizeof(v4T);
	size_t accumLength = SuperSize() * sizeof(v4T);
	const char* loc = __FUNCTION__;

	if (b && !(b = m_Wrapper.AddBuffer(m_EmberBufferName,               sizeof(m_EmberCL))))         { m_ErrorReport.push_back(loc); }
	if (b && !(b = m_Wrapper.AddBuffer(m_XformsBufferName,				SizeOf(m_XformsCL))))		 { m_ErrorReport.push_back(loc); }
	if (b && !(b = m_Wrapper.AddBuffer(m_ParVarsBufferName,             128 * sizeof(T))))           { m_ErrorReport.push_back(loc); }
	if (b && !(b = m_Wrapper.AddBuffer(m_DistBufferName,                CHOOSE_XFORM_GRAIN)))        { m_ErrorReport.push_back(loc); }//Will be resized for xaos.
	if (b && !(b = m_Wrapper.AddBuffer(m_CarToRasBufferName,            sizeof(m_CarToRasCL))))      { m_ErrorReport.push_back(loc); }
	if (b && !(b = m_Wrapper.AddBuffer(m_DEFilterParamsBufferName,      sizeof(m_DensityFilterCL)))) { m_ErrorReport.push_back(loc); }
	if (b && !(b = m_Wrapper.AddBuffer(m_SpatialFilterParamsBufferName, sizeof(m_SpatialFilterCL)))) { m_ErrorReport.push_back(loc); }

	if (b && !(b = m_Wrapper.AddBuffer(m_HistBufferName,   histLength)))								  { m_ErrorReport.push_back(loc); }//Histogram. Will memset to zero later.
	if (b && !(b = m_Wrapper.AddBuffer(m_AccumBufferName,  accumLength)))								  { m_ErrorReport.push_back(loc); }//Accum buffer.
	if (b && !(b = m_Wrapper.AddBuffer(m_PointsBufferName, IterGridKernelCount() * sizeof(PointCL<T>))))  { m_ErrorReport.push_back(loc); }//Points between iter calls.

	LeaveResize();

	if (b && !(b = SetOutputTexture(m_OutputTexID))) { m_ErrorReport.push_back(loc); }

	return b;
}

/// <summary>
/// Clear OpenCL histogram and/or density filtering buffers to all zeroes.
/// </summary>
/// <param name="resetHist">Clear histogram if true, else don't.</param>
/// <param name="resetAccum">Clear density filtering buffer if true, else don't.</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ResetBuckets(bool resetHist, bool resetAccum)
{
	bool b = true;

	if (resetHist)
		b &= ClearHist();

	if (resetAccum)
		b &= ClearAccum();

	return b;
}

/// <summary>
/// Perform log scale density filtering.
/// </summary>
/// <returns>True if success and not aborted, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::LogScaleDensityFilter()
{
	return RunLogScaleFilter();
}

/// <summary>
/// Run gaussian density estimation filtering.
/// </summary>
/// <returns>True if success and not aborted, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::GaussianDensityFilter()
{
	//This commented section is for debugging density filtering by making it run on the CPU
	//then copying the results back to the GPU.
	//if (ReadHist())
	//{
	//	uint accumLength = SuperSize() * sizeof(glm::detail::tvec4<T>);
	//	const char* loc = __FUNCTION__;
	//
	//	Renderer<T, T>::ResetBuckets(false, true);
	//	Renderer<T, T>::GaussianDensityFilter();
	//
	//	if (!m_Wrapper.WriteBuffer(m_AccumBufferName, AccumulatorBuckets(), accumLength)) { m_ErrorReport.push_back(loc); return RENDER_ERROR; }
	//		return RENDER_OK;
	//}
	//else
	//	return RENDER_ERROR;

	//Timing t(4);

	eRenderStatus status = RunDensityFilter();
	//t.Toc(__FUNCTION__ " RunKernel()");

	return status;
}

/// <summary>
/// Run final accumulation.
/// If pixels is nullptr, the output will remain in the OpenCL 2D image.
/// However, if pixels is not nullptr, the output will be copied. This is
/// useful when rendering in OpenCL, but saving the output to a file.
/// </summary>
/// <param name="pixels">The pixels to copy the final image to if not nullptr</param>
/// <param name="finalOffset">Offset in the buffer to store the pixels to</param>
/// <returns>True if success and not aborted, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::AccumulatorToFinalImage(byte* pixels, size_t finalOffset)
{
	eRenderStatus status = RunFinalAccum();

	if (status == RENDER_OK && pixels != nullptr && !m_Wrapper.Shared())
	{
		pixels += finalOffset;

		if (!ReadFinal(pixels))
			status = RENDER_ERROR;
	}

	return status;
}

/// <summary>
/// Run the iteration algorithm for the specified number of iterations.
/// This is only called after all other setup has been done.
/// This will recompile the OpenCL program if this ember differs significantly
/// from the previous run.
/// Note that the bad value count is not recorded when running with OpenCL. If it's
/// needed, run on the CPU.
/// </summary>
/// <param name="iterCount">The number of iterations to run</param>
/// <param name="temporalSample">The temporal sample within the current pass this is running for</param>
/// <returns>Rendering statistics</returns>
template <typename T>
EmberStats RendererCL<T>::Iterate(size_t iterCount, size_t temporalSample)
{
	bool b = true;
	EmberStats stats;//Do not record bad vals with with GPU. If the user needs to investigate bad vals, use the CPU.
	const char* loc = __FUNCTION__;

	IterOpenCLKernelCreator<T>::ParVarIndexDefines(m_Ember, m_Params, true, false);//Always do this to get the values (but no string), regardless of whether a rebuild is necessary.

	//Don't know the size of the parametric varations parameters buffer until the ember is examined.
	//So set it up right before the run.
	if (!m_Params.second.empty())
	{
		if (!m_Wrapper.AddAndWriteBuffer(m_ParVarsBufferName, m_Params.second.data(), m_Params.second.size() * sizeof(m_Params.second[0])))
		{
			m_Abort = true;
			m_ErrorReport.push_back(loc);
			return stats;
		}
	}

	//Rebuilding is expensive, so only do it if it's required.
	if (IterOpenCLKernelCreator<T>::IsBuildRequired(m_Ember, m_LastBuiltEmber))
		b = BuildIterProgramForEmber(true);

	if (b)
	{
		m_IterTimer.Tic();//Tic() here to avoid including build time in iter time measurement.

		if (m_Stats.m_Iters == 0)//Only reset the call count on the beginning of a new render. Do not reset on KEEP_ITERATING.
			m_Calls = 0;

		b = RunIter(iterCount, temporalSample, stats.m_Iters);

		if (!b || stats.m_Iters == 0)//If no iters were executed, something went catastrophically wrong.
			m_Abort = true;

		stats.m_IterMs = m_IterTimer.Toc();
	}
	else
	{
		m_Abort = true;
		m_ErrorReport.push_back(loc);
	}

	return stats;
}

/// <summary>
/// Private functions for making and running OpenCL programs.
/// </summary>

/// <summary>
/// Build the iteration program for the current ember.
/// </summary>
/// <param name="doAccum">Whether to build in accumulation, only for debugging. Default: true.</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::BuildIterProgramForEmber(bool doAccum)
{
	//Timing t;
	const char* loc = __FUNCTION__;
	IterOpenCLKernelCreator<T>::ParVarIndexDefines(m_Ember, m_Params, false, true);//Do with string and no vals.
	m_IterKernel = m_IterOpenCLKernelCreator.CreateIterKernelString(m_Ember, m_Params.first, m_LockAccum, doAccum);
	//cout << "Building: " << endl << iterProgram << endl;

	//A program build is roughly .66s which will detract from the user experience.
	//Need to experiment with launching this in a thread/task and returning once it's done.//TODO
	if (m_Wrapper.AddProgram(m_IterOpenCLKernelCreator.IterEntryPoint(), m_IterKernel, m_IterOpenCLKernelCreator.IterEntryPoint(), m_DoublePrecision))
	{
		//t.Toc(__FUNCTION__ " program build");
		//cout << string(loc) << "():\nBuilding the following program succeeded: \n" << iterProgram << endl;
		m_LastBuiltEmber = m_Ember;
	}
	else
	{
		m_ErrorReport.push_back(string(loc) + "():\nBuilding the following program failed: \n" + m_IterKernel + "\n");
		return false;
	}

	return true;
}

/// <summary>
/// Run the iteration kernel.
/// Fusing on the CPU is done once per sub batch, usually 10,000 iters, however
/// determining when to do it in OpenCL is much more difficult.
/// Currently it's done once every 4 kernel calls which seems to be a good balance
/// between quality of the final image and performance.
/// </summary>
/// <param name="iterCount">The number of iterations to run</param>
/// <param name="temporalSample">The temporal sample this is running for</param>
/// <param name="itersRan">The storage for the number of iterations ran</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::RunIter(size_t iterCount, size_t temporalSample, size_t& itersRan)
{
	Timing t;//, t2(4);
	bool b = true;
	uint fuse, argIndex;
	uint iterCountPerKernel = IterCountPerKernel();
	uint iterCountPerBlock = IterCountPerBlock();
	uint supersize = uint(SuperSize());
	int kernelIndex = m_Wrapper.FindKernelIndex(m_IterOpenCLKernelCreator.IterEntryPoint());
	size_t fuseFreq = Renderer<T, T>::SubBatchSize() / m_IterCountPerKernel;//Use the base sbs to determine when to fuse.
	size_t itersRemaining;
	double percent, etaMs;
	const char* loc = __FUNCTION__;

	itersRan = 0;
#ifdef TEST_CL
	m_Abort = false;
#endif

	if (kernelIndex != -1)
	{
		ConvertEmber(m_Ember, m_EmberCL, m_XformsCL);
		m_CarToRasCL = ConvertCarToRas(*CoordMap());

		if (b && !(b = m_Wrapper.WriteBuffer      (m_EmberBufferName,    reinterpret_cast<void*>(&m_EmberCL),           sizeof(m_EmberCL))))						   { m_ErrorReport.push_back(loc); }
		if (b && !(b = m_Wrapper.WriteBuffer	  (m_XformsBufferName,   reinterpret_cast<void*>(m_XformsCL.data()),    sizeof(m_XformsCL[0]) * m_XformsCL.size()))) { m_ErrorReport.push_back(loc); }
		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DistBufferName,     reinterpret_cast<void*>(const_cast<byte*>(XformDistributions())), XformDistributionsSize())))				   { m_ErrorReport.push_back(loc); }//Will be resized for xaos.
		if (b && !(b = m_Wrapper.WriteBuffer      (m_CarToRasBufferName, reinterpret_cast<void*>(&m_CarToRasCL),        sizeof(m_CarToRasCL))))					   { m_ErrorReport.push_back(loc); }

		if (b && !(b = m_Wrapper.AddAndWriteImage("Palette", CL_MEM_READ_ONLY, m_PaletteFormat, m_DmapCL.m_Entries.size(), 1, 0, m_DmapCL.m_Entries.data()))) { m_ErrorReport.push_back(loc); }

		//If animating, treat each temporal sample as a newly started render for fusing purposes.
		if (temporalSample > 0)
			m_Calls = 0;

		while (b && itersRan < iterCount && !m_Abort)
		{
			argIndex = 0;
#ifdef TEST_CL
			fuse = 0;
#else
			//fuse = 100;
			//fuse = ((m_Calls % fuseFreq) == 0 ? (EarlyClip() ? 100u : 15u) : 0u);
			fuse = uint((m_Calls % fuseFreq) == 0u ? FuseCount() : 0u);
			//fuse = ((m_Calls % 4) == 0 ? 100u : 0u);
#endif
			itersRemaining = iterCount - itersRan;
			uint gridW = uint(min(ceil(double(itersRemaining) / double(iterCountPerBlock)), double(IterGridBlockWidth())));
			uint gridH = uint(min(ceil(double(itersRemaining) / double(gridW * iterCountPerBlock)), double(IterGridBlockHeight())));
			uint iterCountThisLaunch = iterCountPerBlock * gridW * gridH;

			//Similar to what's done in the base class.
			//The number of iters per thread must be adjusted if they've requested less iters than is normally ran in a block (256 * 256).
			if (iterCountThisLaunch > iterCount)
			{
				iterCountPerKernel = uint(ceil(double(iterCount) / double(gridW * gridH * IterBlockKernelCount())));
				iterCountThisLaunch = iterCountPerKernel * (gridW * gridH * IterBlockKernelCount());
			}

			if (b && !(b = m_Wrapper.SetArg      (kernelIndex, argIndex++, iterCountPerKernel)))   { m_ErrorReport.push_back(loc); }//Number of iters for each thread to run.
			if (b && !(b = m_Wrapper.SetArg      (kernelIndex, argIndex++, fuse)))                 { m_ErrorReport.push_back(loc); }//Number of iters to fuse.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_SeedsBufferName)))    { m_ErrorReport.push_back(loc); }//Seeds.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_EmberBufferName)))    { m_ErrorReport.push_back(loc); }//Ember.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_XformsBufferName)))   { m_ErrorReport.push_back(loc); }//Xforms.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_ParVarsBufferName)))  { m_ErrorReport.push_back(loc); }//Parametric variation parameters.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_DistBufferName)))     { m_ErrorReport.push_back(loc); }//Xform distributions.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_CarToRasBufferName))) { m_ErrorReport.push_back(loc); }//Coordinate converter.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_HistBufferName)))     { m_ErrorReport.push_back(loc); }//Histogram.
			if (b && !(b = m_Wrapper.SetArg		 (kernelIndex, argIndex++, supersize)))			   { m_ErrorReport.push_back(loc); }//Histogram size.
			if (b && !(b = m_Wrapper.SetImageArg (kernelIndex, argIndex++, false, "Palette")))     { m_ErrorReport.push_back(loc); }//Palette.
			if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_PointsBufferName)))   { m_ErrorReport.push_back(loc); }//Random start points.

			if (b && !(b = m_Wrapper.RunKernel(kernelIndex,
									 gridW * IterBlockKernelWidth(),//Total grid dims.
									 gridH * IterBlockKernelHeight(),
									 1,
									 IterBlockKernelWidth(),//Individual block dims.
									 IterBlockKernelHeight(),
									 1)))
			{
				m_Abort = true;
				m_ErrorReport.push_back(loc);
				break;
			}

			itersRan += iterCountThisLaunch;
			m_Calls++;

			if (m_Callback)
			{
				percent = 100.0 *
					double
					(
						double
						(
							double
							(
								double(m_LastIter + itersRan) / double(ItersPerTemporalSample())
							) + temporalSample
						) / double(TemporalSamples())
					);

				double percentDiff = percent - m_LastIterPercent;
				double toc = m_ProgressTimer.Toc();

				if (percentDiff >= 10 || (toc > 1000 && percentDiff >= 1))//Call callback function if either 10% has passed, or one second (and 1%).
				{
					etaMs = ((100.0 - percent) / percent) * m_RenderTimer.Toc();

					if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, percent, 0, etaMs))
						Abort();

					m_LastIterPercent = percent;
					m_ProgressTimer.Tic();
				}
			}
		}
	}
	else
	{
		b = false;
		m_ErrorReport.push_back(loc);
	}

	//t2.Toc(__FUNCTION__);
	return b;
}

/// <summary>
/// Run the log scale filter.
/// </summary>
/// <returns>True if success, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::RunLogScaleFilter()
{
	//Timing t(4);
	bool b = true;
	int kernelIndex = m_Wrapper.FindKernelIndex(m_DEOpenCLKernelCreator.LogScaleAssignDEEntryPoint());
	const char* loc = __FUNCTION__;

	if (kernelIndex != -1)
	{
		m_DensityFilterCL = ConvertDensityFilter();
		uint argIndex = 0;
		uint blockW = m_WarpSize;
		uint blockH = 4;//A height of 4 seems to run the fastest.
		uint gridW = m_DensityFilterCL.m_SuperRasW;
		uint gridH = m_DensityFilterCL.m_SuperRasH;

		OpenCLWrapper::MakeEvenGridDims(blockW, blockH, gridW, gridH);

		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DEFilterParamsBufferName, reinterpret_cast<void*>(&m_DensityFilterCL), sizeof(m_DensityFilterCL)))) { m_ErrorReport.push_back(loc); }

		if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_HistBufferName)))           { m_ErrorReport.push_back(loc); }//Histogram.
		if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_AccumBufferName)))          { m_ErrorReport.push_back(loc); }//Accumulator.
		if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, m_DEFilterParamsBufferName))) { m_ErrorReport.push_back(loc); }//DensityFilterCL.

		//t.Tic();
		if (b && !(b = m_Wrapper.RunKernel(kernelIndex, gridW, gridH, 1, blockW, blockH, 1))) { m_ErrorReport.push_back(loc); }
		//t.Toc(loc);
	}
	else
	{
		b = false;
		m_ErrorReport.push_back(loc);
	}

	if (b && m_Callback)
		m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 1, 0.0);

	return b ? RENDER_OK : RENDER_ERROR;
}

/// <summary>
/// Run the Gaussian density filter.
/// Method 7: Each block processes a 32x32 block and exits. No column or row advancements happen.
/// </summary>
/// <returns>True if success and not aborted, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::RunDensityFilter()
{
	bool b = true;
	Timing t(4);// , t2(4);
	m_DensityFilterCL = ConvertDensityFilter();
	int kernelIndex = MakeAndGetDensityFilterProgram(Supersample(), m_DensityFilterCL.m_FilterWidth);
	const char* loc = __FUNCTION__;

	if (kernelIndex != -1)
	{
		uint leftBound  = m_DensityFilterCL.m_Supersample - 1;
		uint rightBound = m_DensityFilterCL.m_SuperRasW - (m_DensityFilterCL.m_Supersample - 1);
		uint topBound   = leftBound;
		uint botBound   = m_DensityFilterCL.m_SuperRasH - (m_DensityFilterCL.m_Supersample - 1);
		uint gridW      = rightBound - leftBound;
		uint gridH      = botBound - topBound;
		uint blockSizeW = m_MaxDEBlockSizeW;//These *must* both be divisible by 16 or else pixels will go missing.
		uint blockSizeH = m_MaxDEBlockSizeH;

		//OpenCL runs out of resources when using double or a supersample of 2.
		//Remedy this by reducing the height of the block by 2.
		if (m_DoublePrecision || m_DensityFilterCL.m_Supersample > 1)
			blockSizeH -= 2;

		//Can't just blindly pass in vals. Must adjust them first to evenly divide the block count
		//into the total grid dimensions.
		OpenCLWrapper::MakeEvenGridDims(blockSizeW, blockSizeH, gridW, gridH);

		//t.Tic();
		//The classic problem with performing DE on adjacent pixels is that the filter will overlap.
		//This can be solved in 2 ways. One is to use atomics, which is unacceptably slow.
		//The other is to proces the entire image in multiple passes, and each pass processes blocks of pixels
		//that are far enough apart such that their filters do not overlap.
		//Do the latter.
		//Gap is in terms of blocks. How many blocks must separate two blocks running at the same time.
		uint gapW = uint(ceil((m_DensityFilterCL.m_FilterWidth * 2.0) / double(blockSizeW)));
		uint chunkSizeW = gapW + 1;
		uint gapH = uint(ceil((m_DensityFilterCL.m_FilterWidth * 2.0) / double(blockSizeH)));
		uint chunkSizeH = gapH + 1;
		double totalChunks = chunkSizeW * chunkSizeH;

		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_DEFilterParamsBufferName, reinterpret_cast<void*>(&m_DensityFilterCL), sizeof(m_DensityFilterCL)))) { m_ErrorReport.push_back(loc); }

#ifdef ROW_ONLY_DE
		blockSizeW = 64;//These *must* both be divisible by 16 or else pixels will go missing.
		blockSizeH = 1;
		gapW = (uint)ceil((m_DensityFilterCL.m_FilterWidth * 2.0) / (double)blockSizeW);
		chunkSizeW = gapW + 1;
		gapH = (uint)ceil((m_DensityFilterCL.m_FilterWidth * 2.0) / (double)32);//Block height is 1, but iterates over 32 rows.
		chunkSizeH = gapH + 1;
		totalChunks = chunkSizeW * chunkSizeH;

		OpenCLWrapper::MakeEvenGridDims(blockSizeW, blockSizeH, gridW, gridH);
		gridW /= chunkSizeW;
		gridH /= chunkSizeH;

		for (uint rowChunk = 0; b && !m_Abort && rowChunk < chunkSizeH; rowChunk++)
		{
			for (uint colChunk = 0; b && !m_Abort && colChunk < chunkSizeW; colChunk++)
			{
				//t2.Tic();
				if (b && !(b = RunDensityFilterPrivate(kernelIndex, gridW, gridH, blockSizeW, blockSizeH, chunkSizeW, chunkSizeH, colChunk, rowChunk))) { m_Abort = true; m_ErrorReport.push_back(loc); }
				//t2.Toc(loc);

				if (b && m_Callback)
				{
					double percent = (double((rowChunk * chunkSizeW) + (colChunk + 1)) / totalChunks) * 100.0;
					double etaMs = ((100.0 - percent) / percent) * t.Toc();

					if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, percent, 1, etaMs))
						Abort();
				}
			}
		}
#else
		gridW /= chunkSizeW;
		gridH /= chunkSizeH;
		OpenCLWrapper::MakeEvenGridDims(blockSizeW, blockSizeH, gridW, gridH);

		for (uint rowChunk = 0; b && !m_Abort && rowChunk < chunkSizeH; rowChunk++)
		{
			for (uint colChunk = 0; b && !m_Abort && colChunk < chunkSizeW; colChunk++)
			{
				//t2.Tic();
				if (b && !(b = RunDensityFilterPrivate(kernelIndex, gridW, gridH, blockSizeW, blockSizeH, chunkSizeW, chunkSizeH, colChunk, rowChunk))) { m_Abort = true; m_ErrorReport.push_back(loc); }
				//t2.Toc(loc);

				if (b && m_Callback)
				{
					double percent = (double((rowChunk * chunkSizeW) + (colChunk + 1)) / totalChunks) * 100.0;
					double etaMs = ((100.0 - percent) / percent) * t.Toc();

					if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, percent, 1, etaMs))
						Abort();
				}
			}
		}
#endif

		if (b && m_Callback)
			m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 1, 0.0);

		//t2.Toc(__FUNCTION__ " all passes");
	}
	else
	{
		b = false;
		m_ErrorReport.push_back(loc);
	}

	return m_Abort ? RENDER_ABORT : (b ? RENDER_OK : RENDER_ERROR);
}

/// <summary>
/// Run final accumulation to the 2D output image.
/// </summary>
/// <returns>True if success and not aborted, else false.</returns>
template <typename T>
eRenderStatus RendererCL<T>::RunFinalAccum()
{
	//Timing t(4);
	bool b = true;
	T alphaBase;
	T alphaScale;
	int accumKernelIndex = MakeAndGetFinalAccumProgram(alphaBase, alphaScale);
	uint argIndex;
	uint gridW;
	uint gridH;
	uint blockW;
	uint blockH;
	const char* loc = __FUNCTION__;

	if (!m_Abort && accumKernelIndex != -1)
	{
		//This is needed with or without early clip.
		m_SpatialFilterCL = ConvertSpatialFilter();

		if (b && !(b = m_Wrapper.AddAndWriteBuffer(m_SpatialFilterParamsBufferName, reinterpret_cast<void*>(&m_SpatialFilterCL), sizeof(m_SpatialFilterCL)))) { m_ErrorReport.push_back(loc); }

		//Since early clip requires gamma correcting the entire accumulator first,
		//it can't be done inside of the normal final accumulation kernel, so
		//an additional kernel must be launched first.
		if (b && EarlyClip())
		{
			int gammaCorrectKernelIndex = MakeAndGetGammaCorrectionProgram();

			if (gammaCorrectKernelIndex != -1)
			{
				argIndex = 0;
				blockW = m_WarpSize;
				blockH = 4;//A height of 4 seems to run the fastest.
				gridW = m_SpatialFilterCL.m_SuperRasW;//Using super dimensions because this processes the density filtering bufer.
				gridH = m_SpatialFilterCL.m_SuperRasH;
				OpenCLWrapper::MakeEvenGridDims(blockW, blockH, gridW, gridH);

				if (b && !(b = m_Wrapper.SetBufferArg(gammaCorrectKernelIndex, argIndex++, m_AccumBufferName)))               { m_ErrorReport.push_back(loc); }//Accumulator.
				if (b && !(b = m_Wrapper.SetBufferArg(gammaCorrectKernelIndex, argIndex++, m_SpatialFilterParamsBufferName))) { m_ErrorReport.push_back(loc); }//SpatialFilterCL.

				if (b && !(b = m_Wrapper.RunKernel(gammaCorrectKernelIndex, gridW, gridH, 1, blockW, blockH, 1)))			  { m_ErrorReport.push_back(loc); }
			}
			else
			{
				b = false;
				m_ErrorReport.push_back(loc);
			}
		}

		argIndex = 0;
		blockW = m_WarpSize;
		blockH = 4;//A height of 4 seems to run the fastest.
		gridW = m_SpatialFilterCL.m_FinalRasW;
		gridH = m_SpatialFilterCL.m_FinalRasH;
		OpenCLWrapper::MakeEvenGridDims(blockW, blockH, gridW, gridH);

		if (b && !(b = m_Wrapper.SetBufferArg(accumKernelIndex, argIndex++, m_AccumBufferName)))                    { m_ErrorReport.push_back(loc); }//Accumulator.
		if (b && !(b = m_Wrapper.SetImageArg (accumKernelIndex, argIndex++, m_Wrapper.Shared(), m_FinalImageName))) { m_ErrorReport.push_back(loc); }//Final image.
		if (b && !(b = m_Wrapper.SetBufferArg(accumKernelIndex, argIndex++, m_SpatialFilterParamsBufferName)))      { m_ErrorReport.push_back(loc); }//SpatialFilterCL.
		if (b && !(b = m_Wrapper.SetBufferArg(accumKernelIndex, argIndex++, m_SpatialFilterCoefsBufferName)))       { m_ErrorReport.push_back(loc); }//Filter coefs.
		if (b && !(b = m_Wrapper.SetArg		 (accumKernelIndex, argIndex++, alphaBase)))                            { m_ErrorReport.push_back(loc); }//Alpha base.
		if (b && !(b = m_Wrapper.SetArg		 (accumKernelIndex, argIndex++, alphaScale)))                           { m_ErrorReport.push_back(loc); }//Alpha scale.

		if (b && m_Wrapper.Shared())
			if (b && !(b = m_Wrapper.EnqueueAcquireGLObjects(m_FinalImageName))) { m_ErrorReport.push_back(loc); }

		if (b && !(b = m_Wrapper.RunKernel(accumKernelIndex, gridW, gridH, 1, blockW, blockH, 1))) { m_ErrorReport.push_back(loc); }

		if (b && m_Wrapper.Shared())
			if (b && !(b = m_Wrapper.EnqueueReleaseGLObjects(m_FinalImageName))) { m_ErrorReport.push_back(loc); }

		//t.Toc((char*)loc);
	}
	else
	{
		b = false;
		m_ErrorReport.push_back(loc);
	}

	return b ? RENDER_OK : RENDER_ERROR;
}

/// <summary>
/// Zeroize a buffer of the specified size.
/// </summary>
/// <param name="bufferName">Name of the buffer to clear</param>
/// <param name="width">Width in elements</param>
/// <param name="height">Height in elements</param>
/// <param name="elementSize">Size of each element</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::ClearBuffer(const string& bufferName, uint width, uint height, uint elementSize)
{
	bool b = true;
	int kernelIndex = m_Wrapper.FindKernelIndex(m_IterOpenCLKernelCreator.ZeroizeEntryPoint());
	uint argIndex = 0;
	const char* loc = __FUNCTION__;

	if (kernelIndex != -1)
	{
		uint blockW = m_NVidia ? 32 : 16;//Max work group size is 256 on AMD, which means 16x16.
		uint blockH = m_NVidia ? 32 : 16;
		uint gridW = width * elementSize;
		uint gridH = height;

		OpenCLWrapper::MakeEvenGridDims(blockW, blockH, gridW, gridH);

		if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex++, bufferName)))          { m_ErrorReport.push_back(loc); }//Buffer of byte.
		if (b && !(b = m_Wrapper.SetArg      (kernelIndex, argIndex++, width * elementSize))) { m_ErrorReport.push_back(loc); }//Width.
		if (b && !(b = m_Wrapper.SetArg      (kernelIndex, argIndex++, height)))              { m_ErrorReport.push_back(loc); }//Height.
		if (b && !(b = m_Wrapper.RunKernel(kernelIndex, gridW, gridH, 1, blockW, blockH, 1))) { m_ErrorReport.push_back(loc); }
	}
	else
	{
		b = false;
		m_ErrorReport.push_back(loc);
	}

	return b;
}

/// <summary>
/// Private wrapper around calling Gaussian density filtering kernel.
/// The parameters are very specific to how the kernel is internally implemented.
/// </summary>
/// <param name="kernelIndex">Index of the kernel to call</param>
/// <param name="gridW">Grid width</param>
/// <param name="gridH">Grid height</param>
/// <param name="blockW">Block width</param>
/// <param name="blockH">Block height</param>
/// <param name="chunkSizeW">Chunk size width (gapW + 1)</param>
/// <param name="chunkSizeH">Chunk size height (gapH + 1)</param>
/// <param name="rowParity">Row parity</param>
/// <param name="colParity">Column parity</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool RendererCL<T>::RunDensityFilterPrivate(uint kernelIndex, uint gridW, uint gridH, uint blockW, uint blockH, uint chunkSizeW, uint chunkSizeH, uint chunkW, uint chunkH)
{
	//Timing t(4);
	bool b = true;
	uint argIndex = 0;
	const char* loc = __FUNCTION__;

	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_HistBufferName)))           { m_ErrorReport.push_back(loc); } argIndex++;//Histogram.
	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_AccumBufferName)))          { m_ErrorReport.push_back(loc); } argIndex++;//Accumulator.
	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_DEFilterParamsBufferName))) { m_ErrorReport.push_back(loc); } argIndex++;//FlameDensityFilterCL.
	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_DECoefsBufferName)))        { m_ErrorReport.push_back(loc); } argIndex++;//Coefs.
	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_DEWidthsBufferName)))       { m_ErrorReport.push_back(loc); } argIndex++;//Widths.
	if (b && !(b = m_Wrapper.SetBufferArg(kernelIndex, argIndex, m_DECoefIndicesBufferName)))  { m_ErrorReport.push_back(loc); } argIndex++;//Coef indices.
	if (b && !(b = m_Wrapper.SetArg(      kernelIndex, argIndex, chunkSizeW)))                 { m_ErrorReport.push_back(loc); } argIndex++;//Chunk size width (gapW + 1).
	if (b && !(b = m_Wrapper.SetArg(      kernelIndex, argIndex, chunkSizeH)))                 { m_ErrorReport.push_back(loc); } argIndex++;//Chunk size height (gapH + 1).
	if (b && !(b = m_Wrapper.SetArg(      kernelIndex, argIndex, chunkW)))					   { m_ErrorReport.push_back(loc); } argIndex++;//Column chunk.
	if (b && !(b = m_Wrapper.SetArg(      kernelIndex, argIndex, chunkH)))					   { m_ErrorReport.push_back(loc); } argIndex++;//Row chunk.
	//t.Toc(__FUNCTION__ " set args");

	//t.Tic();
	if (b && !(b = m_Wrapper.RunKernel(kernelIndex, gridW, gridH, 1, blockW, blockH, 1))) { m_ErrorReport.push_back(loc); }//Method 7, accumulating to temp box area.
	//t.Toc(__FUNCTION__ " RunKernel()");

	return b;
}

/// <summary>
/// Make the Gaussian density filter program and return its index.
/// </summary>
/// <param name="ss">The supersample being used for the current ember</param>
/// <param name="filterWidth">Width of the gaussian filter</param>
/// <returns>The kernel index if successful, else -1.</returns>
template <typename T>
int RendererCL<T>::MakeAndGetDensityFilterProgram(size_t ss, uint filterWidth)
{
	string deEntryPoint = m_DEOpenCLKernelCreator.GaussianDEEntryPoint(ss, filterWidth);
	int kernelIndex = m_Wrapper.FindKernelIndex(deEntryPoint);
	const char* loc = __FUNCTION__;

	if (kernelIndex == -1)//Has not been built yet.
	{
		string kernel = m_DEOpenCLKernelCreator.GaussianDEKernel(ss, filterWidth);
		bool b = m_Wrapper.AddProgram(deEntryPoint, kernel, deEntryPoint, m_DoublePrecision);

		if (b)
		{
			kernelIndex = m_Wrapper.FindKernelIndex(deEntryPoint);//Try to find it again, it will be present if successfully built.
		}
		else
		{
			m_ErrorReport.push_back(string(loc) + "():\nBuilding the following program failed: \n" + kernel + "\n");
			//cout << m_ErrorReport.back();
		}
	}

	return kernelIndex;
}

/// <summary>
/// Make the final accumulation program and return its index.
/// There are many different kernels for final accum, depending on early clip, alpha channel, and transparency.
/// Loading all of these in the beginning is too much, so only load the one for the current case being worked with.
/// </summary>
/// <param name="alphaBase">Storage for the alpha base value used in the kernel. 0 if transparency is true, else 255.</param>
/// <param name="alphaScale">Storage for the alpha scale value used in the kernel. 255 if transparency is true, else 0.</param>
/// <returns>The kernel index if successful, else -1.</returns>
template <typename T>
int RendererCL<T>::MakeAndGetFinalAccumProgram(T& alphaBase, T& alphaScale)
{
	string finalAccumEntryPoint = m_FinalAccumOpenCLKernelCreator.FinalAccumEntryPoint(EarlyClip(), Renderer<T, T>::NumChannels(), Transparency(), alphaBase, alphaScale);
	int kernelIndex = m_Wrapper.FindKernelIndex(finalAccumEntryPoint);
	const char* loc = __FUNCTION__;

	if (kernelIndex == -1)//Has not been built yet.
	{
		string kernel = m_FinalAccumOpenCLKernelCreator.FinalAccumKernel(EarlyClip(), Renderer<T, T>::NumChannels(), Transparency());
		bool b = m_Wrapper.AddProgram(finalAccumEntryPoint, kernel, finalAccumEntryPoint, m_DoublePrecision);

		if (b)
			kernelIndex = m_Wrapper.FindKernelIndex(finalAccumEntryPoint);//Try to find it again, it will be present if successfully built.
		else
    {
      std::vector<std::string> errors = m_Wrapper.ProgramBuildErrors();
      m_ErrorReport.insert(m_ErrorReport.end(), errors.begin(), errors.end());
			m_ErrorReport.push_back(loc);
    }
	}

	return kernelIndex;
}

/// <summary>
/// Make the gamma correction program for early clipping and return its index.
/// </summary>
/// <returns>The kernel index if successful, else -1.</returns>
template <typename T>
int RendererCL<T>::MakeAndGetGammaCorrectionProgram()
{
	string gammaEntryPoint = m_FinalAccumOpenCLKernelCreator.GammaCorrectionEntryPoint(Renderer<T, T>::NumChannels(), Transparency());
	int kernelIndex = m_Wrapper.FindKernelIndex(gammaEntryPoint);
	const char* loc = __FUNCTION__;

	if (kernelIndex == -1)//Has not been built yet.
	{
		string kernel = m_FinalAccumOpenCLKernelCreator.GammaCorrectionKernel(Renderer<T, T>::NumChannels(), Transparency());
		bool b = m_Wrapper.AddProgram(gammaEntryPoint, kernel, gammaEntryPoint, m_DoublePrecision);

		if (b)
			kernelIndex = m_Wrapper.FindKernelIndex(gammaEntryPoint);//Try to find it again, it will be present if successfully built.
		else
			m_ErrorReport.push_back(loc);
	}

	return kernelIndex;
}

/// <summary>
/// Private functions passing data to OpenCL programs.
/// </summary>

/// <summary>
/// Convert the currently used host side DensityFilter object into a DensityFilterCL object
/// for passing to OpenCL.
/// </summary>
/// <returns>The DensityFilterCL object</returns>
template <typename T>
DensityFilterCL<T> RendererCL<T>::ConvertDensityFilter()
{
	DensityFilterCL<T> filterCL;
	DensityFilter<T>* densityFilter = dynamic_cast<DensityFilter<T>*>(GetDensityFilter());

	filterCL.m_Supersample = uint(Supersample());
	filterCL.m_SuperRasW = uint(SuperRasW());
	filterCL.m_SuperRasH = uint(SuperRasH());
	filterCL.m_K1 = K1();
	filterCL.m_K2 = K2();

	if (densityFilter)
	{
		filterCL.m_Curve = densityFilter->Curve();
		filterCL.m_KernelSize = uint(densityFilter->KernelSize());
		filterCL.m_MaxFilterIndex = uint(densityFilter->MaxFilterIndex());
		filterCL.m_MaxFilteredCounts = uint(densityFilter->MaxFilteredCounts());
		filterCL.m_FilterWidth = uint(densityFilter->FilterWidth());
	}

	return filterCL;
}

/// <summary>
/// Convert the currently used host side SpatialFilter object into a SpatialFilterCL object
/// for passing to OpenCL.
/// </summary>
/// <returns>The SpatialFilterCL object</returns>
template <typename T>
SpatialFilterCL<T> RendererCL<T>::ConvertSpatialFilter()
{
	T g, linRange, vibrancy;
	Color<T> background;
	SpatialFilterCL<T> filterCL;

	this->PrepFinalAccumVals(background, g, linRange, vibrancy);

	filterCL.m_SuperRasW = uint(SuperRasW());
	filterCL.m_SuperRasH = uint(SuperRasH());
	filterCL.m_FinalRasW = uint(FinalRasW());
	filterCL.m_FinalRasH = uint(FinalRasH());
	filterCL.m_Supersample = uint(Supersample());
	filterCL.m_FilterWidth = uint(GetSpatialFilter()->FinalFilterWidth());
	filterCL.m_NumChannels = uint(Renderer<T, T>::NumChannels());
	filterCL.m_BytesPerChannel = uint(BytesPerChannel());
	filterCL.m_DensityFilterOffset = uint(DensityFilterOffset());
	filterCL.m_Transparency = Transparency();
	filterCL.m_YAxisUp = uint(m_YAxisUp);
	filterCL.m_Vibrancy = vibrancy;
	filterCL.m_HighlightPower = HighlightPower();
	filterCL.m_Gamma = g;
	filterCL.m_LinRange = linRange;
	filterCL.m_Background = background;

	return filterCL;
}

/// <summary>
/// Convert the host side Ember object into an EmberCL object
/// and a vector of XformCL for passing to OpenCL.
/// </summary>
/// <param name="ember">The Ember object to convert</param>
/// <param name="emberCL">The converted EmberCL</param>
/// <param name="xformsCL">The converted vector of XformCL</param>
template <typename T>
void RendererCL<T>::ConvertEmber(Ember<T>& ember, EmberCL<T>& emberCL, vector<XformCL<T>>& xformsCL)
{
	memset(&emberCL, 0, sizeof(EmberCL<T>));//Might not really be needed.

	emberCL.m_RotA           = m_RotMat.A();
	emberCL.m_RotB           = m_RotMat.B();
	emberCL.m_RotD           = m_RotMat.D();
	emberCL.m_RotE           = m_RotMat.E();
	emberCL.m_CamMat		 = ember.m_CamMat;
	emberCL.m_CenterX        = CenterX();
	emberCL.m_CenterY		 = ember.m_RotCenterY;
	emberCL.m_CamZPos		 = ember.m_CamZPos;
	emberCL.m_CamPerspective = ember.m_CamPerspective;
	emberCL.m_CamYaw		 = ember.m_CamYaw;
	emberCL.m_CamPitch		 = ember.m_CamPitch;
	emberCL.m_CamDepthBlur	 = ember.m_CamDepthBlur;
	emberCL.m_BlurCoef		 = ember.BlurCoef();

	for (uint i = 0; i < ember.TotalXformCount() && i < xformsCL.size(); i++)
	{
		Xform<T>* xform = ember.GetTotalXform(i);

		xformsCL[i].m_A = xform->m_Affine.A();
		xformsCL[i].m_B = xform->m_Affine.B();
		xformsCL[i].m_C = xform->m_Affine.C();
		xformsCL[i].m_D = xform->m_Affine.D();
		xformsCL[i].m_E = xform->m_Affine.E();
		xformsCL[i].m_F = xform->m_Affine.F();

		xformsCL[i].m_PostA = xform->m_Post.A();
		xformsCL[i].m_PostB = xform->m_Post.B();
		xformsCL[i].m_PostC = xform->m_Post.C();
		xformsCL[i].m_PostD = xform->m_Post.D();
		xformsCL[i].m_PostE = xform->m_Post.E();
		xformsCL[i].m_PostF = xform->m_Post.F();

		xformsCL[i].m_DirectColor = xform->m_DirectColor;
		xformsCL[i].m_ColorSpeedCache = xform->ColorSpeedCache();
		xformsCL[i].m_OneMinusColorCache = xform->OneMinusColorCache();
		xformsCL[i].m_Opacity = xform->m_Opacity;
		xformsCL[i].m_VizAdjusted = xform->VizAdjusted();

		for (uint varIndex = 0; varIndex < xform->TotalVariationCount() && varIndex < MAX_CL_VARS; varIndex++)//Assign all variation weights for this xform, with a max of MAX_CL_VARS.
			xformsCL[i].m_VariationWeights[varIndex] = xform->GetVariation(varIndex)->m_Weight;
	}
}

/// <summary>
/// Convert the host side CarToRas object into a CarToRasCL object
/// for passing to OpenCL.
/// </summary>
/// <param name="carToRas">The CarToRas object to convert</param>
/// <returns>The CarToRasCL object</returns>
template <typename T>
CarToRasCL<T> RendererCL<T>::ConvertCarToRas(const CarToRas<T>& carToRas)
{
	CarToRasCL<T> carToRasCL;

	carToRasCL.m_RasWidth = uint(carToRas.RasWidth());
	carToRasCL.m_PixPerImageUnitW = carToRas.PixPerImageUnitW();
	carToRasCL.m_RasLlX = carToRas.RasLlX();
	carToRasCL.m_PixPerImageUnitH = carToRas.PixPerImageUnitH();
	carToRasCL.m_RasLlY = carToRas.RasLlY();
	carToRasCL.m_CarLlX = carToRas.CarLlX();
	carToRasCL.m_CarLlY = carToRas.CarLlY();
	carToRasCL.m_CarUrX = carToRas.CarUrX();
	carToRasCL.m_CarUrY = carToRas.CarUrY();

	return carToRasCL;
}

/// <summary>
/// Fill seeds buffer which gets passed to the iteration kernel.
/// Note, WriteBuffer() must be called after this to actually copy the
/// data from the host to the device.
/// </summary>
template <typename T>
void RendererCL<T>::FillSeeds()
{
	m_Seeds.resize(IterGridKernelCount());

	for (size_t i = 0; i < m_Seeds.size(); i++)
	{
		m_Seeds[i].x = m_Rand[0].Rand();
		m_Seeds[i].y = m_Rand[0].Rand();
	}
}

template EMBERCL_API class RendererCL<float>;

#ifdef DO_DOUBLE
	template EMBERCL_API class RendererCL<double>;
#endif
}
