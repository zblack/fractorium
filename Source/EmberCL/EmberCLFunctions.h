#pragma once

#include "EmberCLPch.h"
#include "EmberCLStructs.h"

/// <summary>
/// OpenCL global function strings.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// OpenCL equivalent of Palette::RgbToHsv().
/// </summary>
static const char* RgbToHsvFunctionString = 
	//rgb 0 - 1,
	//h 0 - 6, s 0 - 1, v 0 - 1
	"static inline void RgbToHsv(real4* rgb, real4* hsv)\n"
	"{\n"
	"	real_t max, min, del, rc, gc, bc;\n"
	"\n"
	//Compute maximum of r, g, b.
	"	if ((*rgb).x >= (*rgb).y)\n"
	"	{\n"
	"		if ((*rgb).x >= (*rgb).z)\n"
	"			max = (*rgb).x;\n"
	"		else\n"
	"			max = (*rgb).z;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		if ((*rgb).y >= (*rgb).z)\n"
	"			max = (*rgb).y;\n"
	"		else\n"
	"			max = (*rgb).z;\n"
	"	}\n"
	"\n"
	//Compute minimum of r, g, b.
	"	if ((*rgb).x <= (*rgb).y)\n"
	"	{\n"
	"		if ((*rgb).x <= (*rgb).z)\n"
	"			min = (*rgb).x;\n"
	"		else\n"
	"			min = (*rgb).z;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		if ((*rgb).y <= (*rgb).z)\n"
	"			min = (*rgb).y;\n"
	"		else\n"
	"			min = (*rgb).z;\n"
	"	}\n"
	"\n"
	"	del = max - min;\n"
	"	(*hsv).z = max;\n"
	"\n"
	"	if (max != 0)\n"
	"		(*hsv).y = del / max;\n"
	"	else\n"
	"		(*hsv).y = 0;\n"
	"\n"
	"	(*hsv).x = 0;\n"
	"	if ((*hsv).y != 0)\n"
	"	{\n"
	"		rc = (max - (*rgb).x) / del;\n"
	"		gc = (max - (*rgb).y) / del;\n"
	"		bc = (max - (*rgb).z) / del;\n"
	"\n"
	"		if ((*rgb).x == max)\n"
	"			(*hsv).x = bc - gc;\n"
	"		else if ((*rgb).y == max)\n"
	"			(*hsv).x = 2 + rc - bc;\n"
	"		else if ((*rgb).z == max)\n"
	"			(*hsv).x = 4 + gc - rc;\n"
	"\n"
	"		if ((*hsv).x < 0)\n"
	"			(*hsv).x += 6;\n"
	"	}\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of Palette::HsvToRgb().
/// </summary>
static const char* HsvToRgbFunctionString = 
	//h 0 - 6, s 0 - 1, v 0 - 1
	//rgb 0 - 1 
	"static inline void HsvToRgb(real4* hsv, real4* rgb)\n"
	"{\n"
	"	int j;\n"
	"	real_t f, p, q, t;\n"
	"\n"
	"	while ((*hsv).x >= 6)\n"
	"		(*hsv).x = (*hsv).x - 6;\n"
	"\n"
	"	while ((*hsv).x <  0)\n"
	"		(*hsv).x = (*hsv).x + 6;\n"
	"\n"
	"	j = (int)floor((*hsv).x);\n"
	"	f = (*hsv).x - j;\n"
	"	p = (*hsv).z * (1 - (*hsv).y);\n"
	"	q = (*hsv).z * (1 - ((*hsv).y * f));\n"
	"	t = (*hsv).z * (1 - ((*hsv).y * (1 - f)));\n"
	"\n"
	"	switch (j)\n"
	"	{\n"
	"		case 0:  (*rgb).x = (*hsv).z; (*rgb).y = t;		   (*rgb).z = p;	    break;\n"
	"		case 1:  (*rgb).x = q;		  (*rgb).y = (*hsv).z; (*rgb).z = p;	    break;\n"
	"		case 2:  (*rgb).x = p;		  (*rgb).y = (*hsv).z; (*rgb).z = t;	    break;\n"
	"		case 3:  (*rgb).x = p;		  (*rgb).y = q;		   (*rgb).z = (*hsv).z; break;\n"
	"		case 4:  (*rgb).x = t;		  (*rgb).y = p;		   (*rgb).z = (*hsv).z; break;\n"
	"		case 5:  (*rgb).x = (*hsv).z; (*rgb).y = p;		   (*rgb).z = q;	    break;\n"
	"		default: (*rgb).x = (*hsv).z; (*rgb).y = t;		   (*rgb).z = p;	    break;\n"
	"	}\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of Palette::CalcAlpha().
