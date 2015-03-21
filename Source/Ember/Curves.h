#pragma once

#include "Utils.h"
#include "Isaac.h"

/// <summary>
/// Curves class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// The Bezier curves used to adjust the colors during final accumulation.
/// This functionality was gotten directly from Apophysis.
/// </summary>
template <typename T>
class EMBER_API Curves
{
public:
	/// <summary>
	/// Constructor which sets the curve and weight values to their defaults.
	/// </summary>
	Curves(bool init = false)
	{
		if (init)
			Init();
		else
			Clear();
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="curves">The Curves object to copy</param>
	Curves(const Curves<T>& curves)
	{
		Curves<T>::operator=<T>(curves);
	}

	/// <summary>
	/// Copy constructor to copy a Curves object of type U.
	/// Special case that must be here in the header because it has
	/// a second template parameter.
	/// </summary>
	/// <param name="curves">The Curves object to copy</param>
	template <typename U>
	Curves(const Curves<U>& curves)
	{
		Curves<T>::operator=<U>(curves);
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="curves">The Curves object to copy</param>
	Curves<T>& operator = (const Curves<T>& curves)
	{
		if (this != &curves)
			Curves<T>::operator=<T>(curves);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Curves object of type U.
	/// </summary>
	/// <param name="curves">The Curves object to copy</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Curves<T>& operator = (const Curves<U>& curves)
	{
		for (uint i = 0; i < 4; i++)
		{
			m_Points[i][0].x = T(curves.m_Points[i][0].x); m_Points[i][0].y = T(curves.m_Points[i][0].y); m_Weights[i].x = T(curves.m_Weights[i].x);
			m_Points[i][1].x = T(curves.m_Points[i][1].x); m_Points[i][1].y = T(curves.m_Points[i][1].y); m_Weights[i].y = T(curves.m_Weights[i].y);
			m_Points[i][2].x = T(curves.m_Points[i][2].x); m_Points[i][2].y = T(curves.m_Points[i][2].y); m_Weights[i].z = T(curves.m_Weights[i].z);
			m_Points[i][3].x = T(curves.m_Points[i][3].x); m_Points[i][3].y = T(curves.m_Points[i][3].y); m_Weights[i].w = T(curves.m_Weights[i].w);
		}

		return *this;
	}

	/// <summary>
	/// Unary addition operator to add a Curves<T> object to this one.
	/// </summary>
	/// <param name="curves">The Curves object to add</param>
	/// <returns>Reference to updated self</returns>
	Curves<T>& operator += (const Curves<T>& curves)
	{
		for (uint i = 0; i < 4; i++)
		{
			m_Points[i][0] += curves.m_Points[i][0];
			m_Points[i][1] += curves.m_Points[i][1];
			m_Points[i][2] += curves.m_Points[i][2];
			m_Points[i][3] += curves.m_Points[i][3];

			m_Weights[i] += curves.m_Weights[i];
		}

		return *this;
	}

	/// <summary>
	/// Unary multiplication operator to multiply this object by another Curves<T> object.
	/// </summary>
	/// <param name="curves">The Curves object to multiply this one by</param>
	/// <returns>Reference to updated self</returns>
	Curves<T>& operator *= (const Curves<T>& curves)
	{
		for (uint i = 0; i < 4; i++)
		{
			m_Points[i][0] *= curves.m_Points[i][0];
			m_Points[i][1] *= curves.m_Points[i][1];
			m_Points[i][2] *= curves.m_Points[i][2];
			m_Points[i][3] *= curves.m_Points[i][3];

			m_Weights[i] *= curves.m_Weights[i];
		}

		return *this;
	}

	/// <summary>
	/// Unary multiplication operator to multiply this object by a scalar of type T.
	/// </summary>
	/// <param name="t">The scalar to multiply this object by</param>
	/// <returns>Reference to updated self</returns>
	Curves<T>& operator *= (const T& t)
	{
		for (uint i = 0; i < 4; i++)
		{
			m_Points[i][0] *= t;
			m_Points[i][1] *= t;
			m_Points[i][2] *= t;
			m_Points[i][3] *= t;

			m_Weights[i] *= t;
		}

		return *this;
	}

	/// <summary>
	/// Set the curve and weight values to their default state.
	/// </summary>
	void Init()
	{
		for (uint i = 0; i < 4; i++)
		{
			m_Points[i][0] = v2T(0);//0,0 -> 0,0 -> 1,1 -> 1,1.
			m_Points[i][1] = v2T(0);
			m_Points[i][2] = v2T(1);
			m_Points[i][3] = v2T(1);

			m_Weights[i] = v4T(1);
		}
	}

	/// <summary>
	/// Set the curve and weight values to an empty state.
	/// </summary>
	void Clear()
	{
		memset(&m_Points, 0, sizeof(m_Points));
		memset(&m_Weights, 0, sizeof(m_Weights));
	}

	/// <summary>
	/// Whether any points are not the default.
	/// </summary>
	/// <returns>True if any point has been set to a value other than the default, else false.</returns>
	bool CurvesSet()
	{
		bool set = false;

		for (uint i = 0; i < 4; i++)
		{
			if ((m_Points[i][0] != v2T(0)) ||
				(m_Points[i][1] != v2T(0)) ||
				(m_Points[i][2] != v2T(1)) ||
				(m_Points[i][3] != v2T(1)))
			{
				set = true;
				break;
			}
		}

		return set;
	}

	/// <summary>
	/// Wrapper around calling BezierSolve() on each of the 4 weight and point vectors.
	/// </summary>
	/// <param name="t">The position to apply</param>
	/// <returns>vec4<T> that contains the y component of the solution for each vector in each component</returns>
	v4T BezierFunc(const T& t)
	{
		v4T result;
		v2T solution(0, 0);

		BezierSolve(t, m_Points[0], &m_Weights[0], solution); result.x = solution.y;
		BezierSolve(t, m_Points[1], &m_Weights[1], solution); result.y = solution.y;
		BezierSolve(t, m_Points[2], &m_Weights[2], solution); result.z = solution.y;
		BezierSolve(t, m_Points[3], &m_Weights[3], solution); result.w = solution.y;

		return result;
	}

private:
	/// <summary>
	/// Solve the given point and weight vectors for the given position and store
	/// the output in the solution vec2 passed in.
	/// </summary>
	/// <param name="t">The position to apply</param>
	/// <param name="src">A pointer to an array of 4 vec2</param>
	/// <param name="w">A pointer to an array of 4 weights</param>
	/// <param name="solution">The vec2 to store the solution in</param>
	void BezierSolve(const T& t, v2T* src, v4T* w, v2T& solution)
	{
		T s, s2, s3, t2, t3, nom_x, nom_y, denom;

		s = 1 - t;
		s2 = s * s;
		s3 = s * s * s;
		t2 = t * t;
		t3 = t * t * t;

		nom_x = (w->x * s3 * src->x) + (w->y * s2 * 3 * t * src[1].x) + (w->z * s * 3 * t2 * src[2].x) + (w->w * t3 * src[3].x);

		nom_y = (w->x * s3 * src->y) + (w->y * s2 * 3 * t * src[1].y) + (w->z * s * 3 * t2 * src[2].y) + (w->w * t3 * src[3].y);

		denom = (w->x * s3) + (w->y * s2 * 3 * t) + (w->z * s * 3 * t2) + (w->w * t3);


		if (isnan(nom_x) || isnan(nom_y) || isnan(denom) || denom == 0)
			return;

		solution.x = nom_x / denom;
		solution.y = nom_y / denom;
	}

public:
	v2T m_Points[4][4];
	v4T m_Weights[4];
};

//Must declare this outside of the class to provide for both orders of parameters.

/// <summary>
/// Multiplication operator to multiply a Curves<T> object by a scalar of type T.
/// </summary>
/// <param name="curves">The curves object to multiply</param>
/// <param name="t">The scalar to multiply curves by by</param>
/// <returns>Copy of new Curves<T></returns>
template<typename T>
Curves<T> operator * (const Curves<T>& curves, const T& t)
{
	Curves<T> c(curves);

	for (uint i = 0; i < 4; i++)
	{
		c.m_Points[i][0] *= t;
		c.m_Points[i][1] *= t;
		c.m_Points[i][2] *= t;
		c.m_Points[i][3] *= t;

		c.m_Weights[i] *= t;
	}

	return c;
}

/// <summary>
/// Multiplication operator for reverse order.
/// </summary>
/// <param name="t">The scalar to multiply curves by by</param>
/// <param name="curves">The curves object to multiply</param>
/// <returns>Copy of new Curves<T></returns>
template<typename T>
Curves<T> operator * (const T& t, const Curves<T>& curves)
{
	return curves * t;
}
}