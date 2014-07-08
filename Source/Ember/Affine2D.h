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

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="affine">The Affine2D object to copy</param>
	Affine2D(const Affine2D<T>& affine)
	{
		Affine2D<T>::operator=<T>(affine);
	}

	/// <summary>
	/// Copy constructor to copy an Affine2D object of type U.
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

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="affine">The Affine2D object to copy</param>
	Affine2D<T>& operator = (const Affine2D<T>& affine)
	{
		if (this != &affine)
			Affine2D<T>::operator=<T>(affine);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign an Affine2D object of type U.
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

	inline void MakeID();
	inline bool IsID() const;
	inline bool IsZero() const;
	inline void Rotate(T angle);
	inline void Translate(v2T& v);
	inline void RotateScaleXTo(v2T& v);
	inline void RotateScaleYTo(v2T& v);
	inline Affine2D<T> Inverse() const;
	inline v2T TransformNormal(const v2T& v) const;
	inline v2T TransformVector(const v2T& v) const;
	inline m2T ToMat2ColMajor() const;
	inline m2T ToMat2RowMajor() const;
	inline m4T ToMat4ColMajor(bool center = false) const;
	inline m4T ToMat4RowMajor(bool center = false) const;

	bool operator == (const Affine2D<T>& affine);
	Affine2D<T>& operator * (const Affine2D<T>& affine);
	v2T operator * (const v2T& v);

	inline T A() const;
	inline T B() const;
	inline T C() const;
	inline T D() const;
	inline T E() const;
	inline T F() const;
					 
	inline void A(T a);
	inline void B(T b);
	inline void C(T c);
	inline void D(T d);
	inline void E(T e);
	inline void F(T f);

	inline v2T X() const;
	inline v2T Y() const;
	inline v2T O() const;

	inline void X(v2T& x);
	inline void Y(v2T& y);
	inline void O(v2T& t);

	//static Affine2D Identity();//Complains about inline.
	static inline Affine2D CalcRotateScale(v2T& from, v2T& to);
	static inline void CalcRSAC(v2T& from, v2T& to, T& a, T& c);

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