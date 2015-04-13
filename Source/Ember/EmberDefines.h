#pragma once

#include "EmberPch.h"

/// <summary>
/// Basic #defines used throughout the library.
/// </summary>

#ifdef _WIN32
	#if defined(BUILDING_EMBER)
		#define EMBER_API __declspec(dllexport)
	#else
		#define EMBER_API __declspec(dllimport)
	#endif
#else
	#define EMBER_API
	#define fopen_s(pFile,filename,mode) ((*(pFile)=fopen((filename),(mode)))==nullptr)
	#define _stat stat
	#define _fstat fstat
	#define _stricmp strcmp
	#define sscanf_s sscanf
	#define sprintf_s snprintf
	#define snprintf_s snprintf
	typedef int errno_t;
#endif

#define RESTRICT __restrict//This might make things faster, unsure if it really does though.
//#define RESTRICT

//Wrap the sincos function for Macs and PC.
#if defined(__APPLE__) || defined(_MSC_VER)
	#define sincos(x, s, c) *(s)=sin(x); *(c)=cos(x);
#else
	//extern void sincos(double x, double *s, double *c);
	//extern void sincos(float x, float *s, float *c);
	static void sincos(float x, float* s, float* c)
	{
		*s = sin(x);
		*c = cos(x);
	}
#endif

namespace EmberNs
{
#define EMBER_VERSION "0.4.1.9"
#define EPS6 T(1e-6)
#define EPS std::numeric_limits<T>::epsilon()//Apoplugin.h uses -20, but it's more mathematically correct to do it this way.
#define ISAAC_SIZE 4
#define MEMALIGN 32
#define DE_THRESH 100
#define MAX_VARS_PER_XFORM 8
#define DEG_2_RAD (M_PI / 180)
#define RAD_2_DEG (180 / M_PI)
#define DEG_2_RAD_T (T(M_PI) / T(180))
#define RAD_2_DEG_T (T(180) / T(M_PI))
#define M_2PI (T(M_PI * 2))
#define M_3PI (T(M_PI * 3))
#define SQRT5 T(2.2360679774997896964091736687313)
#define M_PHI T(1.61803398874989484820458683436563)
#define COLORMAP_LENGTH 256//These will need to change if 2D palette support is ever added, or variable sized palettes.
#define COLORMAP_LENGTH_MINUS_1 255
#define WHITE 255
#define DEFAULT_SBS (1024 * 10)
//#define XC(c) ((const xmlChar*)(c))
#define XC(c) (reinterpret_cast<const xmlChar*>(c))
#define CX(c) (reinterpret_cast<char*>(c))
#define CCX(c) (reinterpret_cast<const char*>(c))
#define BadVal(x) (((x) != (x)) || ((x) > 1e10) || ((x) < -1e10))
#define Rint(A) floor((A) + (((A) < 0) ? T(-0.5) : T(0.5)))
#define Vlen(x) (sizeof(x) / sizeof(*x))
#define SQR(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))
#define TLOW std::numeric_limits<T>::lowest()
#define TMAX std::numeric_limits<T>::max()
#define FLOAT_MAX_TAN 8388607.0f
#define FLOAT_MIN_TAN -FLOAT_MAX_TAN
#define EMPTYFIELD -9999
typedef std::chrono::high_resolution_clock Clock;

/// <summary>
/// Thin wrapper around getting the current time in milliseconds.
/// </summary>
static inline size_t NowMs()
{
	return duration_cast<milliseconds>(Clock::now().time_since_epoch()).count();
}

#ifndef byte
	typedef unsigned char byte;
#endif

#define DO_DOUBLE 1//Comment this out for shorter build times during development. Always uncomment for release.
//#define ISAAC_FLAM3_DEBUG 1//This is almost never needed, but is very useful when troubleshooting difficult bugs. Enable it to do a side by side comparison with flam3.

#if GLM_VERSION >= 96
	#define v2T  glm::tvec2<T, glm::defaultp>
	#define v3T  glm::tvec3<T, glm::defaultp>
	#define v4T  glm::tvec4<T, glm::defaultp>
	#define m2T  glm::tmat2x2<T, glm::defaultp>
	#define m3T  glm::tmat3x3<T, glm::defaultp>
	#define m4T  glm::tmat4x4<T, glm::defaultp>
	#define m23T glm::tmat2x3<T, glm::defaultp>
#else
	#define v2T  glm::detail::tvec2<T, glm::defaultp>
	#define v3T  glm::detail::tvec3<T, glm::defaultp>
	#define v4T  glm::detail::tvec4<T, glm::defaultp>
	#define m2T  glm::detail::tmat2x2<T, glm::defaultp>
	#define m3T  glm::detail::tmat3x3<T, glm::defaultp>
	#define m4T  glm::detail::tmat4x4<T, glm::defaultp>
	#define m23T glm::detail::tmat2x3<T, glm::defaultp>
#endif

enum eInterp : uint { EMBER_INTERP_LINEAR = 0, EMBER_INTERP_SMOOTH = 1 };
enum eAffineInterp : uint { INTERP_LINEAR = 0, INTERP_LOG = 1, INTERP_COMPAT = 2, INTERP_OLDER = 3 };
enum ePaletteMode : uint { PALETTE_STEP = 0, PALETTE_LINEAR = 1 };
enum ePaletteInterp : uint { INTERP_HSV = 0, INTERP_SWEEP = 1 };
enum eMotion : uint { MOTION_SIN = 1, MOTION_TRIANGLE = 2, MOTION_HILL = 3 };
enum eProcessAction : uint { NOTHING = 0, ACCUM_ONLY = 1, FILTER_AND_ACCUM = 2, KEEP_ITERATING = 3, FULL_RENDER = 4 };
enum eProcessState : uint { NONE = 0, ITER_STARTED = 1, ITER_DONE = 2, FILTER_DONE = 3, ACCUM_DONE = 4 };
enum eInteractiveFilter : uint { FILTER_LOG = 0, FILTER_DE = 1 };
enum eScaleType : uint { SCALE_NONE = 0, SCALE_WIDTH = 1, SCALE_HEIGHT = 2 };
enum eRenderStatus : uint { RENDER_OK = 0, RENDER_ERROR = 1, RENDER_ABORT = 2 };
}
