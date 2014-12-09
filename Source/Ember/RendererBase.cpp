#include "EmberPch.h"
#include "RendererBase.h"

namespace EmberNs
{
/// <summary>
/// Constructor that sets default values.
/// The thread count is set to the number of cores detected on the system.
/// </summary>
RendererBase::RendererBase()
{
	m_Abort = false;
	m_LockAccum = false;
	m_EarlyClip = false;
	m_YAxisUp = false;
	m_InsertPalette = false;
	m_ReclaimOnResize = false;
	m_NumChannels = 3;
	m_BytesPerChannel = 1;
	m_SuperSize = 0;
	m_Transparency = false;
	ThreadCount(Timing::ProcessorCount());
	m_Callback = nullptr;
	m_ProgressParameter = nullptr;
	m_LastTemporalSample = 0;
	m_LastIter = 0;
	m_LastIterPercent = 0;
	m_InteractiveFilter = FILTER_LOG;
	m_ProcessState = NONE;
	m_ProcessAction = FULL_RENDER;
	m_InRender = false;
	m_InFinalAccum = false;
}

/// <summary>
/// Non-virtual processing functions.
/// </summary>

/// <summary>
/// Abort the render and call a function to do something, most likely change a value.
/// Then update the current process action to the one specified.
/// The current process action will only be set if it makes sense based
/// on the current process state. If the value specified doesn't make sense
/// the next best choice will be made. If nothing makes sense, a complete
/// re-render will be triggered on the next call to Run().
/// </summary>
/// <param name="func">The function to execute</param>
/// <param name="action">The desired process action</param>
void RendererBase::ChangeVal(std::function<void(void)> func, eProcessAction action)
{
	Abort();
	EnterRender();
	func();

	//If they want a full render, don't bother inspecting process state, just start over.
	if (action == FULL_RENDER)
	{
		m_ProcessState = NONE;
		m_ProcessAction = FULL_RENDER;
	}
	//Keep iterating is when rendering has completed and the user increases the quality.
	//Rendering can be started where it left off by adding just the difference between the
	//new and old quality values.
	else if (action == KEEP_ITERATING)
	{
		if (m_ProcessState == ACCUM_DONE && TemporalSamples() == 1)
		{
			m_ProcessState = ITER_STARTED;
			m_ProcessAction = KEEP_ITERATING;
		}
		else//Invaid process state to handle KEEP_ITERATING, so just start over.
		{
			m_ProcessState = NONE;
			m_ProcessAction = FULL_RENDER;
		}
	}
	else if (action == FILTER_AND_ACCUM)
	{
		//If in the middle of a render, cannot skip to filtering or accum, so just start over.
		if (m_ProcessState == NONE || m_ProcessState == ITER_STARTED)
		{
			m_ProcessState = NONE;
			m_ProcessAction = FULL_RENDER;
		}
		//Set the state to ITER_DONE and the next process action to FILTER_AND_ACCUM.
		else
		{
			m_ProcessState = ITER_DONE;
			m_ProcessAction = FILTER_AND_ACCUM;
		}
	}
	//Run accum only.
	else if (action == ACCUM_ONLY)
	{
		//Doesn't make sense if in the middle of iterating, so just start over.
		if (m_ProcessState == NONE || m_ProcessState == ITER_STARTED)
		{
			m_ProcessAction = FULL_RENDER;
		}
		else if (m_ProcessState == ITER_DONE)//If iterating is done, can start at density filtering and proceed.
		{
			m_ProcessAction = FILTER_AND_ACCUM;
		}
		else if (m_ProcessState == FILTER_DONE)//Density filtering is done, so the process action is assigned as desired.
		{
			m_ProcessAction = ACCUM_ONLY;
		}
		else if (m_ProcessState == ACCUM_DONE)//Final accum is done, so back up and run final accum again.
		{
			m_ProcessState = FILTER_DONE;
			m_ProcessAction = ACCUM_ONLY;
		}
	}

	LeaveRender();
}

/// <summary>
/// Return the amount of memory needed for the histogram.
/// </summary>
/// <returns>The memory required for the histogram to render the current ember</returns>
size_t RendererBase::HistMemoryRequired(size_t strips)
{
	bool newFilterAlloc = false;

	CreateSpatialFilter(newFilterAlloc);
	CreateTemporalFilter(newFilterAlloc);
	ComputeBounds();

	//Because ComputeBounds() was called, this includes gutter.
	return (SuperSize() * HistBucketSize()) / strips;
}

/// <summary>
/// Return a pair whose first member contains the amount of memory needed for the histogram,
/// and whose second member contains the total the amount of memory needed to render the current ember.
/// Optionally include the memory needed for the final output image in pair.second.
/// </summary>
/// <param name="includeFinal">If true include the memory needed for the final output image, else don't.</param>
/// <returns>The histogram memory required in first, and the total memory required in second</returns>
pair<size_t, size_t> RendererBase::MemoryRequired(size_t strips, bool includeFinal)
{
	pair<size_t, size_t> p;

	p.first = HistMemoryRequired(strips);
	p.second = (p.first * 2) + (includeFinal ? FinalBufferSize() : 0);//Multiply hist by 2 to account for the density filtering buffer which is the same size as the histogram.

	return p;
}

/// <summary>
/// Get a copy of the vector of random contexts.
/// Useful for debugging because the returned vector can be used for future renders to
/// produce the exact same output.
/// </summary>
/// <returns>The vector of random contexts to assign</returns>
vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> RendererBase::RandVec() { return m_Rand; }

/// <summary>
/// Set the vector of random contexts.
/// Assignment will only take place if the size of the vector matches
/// the number of threads used for rendering.
/// Reset the rendering process.
/// </summary>
/// <param name="randVec">The vector of random contexts to assign</param>
/// <returns>True if the size of the vector matched the number of threads used for rendering, else false.</returns>
bool RendererBase::RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec)
{
	bool b = false;

	if (randVec.size() == ThreadCount())
	{
		ChangeVal([&]
		{
			m_Rand = randVec;
			b = true;
		}, FULL_RENDER);
	}

	return b;
}

