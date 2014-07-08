#pragma once

/// <summary>
/// Precompiled header file. Place all system includes here with appropriate #defines for different operating systems and compilers.
/// </summary>

#define NOMINMAX
#define _USE_MATH_DEFINES

#ifdef _WIN32
	#define basename(x) _strdup(x)
	#define snprintf _snprintf
	#define snprintf_s _snprintf_s
	#define WIN32_LEAN_AND_MEAN
	#define EMBER_OS "WIN"

	#include <SDKDDKVer.h>
	#include <windows.h>
#elif __APPLE__
	#define EMBER_OS "OSX"
#else
	#include <libgen.h>
	#include <unistd.h>
	#define EMBER_OS "LNX"
#endif

//Standard headers.
#include <algorithm>
#include <complex>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <limits>
#include <malloc.h>
#include <math.h>
#include <numeric>
#include <ostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <vector>

//Third party headers.
#include <libxml/parser.h>

//Intel's Threading Building Blocks is what's used for all threading.
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"

#define GLM_FORCE_RADIANS

//glm is what's used for matrix math.
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

using namespace tbb;
using namespace std;