/// </summary>
static const char* CalcAlphaFunctionString = 
	"static inline real_t CalcAlpha(real_t density, real_t gamma, real_t linrange)\n"//Not the slightest clue what this is doing.//DOC
	"{\n"
	"	real_t frac, alpha, funcval = pow(linrange, gamma);\n"
		"\n"
	"	if (density > 0)\n"
	"	{\n"
	"		if (density < linrange)\n"
	"		{\n"
	"			frac = density / linrange;\n"
	"			alpha = (1.0 - frac) * density * (funcval / linrange) + frac * pow(density, gamma);\n"
	"		}\n"
	"		else\n"
	"			alpha = pow(density, gamma);\n"
	"	}\n"
	"	else\n"
	"		alpha = 0;\n"
	"\n"
	"	return alpha;\n"
	"}\n"
	"\n";


/// <summary>
/// OpenCL equivalent of Renderer::CurveAdjust().
/// Only use float here instead of real_t because the output will be passed to write_imagef()
/// during final accumulation, which only takes floats.
/// </summary>
static const char* CurveAdjustFunctionString =
"static inline void CurveAdjust(__constant real4reals* csa, float* a, uint index)\n"
"{\n"
"	uint tempIndex = (uint)Clamp(*a, 0.0, (float)COLORMAP_LENGTH_MINUS_1);\n"
"	uint tempIndex2 = (uint)Clamp(csa[tempIndex].m_Real4.x, 0.0, (real_t)COLORMAP_LENGTH_MINUS_1);\n"
"\n"
"	*a = (float)round(csa[tempIndex2].m_Reals[index]);\n"
"}\n";

/// <summary>
/// Use MWC 64 from David Thomas at the Imperial College of London for
/// random numbers in OpenCL, instead of ISAAC which was used
/// for CPU rendering.
/// </summary>
static const char* RandFunctionString =
	"enum { MWC64X_A = 4294883355u };\n\n"
	"inline uint MwcNext(uint2* s)\n"
	"{\n"
	"	uint res = (*s).x ^ (*s).y;			\n"//Calculate the result.
	"	uint hi = mul_hi((*s).x, MWC64X_A); \n"//Step the RNG.
	"	(*s).x = (*s).x * MWC64X_A + (*s).y;\n"//Pack the state back up.
	"	(*s).y = hi + ((*s).x < (*s).y);	\n"
	"	return res;							\n"//Return the next result.
	"}\n"
	"\n"
	"inline uint MwcNextRange(uint2* s, uint val)\n"
	"{\n"
	"	return (val == 0) ? MwcNext(s) : (MwcNext(s) % val);\n"
	"}\n"
	"\n"
	"inline real_t MwcNext01(uint2* s)\n"
	"{\n"
	"	return MwcNext(s) * (1.0 / 4294967296.0);\n"
	"}\n"
	"\n"
	"inline real_t MwcNextNeg1Pos1(uint2* s)\n"
	"{\n"
	"	real_t f = (real_t)MwcNext(s) / (real_t)UINT_MAX;\n"
	"	return -1.0 + (f * 2.0);\n"
	"}\n"
	"\n"
	"inline real_t MwcNext0505(uint2* s)\n"
	"{\n"
	"	real_t f = (real_t)MwcNext(s) / (real_t)UINT_MAX;\n"
	"	return -0.5 + f;\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of the global ClampRef().