/// <summary>
/// Resize the passed in vector to be large enough to handle the output image.
/// If m_ReclaimOnResize is true, and the vector is already larger than needed,
/// it will be shrunk to the needed size. However if m_ReclaimOnResize is false,
/// it will be left alone if already large enough.
/// ComputeBounds() must be called before calling this function.
/// </summary>
/// <param name="pixels">The vector to allocate</param>
/// <returns>True if the vector contains enough space to hold the output image</returns>
bool RendererBase::PrepFinalAccumVector(vector<byte>& pixels)
{
	EnterResize();
	size_t size = FinalBufferSize();

	if (m_ReclaimOnResize)
	{
		if (pixels.size() != size)
		{
			pixels.resize(size);
			pixels.shrink_to_fit();
		}
	}
	else
	{
		if (pixels.size() < size)
			pixels.resize(size);
	}

	LeaveResize();

	return pixels.size() >= size;//Ensure allocation went ok.
}

/// <summary>
/// Virtual processing functions.
/// </summary>

/// <summary>
/// Get a status indicating whether this renderer is ok.
/// Return true for this class, derived classes will inspect GPU hardware
/// to determine if they are ok.
/// </summary>
/// <returns>Always true for this class</returns>
bool RendererBase::Ok() const
{
	return true;
}

/// <summary>
/// The amount of RAM available to render with.
/// </summary>
/// <returns>An unsigned 64-bit integer specifying how much memory is available</returns>
size_t RendererBase::MemoryAvailable()
{
	size_t memAvailable = 0;

#ifdef WIN32

	MEMORYSTATUSEX stat;

	stat.dwLength = sizeof(stat);
	GlobalMemoryStatusEx(&stat);
	memAvailable = stat.ullTotalPhys;

#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)

	memAvailable = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);

#elif defined __APPLE__

#ifdef __LP64__
	long physmem;
	size_t len = sizeof(physmem);
	static int mib[2] = { CTL_HW, HW_MEMSIZE };
#else
	size_t physmem;
	size_t len = sizeof(physmem);
	static int mib[2] = { CTL_HW, HW_PHYSMEM };
#endif

	if (sysctl(mib, 2, &physmem, &len, nullptr, 0) == 0 && len == sizeof(physmem))
	{
		memAvailable = physmem;
	}
	else
	{
		cout << "Warning: unable to determine physical memory." << endl;
		memAvailable = 4e9;
	}

#else

	cout << "Warning: unable to determine physical memory." << endl;
	memAvailable = 4e9;

#endif

	return memAvailable;
}

/// <summary>
/// Non-virtual renderer properties, getters only.
/// </summary>

