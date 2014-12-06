#include "EmberCLPch.h"
#include "DEOpenCLKernelCreator.h"

namespace EmberCLns
{
/// <summary>
/// Empty constructor that does nothing. The user must call the one which takes a bool
/// argument before using this class.
/// This constructor only exists so the class can be a member of a class.
/// </summary>
template <typename T>
DEOpenCLKernelCreator<T>::DEOpenCLKernelCreator()
{
}

/// <summary>
/// Constructor for float template type that sets all kernel entry points as well as composes
/// all kernel source strings.
/// No program compilation is done here, the user must explicitly do it.
/// The caller must specify whether they are using an nVidia or AMD card because it changes
/// the amount of local memory available.
/// </summary>
/// <param name="nVidia">True if running on an nVidia card, else false.</param>
template <>
DEOpenCLKernelCreator<float>::DEOpenCLKernelCreator(bool nVidia)
{
	m_NVidia = nVidia;
	m_LogScaleAssignDEEntryPoint              = "LogScaleAssignDensityFilterKernel";
	m_GaussianDEWithoutSsEntryPoint           = "GaussianDEWithoutSsKernel";
	m_GaussianDESsWithScfEntryPoint           = "GaussianDESsWithScfKernel";
	m_GaussianDESsWithoutScfEntryPoint        = "GaussianDESsWithoutScfKernel";
	m_GaussianDEWithoutSsNoCacheEntryPoint    = "GaussianDEWithoutSsNoCacheKernel";
	m_GaussianDESsWithScfNoCacheEntryPoint    = "GaussianDESsWithScfNoCacheKernel";
	m_GaussianDESsWithoutScfNoCacheEntryPoint = "GaussianDESsWithoutScfNoCacheKernel";
	m_LogScaleAssignDEKernel                  = CreateLogScaleAssignDEKernelString();
	m_GaussianDEWithoutSsKernel               = CreateGaussianDEKernel(1);
	m_GaussianDESsWithScfKernel               = CreateGaussianDEKernel(2);
	m_GaussianDESsWithoutScfKernel            = CreateGaussianDEKernel(3);
	m_GaussianDEWithoutSsNoCacheKernel        = CreateGaussianDEKernelNoLocalCache(1);
	m_GaussianDESsWithScfNoCacheKernel        = CreateGaussianDEKernelNoLocalCache(2);
	m_GaussianDESsWithoutScfNoCacheKernel     = CreateGaussianDEKernelNoLocalCache(3);
}

/// <summary>
/// Constructor for double template type that sets all kernel entry points as well as composes
/// all kernel source strings.
/// Note that no versions of kernels that use the cache are compiled because
/// the cache is not big enough to hold double4.
/// No program compilation is done here, the user must explicitly do it.
/// Specifying true or false for the bool parameter has no effect since no local memory
/// is used when instantiated with type double.
/// </summary>
/// <param name="nVidia">True if running on an nVidia card, else false. Ignored.</param>
template <>
DEOpenCLKernelCreator<double>::DEOpenCLKernelCreator(bool nVidia)
{
#ifdef ROW_ONLY_DE
	m_NVidia = nVidia;
	m_LogScaleAssignDEEntryPoint = "LogScaleAssignDensityFilterKernel";
	m_GaussianDEWithoutSsEntryPoint = "GaussianDEWithoutSsKernel";
	m_GaussianDESsWithScfEntryPoint = "GaussianDESsWithScfKernel";
	m_GaussianDESsWithoutScfEntryPoint = "GaussianDESsWithoutScfKernel";
	m_GaussianDEWithoutSsNoCacheEntryPoint = "GaussianDEWithoutSsNoCacheKernel";
	m_GaussianDESsWithScfNoCacheEntryPoint = "GaussianDESsWithScfNoCacheKernel";
	m_GaussianDESsWithoutScfNoCacheEntryPoint = "GaussianDESsWithoutScfNoCacheKernel";
	m_LogScaleAssignDEKernel = CreateLogScaleAssignDEKernelString();
	m_GaussianDEWithoutSsKernel = CreateGaussianDEKernel(1);
	m_GaussianDESsWithScfKernel = CreateGaussianDEKernel(2);
	m_GaussianDESsWithoutScfKernel = CreateGaussianDEKernel(3);
	m_GaussianDEWithoutSsNoCacheKernel = CreateGaussianDEKernelNoLocalCache(1);
	m_GaussianDESsWithScfNoCacheKernel = CreateGaussianDEKernelNoLocalCache(2);
	m_GaussianDESsWithoutScfNoCacheKernel = CreateGaussianDEKernelNoLocalCache(3);
#else
	m_NVidia = nVidia;
	m_LogScaleAssignDEEntryPoint              = "LogScaleAssignDensityFilterKernel";
	m_GaussianDEWithoutSsNoCacheEntryPoint    = "GaussianDEWithoutSsNoCacheKernel";
	m_GaussianDESsWithScfNoCacheEntryPoint    = "GaussianDESsWithScfNoCacheKernel";
	m_GaussianDESsWithoutScfNoCacheEntryPoint = "GaussianDESsWithoutScfNoCacheKernel";
	m_LogScaleAssignDEKernel                  = CreateLogScaleAssignDEKernelString();
	m_GaussianDEWithoutSsNoCacheKernel        = CreateGaussianDEKernelNoLocalCache(1);
	m_GaussianDESsWithScfNoCacheKernel        = CreateGaussianDEKernelNoLocalCache(2);
	m_GaussianDESsWithoutScfNoCacheKernel     = CreateGaussianDEKernelNoLocalCache(3);
#endif
}

/// <summary>
/// Kernel source and entry point properties, getters only.
/// </summary>

template <typename T> string DEOpenCLKernelCreator<T>::LogScaleAssignDEKernel() { return m_LogScaleAssignDEKernel; }
template <typename T> string DEOpenCLKernelCreator<T>::LogScaleAssignDEEntryPoint() { return m_LogScaleAssignDEEntryPoint; }

/// <summary>
/// Get the kernel source for the specified supersample and filterWidth.
/// </summary>
/// <param name="ss">The supersample being used</param>
/// <param name="filterWidth">Filter width</param>
/// <returns>The kernel source</returns>
template <typename T>
string DEOpenCLKernelCreator<T>::GaussianDEKernel(size_t ss, uint filterWidth)
{
#ifndef ROW_ONLY_DE
	if ((typeid(T) == typeid(double)) || (filterWidth > MaxDEFilterSize()))//Type double does not use cache.
	{
		if (ss > 1)
		{
			if (!(ss & 1))
				return m_GaussianDESsWithScfNoCacheKernel;
			else
				return m_GaussianDESsWithoutScfNoCacheKernel;
		}
		else
			return m_GaussianDEWithoutSsNoCacheKernel;
	}
	else
#endif
	{
		if (ss > 1)
		{
			if (!(ss & 1))
				return m_GaussianDESsWithScfKernel;
			else
				return m_GaussianDESsWithoutScfKernel;
		}
		else
			return m_GaussianDEWithoutSsKernel;
	}
}

/// <summary>
/// Get the kernel entry point for the specified supersample and filterWidth.
/// </summary>
/// <param name="ss">The supersample being used</param>
/// <param name="filterWidth">Filter width</param>
/// <returns>The name of the density estimation filtering entry point kernel function</returns>
template <typename T>
string DEOpenCLKernelCreator<T>::GaussianDEEntryPoint(size_t ss, uint filterWidth)
{
#ifndef ROW_ONLY_DE
	if ((typeid(T) == typeid(double)) || (filterWidth > MaxDEFilterSize()))//Type double does not use cache.
	{
		if (ss > 1)
		{
			if (!(ss & 1))
				return m_GaussianDESsWithScfNoCacheEntryPoint;
			else
				return m_GaussianDESsWithoutScfNoCacheEntryPoint;
		}
		else
			return m_GaussianDEWithoutSsNoCacheEntryPoint;
	}
	else
#endif
	{
		if (ss > 1)
		{
			if (!(ss & 1))
				return m_GaussianDESsWithScfEntryPoint;
			else
				return m_GaussianDESsWithoutScfEntryPoint;
		}
		else
			return m_GaussianDEWithoutSsEntryPoint;
	}
}

/// <summary>
/// Get the maximum filter size allowed for running the local memory version of density filtering
/// Filters larger than this value will run the version without local memory caching.
/// </summary>
/// <returns>The maximum filter size allowed for running the local memory version of density filtering</returns>
template <typename T>
uint DEOpenCLKernelCreator<T>::MaxDEFilterSize() { return 9; }//The true max would be (maxBoxSize - 1) / 2, but that's impractical because it can give us a tiny block size.

/// <summary>
/// Solve for the maximum filter radius.
/// The final filter width is calculated by: (uint)(ceil(m_MaxRad) * (T)m_Supersample) + (m_Supersample - 1);
/// Must solve for what max rad should be in order to give a maximum final width of (maxBoxSize - 1) / 2, assuming
/// a minimum block size of 1 which processes 1 pixel.
/// Example: If a box size of 20 was allowed, a filter
/// size of up to 9: (20 - 1) / 2 == (19 / 2) == 9 could be supported.
/// This function is deprecated, the appropriate kernels take care of this problem now.
/// </summary>
/// <param name="maxBoxSize">Maximum size of the box.</param>
/// <param name="desiredFilterSize">Size of the desired filter.</param>
/// <param name="ss">The supersample being used</param>
/// <returns>The maximum filter radius allowed</returns>
template <typename T>
T DEOpenCLKernelCreator<T>::SolveMaxDERad(uint maxBoxSize, T desiredFilterSize, T ss)
{
	uint finalFilterSize = (uint)((ceil(desiredFilterSize) * ss) + (ss - 1.0));

	//Return the desired size if the final size of it will fit.
	if (finalFilterSize <= MaxDEFilterSize())
		return desiredFilterSize;

	//The final size doesn't fit, so scale the original down until it fits.
	return (T)floor((MaxDEFilterSize() - (ss - 1.0)) / ss);
}

/// <summary>
/// Determine the maximum filter box size based on the amount of local memory available
/// to each block.
/// </summary>
/// <param name="localMem">The local memory available to a block</param>
/// <returns>The maximum filter box size allowed</returns>
template <typename T>
uint DEOpenCLKernelCreator<T>::SolveMaxBoxSize(uint localMem)
{
	return (uint)floor(sqrt(floor((T)localMem / 16.0)));//Divide by 16 because each element is float4.
}

/// <summary>
/// Create the log scale kernel string, using assignment.
/// Use this when Passes == 1.
/// </summary>
/// <returns>The kernel string</returns>
template <typename T>
string DEOpenCLKernelCreator<T>::CreateLogScaleAssignDEKernelString()
{
	ostringstream os;

	os	<<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		DensityFilterCLStructString <<
		"__kernel void " << m_LogScaleAssignDEEntryPoint << "(\n"
		"	const __global real4* histogram,\n"
		"	__global real4* accumulator,\n"
		"	__constant DensityFilterCL* logFilter\n"
		"\t)\n"
		"{\n"
		"	if ((GLOBAL_ID_X < logFilter->m_SuperRasW) && (GLOBAL_ID_Y < logFilter->m_SuperRasH))\n"
		"	{\n"
		"		uint index = (GLOBAL_ID_Y * logFilter->m_SuperRasW) + GLOBAL_ID_X;\n"
		"\n"
		"		if (histogram[index].w != 0)\n"
		"		{\n"
		"			real_t logScale = (logFilter->m_K1 * log(1.0 + histogram[index].w * logFilter->m_K2)) / histogram[index].w;\n"
		"\n"
		"			accumulator[index] = histogram[index] * logScale;\n"//Using a single real4 vector operation doubles the speed from doing each component individually.
		"		}\n"
		"\n"
		"		barrier(CLK_GLOBAL_MEM_FENCE);\n"//Just to be safe. Makes no speed difference to do all of the time or only when there's a hit.
		"	}\n"
		"}\n";

	return os.str();
}

#ifdef ROW_ONLY_DE
template <typename T>
string DEOpenCLKernelCreator<T>::CreateGaussianDEKernel(size_t ss)
{
	bool doSS = ss > 1;
	bool doScf = !(ss & 1);
	ostringstream os;

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		DensityFilterCLStructString <<
		UnionCLStructString <<
		"__kernel void " << GaussianDEEntryPoint(ss, MaxDEFilterSize()) << "(\n" <<
		"	const __global real4* histogram,\n"
		"	__global real4reals* accumulator,\n"
		"	__constant DensityFilterCL* densityFilter,\n"
		"	const __global real_t* filterCoefs,\n"
		"	const __global real_t* filterWidths,\n"
		"	const __global uint* coefIndices,\n"
		"	const uint chunkSizeW,\n"
		"	const uint chunkSizeH,\n"
		"	const uint chunkW,\n"
		"	const uint chunkH\n"
		"\t)\n"
		"{\n"
		"	uint rowsToProcess = 32;\n"//Rows to process.
		"\n"
		"	if (((((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X) + THREAD_ID_X >= densityFilter->m_SuperRasW) ||\n"
		"	    ((((BLOCK_ID_Y * chunkSizeH) + chunkH) * rowsToProcess) + THREAD_ID_Y >= densityFilter->m_SuperRasH))\n"
		"			return;\n"
		"\n";

	if (doSS)
	{
		os <<
			"	uint ss = (uint)floor((real_t)densityFilter->m_Supersample / 2.0);\n"
			"	int densityBoxLeftX;\n"
			"	int densityBoxRightX;\n"
			"	int densityBoxTopY;\n"
			"	int densityBoxBottomY;\n"
			"\n";

		if (doScf)
			os <<
			"	real_t scfact = pow(densityFilter->m_Supersample / (densityFilter->m_Supersample + 1.0), 2.0);\n";
	}

	os <<
		"	uint fullTempBoxWidth;\n"
		"	uint leftBound, rightBound, topBound, botBound;\n"
		"	uint blockHistStartRow, blockHistEndRow, histCol;\n"
		"	uint blockHistStartCol, boxReadStartCol, boxReadEndCol;\n"
		"	uint accumWriteStartCol, colsToWrite, colOffset, colsToWriteOffset;\n"
		"	int histRow, filterRow, accumWriteOffset;\n"
		"\n"
		"	fullTempBoxWidth = BLOCK_SIZE_X + (densityFilter->m_FilterWidth * 2);\n"
		//Compute the bounds of the area to be sampled, which is just the ends minus the super sample minus 1.
		"	leftBound = densityFilter->m_Supersample - 1;\n"
		"	rightBound = densityFilter->m_SuperRasW - (densityFilter->m_Supersample - 1);\n"
		"	topBound = densityFilter->m_Supersample - 1;\n"
		"	botBound = densityFilter->m_SuperRasH - (densityFilter->m_Supersample - 1);\n"
		"\n"
		//Start and end values are the indices in the histogram read from
		//and written to in the accumulator. They are not the indices for the local block of data.
		//Before computing local offsets, compute the global offsets first to determine if any rows or cols fall outside of the bounds.
		"	blockHistStartRow = min(botBound, topBound + (((BLOCK_ID_Y * chunkSizeH) + chunkH) * rowsToProcess));\n"//The first histogram row this block will process.
		"	blockHistEndRow = min(botBound, blockHistStartRow + rowsToProcess);\n"//The last histogram row this block will process, clamped to the last row.
		"	blockHistStartCol = min(rightBound, leftBound + (((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X));\n"//The first histogram column this block will process.
		"	boxReadStartCol = densityFilter->m_FilterWidth - min(densityFilter->m_FilterWidth, blockHistStartCol);\n"//The first box col this block will read from when copying to the accumulator.
		"	boxReadEndCol = densityFilter->m_FilterWidth + min(densityFilter->m_FilterWidth + BLOCK_SIZE_X, densityFilter->m_SuperRasW - blockHistStartCol);\n"//The last box col this block will read from when copying to the accumulator.
		"\n"
		//Last, the indices in the global accumulator that the local bounds will be writing to.
		"	accumWriteStartCol = blockHistStartCol - min(densityFilter->m_FilterWidth, blockHistStartCol);\n"//The first column in the accumulator this block will write to.
		"	colsToWrite = ceil((real_t)(boxReadEndCol - boxReadStartCol) / (real_t)BLOCK_SIZE_X);\n"//Elements per thread to be written to the accumulator.
		"	histCol = blockHistStartCol + THREAD_ID_X;\n"//The histogram column this individual thread will be reading from.
		"\n"
		"	if (histCol >= rightBound)\n"
		"		return;\n"
		"\n"
		//Compute the col position in this local box to serve as the center position
		//from which filter application offsets are computed.
		//These are the local indices for the local data that are temporarily accumulated to before
		//writing out to the global accumulator.
		"	uint boxCol = densityFilter->m_FilterWidth + THREAD_ID_X;\n"
		"	uint colsToZeroOffset, colsToZero = ceil((real_t)fullTempBoxWidth / (real_t)(BLOCK_SIZE_X));\n"//Usually is 2.
		"	int i, j, k, jmin, jmax;\n"
		"	uint filterSelectInt, filterCoefIndex;\n"
		"	real_t cacheLog;\n"
		"	real_t filterSelect;\n"
		"	real4 bucket;\n"
		;

	os << " __local real4reals filterBox[192];\n";//Must be >= fullTempBoxWidth.

	os <<
		"\n"
		"	colsToZeroOffset = colsToZero * THREAD_ID_X;\n"
		"	colsToWriteOffset = colsToWrite * THREAD_ID_X;\n"
		"	k = (int)densityFilter->m_FilterWidth;\n"//Need a signed int to use below, really is filter width, but reusing a variable to save space.
		"\n"
		"	for (histRow = blockHistStartRow; histRow < blockHistEndRow; histRow++)\n"//Process pixels by row, for 32 rows.
		"	{\n"
		"		bucket = histogram[(histRow * densityFilter->m_SuperRasW) + histCol];\n"
		"\n"
		"		if (bucket.w != 0)\n"
		"			cacheLog = (densityFilter->m_K1 * log(1.0 + bucket.w * densityFilter->m_K2)) / bucket.w;\n"
		"\n";

	if (doSS)
	{
		os <<
			"	filterSelect = 0;\n"
			"	densityBoxLeftX = histCol - min(histCol, ss);\n"
			"	densityBoxRightX = histCol + min(ss, (densityFilter->m_SuperRasW - histCol) - 1);\n"
			"	densityBoxTopY = histRow - min((uint)histRow, ss);\n"
			"	densityBoxBottomY = histRow + min(ss, (densityFilter->m_SuperRasH - histRow) - 1);\n"
			"\n"
			"	for (j = densityBoxTopY; j <= densityBoxBottomY; j++)\n"
			"	{\n"
			"		for (i = densityBoxLeftX; i <= densityBoxRightX; i++)\n"
			"		{\n"
			"			filterSelect += histogram[(j * densityFilter->m_SuperRasW) + i].w;\n"
			"		}\n"
			"	}\n"
			"\n";

	if (doScf)
		os << "	filterSelect *= scfact;\n";
	}
	else
	{
	os
		<< "	filterSelect = bucket.w;\n";
	}

	os <<
		"\n"
		"		if (filterSelect > densityFilter->m_MaxFilteredCounts)\n"
		"			filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"		else if (filterSelect <= DE_THRESH)\n"
		"			filterSelectInt = (int)ceil(filterSelect) - 1;\n"
		"		else if (filterSelect != 0)\n"
		"			filterSelectInt = (int)DE_THRESH + (int)floor(pow((real_t)(filterSelect - DE_THRESH), densityFilter->m_Curve));\n"
		"		else\n"
		"			filterSelectInt = 0;\n"
		"\n"
		"		if (filterSelectInt > densityFilter->m_MaxFilterIndex)\n"
		"			filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"\n"
		"		filterCoefIndex = filterSelectInt * densityFilter->m_KernelSize;\n"
		"\n"
		//With this new method, only accumulate to the temp local buffer first. Write to the final accumulator last.
		//For each loop through, note that there is a local memory barrier call inside of each call to AddToAccumNoCheck().
		//If this isn't done, pixel errors occurr and even an out of resources error occurrs because too many writes are done to the same place in memory at once.
		"		jmin = min(k, histRow);\n"
		"		jmax = (int)min((densityFilter->m_SuperRasH - 1) - histRow, densityFilter->m_FilterWidth);\n"
		"\n"
		"		for (j = -jmin; j <= jmax; j++)\n"
		"		{\n"
		"			for (i = 0; i < colsToZero && (colsToZeroOffset + i) < fullTempBoxWidth; i++)\n"//Each thread zeroizes a few columns.
		"			{\n"
		"				filterBox[colsToZeroOffset + i].m_Real4 = 0;\n"
		"			}\n"
		"\n"
		"			barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"
		"			if (bucket.w != 0)\n"
		"			{\n"
		"				filterRow = abs(j) * (densityFilter->m_FilterWidth + 1);\n"
		"\n"
		"				for (i = -k; i <= k; i++)\n"
		"				{\n"
		"					filterSelectInt = filterCoefIndex + coefIndices[filterRow + abs(i)];\n"//Really is filterCoeffIndexPlusOffset, but reusing a variable to save space.
		"					filterBox[i + boxCol].m_Real4 += (bucket * (filterCoefs[filterSelectInt] * cacheLog));\n"
		"				}\n"
		"			}\n"
		"\n"
		"			barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"
		//At this point, all threads in this block have applied the filter to their surrounding pixels and stored the results in the temp local box.
		//Add the cells of it that are in bounds to the global accumulator.
		//Compute offsets in local box to read from, and offsets into global accumulator to write to.
		//Use a method here that is similar to the zeroization above: Each thread (column) in the first row iterates through all of the
		//rows and adds a few columns to the accumulator.
		//"			if (THREAD_ID_X == 0)\n"
		//"			{\n"
		//"				for (int kk = boxReadStartCol, i = 0; kk < boxReadEndCol; kk++, i++)\n"//Each thread writes a few columns.//Could do away with kk//TODO//OPT
		//"				{\n"
		//"					accumulator[((histRow + j) * densityFilter->m_SuperRasW) + (accumWriteStartCol + i)].m_Real4 += filterBox[kk].m_Real4;\n"
		//"				}\n"
		//"			}\n"
		"			accumWriteOffset = ((histRow + j) * densityFilter->m_SuperRasW) + accumWriteStartCol;\n"
		"\n"
		"			for (i = 0; i < colsToWrite; i++)\n"//Each thread writes a few columns.
		"			{\n"
		"				colOffset = colsToWriteOffset + i;\n"
		"\n"
		"				if (boxReadStartCol + colOffset < boxReadEndCol)\n"
		"					accumulator[accumWriteOffset + colOffset].m_Real4 += filterBox[boxReadStartCol + colOffset].m_Real4;\n"
		"			}\n"
		"		}\n"//for() filter rows.
		"		barrier(CLK_GLOBAL_MEM_FENCE);\n"
		"	}\n"//for() histogram rows.
		"}\n";

	return os.str();
}

#else
/// <summary>
/// Create the gaussian density filtering kernel string.
/// 6 different methods of processing were tried before settling on this final and fastest 7th one.
/// Each block processes a box and exits. No column or row advancements happen.
/// The block accumulates to a temporary box and writes the contents to the global density filter buffer when done.
/// Note this applies the filter from top to bottom row and not from the center outward like the CPU version does.
/// This allows the image to be filtered without suffering from pixel loss due to race conditions.
/// It is run in multiple passes that are spaced far enough apart on the image so as to not overlap.
/// This allows writing to the global buffer without ever overlapping or using atomics.
/// The supersample parameter will produce three different kernels.
/// SS = 1, SS > 1 && SS even, SS > 1 && SS odd.
/// The width of the kernel this runs in must be evenly divisible by 16 or else artifacts will occur.
/// Note that because this function uses so many variables and is so complex, OpenCL can easily run
/// out of resources in some cases. Certain variables had to be reused to condense the kernel footprint
/// down enough to be able to run a block size of 32x32.
/// For double precision, or for SS > 1, a size of 32x30 is used.
/// Box width = (BLOCK_SIZE_X + (fw * 2)).
/// Box height = (BLOCK_SIZE_Y + (fw * 2)).
/// </summary>
/// <param name="ss">The supersample being used</param>
/// <returns>The kernel string</returns>
template <typename T>
string DEOpenCLKernelCreator<T>::CreateGaussianDEKernel(size_t ss)
{
	bool doSS = ss > 1;
	bool doScf = !(ss & 1);
	ostringstream os;

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		DensityFilterCLStructString <<
		UnionCLStructString <<
		"__kernel void " << GaussianDEEntryPoint(ss, MaxDEFilterSize()) << "(\n" <<
		"	const __global real4* histogram,\n"
		"	__global real4reals* accumulator,\n"
		"	__constant DensityFilterCL* densityFilter,\n"
		"	const __global real_t* filterCoefs,\n"
		"	const __global real_t* filterWidths,\n"
		"	const __global uint* coefIndices,\n"
		"	const uint chunkSizeW,\n"
		"	const uint chunkSizeH,\n"
		"	const uint chunkW,\n"
		"	const uint chunkH\n"
		"\t)\n"
		"{\n"
		"	if (((((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X) + THREAD_ID_X >= densityFilter->m_SuperRasW) ||\n"
		"	    ((((BLOCK_ID_Y * chunkSizeH) + chunkH) * BLOCK_SIZE_Y) + THREAD_ID_Y >= densityFilter->m_SuperRasH))\n"
		"			return;\n"
		"\n";

	if (doSS)
	{
		os <<
		"	uint ss = (uint)floor((real_t)densityFilter->m_Supersample / 2.0);\n"
		"	int densityBoxLeftX;\n"
		"	int densityBoxRightX;\n"
		"	int densityBoxTopY;\n"
		"	int densityBoxBottomY;\n"
		"\n";

		if (doScf)
		os <<
		"	real_t scfact = pow(densityFilter->m_Supersample / (densityFilter->m_Supersample + 1.0), 2.0);\n";
	}

	//Compute the size of the temporary box which is the block width + 2 * filter width x block height + 2 * filter width.
	//Ideally the block width and height are both 32. However, the height might be smaller if there isn't enough memory.
	os <<
		"	uint fullTempBoxWidth, fullTempBoxHeight;\n"
		"	uint leftBound, rightBound, topBound, botBound;\n"
		"	uint blockHistStartRow, blockHistEndRow, boxReadStartRow, boxReadEndRow;\n"
		"	uint blockHistStartCol, boxReadStartCol, boxReadEndCol;\n"
		"	uint accumWriteStartRow, accumWriteStartCol, colsToWrite;\n"

		//If any of the variables above end up being made __local, init them here.
		//At the moment, it's slower even though it's more memory efficient.
		//"	if (THREAD_ID_X == 0 && THREAD_ID_Y == 0)\n"
		//"	{\n"
		//Init local vars here.
		//"	}\n"
		//"\n"
		//"	barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"
		"	fullTempBoxWidth = BLOCK_SIZE_X + (densityFilter->m_FilterWidth * 2);\n"
		"	fullTempBoxHeight = BLOCK_SIZE_Y + (densityFilter->m_FilterWidth * 2);\n"
		//Compute the bounds of the area to be sampled, which is just the ends minus the super sample minus 1.
		"	leftBound = densityFilter->m_Supersample - 1;\n"
		"	rightBound = densityFilter->m_SuperRasW - (densityFilter->m_Supersample - 1);\n"
		"	topBound = densityFilter->m_Supersample - 1;\n"
		"	botBound = densityFilter->m_SuperRasH - (densityFilter->m_Supersample - 1);\n"
		"\n"
		//Start and end values are the indices in the histogram read from
		//and written to in the accumulator. They are not the indices for the local block of data.
		//Before computing local offsets, compute the global offsets first to determine if any rows or cols fall outside of the bounds.
		"	blockHistStartRow = min(botBound, topBound + (((BLOCK_ID_Y * chunkSizeH) + chunkH) * BLOCK_SIZE_Y));\n"//The first histogram row this block will process.
		"	blockHistEndRow = min(botBound, blockHistStartRow + BLOCK_SIZE_Y);\n"//The last histogram row this block will process, clamped to the last row.
		"	boxReadStartRow = densityFilter->m_FilterWidth - min(densityFilter->m_FilterWidth, blockHistStartRow);\n"//The first row in the local box to read from when writing back to the final accumulator for this block.
		"	boxReadEndRow = densityFilter->m_FilterWidth + min(densityFilter->m_FilterWidth + BLOCK_SIZE_Y, densityFilter->m_SuperRasH - blockHistStartRow);\n"//The last row in the local box to read from  when writing back to the final accumulator for this block.
		"	blockHistStartCol = min(rightBound, leftBound + (((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X));\n"//The first histogram column this block will process.
		"	boxReadStartCol = densityFilter->m_FilterWidth - min(densityFilter->m_FilterWidth, blockHistStartCol);\n"//The first box col this block will read from when copying to the accumulator.
		"	boxReadEndCol = densityFilter->m_FilterWidth + min(densityFilter->m_FilterWidth + BLOCK_SIZE_X, densityFilter->m_SuperRasW - blockHistStartCol);\n"//The last box col this block will read from when copying to the accumulator.
		"\n"
		//Last, the indices in the global accumulator that the local bounds will be writing to.
		"	accumWriteStartRow = blockHistStartRow - min(densityFilter->m_FilterWidth,  blockHistStartRow);\n"//Will be fw - 0 except for boundary columns, it will be less.
		"	accumWriteStartCol = blockHistStartCol - min(densityFilter->m_FilterWidth,  blockHistStartCol);\n"
		"	colsToWrite = ceil((real_t)(boxReadEndCol - boxReadStartCol) / (real_t)BLOCK_SIZE_X);\n"
		"\n"
		"	uint threadHistRow = blockHistStartRow + THREAD_ID_Y;\n"//The histogram row this individual thread will be reading from.
		"	uint threadHistCol = blockHistStartCol + THREAD_ID_X;\n"//The histogram column this individual thread will be reading from.
		"\n"

		//Compute the center position in this local box to serve as the center position
		//from which filter application offsets are computed.
		//These are the local indices for the local data that are temporarily accumulated to before
		//writing out to the global accumulator.
		"	uint boxRow = densityFilter->m_FilterWidth + THREAD_ID_Y;\n"
		"	uint boxCol = densityFilter->m_FilterWidth + THREAD_ID_X;\n"
		"	uint colElementsToZero = ceil((real_t)fullTempBoxWidth / (real_t)(BLOCK_SIZE_X));\n"//Usually is 2.
		"	int i, j, k;\n"
		"	uint filterSelectInt, filterCoefIndex;\n"
		"	real_t cacheLog;\n"
		"	real_t filterSelect;\n"
		"	real4 bucket;\n"
		;

		//This will be treated as having dimensions of (BLOCK_SIZE_X + (fw * 2)) x (BLOCK_SIZE_Y + (fw * 2)).
	if (m_NVidia)
		os << "	__local real4reals filterBox[3000];\n";
	else
		os << "	__local real4reals filterBox[1200];\n";

	os <<
		//Zero the temp buffers first. This splits the zeroization evenly across all threads (columns) in the first block row.
		//This is a middle ground solution. Previous methods tried:
		//Thread (0, 0) does all init. This works, but is the slowest.
		//Init is divided among all threads. This is the fastest but exposes a severe flaw in OpenCL,
		//in that it will not get executed by all threads before proceeding, despite the barrier statement
		//below. As a result, strange artifacts will get left around because filtering gets executed on a temp
		//box that has not been properly zeroized.
		//The only way to do it and still achieve reasonable speed is to have the first row do it. This is
		//most likely because the first row gets executed first, ensuring zeroization is done when the rest
		//of the threads execute.
		"\n"//Dummy test zeroization for debugging.
		//"	if (THREAD_ID_Y == 0 && THREAD_ID_X == 0)\n"//First thread of the block takes the responsibility of zeroizing.
		//"	{\n"
		//"		for (k = 0; k < 2 * 1024; k++)\n"
		//"		{\n"
		//"			filterBox[k].m_Real4 = 0;\n"
		//"		}\n"
		//"	}\n"
		"	if (THREAD_ID_Y == 0)\n"//First row of the block takes the responsibility of zeroizing.
		"	{\n"
		"		for (i = 0; i < fullTempBoxHeight; i++)\n"//Each column in the row iterates through all rows.
		"		{\n"
		"			for (j = 0; j < colElementsToZero && ((colElementsToZero * THREAD_ID_X) + j) < fullTempBoxWidth; j++)\n"//And zeroizes a few columns from that row.
		"			{\n"
		"				filterBox[(i * fullTempBoxWidth) + ((colElementsToZero * THREAD_ID_X) + j)].m_Real4 = 0;\n"
		"			}\n"
		"		}\n"
		"	}\n"
		"\n"
		"	barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"
		"	if (threadHistRow < botBound && threadHistCol < rightBound)\n"
		"	{\n"
		"		bucket = histogram[(threadHistRow * densityFilter->m_SuperRasW) + threadHistCol];\n"
		"\n"
		"		if (bucket.w != 0)\n"
		"		{\n"
		"			cacheLog = (densityFilter->m_K1 * log(1.0 + bucket.w * densityFilter->m_K2)) / bucket.w;\n";

	if (doSS)
	{
		os <<
		"			filterSelect = 0;\n"
		"			densityBoxLeftX = threadHistCol - min(threadHistCol, ss);\n"
		"			densityBoxRightX = threadHistCol + min(ss, (densityFilter->m_SuperRasW - threadHistCol) - 1);\n"
		"			densityBoxTopY = threadHistRow - min(threadHistRow, ss);\n"
		"			densityBoxBottomY = threadHistRow + min(ss, (densityFilter->m_SuperRasH - threadHistRow) - 1);\n"
		"\n"
		"			for (j = densityBoxTopY; j <= densityBoxBottomY; j++)\n"
		"			{\n"
		"				for (i = densityBoxLeftX; i <= densityBoxRightX; i++)\n"
		"				{\n"
		"					filterSelect += histogram[i + (j * densityFilter->m_SuperRasW)].w;\n"
		"				}\n"
		"			}\n"
		"\n";

		if (doScf)
			os << "	filterSelect *= scfact;\n";
	}
	else
	{
		os
			<< "	filterSelect = bucket.w;\n";
	}

	os <<
		"\n"
		"			if (filterSelect > densityFilter->m_MaxFilteredCounts)\n"
		"				filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"			else if (filterSelect <= DE_THRESH)\n"
		"				filterSelectInt = (int)ceil(filterSelect) - 1;\n"
		"			else\n"
		"				filterSelectInt = (int)DE_THRESH + (int)floor(pow((real_t)(filterSelect - DE_THRESH), densityFilter->m_Curve));\n"
		"\n"
		"			if (filterSelectInt > densityFilter->m_MaxFilterIndex)\n"
		"				filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"\n"
		"			filterCoefIndex = filterSelectInt * densityFilter->m_KernelSize;\n"
		"\n"
		//With this new method, only accumulate to the temp local buffer first. Write to the final accumulator last.
		//For each loop through, note that there is a local memory barrier call inside of each call to AddToAccumNoCheck().
		//If this isn't done, pixel errors occurr and even an out of resources error occurrs because too many writes are done to the same place in memory at once.
		"			k = (int)densityFilter->m_FilterWidth;\n"//Need a signed int to use below, really is filter width, but reusing a variable to save space.
		"\n"
		"			for (j = -k; j <= k; j++)\n"
		"			{\n"
		"				for (i = -k; i <= k; i++)\n"
		"				{\n"
		"					filterSelectInt = filterCoefIndex + coefIndices[(abs(j) * (densityFilter->m_FilterWidth + 1)) + abs(i)];\n"//Really is filterCoeffIndexPlusOffset, but reusing a variable to save space.
		"\n"
		"					if (filterCoefs[filterSelectInt] != 0)\n"//This conditional actually improves speed, despite SIMT being bad at conditionals.
		"					{\n"
		"						filterBox[(i + boxCol) + ((j + boxRow) * fullTempBoxWidth)].m_Real4 += (bucket * (filterCoefs[filterSelectInt] * cacheLog));\n"
		"					}\n"
		"				}\n"
		"				barrier(CLK_LOCAL_MEM_FENCE);\n"//If this is the only barrier and the block size is exactly 16, it works perfectly. Otherwise, no chunks occur, but a many streaks.
		"			}\n"
		"		}\n"//bucket.w != 0.
		"	}\n"//In bounds.
		"\n"
		"\n"
		"	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);\n"
		"\n"
		"	if (THREAD_ID_Y == 0)\n"
		"	{\n"
				//At this point, all threads in this block have applied the filter to their surrounding pixels and stored the results in the temp local box.
				//Add the cells of it that are in bounds to the global accumulator.
				//Compute offsets in local box to read from, and offsets into global accumulator to write to.
				//Use a method here that is similar to the zeroization above: Each thread (column) in the first row iterates through all of the
				//rows and adds a few columns to the accumulator.
		"		for (i = boxReadStartRow, j = accumWriteStartRow; i < boxReadEndRow; i++, j++)\n"
		"		{\n"
		"			for (k = 0; k < colsToWrite; k++)\n"//Each thread writes a few columns.
		"			{\n"
		"				boxCol = (colsToWrite * THREAD_ID_X) + k;\n"//Really is colOffset, but reusing a variable to save space.
		"\n"
		"				if (boxReadStartCol + boxCol < boxReadEndCol)\n"
		"					accumulator[(j * densityFilter->m_SuperRasW) + (accumWriteStartCol + boxCol)].m_Real4 += filterBox[(i * fullTempBoxWidth) + (boxReadStartCol + boxCol)].m_Real4;\n"
		"			}\n"
		"			barrier(CLK_GLOBAL_MEM_FENCE);\n"//This must be here or else chunks will go missing.
		"		}\n"
		"	}\n"
		"}\n";

	return os.str();
}
#endif

/// <summary>
/// Create the gaussian density filtering kernel string, but use no local cache and perform
/// all writes directly to the global density filtering buffer.
/// Note this applies the filter from top to bottom row and not from the center outward like the CPU version does.
/// This allows the image to be filtered without suffering from pixel loss due to race conditions.
/// This is used for when the filter box is greater than can fit in the local cache.
/// While the cached version is incredibly fast, this version offers no real gain over doing it
/// on the CPU because the frequent global memory access brings performance to a crawl.
/// The supersample parameter will produce three different kernels.
/// SS = 1, SS > 1 && SS even, SS > 1 && SS odd.
/// The width of the kernel this runs in must be evenly divisible by 16 or else artifacts will occur.
/// Note that because this function uses so many variables and is so complex, OpenCL can easily run
/// out of resources in some cases. Certain variables had to be reused to condense the kernel footprint
/// down enough to be able to run a block size of 32x32.
/// For double precision, or for SS > 1, a size of 32x30 is used.
/// </summary>
/// <param name="ss">The supersample being used</param>
/// <returns>The kernel string</returns>
template <typename T>
string DEOpenCLKernelCreator<T>::CreateGaussianDEKernelNoLocalCache(size_t ss)
{
	bool doSS = ss > 1;
	bool doScf = !(ss & 1);
	ostringstream os;

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		DensityFilterCLStructString <<
		UnionCLStructString <<
		AddToAccumWithCheckFunctionString <<
		"__kernel void " << GaussianDEEntryPoint(ss, MaxDEFilterSize() + 1) << "(\n" <<
		"	const __global real4* histogram,\n"
		"	__global real4reals* accumulator,\n"
		"	__constant DensityFilterCL* densityFilter,\n"
		"	const __global real_t* filterCoefs,\n"
		"	const __global real_t* filterWidths,\n"
		"	const __global uint* coefIndices,\n"
		"	const uint chunkSizeW,\n"
		"	const uint chunkSizeH,\n"
		"	const uint chunkW,\n"
		"	const uint chunkH\n"
		"\t)\n"
		"{\n"
		"	if (((((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X) + THREAD_ID_X >= densityFilter->m_SuperRasW) ||\n"
		"	    ((((BLOCK_ID_Y * chunkSizeH) + chunkH) * BLOCK_SIZE_Y) + THREAD_ID_Y >= densityFilter->m_SuperRasH))\n"
		"			return;\n"
		"\n";

	if (doSS)
	{
	os <<
		"	uint ss = (uint)floor((real_t)densityFilter->m_Supersample / 2.0);\n"
		"	int densityBoxLeftX;\n"
		"	int densityBoxRightX;\n"
		"	int densityBoxTopY;\n"
		"	int densityBoxBottomY;\n";

	if (doScf)
	os << "	real_t scfact = pow((real_t)densityFilter->m_Supersample / ((real_t)densityFilter->m_Supersample + 1.0), 2.0);\n";
	}

	os <<
		//Compute the bounds of the area to be sampled, which is just the ends minus the super sample minus 1.
		"	uint leftBound = densityFilter->m_Supersample - 1;\n"
		"	uint rightBound = densityFilter->m_SuperRasW - (densityFilter->m_Supersample - 1);\n"
		"	uint topBound = densityFilter->m_Supersample - 1;\n"
		"	uint botBound = densityFilter->m_SuperRasH - (densityFilter->m_Supersample - 1);\n"
		"\n"
		//Start and end values are the indices in the histogram read from and written to in the accumulator.
		//Before computing local offsets, compute the global offsets first to determine if any rows or cols fall outside of the bounds.
		"	uint blockHistStartRow = min(botBound, topBound + (((BLOCK_ID_Y * chunkSizeH) + chunkH) * BLOCK_SIZE_Y));\n"//The first histogram row this block will process.
		"	uint threadHistRow = blockHistStartRow + THREAD_ID_Y;\n"//The histogram row this individual thread will be reading from.
		"\n"
		"	uint blockHistStartCol = min(rightBound, leftBound + (((BLOCK_ID_X * chunkSizeW) + chunkW) * BLOCK_SIZE_X));\n"//The first histogram column this block will process.
		"	uint threadHistCol = blockHistStartCol + THREAD_ID_X;\n"//The histogram column this individual thread will be reading from.
		"\n"
		"	int i, j;\n"
		"	uint filterSelectInt, filterCoefIndex;\n"
		"	real_t cacheLog;\n"
		"	real_t logScale;\n"
		"	real_t filterSelect;\n"
		"	real4 bucket;\n"
		"\n"
		"	if (threadHistRow < botBound && threadHistCol < rightBound)\n"
		"	{\n"
		"		bucket = histogram[(threadHistRow * densityFilter->m_SuperRasW) + threadHistCol];\n"
		"\n"
		"		if (bucket.w != 0)\n"
		"		{\n"
		"			cacheLog = (densityFilter->m_K1 * log(1.0 + bucket.w * densityFilter->m_K2)) / bucket.w;\n";

	if (doSS)
	{
		os <<
		"			filterSelect = 0;\n"
		"			densityBoxLeftX = threadHistCol - min(threadHistCol, ss);\n"
		"			densityBoxRightX = threadHistCol + min(ss, (densityFilter->m_SuperRasW - threadHistCol) - 1);\n"
		"			densityBoxTopY = threadHistRow - min(threadHistRow, ss);\n"
		"			densityBoxBottomY = threadHistRow + min(ss, (densityFilter->m_SuperRasH - threadHistRow) - 1);\n"
		"\n"
		"			for (j = densityBoxTopY; j <= densityBoxBottomY; j++)\n"
		"			{\n"
		"				for (i = densityBoxLeftX; i <= densityBoxRightX; i++)\n"
		"				{\n"
		"					filterSelect += histogram[i + (j * densityFilter->m_SuperRasW)].w;\n"
		"				}\n"
		"			}\n"
		"\n";

		if (doScf)
		os << "		filterSelect *= scfact;\n";
	}
	else
	{
		os
	<< "			filterSelect = bucket.w;\n";
	}

	os <<
		"\n"
		"			if (filterSelect > densityFilter->m_MaxFilteredCounts)\n"
		"				filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"			else if (filterSelect <= DE_THRESH)\n"
		"				filterSelectInt = (int)ceil(filterSelect) - 1;\n"
		"			else\n"
		"				filterSelectInt = (int)DE_THRESH + (int)floor(pow((real_t)(filterSelect - DE_THRESH), densityFilter->m_Curve));\n"
		"\n"
		"			if (filterSelectInt > densityFilter->m_MaxFilterIndex)\n"
		"				filterSelectInt = densityFilter->m_MaxFilterIndex;\n"
		"\n"
		"			filterCoefIndex = filterSelectInt * densityFilter->m_KernelSize;\n"
		"\n"
		"			int fw = (int)densityFilter->m_FilterWidth;\n"//Need a signed int to use below.
		"\n"
		"			for (j = -fw; j <= fw; j++)\n"
		"			{\n"
		"				for (i = -fw; i <= fw; i++)\n"
		"				{\n"
		"					if (AccumCheck(densityFilter->m_SuperRasW, densityFilter->m_SuperRasH, threadHistCol, i, threadHistRow, j))\n"
		"					{\n"
		"						filterSelectInt = filterCoefIndex + coefIndices[(abs(j) * (densityFilter->m_FilterWidth + 1)) + abs(i)];\n"//Really is filterCoeffIndexPlusOffset, but reusing a variable to save space.
		"\n"
		"						if (filterCoefs[filterSelectInt] != 0)\n"
		"						{\n"
		"							accumulator[(i + threadHistCol) + ((j + threadHistRow) * densityFilter->m_SuperRasW)].m_Real4 += (bucket * (filterCoefs[filterSelectInt] * cacheLog));\n"
		"						}\n"
		"					}\n"
		"\n"
		"					barrier(CLK_GLOBAL_MEM_FENCE);\n"//Required to avoid streaks.
		"				}\n"
		"			}\n"
		"		}\n"//bucket.w != 0.
		"	}\n"//In bounds.
		"\n"
		//"	barrier(CLK_GLOBAL_MEM_FENCE);\n"//Just to be safe.
		"}\n";

	return os.str();
}

template EMBERCL_API class DEOpenCLKernelCreator<float>;

#ifdef DO_DOUBLE
	template EMBERCL_API class DEOpenCLKernelCreator<double>;
#endif
}
