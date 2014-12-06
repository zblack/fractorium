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
#include <chrono>
#include <complex>
#include <cstdint>
#include <fstream>
#include <functional>
#include <inttypes.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <malloc.h>
#include <math.h>
#include <memory>
#include <numeric>
#include <ostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <time.h>
#include <vector>

//Third party headers.
#ifdef _WIN32
#include "libxml/parser.h"
#else
#include "libxml2/libxml/parser.h"
#endif

//Intel's Threading Building Blocks is what's used for all threading.
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"

#define GLM_FORCE_RADIANS

//glm is what's used for matrix math.
#include "glm/glm.hpp"
#include "glm/detail/type_int.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

using namespace tbb;
using namespace std;
using namespace std::chrono;
using glm::uint;
using glm::uint16;