size_t		   RendererBase::SuperRasW()				   const { return m_SuperRasW; }
size_t		   RendererBase::SuperRasH()				   const { return m_SuperRasH; }
size_t		   RendererBase::SuperSize()				   const { return m_SuperSize; }
size_t		   RendererBase::FinalRowSize()				   const { return FinalRasW() * PixelSize(); }
size_t		   RendererBase::FinalDimensions()			   const { return FinalRasW() * FinalRasH(); }
size_t		   RendererBase::FinalBufferSize()			   const { return FinalRowSize() * FinalRasH(); }
size_t		   RendererBase::PixelSize()				   const { return NumChannels() * BytesPerChannel(); }
size_t		   RendererBase::GutterWidth()				   const { return m_GutterWidth; }
size_t		   RendererBase::DensityFilterOffset()		   const { return m_DensityFilterOffset; }
size_t         RendererBase::TotalIterCount(size_t strips) const { return size_t(size_t(Round(ScaledQuality())) * FinalRasW() * FinalRasH() * strips); }//Use Round() because there can be some roundoff error when interpolating.
size_t         RendererBase::ItersPerTemporalSample()	   const { return size_t(ceil(double(TotalIterCount(1)) / double(TemporalSamples()))); }//Temporal samples is used with animation, which doesn't support strips, so pass 1.
eProcessState  RendererBase::ProcessState()				   const { return m_ProcessState; }
eProcessAction RendererBase::ProcessAction()			   const { return m_ProcessAction; }
EmberStats     RendererBase::Stats()					   const { return m_Stats; }

/// <summary>
/// Non-virtual render properties, getters and setters.
/// </summary>

/// <summary>
/// Get whether the histogram is locked during accumulation.
/// This is to prevent two threads from writing to the same histogram
/// bucket at once.
/// The current implementation matches flam3 and is very innefficient
/// to the point of negating any gains gotten from multi-threading.
/// Future workarounds may be tried in the future.
/// Default: false.
/// </summary>
/// <returns>True if the histogram is locked during accumulation, else false.</returns>
bool RendererBase::LockAccum() const { return m_LockAccum; }

/// <summary>
/// Set whether the histogram is locked during accumulation.
/// This is to prevent two threads from writing to the same histogram
/// bucket at once.
/// The current implementation matches flam3 and is very innefficient
/// to the point of negating any gains gotten from multi-threading.
/// Different workarounds may be tried in the future.
/// Reset the rendering process.
/// </summary>
/// <param name="lockAccum">True if the histogram should be locked when accumulating, else false</param>
void RendererBase::LockAccum(bool lockAccum)
{
	ChangeVal([&] { m_LockAccum = lockAccum; }, FULL_RENDER);
}

/// <summary>
/// Get whether color clipping and gamma correction is done before
/// or after spatial filtering.
/// Default: false.
/// </summary>
/// <returns>True if early clip, else false.</returns>
bool RendererBase::EarlyClip() const { return m_EarlyClip; }

/// <summary>
/// Set whether color clipping and gamma correction is done before
/// or after spatial filtering.
/// Set the render state to FILTER_AND_ACCUM.
/// </summary>
/// <param name="earlyClip">True if early clip, else false.</param>
void RendererBase::EarlyClip(bool earlyClip)
{
	ChangeVal([&] { m_EarlyClip = earlyClip; }, FILTER_AND_ACCUM);
}

/// <summary>
/// Get whether the positive Y coordinate of the final output image is up.
/// Default: false.
/// </summary>
/// <returns>True if up, else false.</returns>
bool RendererBase::YAxisUp() const { return m_YAxisUp; }

/// <summary>
/// Set whether the positive Y axis of the final output image is up.
/// </summary>
/// <param name="yup">True if the positive y axis is up, else false.</param>
void RendererBase::YAxisUp(bool yup)
{
	ChangeVal([&] { m_YAxisUp = yup; }, ACCUM_ONLY);
}

/// <summary>
/// Get whether to insert the palette as a block of colors in the final output image.
/// This is useful for debugging palette issues.
/// Default: 1.
/// </summary>
/// <returns>True if inserting the palette, else false.</returns>
bool RendererBase::InsertPalette() const { return m_InsertPalette; }

/// <summary>
/// Set whether to insert the palette as a block of colors in the final output image.
/// This is useful for debugging palette issues.
/// Set the render state to ACCUM_ONLY.
/// </summary>
/// <param name="insertPalette">True if inserting the palette, else false.</param>
void RendererBase::InsertPalette(bool insertPalette)
{
	ChangeVal([&] { m_InsertPalette = insertPalette; }, ACCUM_ONLY);
}

