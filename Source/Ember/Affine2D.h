#pragma once

#include "Utils.h"

/// <summary>
/// Affine2D class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Uses matrix composition to handle the
/// affine matrix. Taken almost entirely from
/// Fractron, but using glm, and in C++.
/// Note that the matrix layout differs from flam3 so it's best to use
/// the A, B, C, D, E, F wrappers around the underlying matrix indices. But if the matrix must
/// be accessed directly, the two are laid out as such:
/// flam3: 3 columns of 2 rows each. Accessed col, row.
/// [a(0,0)][b(1,0)][c(2,0)]
/// [d(0,1)][e(1,1)][f(2,1)]
/// Ember: 2 columns of 3 rows each. Accessed col, row.
/// [a(0,0)][d(1,0)]
/// [b(0,1)][e(1,1)]
/// [c(0,2)][f(1,2)]
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Affine2D
{
public:
	Affine2D();
	Affine2D(const Affine2D<T>& affine);

	/// <summary>
	/// Copy constructor to copy an Affine2D object of type U.
	/// Special case that must be here in the header because it has
	/// a second template parameter.
	/// </summary>
	/// <param name="affine">The Affine2D object to copy</param>
	template <typename U>
	Affine2D(const Affine2D<U>& affine)
	{
		Affine2D<T>::operator=<U>(affine);
	}

	Affine2D(v2T& x, v2T& y, v2T& t);
	Affine2D(T xx, T xy, T yx, T yy, T tx, T ty);
	Affine2D(m4T& mat);
	Affine2D<T>& operator = (const Affine2D<T>& affine);

	/// <summary>
	/// Assignment operator to assign an Affine2D object of type U.
	/// Special case that must be here in the header because it has
	/// a second template parameter.
	/// </summary>
	/// <param name="affine">The Affine2D object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Affine2D<T>& operator = (const Affine2D<U>& affine)
	{
		A(T(affine.A()));
		B(T(affine.B()));
		C(T(affine.C()));
		D(T(affine.D()));
		E(T(affine.E()));
		F(T(affine.F()));

		return *this;
	}

	bool operator == (const Affine2D<T>& affine);
	Affine2D<T>& operator * (const Affine2D<T>& affine);
	v2T operator * (const v2T& v);

	void MakeID();
	bool IsID() const;
	bool IsZero() const;
	void Rotate(T angle);
	void Translate(v2T& v);
	void RotateScaleXTo(v2T& v);
	void RotateScaleYTo(v2T& v);
	Affine2D<T> Inverse() const;
	v2T TransformNormal(const v2T& v) const;
	v2T TransformVector(const v2T& v) const;
	m2T ToMat2ColMajor() const;
	m2T ToMat2RowMajor() const;
	m4T ToMat4ColMajor(bool center = false) const;
	m4T ToMat4RowMajor(bool center = false) const;

	T A() const;
	T B() const;
	T C() const;
	T D() const;
	T E() const;
	T F() const;

	void A(T a);
	void B(T b);
	void C(T c);
	void D(T d);
	void E(T e);
	void F(T f);

	v2T X() const;
	v2T Y() const;
	v2T O() const;

	void X(const v2T& x);
	void Y(const v2T& y);
	void O(const v2T& t);

	static Affine2D CalcRotateScale(const v2T& from, const v2T& to);
	static void CalcRSAC(const v2T& from, const v2T& to, T& a, T& c);

	m23T m_Mat;
};

//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
template EMBER_API class Affine2D<float>;

#ifdef DO_DOUBLE
	template EMBER_API class Affine2D<double>;
#endif
}