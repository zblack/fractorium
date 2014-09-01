#pragma once

#include "EmberDefines.h"
#include "Affine2D.h"
#include "Timing.h"

/// <summary>
/// Basic point and color structures used in iteration.
/// </summary>

namespace EmberNs
{
/// <summary>
/// The point used to store the result of each iteration, which is
/// a spatial coordinate, a color index/coordinate and a visibility value.
/// Note that a Y color coordinate is not used at the moment because
/// only 1D palettes are supported like the original. However, in the future
/// 2D palettes may be supported like Fractron does.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Point
{
public:
	/// <summary>
	/// Constructor to initialize spatial and color coordinates to zero, with full visibility.
	/// </summary>
	Point()
	{
		Init();
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="point">The Point object to copy</param>
	Point(const Point<T>& point)
	{
		Point<T>::operator=<T>(point);
	}

	/// <summary>
	/// Copy constructor to copy a Point object of type U.
	/// </summary>
	/// <param name="point">The Point object to copy</param>
	template <typename U>
	Point(const Point<U>& point)
	{
		Point<T>::operator=<U>(point);
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="point">The Point object to copy</param>
	Point<T>& operator = (const Point<T>& point)
	{
		if (this != &point)
			Point<T>::operator=<T>(point);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Point object of type U.
	/// </summary>
	/// <param name="point">The Point object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Point<T>& operator = (const Point<U>& point)
	{
		m_X = point.m_X;
		m_Y = point.m_Y;
		m_Z = point.m_Z;
		m_ColorX = point.m_ColorX;
		//m_ColorY = point.m_ColorY;
		m_VizAdjusted = point.m_VizAdjusted;

		return *this;
	}

	/// <summary>
	/// Set spatial and color coordinates to zero, with full visibility.
	/// </summary>
	void Init()
	{
		m_X = 0;
		m_Y = 0;
		m_Z = 0;
		m_ColorX = 0;
		//m_ColorY = 0;
		m_VizAdjusted = 1;
	}

	T m_X;
	T m_Y;
	T m_Z;
	T m_ColorX;
	//T m_ColorY;
	T m_VizAdjusted;
};

/// <summary>
/// Comparer used for sorting the results of iteration by their spatial x coordinates.
/// </summary>
/// <param name="a">The first point to compare</param>
/// <param name="b">The second point to compare</param>
/// <returns>1 if the first point had an x coordinate less than the second point, else 0</returns>
template <typename T>
static int SortPointByX(const Point<T>& a, const Point<T>& b)
{
	return a.m_X < b.m_X;
}

/// <summary>
/// Comparer used for sorting the results of iteration by their spatial y coordinates.
/// </summary>
/// <param name="a">The first point to compare</param>
/// <param name="b">The second point to compare</param>
/// <returns>1 if the first point had an y coordinate less than the second point, else 0</returns>
template <typename T>
static int SortPointByY(const Point<T>& a, const Point<T>& b)
{
	return a.m_Y < b.m_Y;
}

/// <summary>
/// Thin override of a glm::vec4 which adds a couple of functions
/// specific to color handling.
/// </summary>
template <typename T>
struct EMBER_API Color : public v4T
{
#ifndef _WIN32
using v4T::r;
using v4T::g;
using v4T::b;
using v4T::a;
#endif
public:
	/// <summary>
	/// Constructor to set color values to zero, with full visibility.
	/// </summary>
	Color()
	{
		Reset();
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="color">The Color object to copy</param>
	Color(const Color<T>& color)
	{
		Color<T>::operator=<T>(color);
	}

	/// <summary>
	/// Copy constructor to copy a Color object of type U.
	/// </summary>
	/// <param name="color">The Color object to copy</param>
	template <typename U>
	Color(const Color<U>& color)
	{
		Color<T>::operator=<U>(color);
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="color">The Color object to copy</param>
	Color<T>& operator = (const Color<T>& color)
	{
		if (this != &color)
			Color<T>::operator=<T>(color);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Color object of type U.
	/// </summary>
	/// <param name="color">The Color object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Color<T>& operator = (const Color<U>& color)
	{
#ifdef _WIN32
		v4T::operator=<U>(color);
#else
		v4T::template operator=<U>(color);
#endif
		return *this;
	}

	/// <summary>
	/// Member-wise constructor.
	/// </summary>
	Color(T rr, T gg, T bb, T aa)
		: v4T(rr, gg, bb, aa)
	{
	}

	/// <summary>
	/// Set color values and visibility to zero.
	/// </summary>
	inline void Clear()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}

	/// <summary>
	/// Set color values to zero, with full visibility.
	/// </summary>
	/// <param name="norm">If norm is true, the color fields are expected to have a range of 0-1, else 0-255</param>
	inline void Reset(bool norm = true)
	{
		r = 0;
		g = 0;
		b = 0;
		a = norm ? T(1) : T(255);
	}
};
}