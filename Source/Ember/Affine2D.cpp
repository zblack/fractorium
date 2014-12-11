#include "EmberPch.h"
#include "Affine2D.h"

namespace EmberNs
{
/// <summary>
/// Default constructor which sets the matrix to the identity.
/// </summary>
template <typename T>
Affine2D<T>::Affine2D()
{
	MakeID();
}

/// <summary>
/// Default copy constructor.
/// </summary>
/// <param name="affine">The Affine2D object to copy</param>
template <typename T>
Affine2D<T>::Affine2D(const Affine2D<T>& affine)
{
	Affine2D<T>::operator=<T>(affine);
}

/// <summary>
/// Constructor which takes each column of the affine as a separate parameter.
/// </summary>
/// <param name="x">A and D</param>
/// <param name="y">B and E</param>
/// <param name="t">C and F</param>
template <typename T>
Affine2D<T>::Affine2D(v2T& x, v2T& y, v2T& t)
{
	X(x);
	Y(y);
	O(t);
}

/// <summary>
/// Constructor which takes all six of the affine values as parameters.
/// </summary>
/// <param name="xx">A</param>
/// <param name="xy">D</param>
/// <param name="yx">B</param>
/// <param name="yy">E</param>
/// <param name="tx">C</param>
/// <param name="ty">F</param>
template <typename T>
Affine2D<T>::Affine2D(T xx, T xy, T yx, T yy, T tx, T ty)
{
	A(xx);
	D(xy);
	B(yx);
	E(yy);
	C(tx);
	F(ty);
}

/// <summary>
/// Constructor which takes a 4x4 matrix and assigns the
/// corresponding values in the 2x3 affine matrix.
/// </summary>
/// <param name="mat">The 4x4 affine matrix to read from</param>
template <typename T>
Affine2D<T>::Affine2D(m4T& mat)
{
	A(mat[0][0]);
	B(mat[0][1]);
	C(mat[0][3]);
	D(mat[1][0]);
	E(mat[1][1]);
	F(mat[1][3]);
}

/// <summary>
/// Default assignment operator.
/// </summary>
/// <param name="affine">The Affine2D object to copy</param>
template <typename T>
Affine2D<T>& Affine2D<T>::operator = (const Affine2D<T>& affine)
{
	if (this != &affine)
		Affine2D<T>::operator=<T>(affine);

	return *this;
}

/// <summary>
/// == operator which tests if all fields are equal with another Affine2D.
/// </summary>
/// <param name="affine">The Affine2D to compare to</param>
/// <returns>True if all fields are equal, else false</returns>
template <typename T>
bool Affine2D<T>::operator == (const Affine2D<T>& affine)
{
	return IsClose(A(), affine.A()) &&
		   IsClose(B(), affine.B()) &&
		   IsClose(C(), affine.C()) &&
		   IsClose(D(), affine.D()) &&
		   IsClose(E(), affine.E()) &&
		   IsClose(F(), affine.F());
}

/// <summary>
/// * operator to multiply this affine transform by a vec2 and return the result as a vec2.
/// </summary>
/// <param name="v">The vec2 to multiply by</param>
/// <returns>A new vec2 which is the product of the multiplication</returns>
template <typename T>
typename v2T Affine2D<T>::operator * (const v2T& v)
{
	return TransformVector(v);
}

/// <summary>
/// Make this affine transform the identity matrix.
/// A and E = 1, all else 0.
/// </summary>
template <typename T>
void Affine2D<T>::MakeID()
{
	A(1);
	B(0);
	C(0);
	D(0);
	E(1);
	F(0);
}

/// <summary>
/// Determine whether this affine transform is the identity matrix.
/// </summary>
/// <returns>True if A and E are equal to 1 and all others are 0, else false.</returns>
template <typename T>
bool Affine2D<T>::IsID() const
{
	return (IsClose<T>(A(), 1)) &&
		   (IsClose<T>(B(), 0)) &&
		   (IsClose<T>(C(), 0)) &&
		   (IsClose<T>(D(), 0)) &&
		   (IsClose<T>(E(), 1)) &&
		   (IsClose<T>(F(), 0));
}

/// <summary>
/// Determine whether this affine transform is all zeroes.
/// </summary>
/// <returns>True if all 6 elements equal zero, else false.</returns>
template <typename T>
bool Affine2D<T>::IsZero() const
{
	return (IsClose<T>(A(), 0)) &&
		   (IsClose<T>(B(), 0)) &&
		   (IsClose<T>(C(), 0)) &&
		   (IsClose<T>(D(), 0)) &&
		   (IsClose<T>(E(), 0)) &&
		   (IsClose<T>(F(), 0));
}

/// <summary>
/// Rotate this affine transform around its origin by the specified angle in degrees.
/// </summary>
/// <param name="angle">The angle to rotate by</param>
template <typename T>
void Affine2D<T>::Rotate(T angle)
{
	m4T origMat4 = ToMat4ColMajor(true);//Must center and use column major for glm to work.
	m4T newMat4 = glm::rotate(origMat4, angle * DEG_2_RAD_T, v3T(0, 0, 1));//Assuming only rotating around z.

	A(newMat4[0][0]);//Use direct assignments instead of constructor to skip assigning C and F.
	B(newMat4[0][1]);
	D(newMat4[1][0]);
	E(newMat4[1][1]);
}

/// <summary>
/// Move by v.
/// </summary>
/// <param name="v">The vec2 describing how far to move in the x and y directions</param>
template <typename T>
void Affine2D<T>::Translate(const v2T& v)
{
	O(O() + v);
}

/// <summary>
/// Rotate and scale the X and Y components by a certain amount based on X.
/// </summary>
/// <param name="v">The vec2 describing how much to rotate and scale the X and Y components</param>
template <typename T>
void Affine2D<T>::RotateScaleXTo(const v2T& v)
{
	Affine2D<T> rs = CalcRotateScale(X(), v);

	X(rs.TransformNormal(X()));
	Y(rs.TransformNormal(Y()));
}

/// <summary>
/// Rotate and scale the X and Y components by a certain amount based on Y.
/// </summary>
/// <param name="v">The vec2 describing how much to rotate and scale the X and Y components</param>
template <typename T>
void Affine2D<T>::RotateScaleYTo(const v2T& v)
{
	Affine2D<T> rs = CalcRotateScale(Y(), v);

	X(rs.TransformNormal(X()));
	Y(rs.TransformNormal(Y()));
}

/// <summary>
/// Return the inverse of the 2x3 affine matrix.
/// </summary>
/// <returns>The inverse of this affine transform</returns>
template <typename T>
Affine2D<T> Affine2D<T>::Inverse() const
{
	T det = A() * E() - D() * B();

	return Affine2D<T>(E() / det, -D() / det,
				   -B() / det,  A() / det,
				   (F() * B() - C() * E()) / det, (C() * D() - F() * A()) / det);
}

/// <summary>
/// Return a vec2 gotten from transforming this affine transform
/// by the vec2 passed in, but with a T component of 0, 0.
/// </summary>
/// <param name="v">The vec2 describing how much to transform by</param>
/// <returns>The centered, transformed vec2</returns>
template <typename T>
typename v2T Affine2D<T>::TransformNormal(const v2T& v) const
{
	return v2T(A() * v.x + B() * v.y, D() * v.x + E() * v.y);
}

/// <summary>
/// Return a vec2 gotten from transforming this affine transform
/// by the vec2 passed in, and applying T translation.
/// </summary>
/// <param name="v">The vec2 describing how much to transform by</param>
/// <returns>The translated, transformed vec2</returns>
template <typename T>
typename v2T Affine2D<T>::TransformVector(const v2T& v) const
{
	return v2T(A() * v.x + B() * v.y + C(), D() * v.x + E() * v.y + F());
}

/// <summary>
/// Return the X and Y components as a 2x2 matrix in column major order.
/// </summary>
/// <returns>The 2x2 matrix</returns>
template <typename T>
typename m2T Affine2D<T>::ToMat2ColMajor() const
{
	return m2T(A(), B(),//Col0...
			   D(), E());//1
}

/// <summary>
/// Return the X and Y components as a 2x2 matrix in row major order.
/// </summary>
/// <returns>The 2x2 matrix</returns>
template <typename T>
typename m2T Affine2D<T>::ToMat2RowMajor() const
{
	return m2T(A(), D(),//Col0...
			   B(), E());//1
}

/// <summary>
/// Return the 2x3 affine transform matrix as a 4x4 matrix in column major order.
/// </summary>
/// <param name="center">Whether to use T translation value or just 0 for center</param>
/// <returns>The 4x4 matrix</returns>
template <typename T>
typename m4T Affine2D<T>::ToMat4ColMajor(bool center) const
{
	m4T mat(A(), B(), 0, center ? 0 : C(), //Col0...
		    D(), E(), 0, center ? 0 : F(), //1
			  0,   0, 1,			    0, //2
			  0,   0, 0,			    1);//3

	return mat;
}

/// <summary>
/// Return the 2x3 affine transform matrix as a 4x4 matrix in row major order.
/// </summary>
/// <param name="center">Whether to use T translation value or just 0 for center</param>
/// <returns>The 4x4 matrix</returns>
template <typename T>
typename m4T Affine2D<T>::ToMat4RowMajor(bool center) const
{
	m4T mat(A(), D(), 0, 0,
		    B(), E(), 0, 0,
			  0,   0, 1, 0,
			center ? 0 : C(), center ? 0 : F(), 0, 1);

	return mat;
}

/// <summary>
/// Accessors.
/// </summary>
template <typename T> T Affine2D<T>::A() const { return m_Mat[0][0]; }//[0][0]//flam3
template <typename T> T Affine2D<T>::B() const { return m_Mat[0][1]; }//[1][0]
template <typename T> T Affine2D<T>::C() const { return m_Mat[0][2]; }//[2][0]
template <typename T> T Affine2D<T>::D() const { return m_Mat[1][0]; }//[0][1]
template <typename T> T Affine2D<T>::E() const { return m_Mat[1][1]; }//[1][1]
template <typename T> T Affine2D<T>::F() const { return m_Mat[1][2]; }//[2][1]

template <typename T> void Affine2D<T>::A(T a) { m_Mat[0][0] = a; }
template <typename T> void Affine2D<T>::B(T b) { m_Mat[0][1] = b; }
template <typename T> void Affine2D<T>::C(T c) { m_Mat[0][2] = c; }
template <typename T> void Affine2D<T>::D(T d) { m_Mat[1][0] = d; }
template <typename T> void Affine2D<T>::E(T e) { m_Mat[1][1] = e; }
template <typename T> void Affine2D<T>::F(T f) { m_Mat[1][2] = f; }

template <typename T> typename v2T Affine2D<T>::X() const { return v2T(A(), D()); }//X Axis.
template <typename T> typename v2T Affine2D<T>::Y() const { return v2T(B(), E()); }//Y Axis.
template <typename T> typename v2T Affine2D<T>::O() const { return v2T(C(), F()); }//Translation.

template <typename T> void Affine2D<T>::X(const v2T& x) { A(x.x); D(x.y); }//X Axis.
template <typename T> void Affine2D<T>::Y(const v2T& y) { B(y.x); E(y.y); }//Y Axis.
template <typename T> void Affine2D<T>::O(const v2T& t) { C(t.x); F(t.y); }//Translation.

/// <summary>
/// Rotate and scale this affine transform and return as a copy. Orginal is unchanged.
/// </summary>
/// <param name="from">The starting point to rotate and scale from</param>
/// <param name="to">The ending point to rotate and scale to</param>
/// <returns>The newly rotated and scalled Affine2D</returns>
template <typename T>
Affine2D<T> Affine2D<T>::CalcRotateScale(const v2T& from, const v2T& to)
{
	T a, c;

	CalcRSAC(from, to, a, c);
	return Affine2D<T>(a, c, -c, a, 0, 0);
}

/// <summary>
/// Never fully understood what this did or why it's named what it is.
/// But it seems to handle some rotating and scaling.
/// </summary>
/// <param name="from">The starting point to rotate and scale from</param>
/// <param name="to">The ending point to rotate and scale to</param>
/// <param name="a">a</param>
/// <param name="c">c</param>
template <typename T>
void Affine2D<T>::CalcRSAC(const v2T& from, const v2T& to, T& a, T& c)
{
	T lsq = from.x * from.x + from.y * from.y;

	a = (from.y * to.y + from.x * to.x) / lsq;
	c = (from.x * to.y - from.y * to.x) / lsq;
}

//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
template EMBER_API class Affine2D<float>;

#ifdef DO_DOUBLE
	template EMBER_API class Affine2D<double>;
#endif
}
