#include "EmberCLPch.h"
#include "FinalAccumOpenCLKernelCreator.h"

namespace EmberCLns
{
/// <summary>
/// Constructor that creates all kernel strings.
/// The caller will access these strings through the accessor functions.
/// </summary>
template <typename T>
FinalAccumOpenCLKernelCreator<T>::FinalAccumOpenCLKernelCreator()
{
	m_GammaCorrectionWithAlphaCalcEntryPoint    = "GammaCorrectionWithAlphaCalcKernel";
	m_GammaCorrectionWithoutAlphaCalcEntryPoint = "GammaCorrectionWithoutAlphaCalcKernel";

	m_GammaCorrectionWithAlphaCalcKernel    = CreateGammaCorrectionKernelString(true);
	m_GammaCorrectionWithoutAlphaCalcKernel = CreateGammaCorrectionKernelString(false);

	m_FinalAccumEarlyClipEntryPoint                               = "FinalAccumEarlyClipKernel";
	m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint    = "FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel";
	m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint = "FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel";

	m_FinalAccumEarlyClipKernel                               = CreateFinalAccumKernelString(true, false, false);
	m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel    = CreateFinalAccumKernelString(true, true,  true);
	m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel = CreateFinalAccumKernelString(true, false, true);

	m_FinalAccumLateClipEntryPoint                               = "FinalAccumLateClipKernel";
	m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint    = "FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel";
	m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint = "FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel";

	m_FinalAccumLateClipKernel                               = CreateFinalAccumKernelString(false, false, false);
	m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel    = CreateFinalAccumKernelString(false, true,  true);
	m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel = CreateFinalAccumKernelString(false, false, true);
}

/// <summary>
/// Kernel source and entry point properties, getters only.
/// </summary>

template <typename T> string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionWithAlphaCalcKernel()        { return m_GammaCorrectionWithAlphaCalcKernel;	    }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionWithAlphaCalcEntryPoint()    { return m_GammaCorrectionWithAlphaCalcEntryPoint;    }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionWithoutAlphaCalcKernel()     { return m_GammaCorrectionWithoutAlphaCalcKernel;     }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionWithoutAlphaCalcEntryPoint() { return m_GammaCorrectionWithoutAlphaCalcEntryPoint; }

template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipKernel()                                   { return m_FinalAccumEarlyClipKernel;                                   }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipEntryPoint()                               { return m_FinalAccumEarlyClipEntryPoint;                               }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel()        { return m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel;        }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint()    { return m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint;    }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel()     { return m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel;     }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint() { return m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint; }

template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipKernel()                                   { return m_FinalAccumLateClipKernel;                                   }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipEntryPoint()                               { return m_FinalAccumLateClipEntryPoint;                               }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel()        { return m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel;        }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint()    { return m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint;    }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel()     { return m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel;     }
template <typename T> string FinalAccumOpenCLKernelCreator<T>::FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint() { return m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint; }

/// <summary>
/// Get the gamma correction entry point.
/// </summary>
/// <param name="channels">The number of channels used, 3 or 4.</param>
/// <param name="transparency">True if channels equals 4 and using transparency, else false.</param>
/// <returns>The name of the gamma correction entry point kernel function</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionEntryPoint(unsigned int channels, bool transparency)
{
	bool alphaCalc = (channels > 3 && transparency);
	return alphaCalc ? m_GammaCorrectionWithAlphaCalcEntryPoint : m_GammaCorrectionWithoutAlphaCalcEntryPoint;
}

/// <summary>
/// Get the gamma correction kernel string.
/// </summary>
/// <param name="channels">The number of channels used, 3 or 4.</param>
/// <param name="transparency">True if channels equals 4 and using transparency, else false.</param>
/// <returns>The gamma correction kernel string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::GammaCorrectionKernel(unsigned int channels, bool transparency)
{
	bool alphaCalc = (channels > 3 && transparency);
	return alphaCalc ? m_GammaCorrectionWithAlphaCalcKernel : m_GammaCorrectionWithoutAlphaCalcKernel;
}

/// <summary>
/// Get the final accumulation entry point.
/// </summary>
/// <param name="earlyClip">True if early clip is desired, else false.</param>
/// <param name="channels">The number of channels used, 3 or 4.</param>
/// <param name="transparency">True if channels equals 4 and using transparency, else false.</param>
/// <param name="alphaBase">Storage for the alpha base value used in the kernel. 0 if transparency is true, else 255.</param>
/// <param name="alphaScale">Storage for the alpha scale value used in the kernel. 255 if transparency is true, else 0.</param>
/// <returns>The name of the final accumulation entry point kernel function</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::FinalAccumEntryPoint(bool earlyClip, unsigned int channels, bool transparency, T& alphaBase, T& alphaScale)
{
	bool alphaCalc = (channels > 3 && transparency);
	bool alphaAccum = channels > 3;

	if (alphaAccum)
	{
		alphaBase = transparency ? 0.0f : 255.0f;//See the table below.
		alphaScale = transparency ? 255.0f : 0.0f;
	}

	if (earlyClip)
	{
		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			return FinalAccumEarlyClipEntryPoint();
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			return FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint();
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			return FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint();
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}
	else
	{
		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			return FinalAccumLateClipEntryPoint();
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			return FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint();
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			return FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint();
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}
}

/// <summary>
/// Get the final accumulation kernel string.
/// </summary>
/// <param name="earlyClip">True if early clip is desired, else false.</param>
/// <param name="channels">The number of channels used, 3 or 4.</param>
/// <param name="transparency">True if channels equals 4 and using transparency, else false.</param>
/// <returns>The final accumulation kernel string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::FinalAccumKernel(bool earlyClip, unsigned int channels, bool transparency)
{
	bool alphaCalc = (channels > 3 && transparency);
	bool alphaAccum = channels > 3;

	if (earlyClip)
	{
		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			return FinalAccumEarlyClipKernel();
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			return FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumKernel();
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			return FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumKernel();
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}
	else
	{
		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			return FinalAccumLateClipKernel();
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			return FinalAccumLateClipWithAlphaCalcWithAlphaAccumKernel();
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			return FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumKernel();
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}
}

/// <summary>
/// Wrapper around CreateFinalAccumKernelString().
/// </summary>
/// <param name="earlyClip">True if early clip is desired, else false.</param>
/// <param name="channels">The number of channels used, 3 or 4.</param>
/// <param name="transparency">True if channels equals 4 and using transparency, else false.</param>
/// <returns>The final accumulation kernel string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::CreateFinalAccumKernelString(bool earlyClip, unsigned int channels, bool transparency)
{
	return CreateFinalAccumKernelString(earlyClip, (channels > 3 && transparency), channels > 3);
}

/// <summary>
/// Create the final accumulation kernel string
/// </summary>
/// <param name="earlyClip">True if early clip is desired, else false.</param>
/// <param name="alphaCalc">True if channels equals 4 and transparency is desired, else false.</param>
/// <param name="alphaAccum">True if channels equals 4</param>
/// <returns>The final accumulation kernel string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::CreateFinalAccumKernelString(bool earlyClip, bool alphaCalc, bool alphaAccum)
{
	ostringstream os;
	string channels = alphaAccum ? "4" : "3";

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		ClampRealFunctionString <<
		UnionCLStructString <<
		RgbToHsvFunctionString <<
		HsvToRgbFunctionString <<
		CalcAlphaFunctionString <<
		SpatialFilterCLStructString;

	if (earlyClip)
	{
		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			os << "__kernel void " << m_FinalAccumEarlyClipEntryPoint << "(\n";
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			os << "__kernel void " << m_FinalAccumEarlyClipWithAlphaCalcWithAlphaAccumEntryPoint << "(\n";
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			os << "__kernel void " << m_FinalAccumEarlyClipWithoutAlphaCalcWithAlphaAccumEntryPoint << "(\n";
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}
	else
	{
		os <<
			CreateCalcNewRgbFunctionString(false) <<
			CreateGammaCorrectionFunctionString(false, alphaCalc, alphaAccum, true);

		if (!alphaCalc && !alphaAccum)//Rgb output, the most common case.
			os << "__kernel void " << m_FinalAccumLateClipEntryPoint << "(\n";
		else if (alphaCalc && alphaAccum)//Rgba output and Transparency.
			os << "__kernel void " << m_FinalAccumLateClipWithAlphaCalcWithAlphaAccumEntryPoint << "(\n";
		else if (!alphaCalc && alphaAccum)//Rgba output and !Transparency.
			os << "__kernel void " << m_FinalAccumLateClipWithoutAlphaCalcWithAlphaAccumEntryPoint << "(\n";
		else
			return "";//Cannot have alphaCalc and !alphaAccum, it makes no sense.
	}

	os <<
		"	const __global real4reals* accumulator,\n"
		"	__write_only image2d_t pixels,\n"
		"	__constant SpatialFilterCL* spatialFilter,\n"
		"	__constant real_t* filterCoefs,\n"
		"	const real_t alphaBase,\n"
		"	const real_t alphaScale\n"
		"\t)\n"
		"{\n"
		"\n"
		"	if ((GLOBAL_ID_Y >= spatialFilter->m_FinalRasH) || (GLOBAL_ID_X >= spatialFilter->m_FinalRasW))\n"
		"		return;\n"
		"\n"
		"	unsigned int accumX = spatialFilter->m_DensityFilterOffset + (GLOBAL_ID_X * spatialFilter->m_Supersample);\n"
		"	unsigned int accumY = spatialFilter->m_DensityFilterOffset + (GLOBAL_ID_Y * spatialFilter->m_Supersample);\n"

		"	int2 finalCoord;\n"
		"	finalCoord.x = GLOBAL_ID_X;\n"
		"	finalCoord.y = GLOBAL_ID_Y;\n"
		"	float4floats finalColor;\n"
		"	real_t alpha, ls;\n"
		"	int ii, jj;\n"
		"	unsigned int filterKRowIndex;\n"
		"	const __global real4reals* accumBucket;\n"
		"	real4reals newBucket;\n"
		"	newBucket.m_Real4 = 0;\n"
		"	real4reals newRgb;\n"
		"	newRgb.m_Real4 = 0;\n"
		"\n"
		"	for (jj = 0; jj < spatialFilter->m_FilterWidth; jj++)\n"
		"	{\n"
		"		filterKRowIndex = jj * spatialFilter->m_FilterWidth;\n"
		"\n"
		"		for (ii = 0; ii < spatialFilter->m_FilterWidth; ii++)\n"
		"		{\n"
		"			real_t k = filterCoefs[ii + filterKRowIndex];\n"
		"\n"
		"			accumBucket = accumulator + (accumX + ii) + ((accumY + jj) * spatialFilter->m_SuperRasW);\n"
		"			newBucket.m_Real4 += (k * accumBucket->m_Real4);\n"
		"		}\n"
		"	}\n"
		"\n";

	//Not supporting 2 bytes per channel on the GPU. If the user wants it, run on the CPU.
	if (earlyClip)//If early clip, simply assign values directly to the temp uint4 since they've been gamma corrected already, then write it straight to the output image below.
	{
		os <<
		"	finalColor.m_Float4.x = (float)newBucket.m_Real4.x;\n"//CPU side clamps, skip here because write_imagef() does the clamping for us.
		"	finalColor.m_Float4.y = (float)newBucket.m_Real4.y;\n"
		"	finalColor.m_Float4.z = (float)newBucket.m_Real4.z;\n";
	
		if (alphaAccum)
		{
			if (alphaCalc)
				os << "	finalColor.m_Float4.w = (float)newBucket.m_Real4.w * 255.0f;\n";
			else
				os << "	finalColor.m_Float4.w = 255;\n";
		}
	}
	else
	{
		//Late clip, so must gamma correct from the temp new bucket to temp float4.
		if (typeid(T) == typeid(double))
		{
			os <<
		"	real4reals realFinal;\n"
		"\n"
		"	GammaCorrectionFloats(&newBucket, &(spatialFilter->m_Background[0]), spatialFilter->m_Gamma, spatialFilter->m_LinRange, spatialFilter->m_Vibrancy, spatialFilter->m_HighlightPower, alphaBase, alphaScale, &(realFinal.m_Reals[0]));\n"
		"	finalColor.m_Float4.x = (float)realFinal.m_Real4.x;\n"
		"	finalColor.m_Float4.y = (float)realFinal.m_Real4.y;\n"
		"	finalColor.m_Float4.z = (float)realFinal.m_Real4.z;\n"
		"	finalColor.m_Float4.w = (float)realFinal.m_Real4.w;\n"
		;
		}
		else
		{
			os <<
		"	GammaCorrectionFloats(&newBucket, &(spatialFilter->m_Background[0]), spatialFilter->m_Gamma, spatialFilter->m_LinRange, spatialFilter->m_Vibrancy, spatialFilter->m_HighlightPower, alphaBase, alphaScale, &(finalColor.m_Floats[0]));\n";
		}
	}

	os <<
		"	finalColor.m_Float4 /= 255.0f;\n"
		"	write_imagef(pixels, finalCoord, finalColor.m_Float4);\n"//Use write_imagef instead of write_imageui because only the former works when sharing with an OpenGL texture.
		"	barrier(CLK_GLOBAL_MEM_FENCE);\n"//Required, or else page tearing will occur during interactive rendering.
		"}\n"
		;

	return os.str();
}

/// <summary>
/// Creates the gamma correction function string.
/// This is not a full kernel, just a function that is used in the kernels.
/// </summary>
/// <param name="globalBucket">True if writing to a global buffer (early clip), else false (late clip).</param>
/// <param name="alphaCalc">True if channels equals 4 and transparency is desired, else false.</param>
/// <param name="alphaAccum">True if channels equals 4</param>
/// <param name="finalOut">True if writing to global buffer (late clip), else false (early clip).</param>
/// <returns>The gamma correction function string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::CreateGammaCorrectionFunctionString(bool globalBucket, bool alphaCalc, bool alphaAccum, bool finalOut)
{
	ostringstream os;
	string dataType;
	string unionMember;
	dataType = "real_t";

	//Use real_t for all cases, early clip and final accum.
	os << "void GammaCorrectionFloats(" << (globalBucket ? "__global " : "") << "real4reals* bucket, __constant real_t* background, real_t g, real_t linRange, real_t vibrancy, real_t highlightPower, real_t alphaBase, real_t alphaScale, " << (finalOut ? "" : "__global") << " real_t* correctedChannels)\n";

	os
	<< "{\n"
	<< "	real_t alpha, ls, tmp, a;\n"
	<< "	real4reals newRgb;\n"
	<< "\n"
	<< "	if (bucket->m_Reals[3] <= 0)\n"
	<< "	{\n"
	<< "		alpha = 0;\n"
	<< "		ls = 0;\n"
	<< "	}\n"
	<< "	else\n"
	<< "	{\n"
	<< "		tmp = bucket->m_Reals[3];\n"
	<< "		alpha = CalcAlpha(tmp, g, linRange);\n"
	<< "		ls = vibrancy * 256.0 * alpha / tmp;\n"
	<< "		ClampRef(&alpha, 0.0, 1.0);\n"
	<< "	}\n"
	<< "\n"
	<< "	CalcNewRgb(bucket, ls, highlightPower, &newRgb);\n"
	<< "\n"
	<< "	for (unsigned int rgbi = 0; rgbi < 3; rgbi++)\n"
	<< "	{\n"
	<< "		a = newRgb.m_Reals[rgbi] + ((1.0 - vibrancy) * 256.0 * pow(bucket->m_Reals[rgbi], g));\n"
	<< "\n";

	if (!alphaCalc)
	{
		os <<
		"		a += ((1.0 - alpha) * background[rgbi]);\n";
	}
	else
	{
		os
	<< "		if (alpha > 0)\n"
	<< "			a /= alpha;\n"
	<< "		else\n"
	<< "			a = 0;\n";
	}
	
	os <<
	"\n"
	"			correctedChannels[rgbi] = (" << dataType << ")clamp(a, 0.0, 255.0);\n"
	"		}\n"
	"\n";
	
	//The CPU code has 3 cases for assigning alpha:
	//[3] = alpha.//Early clip.
	//[3] = alpha * 255.//Final Rgba with transparency.
	//[3] = 255.//Final Rgba without transparency.
	//Putting conditionals in GPU code is to be avoided. So do base + alpha * scale which will
	//work for all 3 cases without using a conditional, which should be faster on a GPU. This gives:
	//Base = 0,   scale = 1.   [3] = (0 +   (alpha * 1)).   [3] = alpha.
	//Base = 0,   scale = 255. [3] = (0 +   (alpha * 255)). [3] = alpha * 255.
	//Base = 255, scale = 0.   [3] = (255 + (alpha * 0)).   [3] = 255.
	if (alphaAccum)
	{
		os
	<< "	correctedChannels[3] = (" << dataType << ")(alphaBase + (alpha * alphaScale));\n";
	}

	os <<
	"}\n"
	"\n";

	return os.str();
}

/// <summary>
/// OpenCL equivalent of Palette::CalcNewRgb().
/// </summary>
/// <param name="globalBucket">True if writing the corrected value to a global buffer (early clip), else false (late clip).</param>
/// <returns>The CalcNewRgb function string</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::CreateCalcNewRgbFunctionString(bool globalBucket)
{
	ostringstream os;

	os <<
	"static void CalcNewRgb(" << (globalBucket ? "__global " : "") << "real4reals* oldRgb, real_t ls, real_t highPow, real4reals* newRgb)\n"
	"{\n"
	"	int rgbi;\n"
	"	real_t newls, lsratio;\n"
	"	real4reals newHsv;\n"
	"	real_t maxa, maxc;\n"
	"	real_t adjhlp;\n"
	"\n"
	"	if (ls == 0 || (oldRgb->m_Real4.x == 0 && oldRgb->m_Real4.y == 0 && oldRgb->m_Real4.z == 0))\n"//Can't do a vector compare to zero.
	"	{\n"
	"		newRgb->m_Real4 = 0;\n"
	"		return;\n"
	"	}\n"
	"\n"
	//Identify the most saturated channel.
	"	maxc = max(max(oldRgb->m_Reals[0], oldRgb->m_Reals[1]), oldRgb->m_Reals[2]);\n"
	"	maxa = ls * maxc;\n"
	"\n"
	//If a channel is saturated and highlight power is non-negative
	//modify the color to prevent hue shift.
	"	if (maxa > 255 && highPow >= 0)\n"
	"	{\n"
	"		newls = 255.0 / maxc;\n"
	"		lsratio = pow(newls / ls, highPow);\n"
	"\n"
	//Calculate the max-value color (ranged 0 - 1).
	"		for (rgbi = 0; rgbi < 3; rgbi++)\n"
	"			newRgb->m_Reals[rgbi] = newls * oldRgb->m_Reals[rgbi] / 255.0;\n"
	"\n"
	//Reduce saturation by the lsratio.
	"		RgbToHsv(&(newRgb->m_Real4), &(newHsv.m_Real4));\n"
	"		newHsv.m_Real4.y *= lsratio;\n"
	"		HsvToRgb(&(newHsv.m_Real4), &(newRgb->m_Real4));\n"
	"\n"
	"		for (rgbi = 0; rgbi < 3; rgbi++)\n"//Unrolling and vectorizing makes no difference.
	"			newRgb->m_Reals[rgbi] *= 255.0;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		newls = 255.0 / maxc;\n"
	"		adjhlp = -highPow;\n"
	"\n"
	"		if (adjhlp > 1)\n"
	"			adjhlp = 1;\n"
	"\n"
	"		if (maxa <= 255)\n"
	"			adjhlp = 1;\n"
	"\n"
	//Calculate the max-value color (ranged 0 - 1) interpolated with the old behavior.
	"		for (rgbi = 0; rgbi < 3; rgbi++)\n"//Unrolling, caching and vectorizing makes no difference.
	"			newRgb->m_Reals[rgbi] = ((1.0 - adjhlp) * newls + adjhlp * ls) * oldRgb->m_Reals[rgbi];\n"
	"	}\n"
	"}\n"
	"\n";

	return os.str();
}

/// <summary>
/// Create the gamma correction kernel string used for early clipping.
/// </summary>
/// <param name="alphaCalc">True if channels equals 4 and transparency is desired, else false.</param>
/// <returns>The gamma correction kernel string used for early clipping</returns>
template <typename T>
string FinalAccumOpenCLKernelCreator<T>::CreateGammaCorrectionKernelString(bool alphaCalc)
{
	ostringstream os;
	string dataType;

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<
		ClampRealFunctionString <<
		UnionCLStructString <<
		RgbToHsvFunctionString <<
		HsvToRgbFunctionString <<
		CalcAlphaFunctionString <<
		CreateCalcNewRgbFunctionString(true) <<
		SpatialFilterCLStructString <<
		CreateGammaCorrectionFunctionString(true, alphaCalc, true, false);//Will only be used with float in this case, early clip. Will always alpha accum.

		os << "__kernel void " << (alphaCalc ? m_GammaCorrectionWithAlphaCalcEntryPoint : m_GammaCorrectionWithoutAlphaCalcEntryPoint) << "(\n" <<
			"	__global real4reals* accumulator,\n"
			"	__constant SpatialFilterCL* spatialFilter\n"
			")\n"
			"{\n"
			"	int testGutter = 0;\n"
			"\n"
			"	if (GLOBAL_ID_Y >= (spatialFilter->m_SuperRasH - testGutter) || GLOBAL_ID_X >= (spatialFilter->m_SuperRasW - testGutter))\n"
			"		return;\n"
			"\n"
			"	unsigned int superIndex = (GLOBAL_ID_Y * spatialFilter->m_SuperRasW) + GLOBAL_ID_X;\n"
			"	__global real4reals* bucket = accumulator + superIndex;\n"
			//Pass in an alphaBase and alphaScale of 0, 1 which means to just directly assign the computed alpha value.
			"	GammaCorrectionFloats(bucket, &(spatialFilter->m_Background[0]), spatialFilter->m_Gamma, spatialFilter->m_LinRange, spatialFilter->m_Vibrancy, spatialFilter->m_HighlightPower, 0.0, 1.0, &(bucket->m_Reals[0]));\n"
			"}\n"
		;

	return os.str();
}
}