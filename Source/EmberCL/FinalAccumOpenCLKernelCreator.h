#pragma once

#include "EmberCLPch.h"
#include "EmberCLStructs.h"
#include "EmberCLFunctions.h"

/// <summary>
/// FinalAccumOpenCLKernelCreator class.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Class for creating the final accumulation code in OpenCL.
/// There are many conditionals in the CPU code to create the
/// final output image. This class creates many different kernels
/// with all conditionals and unnecessary calculations stripped out.
/// The conditionals are:
/// Early clip/late clip
/// Alpha channel, no alpha channel
/// Alpha with/without transparency
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBERCL_API FinalAccumOpenCLKernelCreator
{
public:
	FinalAccumOpenCLKernelCreator();

	string GammaCorrectionWithAlphaCalcKernel();
	string GammaCorrectionWithAlphaCalcEntryPoint();

	string GammaCorrectionWithoutAlphaCalcKernel();
	string GammaCorrectionWithoutAlphaCalcEntryPoint();

	string FinalAccumEarlyClipKernel();
	string FinalAccumEarlyClipEntryPoint();
	string FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel();
	string FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint();
	string FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel();
	string FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint();

	string FinalAccumLateClipKernel();
	string FinalAccumLateClipEntryPoint();
	string FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel();
	string FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint();
	string FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel();
	string FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint();
	string GammaCorrectionEntryPoint(size_t channels, bool transparency);
	string GammaCorrectionKernel(size_t channels, bool transparency);
	string FinalAccumEntryPoint(bool earlyClip, size_t channels, bool transparency, T& alphaBase, T& alphaScale);
	string FinalAccumKernel(bool earlyClip, size_t channels, bool transparency);

private:
	string CreateFinalAccumKernelString(bool earlyClip, size_t channels, bool transparency);
	string CreateGammaCorrectionKernelString(bool alphaCalc);

	string CreateFinalAccumKernelString(bool earlyClip, bool alphaCalc, bool alphaAccum);
	string CreateGammaCorrectionFunctionString(bool globalBucket, bool alphaCalc, bool alphaAccum, bool finalOut);
	string CreateCalcNewRgbFunctionString(bool globalBucket);
	string m_GammaCorrectionWithAlphaCalcKernel;
	string m_GammaCorrectionWithAlphaCalcEntryPoint;

	string m_GammaCorrectionWithoutAlphaCalcKernel;
	string m_GammaCorrectionWithoutAlphaCalcEntryPoint;

	string m_FinalAccumEarlyClipKernel;//False, false.
	string m_FinalAccumEarlyClipEntryPoint;
	string m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel;//True, true.
	string m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint;
	string m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel;//False, true.
	string m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint;

	string m_FinalAccumLateClipKernel;//False, false.
	string m_FinalAccumLateClipEntryPoint;
	string m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel;//True, true.
	string m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint;
	string m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel;//False, true.
	string m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint;
};
}
