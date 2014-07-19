#pragma once

#include "EmberPch.h"

/// <summary>
/// Basic #defines used throughout the library.
/// </summary>

//MSVC specific?
#if defined(BUILDING_EMBER)
#define EMBER_API __declspec(dllexport)
#else
#define EMBER_API __declspec(dllimport)
#endif

#define RESTRICT __restrict//This might make things faster, unsure if it really does though.
//#define RESTRICT

namespace EmberNs
{
//Wrap the sincos function for Macs and PC.
#if defined(__APPLE__) || defined(_MSC_VER)
	#define sincos(x, s, c) *(s)=sin(x); *(c)=cos(x);
#else
	extern void sincos(double x, double *s, double *c);
#endif

#define EMBER_VERSION "0.4.0.5"
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
#define XC (const xmlChar*)
#define BadVal(x) (((x) != (x)) || ((x) > 1e10) || ((x) < -1e10))
#define Rint(A) floor((A) + (((A) < 0) ? T(-0.5) : T(0.5)))
#define Vlen(x) (sizeof(x) / sizeof(*x))
#define SQR(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))
#define TLOW std::numeric_limits<T>::lowest()
#define TMAX std::numeric_limits<T>::max()

#ifndef acosh
#define acosh(x) (log(x + sqrt(SQR(x) - 1)))//Remove this once you upgrade compilers to VS 2013 or later.//TODO
#endif

#ifndef fma
#define fma(x, y, z) ((x * y) + z)
#endif

#define DO_DOUBLE 1//Comment this out for shorter build times during development. Always uncomment for release.
//#define ISAAC_FLAM3_DEBUG 1//This is almost never needed, but is very useful when troubleshooting difficult bugs. Enable it to do a side by side comparison with flam3.

#define v2T  glm::detail::tvec2<T, glm::defaultp>
#define v3T  glm::detail::tvec3<T, glm::defaultp>
#define v4T  glm::detail::tvec4<T, glm::defaultp>
#define m2T  glm::detail::tmat2x2<T, glm::defaultp>
#define m3T  glm::detail::tmat3x3<T, glm::defaultp>
#define m4T  glm::detail::tmat4x4<T, glm::defaultp>
#define m23T glm::detail::tmat2x3<T, glm::defaultp>

enum eInterp : unsigned int { EMBER_INTERP_LINEAR = 0, EMBER_INTERP_SMOOTH = 1 };
enum eAffineInterp : unsigned int { INTERP_LINEAR = 0, INTERP_LOG = 1, INTERP_COMPAT = 2, INTERP_OLDER = 3 };
enum ePaletteMode : unsigned int { PALETTE_STEP = 0, PALETTE_LINEAR = 1 };
enum ePaletteInterp : unsigned int { INTERP_HSV = 0, INTERP_SWEEP = 1 };
enum eMotion : unsigned int { MOTION_SIN = 1, MOTION_TRIANGLE = 2, MOTION_HILL = 3 };
enum eProcessAction : unsigned int { NOTHING = 0, ACCUM_ONLY = 1, FILTER_AND_ACCUM = 2, KEEP_ITERATING = 3, FULL_RENDER = 4 };
enum eProcessState : unsigned int { NONE = 0, ITER_STARTED = 1, ITER_DONE = 2, FILTER_DONE = 3, ACCUM_DONE = 4 };
enum eInteractiveFilter : unsigned int { FILTER_LOG = 0, FILTER_DE = 1 };
enum eScaleType : unsigned int { SCALE_NONE = 0, SCALE_WIDTH = 1, SCALE_HEIGHT = 2 };
enum eRenderStatus : unsigned int { RENDER_OK = 0, RENDER_ERROR = 1, RENDER_ABORT = 2 };
}