/// </summary>
static const char* ClampRealFunctionString =
	"inline real_t Clamp(real_t val, real_t min, real_t max)\n"
	"{\n"
	"	if (val < min)\n"
	"		return min;\n"
	"	else if (val > max)\n"
	"		return max;\n"
	"	else\n"
	"		return val;\n"
	"}\n"
	"\n"
	"inline void ClampRef(real_t* val, real_t min, real_t max)\n"
	"{\n"
	"	if (*val < min)\n"
	"		*val = min;\n"
	"	else if (*val > max)\n"
	"		*val = max;\n"
	"}\n"
	"\n"
	"inline real_t ClampGte(real_t val, real_t gte)\n"
	"{\n"
	"	return (val < gte) ? gte : val;\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of the global LRint().
/// </summary>
static const char* InlineMathFunctionsString = 
	"inline real_t LRint(real_t x)\n"
	"{\n"
	"    intPrec temp = (x >= 0.0 ? (intPrec)(x + 0.5) : (intPrec)(x - 0.5));\n"
	"    return (real_t)temp;\n"
	"}\n"
	"\n"
	"inline real_t Round(real_t r)\n"
	"{\n"
	"	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);\n"
	"}\n"
	"\n"
	"inline real_t Sign(real_t v)\n"
	"{\n"
	"	return (v < 0.0) ? -1 : (v > 0.0) ? 1 : 0.0;\n"
	"}\n"
	"\n"
	"inline real_t SignNz(real_t v)\n"
	"{\n"
	"	return (v < 0.0) ? -1.0 : 1.0;\n"
	"}\n"
	"\n"
	"inline real_t Sqr(real_t v)\n"
	"{\n"
	"	return v * v;\n"
	"}\n"
	"\n"
	"inline real_t SafeSqrt(real_t x)\n"
	"{\n"
	"	if (x <= 0.0)\n"
	"		return 0.0;\n"
	"\n"
	"	return sqrt(x);\n"
	"}\n"
	"\n"
	"inline real_t Cube(real_t v)\n"
	"{\n"
	"	return v * v * v;\n"
	"}\n"
	"\n"
	"inline real_t Hypot(real_t x, real_t y)\n"
	"{\n"
	"	return sqrt(SQR(x) + SQR(y));\n"
	"}\n"
	"\n"
	"inline real_t Spread(real_t x, real_t y)\n"
	"{\n"
	"	return Hypot(x, y) * ((x) > 0.0 ? 1.0 : -1.0);\n"
	"}\n"
	"\n"
	"inline real_t Powq4(real_t x, real_t y)\n"
	"{\n"
	"	return pow(fabs(x), y) * SignNz(x);\n"
	"}\n"
	"\n"
	"inline real_t Powq4c(real_t x, real_t y)\n"
	"{\n"
	"	return y == 1.0 ? x : Powq4(x, y);\n"
	"}\n"
	"\n"
	"inline real_t Zeps(real_t x)\n"
	"{\n"
	"	return x == 0.0 ? EPS : x;\n"
	"}\n"
	"\n"
	"inline real_t Lerp(real_t a, real_t b, real_t p)\n"
	"{\n"
	"	return a + (b - a) * p;\n"
	"}\n"
	"\n"
	"inline real_t Fabsmod(real_t v)\n"
	"{\n"
	"	real_t dummy;\n"
	"\n"
	"	return modf(v, &dummy);\n"
	"}\n"
	"\n"
	"inline real_t Fosc(real_t p, real_t amp, real_t ph)\n"
	"{\n"
	"	return 0.5 - cos(p * amp + ph) * 0.5;\n"
	"}\n"
	"\n"
	"inline real_t Foscn(real_t p, real_t ph)\n"
	"{\n"
	"	return 0.5 - cos(p + ph) * 0.5;\n"
	"}\n"
	"\n"
	"inline real_t LogScale(real_t x)\n"
	"{\n"
	"	return x == 0.0 ? 0.0 : log((fabs(x) + 1) * M_E) * SignNz(x) / M_E;\n"
	"}\n"
	"\n"
	"inline real_t LogMap(real_t x)\n"
	"{\n"
	"	return x == 0.0 ? 0.0 : (M_E + log(x * M_E)) * 0.25 * SignNz(x);\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent Renderer::AddToAccum().
