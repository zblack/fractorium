#pragma once

/// <summary>
/// Precompiled header file. Place all system includes here with appropriate #defines for different operating systems and compilers.
/// </summary>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN//Exclude rarely-used stuff from Windows headers.
#define _USE_MATH_DEFINES

#ifdef _WIN32
	#include <windows.h>
	#include <SDKDDKVer.h>
#endif

#include <utility>
#include <CL/cl.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <time.h>

#include "Timing.h"
#include "Renderer.h"

#if defined(BUILDING_EMBERCL)
#define EMBERCL_API __declspec(dllexport)
#else
#define EMBERCL_API __declspec(dllimport)
#endif

using namespace std;
using namespace EmberNs;
//#define TEST_CL 1
//#define TEST_CL_BUFFERS 1