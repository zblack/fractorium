#pragma once

#include "EmberCLPch.h"

/// <summary>
/// Various data structures defined for the CPU and OpenCL.
/// These are stripped down versions of THE classes in Ember, for use with OpenCL.
/// Their sole purpose is to pass values from the host to the device.
/// They retain most of the member variables, but do not contain the functions.
/// Visual Studio defaults to alighment of 16, but it's made explicit in case another compiler is used.
/// This must match the alignment specified in the kernel.
/// </summary>

namespace EmberCLns
{
#define ALIGN __declspec(align(16))//These two must always match.
#define ALIGN_CL "((aligned (16)))"//The extra parens are necessary.

/// <summary>
/// Various constants needed for rendering.
/// </summary>
static string ConstantDefinesString(bool doublePrecision)
{
	ostringstream os;

	if (doublePrecision)
	{
		os << "#if defined(cl_amd_fp64)\n"//AMD extension available?
		   << "	#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n"
		   << "#endif\n"
		   << "#if defined(cl_khr_fp64)\n"//Khronos extension available?
		   << "	#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
		   << "#endif\n"
		   << "#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable\n"//Only supported on nVidia.
		   << "typedef long intPrec;\n"
		   << "typedef ulong atomi;\n"
		   << "typedef double real_t;\n"
		   << "typedef double4 real4;\n"
		   << "#define EPS (DBL_EPSILON)\n"
		   ;
	}
	else
	{
		os << "typedef int intPrec;\n"
			  "typedef unsigned int atomi;\n"
			  "typedef float real_t;\n"
			  "typedef float4 real4;\n"
			  "#define EPS (FLT_EPSILON)\n"
			  ;
	}

	os <<
		"typedef          long int int64;\n"
		"typedef unsigned long int uint64;\n"
		"\n"
		"#define EPS6 ((1e-6))\n"
		"\n"
		"//The number of threads per block used in the iteration function. Don't change\n"
		"//it lightly; the block size is hard coded to be exactly 32 x 8.\n"
		"#define NTHREADS 256u\n"
		"#define THREADS_PER_WARP 32u\n"
		"#define NWARPS (NTHREADS / THREADS_PER_WARP)\n"
		"#define COLORMAP_LENGTH 256u\n"
		"#define COLORMAP_LENGTH_MINUS_1 255u\n"
		"#define DE_THRESH 100u\n"
		"#define BadVal(x) (((x) != (x)) || ((x) > 1e10) || ((x) < -1e10))\n"
		"#define Rint(A) floor((A) + (((A) < 0) ? -0.5 : 0.5))\n"
		"#define SQR(x) ((x) * (x))\n"
		"#define CUBE(x) ((x) * (x) * (x))\n"
		"#define M_2PI (M_PI * 2)\n"
		"#define M_3PI (M_PI * 3)\n"
		"#define SQRT5 2.2360679774997896964091736687313\n"
		"#define M_PHI 1.61803398874989484820458683436563\n"
		"#define DEG_2_RAD (M_PI / 180)\n"
		"\n"
		"//Index in each dimension of a thread within a block.\n"
		"#define THREAD_ID_X   (get_local_id(0))\n"
		"#define THREAD_ID_Y   (get_local_id(1))\n"
		"#define THREAD_ID_Z   (get_local_id(2))\n"
		"\n"
		"//Index in each dimension of a block within a grid.\n"
		"#define BLOCK_ID_X    (get_group_id(0))\n"
		"#define BLOCK_ID_Y    (get_group_id(1))\n"
		"#define BLOCK_ID_Z    (get_group_id(2))\n"
		"\n"
		"//Absolute index in each dimension of a thread within a grid.\n"
		"#define GLOBAL_ID_X   (get_global_id(0))\n"
		"#define GLOBAL_ID_Y   (get_global_id(1))\n"
		"#define GLOBAL_ID_Z   (get_global_id(2))\n"
		"\n"
		"//Dimensions of a block.\n"
		"#define BLOCK_SIZE_X  (get_local_size(0))\n"
		"#define BLOCK_SIZE_Y  (get_local_size(1))\n"
		"#define BLOCK_SIZE_Z  (get_local_size(2))\n"
		"\n"
		"//Dimensions of a grid, in terms of blocks.\n"
		"#define GRID_SIZE_X   (get_num_groups(0))\n"
		"#define GRID_SIZE_Y   (get_num_groups(1))\n"
		"#define GRID_SIZE_Z   (get_num_groups(2))\n"
		"\n"
		"//Dimensions of a grid, in terms of threads.\n"
		"#define GLOBAL_SIZE_X (get_global_size(0))\n"
		"#define GLOBAL_SIZE_Y (get_global_size(1))\n"
		"#define GLOBAL_SIZE_Z (get_global_size(2))\n"
		"\n"
		"#define INDEX_IN_BLOCK_2D (THREAD_ID_Y * BLOCK_SIZE_X + THREAD_ID_X)\n"
		"#define INDEX_IN_BLOCK_3D ((BLOCK_SIZE_X * BLOCK_SIZE_Y * THREAD_ID_Z) + INDEX_IN_BLOCK_2D)\n"
		"\n"
		"#define INDEX_IN_GRID_2D (GLOBAL_ID_Y * GLOBAL_SIZE_X + GLOBAL_ID_X)\n"
		"#define INDEX_IN_GRID_3D ((GLOBAL_SIZE_X * GLOBAL_SIZE_Y * GLOBAL_ID_Z) + INDEX_IN_GRID_2D)\n"
		"\n";

	return os.str();
}

/// <summary>
/// A point structure on the host that maps to the one used on the device to iterate in OpenCL.
/// It might seem better to use vec4, however 2D palettes and even 3D coordinates may eventually
/// be supported, which will make it more than 4 members.
/// </summary>
template <typename T>
struct ALIGN PointCL
{
	T m_X;
	T m_Y;
	T m_Z;
	T m_ColorX;
	unsigned int m_LastXfUsed;
};

/// <summary>
/// The point structure used to iterate in OpenCL.
/// It might seem better to use float4, however 2D palettes and even 3D coordinates may eventually
/// be supported, which will make it more than 4 members.
/// </summary>
static const char* PointCLStructString =
"typedef struct __attribute__ " ALIGN_CL " _Point\n"
"{\n"
"	real_t m_X;\n"
"	real_t m_Y;\n"
"	real_t m_Z;\n"
"	real_t m_ColorX;\n"
"	uint m_LastXfUsed;\n"
"} Point;\n"
"\n";

#define MAX_CL_VARS 8//These must always match.
#define MAX_CL_VARS_STRING "8"

/// <summary>
/// A structure on the host used to hold all of the needed information for an xform used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN XformCL
{
	T m_A, m_B, m_C, m_D, m_E, m_F;//24 (48)
	T m_VariationWeights[MAX_CL_VARS];//56 (112)
	T m_PostA, m_PostB, m_PostC, m_PostD, m_PostE, m_PostF;//80 (160)
	T m_DirectColor;//84 (168)
	T m_ColorSpeedCache;//88 (176)
	T m_OneMinusColorCache;//92 (184)
	T m_Opacity;//96 (192)
	T m_VizAdjusted;//100 (200)
};

/// <summary>
/// The xform structure used to iterate in OpenCL.
/// </summary>
static const char* XformCLStructString =
"typedef struct __attribute__ " ALIGN_CL " _XformCL\n"
"{\n"
"	real_t m_A, m_B, m_C, m_D, m_E, m_F;\n"
"	real_t m_VariationWeights[" MAX_CL_VARS_STRING "];\n"
"	real_t m_PostA, m_PostB, m_PostC, m_PostD, m_PostE, m_PostF;\n"
"	real_t m_DirectColor;\n"
"	real_t m_ColorSpeedCache;\n"
"	real_t m_OneMinusColorCache;\n"
"	real_t m_Opacity;\n"
"	real_t m_VizAdjusted;\n"
"} XformCL;\n"
"\n";

#define MAX_CL_XFORM 21//These must always match.
#define MAX_CL_XFORM_STRING "21"

/// <summary>
/// A structure on the host used to hold all of the needed information for an ember used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN EmberCL
{
	XformCL<T> m_Xforms[MAX_CL_XFORM];
	T m_CamZPos;
	T m_CamPerspective;
	T m_CamYaw;
	T m_CamPitch;
	T m_CamDepthBlur;
	T m_BlurCoef;
	m3T m_CamMat;
	T m_CenterX, m_CenterY;
	T m_RotA, m_RotB, m_RotD, m_RotE;
};

/// <summary>
/// The ember structure used to iterate in OpenCL.
/// </summary>
static const char* EmberCLStructString =
"typedef struct __attribute__ " ALIGN_CL " _EmberCL\n"
"{\n"
"	XformCL m_Xforms[" MAX_CL_XFORM_STRING "];\n"
"	real_t m_CamZPos;\n"
"	real_t m_CamPerspective;\n"
"	real_t m_CamYaw;\n"
"	real_t m_CamPitch;\n"
"	real_t m_CamDepthBlur;\n"
"	real_t m_BlurCoef;\n"
"	real_t m_C00;\n"
"	real_t m_C01;\n"
"	real_t m_C02;\n"
"	real_t m_C10;\n"
"	real_t m_C11;\n"
"	real_t m_C12;\n"
"	real_t m_C20;\n"
"	real_t m_C21;\n"
"	real_t m_C22;\n"
"	real_t m_CenterX, m_CenterY;\n"
"	real_t m_RotA, m_RotB, m_RotD, m_RotE;\n"
"} EmberCL;\n"
"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for cartesian to raster mapping used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN CarToRasCL
{
	T m_PixPerImageUnitW, m_RasLlX;
	unsigned int m_RasWidth;
	T m_PixPerImageUnitH, m_RasLlY;
	T m_CarLlX, m_CarUrX, m_CarUrY, m_CarLlY;
};

/// <summary>
/// The cartesian to raster structure used to iterate in OpenCL.
/// </summary>
static const char* CarToRasCLStructString =
"typedef struct __attribute__ " ALIGN_CL " _CarToRasCL\n"
"{\n"
"	real_t m_PixPerImageUnitW, m_RasLlX;\n"
"	uint m_RasWidth;\n"
"	real_t m_PixPerImageUnitH, m_RasLlY;\n"
"	real_t m_CarLlX, m_CarUrX, m_CarUrY, m_CarLlY;\n"
"} CarToRasCL;\n"
"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for density filtering used on the device to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN DensityFilterCL
{
	T m_Curve;
	T m_K1;
	T m_K2;
	unsigned int m_Supersample;
	unsigned int m_SuperRasW;
	unsigned int m_SuperRasH;
	unsigned int m_KernelSize;
	unsigned int m_MaxFilterIndex;
	unsigned int m_MaxFilteredCounts;
	unsigned int m_FilterWidth;
};

/// <summary>
/// The density filtering structure used to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
static const char* DensityFilterCLStructString =
"typedef struct __attribute__ " ALIGN_CL " _DensityFilterCL\n"
"{\n"
"	real_t m_Curve;\n"
"	real_t m_K1;\n"
"	real_t m_K2;\n"
"	uint m_Supersample;\n"
"	uint m_SuperRasW;\n"
"	uint m_SuperRasH;\n"
"	uint m_KernelSize;\n"
"	uint m_MaxFilterIndex;\n"
"	uint m_MaxFilteredCounts;\n"
"	uint m_FilterWidth;\n"
"} DensityFilterCL;\n"
"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for spatial filtering used on the device to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
template <typename T>
struct ALIGN SpatialFilterCL
{
	unsigned int m_SuperRasW;
	unsigned int m_SuperRasH;
	unsigned int m_FinalRasW;
	unsigned int m_FinalRasH;
	unsigned int m_Supersample;
	unsigned int m_FilterWidth;
	unsigned int m_NumChannels;
	unsigned int m_BytesPerChannel;
	unsigned int m_DensityFilterOffset;
	unsigned int m_Transparency;
	unsigned int m_YAxisUp;
	T m_Vibrancy;
	T m_HighlightPower;
	T m_Gamma;
	T m_LinRange;
	Color<T> m_Background;
};

/// <summary>
/// The spatial filtering structure used to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
static const char* SpatialFilterCLStructString =
"typedef struct __attribute__ ((aligned (16))) _SpatialFilterCL\n"
"{\n"
"	uint m_SuperRasW;\n"
"	uint m_SuperRasH;\n"
"	uint m_FinalRasW;\n"
"	uint m_FinalRasH;\n"
"	uint m_Supersample;\n"
"	uint m_FilterWidth;\n"
"	uint m_NumChannels;\n"
"	uint m_BytesPerChannel;\n"
"	uint m_DensityFilterOffset;\n"
"	uint m_Transparency;\n"
"	uint m_YAxisUp;\n"
"	real_t m_Vibrancy;\n"
"	real_t m_HighlightPower;\n"
"	real_t m_Gamma;\n"
"	real_t m_LinRange;\n"
"	real_t m_Background[4];\n"//For some reason, using float4/double4 here does not align no matter what. So just use an array of 4.
"} SpatialFilterCL;\n"
"\n";

/// <summary>
/// EmberCL makes extensive use of the build in vector types, however accessing
/// their members as a buffer is not natively supported.
/// Declaring them in a union with a buffer resolves this problem.
/// </summary>
static const char* UnionCLStructString =
"typedef union\n"
"{\n"
"	uchar3 m_Uchar3;\n"
"	uchar m_Uchars[3];\n"
"} uchar3uchars;\n"
"\n"
"typedef union\n"
"{\n"
"	uchar4 m_Uchar4;\n"
"	uchar m_Uchars[4];\n"
"} uchar4uchars;\n"
"\n"
"typedef union\n"
"{\n"
"	uint4 m_Uint4;\n"
"	uint m_Uints[4];\n"
"} uint4uints;\n"
"\n"
"typedef union\n"//Use in places where float is required.
"{\n"
"	float4 m_Float4;\n"
"	float m_Floats[4];\n"
"} float4floats;\n"
"\n"
"typedef union\n"//Use in places where float or double can be used depending on the template type.
"{\n"
"	real4 m_Real4;\n"
"	real_t m_Reals[4];\n"
"} real4reals;\n"
"\n";
}