/// <summary>
/// Get whether to reclaim unused memory in the final output buffer
/// when a smaller size is requested than has been previously allocated.
/// Default: false.
/// </summary>
/// <returns>True if reclaim, else false.</returns>
bool RendererBase::ReclaimOnResize() const { return m_ReclaimOnResize; }

/// <summary>
/// Set whether to reclaim unused memory in the final output buffer
/// when a smaller size is requested than has been previously allocated.
/// Reset the rendering process.
/// </summary>
/// <param name="reclaimOnResize">True if reclaim, else false.</param>
void RendererBase::ReclaimOnResize(bool reclaimOnResize)
{
	ChangeVal([&] { m_ReclaimOnResize = reclaimOnResize; }, FULL_RENDER);
}

/// <summary>
/// Get whether to use transparency in the alpha channel.
/// This only applies when the number of channels is 4 and the output
/// image is Png.
/// Default: false.
/// </summary>
/// <returns>True if using transparency, else false.</returns>
bool RendererBase::Transparency() const { return m_Transparency; }

/// <summary>
/// Set whether to use transparency in the alpha channel.
/// This only applies when the number of channels is 4 and the output
/// image is Png.
/// Set the render state to ACCUM_ONLY.
/// </summary>
/// <param name="transparency">True if using transparency, else false.</param>
void RendererBase::Transparency(bool transparency)
{
	ChangeVal([&] { m_Transparency = transparency; }, ACCUM_ONLY);
}

/// <summary>
/// Set the callback object.
/// </summary>
/// <param name="callback">The callback object to set</param>
void RendererBase::Callback(RenderCallback* callback)
{
	m_Callback = callback;
}

/// <summary>
/// Set the number of threads to use when rendering.
/// This will also reset the vector of random contexts to be the same size
/// as the number of specified threads.
/// Since this is where they get set up, the caller can optionally pass in
/// a seed string, however it's only used if threads is 1.
/// This is useful for debugging since it will run the same point trajectory
/// every time.
/// Reset the rendering process.
/// </summary>
/// <param name="threads">The number of threads to use</param>
/// <param name="seedString">The seed string to use if threads is 1. Default: nullptr.</param>
void RendererBase::ThreadCount(size_t threads, const char* seedString)
{
	ChangeVal([&]
	{
		Timing t;
		size_t i, size;
		const size_t isaacSize = 1 << ISAAC_SIZE;
		ISAAC_INT seeds[isaacSize];
		m_ThreadsToUse = threads > 0 ? threads : 1;
		m_Rand.clear();
		m_SubBatch.clear();
		m_SubBatch.resize(m_ThreadsToUse);
		m_BadVals.resize(m_ThreadsToUse);

		if (seedString)
		{
			memset(seeds, 0, isaacSize * sizeof(ISAAC_INT));
			memcpy(reinterpret_cast<char*>(seeds), seedString, min(strlen(seedString), isaacSize * sizeof(ISAAC_INT)));
		}

		//This is critical for multithreading, otherwise the threads all happen
		//too close to each other in time, resulting in bad randomization.
		while (m_Rand.size() < m_ThreadsToUse)
		{
			size = m_Rand.size();

			if (seedString)
			{
				ISAAC_INT newSize = ISAAC_INT(size + 5 + (t.Toc() + t.EndTime()));

#ifdef ISAAC_FLAM3_DEBUG
				QTIsaac<ISAAC_SIZE, ISAAC_INT> isaac(0, 0, 0, seeds);
#else
				QTIsaac<ISAAC_SIZE, ISAAC_INT> isaac(newSize, newSize * 2, newSize * 3, seeds);
#endif
				m_Rand.push_back(isaac);

				for (i = 0; i < (isaacSize * sizeof(ISAAC_INT)); i++)
					reinterpret_cast<byte*>(seeds)[i]++;
			}
			else
			{
				for (i = 0; i < isaacSize; i++)
				{
					t.Toc();
					seeds[i] = ISAAC_INT((t.EndTime() * i) + (size + 1));
				}

				t.Toc();
				ISAAC_INT r = ISAAC_INT((size * i) + i + t.EndTime());
				QTIsaac<ISAAC_SIZE, ISAAC_INT> isaac(r, r * 2, r * 3, seeds);

				m_Rand.push_back(isaac);
			}
		}
	}, FULL_RENDER);
}

