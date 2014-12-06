#pragma once

#include "EmberCLPch.h"
#include "EmberCLStructs.h"
#include "EmberCLFunctions.h"

/// <summary>
/// IterOpenCLKernelCreator class.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Class for creating the main iteration code in OpenCL.
/// It uses the Cuburn method of iterating where all conditionals
/// are stripped out and a specific kernel is compiled at run-time.
/// It uses a very sophisticated method for randomization that avoids
/// the problem of warp/wavefront divergence that would occur if every
/// thread selected a random xform to apply.
/// This only works with embers of type float, double is not supported.
/// </summary>
template <typename T>
class EMBERCL_API IterOpenCLKernelCreator
{
public:
	IterOpenCLKernelCreator();
	IterOpenCLKernelCreator(bool nVidia);
	string ZeroizeKernel();
	string ZeroizeEntryPoint();
	string IterEntryPoint();
	string CreateIterKernelString(Ember<T>& ember, string& parVarDefines, bool lockAccum = false, bool doAccum = true);
	static void ParVarIndexDefines(Ember<T>& ember, pair<string, vector<T>>& params, bool doVals = true, bool doString = true);
	static bool IsBuildRequired(Ember<T>& ember1, Ember<T>& ember2);

private:
	string CreateZeroizeKernelString();
	string CreateProjectionString(Ember<T>& ember);

	string m_IterEntryPoint;
	string m_ZeroizeKernel;
	string m_ZeroizeEntryPoint;
	bool m_NVidia;
};
//
//template EMBERCL_API class IterOpenCLKernelCreator<float>;
//
//#ifdef DO_DOUBLE
//	template EMBERCL_API class IterOpenCLKernelCreator<double>;
//#endif

//
//template EMBERCL_API string IterOpenCLKernelCreator::CreateIterKernelString<float>(Ember<float>& ember, string& parVarDefines, bool lockAccum, bool doAccum);
//template EMBERCL_API string IterOpenCLKernelCreator::CreateIterKernelString<double>(Ember<double>& ember, string& parVarDefines, bool lockAccum, bool doAccum);
//
//template EMBERCL_API void IterOpenCLKernelCreator::ParVarIndexDefines<float>(Ember<float>& ember, pair<string, vector<float>>& params, bool doVals, bool doString);
//template EMBERCL_API void IterOpenCLKernelCreator::ParVarIndexDefines<double>(Ember<double>& ember, pair<string, vector<double>>& params, bool doVals, bool doString);
//
//template EMBERCL_API bool IterOpenCLKernelCreator::IsBuildRequired<float>(Ember<float>& ember1, Ember<float>& ember2);
//template EMBERCL_API bool IterOpenCLKernelCreator::IsBuildRequired<double>(Ember<double>& ember1, Ember<double>& ember2);

#ifdef OPEN_CL_TEST_AREA
typedef void (*KernelFuncPointer) (unsigned int gridWidth, unsigned int gridHeight, unsigned int blockWidth, unsigned int blockHeight,
								   unsigned int BLOCK_ID_X, unsigned int BLOCK_ID_Y, unsigned int THREAD_ID_X, unsigned int THREAD_ID_Y);

static void OpenCLSim(unsigned int gridWidth, unsigned int gridHeight, unsigned int blockWidth, unsigned int blockHeight, KernelFuncPointer func)
{
	cout << "OpenCLSim(): " << endl;
	cout << "	Params: " << endl;
	cout << "		gridW: " << gridWidth << endl;
	cout << "		gridH: " << gridHeight << endl;
	cout << "		blockW: " << blockWidth << endl;
	cout << "		blockH: " << blockHeight << endl;

	for (unsigned int i = 0; i < gridHeight; i += blockHeight)
	{
		for (unsigned int j = 0; j < gridWidth; j += blockWidth)
		{
			for (unsigned int k = 0; k < blockHeight; k++)
			{
				for (unsigned int l = 0; l < blockWidth; l++)
				{
					func(gridWidth, gridHeight, blockWidth, blockHeight, j / blockWidth, i / blockHeight, l, k);
				}
			}
		}
	}
}
#endif
}
