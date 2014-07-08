#pragma once

#include "EmberOptions.h"

/// <summary>
/// Declaration for the EmberGenome() and SetDefaultTestValues() functions.
/// </summary>

/// <summary>
/// Set various default test values on the passed in ember.
/// </summary>
/// <param name="ember">The ember to test</param>
template <typename T>
static void SetDefaultTestValues(Ember<T>& ember);

/// <summary>
/// The core of the EmberGenome.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
static bool EmberGenome(EmberOptions& opt);