/// <summary>
/// Get the bytes per channel of the output image.
/// The only acceptable values are 1 and 2, and 2 is only
/// used when the output is Png.
/// Default: 1.
/// </summary>
/// <returns></returns>
size_t RendererBase::BytesPerChannel() const { return m_BytesPerChannel; }

/// <summary>
/// Set the bytes per channel of the output image.
/// The only acceptable values are 1 and 2, and 2 is only
/// used when the output is Png.
/// Set the render state to ACCUM_ONLY.
/// </summary>
/// <param name="bytesPerChannel">The bytes per channel.</param>
void RendererBase::BytesPerChannel(size_t bytesPerChannel)
{
	ChangeVal([&]
	{
		if (bytesPerChannel == 0 || bytesPerChannel > 2)
			m_BytesPerChannel = 1;
		else
			m_BytesPerChannel = bytesPerChannel;
	}, ACCUM_ONLY);
}

/// <summary>
/// Get the number of channels per pixel in the output image. 3 for RGB images
/// like Bitmap and Jpeg, 4 for Png.
/// Default is 3.
/// </summary>
/// <returns>The number of channels per pixel in the output image</returns>
size_t RendererBase::NumChannels() const { return m_NumChannels; }

/// <summary>
/// Get the type of filter to use for preview renders during interactive rendering.
/// Using basic log scaling is quicker, but doesn't provide any bluring.
/// Full DE is much slower, but provides a more realistic preview of what the final image
/// will look like.
/// Default: FILTER_LOG.
/// </summary>
/// <returns>The type of filter to use</returns>
eInteractiveFilter RendererBase::InteractiveFilter() const { return m_InteractiveFilter; }

/// <summary>
/// Set the type of filter to use for preview renders during interactive rendering.
/// Using basic log scaling is quicker, but doesn't provide any bluring.
/// Full DE is much slower, but provides a more realistic preview of what the final image
/// will look like.
/// Reset the rendering process.
/// </summary>
/// <param name="filter">The filter.</param>
void RendererBase::InteractiveFilter(eInteractiveFilter filter)
{
	ChangeVal([&] { m_InteractiveFilter = filter; }, FULL_RENDER);
}

/// <summary>
/// Virtual render properties, getters and setters.
/// </summary>

/// <summary>
/// Set the number of channels per pixel in the output image. 3 for RGB images
/// like Bitmap and Jpeg, 4 for Png.
/// Default is 3.
/// Set the render state to ACCUM_ONLY.
/// </summary>
/// <param name="numChannels">The number of channels per pixel in the output image</param>
void RendererBase::NumChannels(size_t numChannels)
{
	ChangeVal([&] { m_NumChannels = numChannels; }, ACCUM_ONLY);
}

/// <summary>
/// Get the number of threads used when rendering.
/// Default: use all avaliable cores.
/// </summary>
/// <returns>The number of threads used when rendering</returns>
size_t RendererBase::ThreadCount() const { return m_ThreadsToUse; }

/// <summary>
/// Get the renderer type enum.
/// CPU_RENDERER for this class, other values for derived classes.
/// </summary>
/// <returns>CPU_RENDERER</returns>
eRendererType RendererBase::RendererType() const { return CPU_RENDERER; }

/// <summary>
/// //Non-virtual threading control.
/// </summary>

/// <summary>
/// Stop rendering, ensure all locks are exited and reset the rendering state.
/// </summary>
void RendererBase::Reset()
{
	Abort();
	EnterRender();
	EnterFinalAccum();
	LeaveFinalAccum();
	LeaveRender();
	m_ProcessState = NONE;
	m_ProcessAction = FULL_RENDER;
}

void RendererBase::EnterRender() { m_RenderingCs.Enter(); }
void RendererBase::LeaveRender() { m_RenderingCs.Leave(); }

void RendererBase::EnterFinalAccum() { m_FinalAccumCs.Enter(); m_InFinalAccum = true; }
void RendererBase::LeaveFinalAccum() { m_FinalAccumCs.Leave(); m_InFinalAccum = false; }

void RendererBase::EnterResize() { m_ResizeCs.Enter(); }
void RendererBase::LeaveResize() { m_ResizeCs.Leave(); }

void RendererBase::Abort()   { m_Abort = true; }
bool RendererBase::Aborted() { return m_Abort; }

bool RendererBase::InRender()	  { return m_InRender; }
bool RendererBase::InFinalAccum() { return m_InFinalAccum; }

}
