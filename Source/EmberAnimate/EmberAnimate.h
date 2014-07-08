#pragma once

#include "EmberOptions.h"

/// <summary>
/// Declaration for the EmberAnimate() function.
/// </summary>

/// <summary>
/// The core of the EmberAnimate.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
static bool EmberAnimate(EmberOptions& opt);