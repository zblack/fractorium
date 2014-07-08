#pragma once

#include "EmberOptions.h"

/// <summary>
/// Declaration for the EmberRender() function.
/// </summary>

/// <summary>
/// The core of the EmberRender.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
static bool EmberRender(EmberOptions& opt);