/// </summary>
static const char* AddToAccumWithCheckFunctionString = 
	"inline bool AccumCheck(int superRasW, int superRasH, int i, int ii, int j, int jj)\n"
	"{\n"
	"	return (j + jj >= 0 && j + jj < superRasH && i + ii >= 0 && i + ii < superRasW);\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent various CarToRas member functions.
/// </summary>
static const char* CarToRasFunctionString = 
	"inline void CarToRasConvertPointToSingle(__constant CarToRasCL* carToRas, Point* point, uint* singleBufferIndex)\n"
	"{\n"
	"	*singleBufferIndex = (uint)(carToRas->m_PixPerImageUnitW * point->m_X - carToRas->m_RasLlX) + (carToRas->m_RasWidth * (uint)(carToRas->m_PixPerImageUnitH * point->m_Y - carToRas->m_RasLlY));\n"
	"}\n"
	"\n"
	"inline bool CarToRasInBounds(__constant CarToRasCL* carToRas, Point* point)\n"
	"{\n"
	"	return point->m_X >= carToRas->m_CarLlX &&\n"
	"		point->m_X < carToRas->m_CarUrX &&\n"
	"		point->m_Y < carToRas->m_CarUrY &&\n"
	"		point->m_Y >= carToRas->m_CarLlY;\n"
	"}\n"
	"\n";

static string AtomicString(bool doublePrecision, bool dp64AtomicSupport)
{
	ostringstream os;

	//If they want single precision, or if they want double precision and have dp atomic support.
	if (!doublePrecision || dp64AtomicSupport)
	{
		os <<
		"void AtomicAdd(volatile __global real_t* source, const real_t operand)\n"
		"{\n"
		"	union\n"
		"	{\n"
		"		atomi intVal;\n"
		"		real_t realVal;\n"
		"	} newVal;\n"
		"\n"
		"	union\n"
		"	{\n"
		"		atomi intVal;\n"
		"		real_t realVal;\n"
		"	} prevVal;\n"
		"\n"
		"	do\n"
		"	{\n"
		"		prevVal.realVal = *source;\n"
		"		newVal.realVal = prevVal.realVal + operand;\n"
		"	} while (atomic_cmpxchg((volatile __global atomi*)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);\n"
		"}\n";
	}
	else//They want double precision and do not have dp atomic support.
	{
		os <<
		"void AtomicAdd(volatile __global real_t* source, const real_t operand)\n"
		"{\n"
		"	union\n"
		"	{\n"
		"		uint intVal[2];\n"
		"		real_t realVal;\n"
		"	} newVal;\n"
		"\n"
		"	union\n"
		"	{\n"
		"		uint intVal[2];\n"
		"		real_t realVal;\n"
		"	} prevVal;\n"
		"\n"
		"	do\n"
		"	{\n"
		"		prevVal.realVal = *source;\n"
		"		newVal.realVal = prevVal.realVal + operand;\n"
		"	} while ((atomic_cmpxchg((volatile __global uint*)source, prevVal.intVal[0], newVal.intVal[0])     != prevVal.intVal[0]) ||\n"
		"			 (atomic_cmpxchg((volatile __global uint*)source + 1, prevVal.intVal[1], newVal.intVal[1]) != prevVal.intVal[1]));\n"
		"}\n";
	}

	return os.str();
}

#ifdef GRAVEYARD
/*"void AtomicLocalAdd(volatile __local real_t* source, const real_t operand)\n"
	"{\n"
	"	union\n"
	"	{\n"
	"		atomi intVal;\n"
	"		real_t realVal;\n"
	"	} newVal;\n"
	"\n"
	"	union\n"
	"	{\n"
	"		atomi intVal;\n"
	"		real_t realVal;\n"
	"	} prevVal;\n"
	"\n"
	"	do\n"
	"	{\n"
	"		prevVal.realVal = *source;\n"
	"		newVal.realVal = prevVal.realVal + operand;\n"
	"	} while (atomic_cmpxchg((volatile __local atomi*)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);\n"
	"}\n"*/
#endif
}