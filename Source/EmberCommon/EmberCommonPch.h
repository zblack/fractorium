#pragma once

/// <summary>
/// Precompiled header file. Place all system includes here with appropriate #defines for different operating systems and compilers.
/// </summary>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN//Exclude rarely-used stuff from Windows headers.
#define _USE_MATH_DEFINES

#ifdef _WIN32
#include <SDKDDKVer.h>
#include <windows.h>
#include <winsock.h>//For htons().
#include <BaseTsd.h>
#include <crtdbg.h>
#include <tchar.h>
#define snprintf _snprintf
#else
#include <arpa/inet.h>
#define _TCHAR char
#define _tmain main
#define _T
#define fprintf_s fprintf
#endif

#include <iostream>
#include <iomanip>
#include <ostream>
#include <random>
#include <sstream>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jconfig.h"
#include "jpeglib.h"

#define PNG_SKIP_SETJMP_CHECK 1

#include "png.h"
//#include "pnginfo.h"

//Ember.
#include "Ember.h"
#include "Variation.h"
#include "EmberToXml.h"
#include "XmlToEmber.h"
#include "PaletteList.h"
#include "Iterator.h"
#include "Renderer.h"
#include "RendererCL.h"
#include "SheepTools.h"

//Options.
#include "SimpleGlob.h"
#include "SimpleOpt.h"

using namespace EmberNs;
using namespace EmberCLns;
