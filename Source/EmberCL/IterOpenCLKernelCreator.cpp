#include "EmberCLPch.h"
#include "IterOpenCLKernelCreator.h"

namespace EmberCLns
{
/// <summary>
/// Empty constructor that does nothing. The user must call the one which takes a bool
/// argument before using this class.
/// This constructor only exists so the class can be a member of a class.
/// </summary>
template <typename T>
IterOpenCLKernelCreator<T>::IterOpenCLKernelCreator()
{
}

/// <summary>
/// Constructor that sets up some basic entry point strings and creates
/// the zeroization kernel string since it requires no conditional inputs.
/// </summary>
template <typename T>
IterOpenCLKernelCreator<T>::IterOpenCLKernelCreator(bool nVidia)
{
	m_NVidia = nVidia;
	m_IterEntryPoint = "IterateKernel";
	m_ZeroizeEntryPoint = "ZeroizeKernel";
	m_ZeroizeKernel = CreateZeroizeKernelString();
}

/// <summary>
/// Accessors.
/// </summary>

template <typename T> string IterOpenCLKernelCreator<T>::ZeroizeKernel() { return m_ZeroizeKernel; }
template <typename T> string IterOpenCLKernelCreator<T>::ZeroizeEntryPoint() { return m_ZeroizeEntryPoint; }
template <typename T> string IterOpenCLKernelCreator<T>::IterEntryPoint() { return m_IterEntryPoint; }

/// <summary>
/// Create the iteration kernel string using the Cuburn method.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="ember">The ember to create the kernel string for</param>
/// <param name="params">The parametric variation #define string</param>
/// <param name="doAccum">Debugging parameter to include or omit accumulating to the histogram. Default: true.</param>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateIterKernelString(Ember<T>& ember, string& parVarDefines, bool lockAccum, bool doAccum)
{
	bool doublePrecision = typeid(T) == typeid(double);
	size_t i, v, varIndex, varCount, totalXformCount = ember.TotalXformCount();
	ostringstream kernelIterBody, xformFuncs, os;
	vector<Variation<T>*> variations;

	xformFuncs << "\n" << parVarDefines << endl;
	ember.GetPresentVariations(variations);
	ForEach(variations, [&](Variation<T>* var) { if (var) xformFuncs << var->OpenCLFuncsString(); });

	for (i = 0; i < totalXformCount; i++)
	{
		Xform<T>* xform = ember.GetTotalXform(i);
		size_t totalVarCount = xform->TotalVariationCount();
		bool needPrecalcSumSquares = false;
		bool needPrecalcSqrtSumSquares = false;
		bool needPrecalcAngles = false;
		bool needPrecalcAtanXY = false;
		bool needPrecalcAtanYX = false;

		v = varIndex = varCount = 0;
		xformFuncs <<
			"void Xform" << i << "(__constant XformCL* xform, __constant real_t* parVars, Point* inPoint, Point* outPoint, uint2* mwc)\n" <<
			"{\n"
			"	real_t transX, transY, transZ;\n"
			"	real4 vIn, vOut = 0.0;\n";
		
		//Determine if any variations, regular, pre, or post need precalcs.
		while (Variation<T>* var = xform->GetVariation(v++))
		{
			needPrecalcSumSquares     |= var->NeedPrecalcSumSquares();
			needPrecalcSqrtSumSquares |= var->NeedPrecalcSqrtSumSquares();
			needPrecalcAngles         |= var->NeedPrecalcAngles();
			needPrecalcAtanXY         |= var->NeedPrecalcAtanXY();
			needPrecalcAtanYX         |= var->NeedPrecalcAtanYX();
		}

		if (needPrecalcSumSquares)
			xformFuncs << "\treal_t precalcSumSquares;\n";

		if (needPrecalcSqrtSumSquares)
			xformFuncs << "\treal_t precalcSqrtSumSquares;\n";

		if (needPrecalcAngles)
		{
			xformFuncs << "\treal_t precalcSina;\n";
			xformFuncs << "\treal_t precalcCosa;\n";
		}

		if (needPrecalcAtanXY)
			xformFuncs << "\treal_t precalcAtanxy;\n";

		if (needPrecalcAtanYX)
			xformFuncs << "\treal_t precalcAtanyx;\n";

		xformFuncs << "\treal_t tempColor = outPoint->m_ColorX = xform->m_ColorSpeedCache + (xform->m_OneMinusColorCache * inPoint->m_ColorX);\n\n";

		if (xform->PreVariationCount() + xform->VariationCount() == 0)
		{
			xformFuncs <<
			"	outPoint->m_X = (xform->m_A * inPoint->m_X) + (xform->m_B * inPoint->m_Y) + xform->m_C;\n" <<
			"	outPoint->m_Y = (xform->m_D * inPoint->m_X) + (xform->m_E * inPoint->m_Y) + xform->m_F;\n" <<
			"	outPoint->m_Z = inPoint->m_Z;\n";
		}
		else
		{
			xformFuncs <<
			"	transX = (xform->m_A * inPoint->m_X) + (xform->m_B * inPoint->m_Y) + xform->m_C;\n" <<
			"	transY = (xform->m_D * inPoint->m_X) + (xform->m_E * inPoint->m_Y) + xform->m_F;\n" <<
			"	transZ = inPoint->m_Z;\n";

			varCount = xform->PreVariationCount();

			if (varCount > 0)
			{
				xformFuncs << "\n\t//Apply each of the " << varCount << " pre variations in this xform.\n";

				//Output the code for each pre variation in this xform.
				for (varIndex = 0; varIndex < varCount; varIndex++)
				{
					if (Variation<T>* var = xform->GetVariation(varIndex))
					{
						xformFuncs << "\n\t//" << var->Name() << ".\n";
						xformFuncs << var->PrecalcOpenCLString();
						xformFuncs << xform->ReadOpenCLString(VARTYPE_PRE) << endl;
						xformFuncs << var->OpenCLString() << endl;
						xformFuncs << xform->WriteOpenCLString(VARTYPE_PRE, var->AssignType()) << endl;
					}
				}
			}

			if (xform->VariationCount() > 0)
			{
				if (xform->NeedPrecalcSumSquares())
					xformFuncs << "\tprecalcSumSquares = SQR(transX) + SQR(transY);\n";

				if (xform->NeedPrecalcSqrtSumSquares())
					xformFuncs << "\tprecalcSqrtSumSquares = sqrt(precalcSumSquares);\n";

				if (xform->NeedPrecalcAngles())
				{
					xformFuncs << "\tprecalcSina = transX / Zeps(precalcSqrtSumSquares);\n";
					xformFuncs << "\tprecalcCosa = transY / Zeps(precalcSqrtSumSquares);\n";
				}

				if (xform->NeedPrecalcAtanXY())
					xformFuncs << "\tprecalcAtanxy = atan2(transX, transY);\n";

				if (xform->NeedPrecalcAtanYX())
					xformFuncs << "\tprecalcAtanyx = atan2(transY, transX);\n";

				xformFuncs << "\n\toutPoint->m_X = 0;";
				xformFuncs << "\n\toutPoint->m_Y = 0;";
				xformFuncs << "\n\toutPoint->m_Z = 0;\n";
				xformFuncs << "\n\t//Apply each of the " << xform->VariationCount() << " regular variations in this xform.\n\n";
				xformFuncs << xform->ReadOpenCLString(VARTYPE_REG);

				varCount += xform->VariationCount();

				//Output the code for each regular variation in this xform.
				for (; varIndex < varCount; varIndex++)
				{
					if (Variation<T>* var = xform->GetVariation(varIndex))
					{
						xformFuncs << "\n\t//" << var->Name() << ".\n"
							<< var->OpenCLString() << (varIndex == varCount - 1 ? "\n" : "\n\n")
							<< xform->WriteOpenCLString(VARTYPE_REG, ASSIGNTYPE_SUM);
					}
				}
			}
			else
			{
				xformFuncs <<
				"	outPoint->m_X = transX;\n"
				"	outPoint->m_Y = transY;\n"
				"	outPoint->m_Z = transZ;\n";
			}
		}

		if (xform->PostVariationCount() > 0)
		{
			varCount += xform->PostVariationCount();
			xformFuncs << "\n\t//Apply each of the " << xform->PostVariationCount() << " post variations in this xform.\n";

			//Output the code for each post variation in this xform.
			for (; varIndex < varCount; varIndex++)
			{
				if (Variation<T>* var = xform->GetVariation(varIndex))
				{
					xformFuncs << "\n\t//" << var->Name() << ".\n";
					xformFuncs << var->PrecalcOpenCLString();
					xformFuncs << xform->ReadOpenCLString(VARTYPE_POST) << endl;
					xformFuncs << var->OpenCLString() << endl;
					xformFuncs << xform->WriteOpenCLString(VARTYPE_POST, var->AssignType()) << (varIndex == varCount - 1 ? "\n" : "\n\n");
				}
			}
		}
		
		if (xform->HasPost())
		{
			xformFuncs <<
				"\n\t//Apply post affine transform.\n"
				"\treal_t tempX = outPoint->m_X;\n"
				"\n"
				"\toutPoint->m_X = (xform->m_PostA * tempX) + (xform->m_PostB * outPoint->m_Y) + xform->m_PostC;\n" <<
				"\toutPoint->m_Y = (xform->m_PostD * tempX) + (xform->m_PostE * outPoint->m_Y) + xform->m_PostF;\n";
		}

		xformFuncs << "\toutPoint->m_ColorX = outPoint->m_ColorX + xform->m_DirectColor * (tempColor - outPoint->m_ColorX);\n";
		xformFuncs << "}\n"
				   << "\n";
	}

	os <<
		ConstantDefinesString(doublePrecision) <<
		InlineMathFunctionsString <<
		ClampRealFunctionString <<
		RandFunctionString <<
		PointCLStructString <<
		XformCLStructString <<
		EmberCLStructString <<
		UnionCLStructString <<
		CarToRasCLStructString <<
		CarToRasFunctionString <<
		AtomicString(doublePrecision, m_NVidia) <<
		xformFuncs.str() <<
		"__kernel void " << m_IterEntryPoint << "(\n" <<
		"	uint iterCount,\n"
		"	uint fuseCount,\n"
		"	uint seed,\n"
		"	__constant EmberCL* ember,\n"
		"	__constant real_t* parVars,\n"
		"	__global uchar* xformDistributions,\n"//Using uchar is quicker than uint. Can't be constant because the size can be too large to fit when using xaos.//FINALOPT
		"	__constant CarToRasCL* carToRas,\n"
		"	__global real4reals* histogram,\n"
		"	uint histSize,\n"
		"	__read_only image2d_t palette,\n"
		"	__global Point* points\n"
		"\t)\n"
		"{\n"
		"	bool fuse, ok;\n"
		"	uint threadIndex = INDEX_IN_BLOCK_2D;\n"
		"	uint i, itersToDo;\n"
		"	uint consec = 0;\n"
		//"	int badvals = 0;\n"
		"	uint histIndex;\n"
		"	real_t p00, p01;\n"
		"	Point firstPoint, secondPoint, tempPoint;\n"
		"	uint2 mwc;\n"
		"	float4 palColor1;\n"
		"	int2 iPaletteCoord;\n"
		"	const sampler_t paletteSampler = CLK_NORMALIZED_COORDS_FALSE |\n"//Coords from 0 to 255.
		"		CLK_ADDRESS_CLAMP_TO_EDGE |\n"//Clamp to edge
		"		CLK_FILTER_NEAREST;\n"//Don't interpolate
		"	uint threadXY = (THREAD_ID_X + THREAD_ID_Y);\n"
		"	uint threadXDivRows = (THREAD_ID_X / (NTHREADS / THREADS_PER_WARP));\n"
		"	uint threadsMinus1 = NTHREADS - 1;\n"
		;
	
	os <<
		"\n"
		"	__local Point swap[NTHREADS];\n"
		"	__local uint xfsel[NWARPS];\n"
		"\n"
		"	uint pointsIndex = INDEX_IN_GRID_2D;\n"
		"	mwc.x = (pointsIndex + 1 * seed);\n"
		"	mwc.y = ((BLOCK_ID_X + 1) * (pointsIndex + 1) * seed);\n"
		"	iPaletteCoord.y = 0;\n"
		"\n"
		"	if (fuseCount > 0)\n"
		"	{\n"
		"		fuse = true;\n"
		"		itersToDo = fuseCount;\n"
		"		firstPoint.m_X = MwcNextNeg1Pos1(&mwc);\n"
		"		firstPoint.m_Y = MwcNextNeg1Pos1(&mwc);\n"
		"		firstPoint.m_Z = 0.0;\n"
		"		firstPoint.m_ColorX = MwcNext01(&mwc);\n"
		"		firstPoint.m_LastXfUsed = 0;\n"
		"	}\n"
		"	else\n"
		"	{\n"
		"		fuse = false;\n"
		"		itersToDo = iterCount;\n"
		"		firstPoint = points[pointsIndex];\n"
		"	}\n"
		"\n";
	
		//This is done once initially here and then again after each swap-sync in the main loop.
		//This along with the randomness that the point shuffle provides gives sufficient randomness
		//to produce results identical to those produced on the CPU.
	os <<
		"	if (THREAD_ID_Y == 0 && THREAD_ID_X < NWARPS)\n"
		"		xfsel[THREAD_ID_X] = MwcNext(&mwc) % " << CHOOSE_XFORM_GRAIN << ";\n"//It's faster to do the % here ahead of time than every time an xform is looked up to use inside the loop.
		"\n"
		"	barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"
		"	for (i = 0; i < itersToDo; i++)\n"
		"	{\n";

		os <<
		"		consec = 0;\n"
		"\n"
		"		do\n"
		"		{\n";

		//If xaos is present, the cuburn method is effectively ceased. Every thread will be picking a random xform.
		if (ember.XaosPresent())
		{
			os <<
		"			secondPoint.m_LastXfUsed = xformDistributions[MwcNext(&mwc) % " << CHOOSE_XFORM_GRAIN << " + (" << CHOOSE_XFORM_GRAIN << " * (firstPoint.m_LastXfUsed + 1u))];\n\n";
		//"			secondPoint.m_LastXfUsed = xformDistributions[xfsel[THREAD_ID_Y] + (" << CHOOSE_XFORM_GRAIN << " * (firstPoint.m_LastXfUsed + 1u))];\n\n";//Partial cuburn hybrid.
		}
		else
		{
			os <<
		//"			secondPoint.m_LastXfUsed = xformDistributions[MwcNext(&mwc) % " << CHOOSE_XFORM_GRAIN << "];\n\n";//For testing, using straight rand flam4/fractron style instead of cuburn.
		"			secondPoint.m_LastXfUsed = xformDistributions[xfsel[THREAD_ID_Y]];\n\n";
		}

		for (i = 0; i < ember.XformCount(); i++)
		{
			if (i == 0)
				os <<
		"			if (secondPoint.m_LastXfUsed == " << i << ")\n";
			else
				os <<
		"			else if (secondPoint.m_LastXfUsed == " << i << ")\n";

		os <<
		"			{\n" <<
		"				Xform" << i << "(&(ember->m_Xforms[" << i << "]), parVars, &firstPoint, &secondPoint, &mwc);\n" <<
		"			}\n";
		}
		os <<
		"\n"
		"			ok = !BadVal(secondPoint.m_X) && !BadVal(secondPoint.m_Y);\n"
		//"			ok = !BadVal(secondPoint.m_X) && !BadVal(secondPoint.m_Y) && !BadVal(secondPoint.m_Z);\n"
		"\n"
		"			if (!ok)\n"
		"			{\n"
		"				firstPoint.m_X = MwcNextNeg1Pos1(&mwc);\n"
		"				firstPoint.m_Y = MwcNextNeg1Pos1(&mwc);\n"
		"				firstPoint.m_Z = 0.0;\n"
		"				firstPoint.m_ColorX = secondPoint.m_ColorX;\n"
		"				consec++;\n"
		//"				badvals++;\n"
		"			}\n"
		"		}\n"
		"		while (!ok && consec < 5);\n"
		"\n"
		"		if (!ok)\n"
		"		{\n"
		"			secondPoint.m_X = MwcNextNeg1Pos1(&mwc);\n"
		"			secondPoint.m_Y = MwcNextNeg1Pos1(&mwc);\n"
		"			secondPoint.m_Z = 0.0;\n"
		"		}\n"
		"\n"//Rotate points between threads. This is how randomization is achieved.
		"		uint swr = threadXY + ((i & 1u) * threadXDivRows);\n"
		"		uint sw = (swr * THREADS_PER_WARP + THREAD_ID_X) & threadsMinus1;\n"
		"\n"

		//Write to another thread's location.
		"		swap[sw] = secondPoint;\n"
		"\n"

		//Populate randomized xform index buffer with new random values.
		"		if (THREAD_ID_Y == 0 && THREAD_ID_X < NWARPS)\n"
		"			xfsel[THREAD_ID_X] = MwcNext(&mwc) % " << CHOOSE_XFORM_GRAIN << ";\n"
		"\n"
		"		barrier(CLK_LOCAL_MEM_FENCE);\n"
		"\n"

		//Another thread will have written to this thread's location, so read the new value and use it for accumulation below.
		"		firstPoint = swap[threadIndex];\n"
		"\n"
		"		if (fuse)\n"
		"		{\n"
		"			if (i >= fuseCount - 1)\n"
		"			{\n"
		"				i = 0;\n"
		"				fuse = false;\n"
		"				itersToDo = iterCount;\n"
		"				barrier(CLK_LOCAL_MEM_FENCE);\n"//Sort of seems necessary, sort of doesn't. Makes no speed difference.
		"			}\n"
		"\n"
		"			continue;\n"
		"		}\n"
		"\n";
		
		if (ember.UseFinalXform())
		{
			size_t finalIndex = ember.TotalXformCount() - 1;

			//CPU takes an extra step here to preserve the opacity of the randomly selected xform, rather than the final xform's opacity.
			//The same thing takes place here automatically because secondPoint.m_LastXfUsed is used below to retrieve the opacity when accumulating.
			os <<
		"		if ((ember->m_Xforms[" << finalIndex << "].m_Opacity == 1) || (MwcNext01(&mwc) < ember->m_Xforms[" << finalIndex << "].m_Opacity))\n"
		"		{\n"
		"			tempPoint.m_LastXfUsed = secondPoint.m_LastXfUsed;\n"
		"			Xform" << finalIndex << "(&(ember->m_Xforms[" << finalIndex << "]), parVars, &secondPoint, &tempPoint, &mwc);\n"
		"			secondPoint = tempPoint;\n"
		"		}\n"
		"\n";
		}
		
		os << CreateProjectionString(ember);

		if (doAccum)
		{
			os <<
		"		p00 = secondPoint.m_X - ember->m_CenterX;\n"
		"		p01 = secondPoint.m_Y - ember->m_CenterY;\n"
		"		tempPoint.m_X = (p00 * ember->m_RotA) + (p01 * ember->m_RotB) + ember->m_CenterX;\n"
		"		tempPoint.m_Y = (p00 * ember->m_RotD) + (p01 * ember->m_RotE) + ember->m_CenterY;\n"
		"\n"
		//Add this point to the appropriate location in the histogram.
		"		if (CarToRasInBounds(carToRas, &tempPoint))\n"
		"		{\n"
		"			CarToRasConvertPointToSingle(carToRas, &tempPoint, &histIndex);\n"
		"\n"
		"			if (histIndex < histSize)\n"//Provides an extra level of safety and makes no speed difference.
		"			{\n";

			//Basic texture index interoplation does not produce identical results
			//to the CPU. So the code here must explicitly do the same thing and not
			//rely on the GPU texture coordinate lookup.
			if (ember.m_PaletteMode == PALETTE_LINEAR)
			{
			os <<
		"				real_t colorIndexFrac;\n"
		"				real_t colorIndex = secondPoint.m_ColorX * COLORMAP_LENGTH;\n"
		"				int intColorIndex = (int)colorIndex;\n"
		"				float4 palColor2;\n"
		"\n"
		"				if (intColorIndex < 0)\n"
		"				{\n"
		"					intColorIndex = 0;\n"
		"					colorIndexFrac = 0;\n"
		"				}\n"
		"				else if (intColorIndex >= COLORMAP_LENGTH_MINUS_1)\n"
		"				{\n"
		"					intColorIndex = COLORMAP_LENGTH_MINUS_1 - 1;\n"
		"					colorIndexFrac = 1.0;\n"
		"				}\n"
		"				else\n"
		"				{\n"
		"					colorIndexFrac = colorIndex - (real_t)intColorIndex;\n"//Interpolate between intColorIndex and intColorIndex + 1.
		"				}\n"
		"\n"
		"				iPaletteCoord.x = intColorIndex;\n"//Palette operations are strictly float because OpenCL does not support dp64 textures.
		"				palColor1 = read_imagef(palette, paletteSampler, iPaletteCoord);\n"
		"				iPaletteCoord.x += 1;\n"
		"				palColor2 = read_imagef(palette, paletteSampler, iPaletteCoord);\n"
		"				palColor1 = (palColor1 * (1.0f - (float)colorIndexFrac)) + (palColor2 * (float)colorIndexFrac);\n";//The 1.0f here *must* have the 'f' suffix at the end to compile.
			}
			else if (ember.m_PaletteMode == PALETTE_STEP)
			{
				os <<
		"				iPaletteCoord.x = (int)(secondPoint.m_ColorX * COLORMAP_LENGTH);\n"
		"				palColor1 = read_imagef(palette, paletteSampler, iPaletteCoord);\n";
			}

			if (lockAccum)
			{
				if (typeid(T) == typeid(double))
				{
					os <<
		"				AtomicAdd(&(histogram[histIndex].m_Reals[0]), (real_t)palColor1.x * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"//Always apply opacity, even though it's usually 1.
		"				AtomicAdd(&(histogram[histIndex].m_Reals[1]), (real_t)palColor1.y * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"
		"				AtomicAdd(&(histogram[histIndex].m_Reals[2]), (real_t)palColor1.z * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"
		"				AtomicAdd(&(histogram[histIndex].m_Reals[3]), (real_t)palColor1.w * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n";
				}
				else
				{
				os <<
		"				AtomicAdd(&(histogram[histIndex].m_Reals[0]), palColor1.x * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"//Always apply opacity, even though it's usually 1.
		"				AtomicAdd(&(histogram[histIndex].m_Reals[1]), palColor1.y * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"
		"				AtomicAdd(&(histogram[histIndex].m_Reals[2]), palColor1.z * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n"
		"				AtomicAdd(&(histogram[histIndex].m_Reals[3]), palColor1.w * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n";
				}
			}
			else
			{
				if (typeid(T) == typeid(double))
				{
					os <<
		"				real4 realColor;\n"
		"\n"
		"				realColor.x = (real_t)palColor1.x;\n"
		"				realColor.y = (real_t)palColor1.y;\n"
		"				realColor.z = (real_t)palColor1.z;\n"
		"				realColor.w = (real_t)palColor1.w;\n"
		"				histogram[histIndex].m_Real4 += (realColor * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n";
				}
				else
				{
				os <<
		"				histogram[histIndex].m_Real4 += (palColor1 * ember->m_Xforms[secondPoint.m_LastXfUsed].m_VizAdjusted);\n";
				}
			}

			os <<
		"			}\n"//histIndex < histSize.
		"		}\n"//CarToRasInBounds.
		"\n"
		"		barrier(CLK_GLOBAL_MEM_FENCE);\n";//Barrier every time, whether or not the point was in bounds, else artifacts will occur when doing strips.
		}
		
	os <<
		"	}\n"//Main for loop.
		"\n"
		//At this point, iterating for this round is done, so write the final points back out
		//to the global points buffer to be used as inputs for the next round. This preserves point trajectory
		//between kernel calls.
#ifdef TEST_CL_BUFFERS//Use this to populate with test values and read back in EmberTester.
		"	points[pointsIndex].m_X = MwcNextNeg1Pos1(&mwc);\n"
		"	points[pointsIndex].m_Y = MwcNextNeg1Pos1(&mwc);\n"
		"	points[pointsIndex].m_Z = MwcNextNeg1Pos1(&mwc);\n"
		"	points[pointsIndex].m_ColorX = MwcNextNeg1Pos1(&mwc);\n"
#else
		"	points[pointsIndex] = firstPoint;\n"
#endif
		"	barrier(CLK_GLOBAL_MEM_FENCE);\n"
		"}\n";

	return os.str();
}

/// <summary>
/// Create an OpenCL string of #defines and a corresponding host side vector for parametric variation values.
/// Parametric variations present a special problem in the iteration code.
/// The values can't be passed in with the array of other xform values because
/// the length of the parametric values is unknown.
/// This is solved by passing a separate buffer of values dedicated specifically
/// to parametric variations.
/// In OpenCL, a series of #define constants are declared which specify the indices in 
/// the buffer where the various values are stored.
/// The possibility of a parametric variation type being present in multiple xforms is taken
/// into account by appending the xform index to the #define, thus making each unique.
/// The kernel creator then uses these to retrieve the values in the iteration code.
/// Example:
/// Xform1: Curl (curl_c1: 1.1, curl_c2: 2.2)
/// Xform2: Curl (curl_c1: 4.4, curl_c2: 5.5)
/// Xform3: Blob (blob_low: 1, blob_high: 2, blob_waves: 3)
///
/// Host vector to be passed as arg to the iter kernel call:
/// [1.1][2.2][4.4][5.5][1][2][3]
///
/// #defines in OpenCL to access the buffer:
///
/// #define CURL_C1_1 0
/// #define CURL_C2_1 1
/// #define CURL_C1_2 2
/// #define CURL_C2_2 3
/// #define BLOB_LOW_3 4
/// #define BLOB_HIGH_3 5
/// #define BLOB_WAVES_ 6
///
/// The variations the use these #defines by first looking up the index of the
/// xform they belong to in the parent ember and generating the OpenCL string based on that
/// in their overridden OpenCLString() functions.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="ember">The ember to create the values from</param>
/// <param name="params">The string,vector pair to store the values in</param>
/// <param name="doVals">True if the vector should be populated, else false. Default: true.</param>
/// <param name="doString">True if the string should be populated, else false. Default: true.</param>
template <typename T>
void IterOpenCLKernelCreator<T>::ParVarIndexDefines(Ember<T>& ember, pair<string, vector<T>>& params, bool doVals, bool doString)
{
	size_t i, j, k, size = 0, xformCount = ember.TotalXformCount();
	Xform<T>* xform;
	ostringstream os;

	if (doVals)
		params.second.clear();

	for (i = 0; i < xformCount; i++)
	{
		if (xform = ember.GetTotalXform(i))
		{
			size_t varCount = xform->TotalVariationCount();

			for (j = 0; j < varCount; j++)
			{
				if (ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(xform->GetVariation(j)))
				{
					for (k = 0; k < parVar->ParamCount(); k++)
					{
						if (doString)
							os << "#define " << ToUpper(parVar->Params()[k].Name()) << "_" << i << " " << size << endl;//Uniquely identify this param in this variation in this xform.

						if (doVals)
							params.second.push_back(parVar->Params()[k].ParamVal());

						size++;
					}
				}
			}
		}
	}

	if (doString)
	{
		os << "\n";
		params.first = os.str();
	}
}

/// <summary>
/// Determine whether the two embers passed in differ enough
/// to require a rebuild of the iteration code.
/// A rebuild is required if they differ in the following ways:
/// Xform count
/// Final xform presence
/// Xaos presence
/// Palette accumulation mode
/// Xform post affine presence
/// Variation count
/// Variation type
/// Template argument expected to be float or double.
/// </summary>
/// <param name="ember1">The first ember to compare</param>
/// <param name="ember2">The second ember to compare</param>
/// <returns>True if a rebuild is required, else false</returns>
template <typename T>
bool IterOpenCLKernelCreator<T>::IsBuildRequired(Ember<T>& ember1, Ember<T>& ember2)
{
	size_t i, j, xformCount = ember1.TotalXformCount();
	
	if (xformCount != ember2.TotalXformCount())
		return true;

	if (ember1.UseFinalXform() != ember2.UseFinalXform())
		return true;

	if (ember1.XaosPresent() != ember2.XaosPresent())
		return true;

	if (ember1.m_PaletteMode != ember2.m_PaletteMode)
		return true;

	if (ember1.ProjBits() != ember2.ProjBits())
		return true;

	for (i = 0; i < xformCount; i++)
	{
		Xform<T>* xform1 = ember1.GetTotalXform(i);
		Xform<T>* xform2 = ember2.GetTotalXform(i);
		size_t varCount = xform1->TotalVariationCount();

		if (xform1->HasPost() != xform2->HasPost())
			return true;

		if (varCount != xform2->TotalVariationCount())
			return true;

		for (j = 0; j < varCount; j++)
			if (xform1->GetVariation(j)->VariationId() != xform2->GetVariation(j)->VariationId())
				return true;
	}

	return false;
}

/// <summary>
/// Create the zeroize kernel string.
/// OpenCL comes with no way to zeroize a buffer like memset()
/// would do on the CPU. So a special kernel must be ran to set a range
/// of memory addresses to zero.
/// </summary>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateZeroizeKernelString()
{
	ostringstream os;

	os <<
		ConstantDefinesString(typeid(T) == typeid(double)) <<//Double precision doesn't matter here since it's not used.
		"__kernel void " << m_ZeroizeEntryPoint << "(__global uchar* buffer, uint width, uint height)\n"
		"{\n"
		"	if (GLOBAL_ID_X >= width || GLOBAL_ID_Y >= height)\n"
		"		return;\n"
		"\n"
		"	buffer[(GLOBAL_ID_Y * width) + GLOBAL_ID_X] = 0;\n"//Can't use INDEX_IN_GRID_2D here because the grid might be larger than the buffer to make even dimensions.
		"	barrier(CLK_GLOBAL_MEM_FENCE);\n"//Just to be safe.
		"}\n"
		"\n";

	return os.str();
}

/// <summary>
/// Create the string for 3D projection based on the 3D values of the ember.
/// Projection is done on the second point.
/// If any of these fields toggle between 0 and nonzero between runs, a recompile is triggered.
/// </summary>
/// <param name="ember">The ember to create the projection string for</param>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateProjectionString(Ember<T>& ember)
{
	size_t projBits = ember.ProjBits();
	ostringstream os;

	if (projBits)
	{
		if (projBits & PROJBITS_BLUR)
		{
			if (projBits & PROJBITS_YAW)
			{
				os <<
	"		real_t dsin, dcos;\n"
	"		real_t t = MwcNext01(&mwc) * M_2PI;\n"
	"		real_t z = secondPoint.m_Z - ember->m_CamZPos;\n"
	"		real_t x = ember->m_C00 * secondPoint.m_X + ember->m_C10 * secondPoint.m_Y;\n"
	"		real_t y = ember->m_C01 * secondPoint.m_X + ember->m_C11 * secondPoint.m_Y + ember->m_C21 * z;\n"
	"\n"
	"		z = ember->m_C02 * secondPoint.m_X + ember->m_C12 * secondPoint.m_Y + ember->m_C22 * z;\n"
	"\n"
	"		real_t zr = Zeps(1 - ember->m_CamPerspective * z);\n"
	"		real_t dr = MwcNext01(&mwc) * ember->m_BlurCoef * z;\n"
	"\n"
	"		dsin = sin(t);\n"
	"		dcos = cos(t);\n"
	"\n"
	"		secondPoint.m_X  = (x + dr * dcos) / zr;\n"
	"		secondPoint.m_Y  = (y + dr * dsin) / zr;\n"
	"		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
			else
			{
				os <<
	"		real_t y, z, zr;\n"
	"		real_t dsin, dcos;\n"
	"		real_t t = MwcNext01(&mwc) * M_2PI;\n"
	"\n"
	"		z = secondPoint.m_Z - ember->m_CamZPos;\n"
	"		y = ember->m_C11 * secondPoint.m_Y + ember->m_C21 * z;\n"
	"		z = ember->m_C12 * secondPoint.m_Y + ember->m_C22 * z;\n"
	"		zr = Zeps(1 - ember->m_CamPerspective * z);\n"
	"\n"
	"		dsin = sin(t);\n"
	"		dcos = cos(t);\n"
	"\n"
	"		real_t dr = MwcNext01(&mwc) * ember->m_BlurCoef * z;\n"
	"\n"
	"		secondPoint.m_X = (secondPoint.m_X + dr * dcos) / zr;\n"
	"		secondPoint.m_Y = (y + dr * dsin) / zr;\n"
	"		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
		}
		else if ((projBits & PROJBITS_PITCH) || (projBits & PROJBITS_YAW))
		{
			if (projBits & PROJBITS_YAW)
			{
				os <<
	"		real_t z  = secondPoint.m_Z - ember->m_CamZPos;\n"
	"		real_t x  = ember->m_C00 * secondPoint.m_X + ember->m_C10 * secondPoint.m_Y;\n"
	"		real_t y  = ember->m_C01 * secondPoint.m_X + ember->m_C11 * secondPoint.m_Y + ember->m_C21 * z;\n"
	"		real_t zr = Zeps(1 - ember->m_CamPerspective * (ember->m_C02 * secondPoint.m_X + ember->m_C12 * secondPoint.m_Y + ember->m_C22 * z));\n"
	"\n"
	"		secondPoint.m_X = x / zr;\n"
	"		secondPoint.m_Y = y / zr;\n"
	"		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
			else
			{
				os <<
	"		real_t z  = secondPoint.m_Z - ember->m_CamZPos;\n"
	"		real_t y  = ember->m_C11 * secondPoint.m_Y + ember->m_C21 * z;\n"
	"		real_t zr = Zeps(1 - ember->m_CamPerspective * (ember->m_C12 * secondPoint.m_Y + ember->m_C22 * z));\n"
	"\n"
	"		secondPoint.m_X /= zr;\n"
	"		secondPoint.m_Y  = y / zr;\n"
	"		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
		}
		else
		{
			os <<
	"		real_t zr = Zeps(1 - ember->m_CamPerspective * (secondPoint.m_Z - ember->m_CamZPos));\n"
	"\n"
	"		secondPoint.m_X /= zr;\n"
	"		secondPoint.m_Y /= zr;\n"
	"		secondPoint.m_Z -= ember->m_CamZPos;\n";
		}
	}

	return os.str();
}
}