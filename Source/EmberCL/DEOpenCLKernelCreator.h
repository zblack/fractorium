#pragma once
 
#include "EmberCLPch.h"
#include "EmberCLStructs.h"
#include "EmberCLFunctions.h"

/// <summary>
/// DEOpenCLKernelCreator class.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Kernel creator for density filtering.
/// This implements both basic log scale filtering
/// as well as the full flam3 density estimation filtering
/// in OpenCL.
/// Several conditionals are present in the CPU version. They
/// are stripped out of the kernels and instead a separate kernel
/// is created for every possible case.
/// If the filter width is 9 or less, then the entire process can be
/// done in shared memory which is very fast.
/// However, if the filter width is greater than 9, shared memory is not
/// used and all filtering is done directly with main global VRAM. This
/// ends up being not much faster than doing it on the CPU.
/// String members are kept for the program source and entry points
/// for each version of the program.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBERCL_API DEOpenCLKernelCreator
{
public:
	DEOpenCLKernelCreator();
	DEOpenCLKernelCreator(bool nVidia);

	//Accessors.
	string LogScaleSumDEKernel();
	string LogScaleSumDEEntryPoint();
	string LogScaleAssignDEKernel();
	string LogScaleAssignDEEntryPoint();
	string GaussianDEKernel(unsigned int ss, unsigned int filterWidth);
	string GaussianDEEntryPoint(unsigned int ss, unsigned int filterWidth);

	//Miscellaneous static functions.
	static unsigned int MaxDEFilterSize();
	static T SolveMaxDERad(unsigned int maxBoxSize, T desiredFilterSize, T ss);
	static unsigned int SolveMaxBoxSize(unsigned int localMem);

private:
	//Kernel creators.
	string CreateLogScaleSumDEKernelString();
	string CreateLogScaleAssignDEKernelString();
	string CreateGaussianDEKernel(unsigned int ss);
	string CreateGaussianDEKernelNoLocalCache(unsigned int ss);

	string m_LogScaleSumDEKernel;
	string m_LogScaleSumDEEntryPoint;

	string m_LogScaleAssignDEKernel;
	string m_LogScaleAssignDEEntryPoint;

	string m_GaussianDEWithoutSsKernel;
	string m_GaussianDEWithoutSsEntryPoint;

	string m_GaussianDESsWithScfKernel;
	string m_GaussianDESsWithScfEntryPoint;

	string m_GaussianDESsWithoutScfKernel;
	string m_GaussianDESsWithoutScfEntryPoint;

	string m_GaussianDEWithoutSsNoCacheKernel;
	string m_GaussianDEWithoutSsNoCacheEntryPoint;

	string m_GaussianDESsWithScfNoCacheKernel;
	string m_GaussianDESsWithScfNoCacheEntryPoint;

	string m_GaussianDESsWithoutScfNoCacheKernel;
	string m_GaussianDESsWithoutScfNoCacheEntryPoint;

	bool m_NVidia;
};

template EMBERCL_API class DEOpenCLKernelCreator<float>;

#ifdef DO_DOUBLE
	template EMBERCL_API class DEOpenCLKernelCreator<double>;
#endif
}
