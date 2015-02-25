#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// Linear:
/// nx = tx;
/// ny = ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API LinearVariation : public Variation<T>
{
public:
	LinearVariation(T weight = 1.0) : Variation<T>("linear", VAR_LINEAR, weight) { }

	VARCOPY(LinearVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
			<< "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
			<< "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
			<< "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
			<< "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Sinusoidal:
/// nx = sin(tx);
/// ny = sin(ty);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API SinusoidalVariation : public Variation<T>
{
public:
	SinusoidalVariation(T weight = 1.0) : Variation<T>("sinusoidal", VAR_SINUSOIDAL, weight) { }

	VARCOPY(SinusoidalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * sin(helper.In.x);
		helper.Out.y = m_Weight * sin(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Spherical:
/// T r2 = tx * tx + ty * ty + 1e-6;
/// nx = tx / r2;
/// ny = ty / r2;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <class T>
class EMBER_API SphericalVariation : public Variation<T>
{
public:
	SphericalVariation(T weight = 1.0) : Variation<T>("spherical", VAR_SPHERICAL, weight, true) { }

	VARCOPY(SphericalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = m_Weight / Zeps(helper.m_PrecalcSumSquares);

		helper.Out.x = r2 * helper.In.x;
		helper.Out.y = r2 * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r2 = xform->m_VariationWeights[" << varIndex << "] / Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = r2 * vIn.x;\n"
		   << "\t\tvOut.y = r2 * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Swirl:
/// double r2 = tx * tx + ty * ty;
/// double c1 = sin(r2);
/// double c2 = cos(r2);
/// nx = c1 * tx - c2 * ty;
/// ny = c2 * tx + c1 * ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API SwirlVariation : public Variation<T>
{
public:
	SwirlVariation(T weight = 1.0) : Variation<T>("swirl", VAR_SWIRL, weight, true) { }

	VARCOPY(SwirlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c1, c2;

		sincos(helper.m_PrecalcSumSquares, &c1, &c2);
		helper.Out.x = m_Weight * (c1 * helper.In.x - c2 * helper.In.y);
		helper.Out.y = m_Weight * (c2 * helper.In.x + c1 * helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t c1 = sin(precalcSumSquares);\n"
		   << "\t\treal_t c2 = cos(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (c1 * vIn.x - c2 * vIn.y);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (c2 * vIn.x + c1 * vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Horseshoe:
/// a = atan2(tx, ty);
/// c1 = sin(a);
/// c2 = cos(a);
/// nx = c1 * tx - c2 * ty;
/// ny = c2 * tx + c1 * ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API HorseshoeVariation : public Variation<T>
{
public:
	HorseshoeVariation(T weight = 1.0) : Variation<T>("horseshoe", VAR_HORSESHOE, weight, true, true) { }

	VARCOPY(HorseshoeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(helper.m_PrecalcSqrtSumSquares);

		helper.Out.x = (helper.In.x - helper.In.y) * (helper.In.x + helper.In.y) * r;
		helper.Out.y = 2 * helper.In.x * helper.In.y * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / Zeps(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = (vIn.x - vIn.y) * (vIn.x + vIn.y) * r;\n"
		   << "\t\tvOut.y = T(2.0) * vIn.x * vIn.y * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Polar:
/// nx = atan2(tx, ty) / M_PI;
/// ny = sqrt(tx * tx + ty * ty) - 1.0;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API PolarVariation : public Variation<T>
{
public:
	PolarVariation(T weight = 1.0) : Variation<T>("polar", VAR_POLAR, weight, true, true, false, true, false) { }

	VARCOPY(PolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.m_PrecalcAtanxy * T(M_1_PI));
		helper.Out.y = m_Weight * (helper.m_PrecalcSqrtSumSquares - 1);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (precalcAtanxy * M_1_PI);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (precalcSqrtSumSquares - T(1.0));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Handkerchief:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// p[0] += weight * sin(a + r) * r;
/// p[1] += weight * cos(a - r) * r;
/// </summary>
template <typename T>
class EMBER_API HandkerchiefVariation : public Variation<T>
{
public:
	HandkerchiefVariation(T weight = 1.0) : Variation<T>("handkerchief", VAR_HANDKERCHIEF, weight, true, true, false, true) { }

	VARCOPY(HandkerchiefVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.m_PrecalcSqrtSumSquares * sin(helper.m_PrecalcAtanxy + helper.m_PrecalcSqrtSumSquares);
		helper.Out.y = m_Weight * helper.m_PrecalcSqrtSumSquares * cos(helper.m_PrecalcAtanxy - helper.m_PrecalcSqrtSumSquares);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares * sin(precalcAtanxy + precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares * cos(precalcAtanxy - precalcSqrtSumSquares);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Heart:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// a *= r;
/// p[0] += weight * sin(a) * r;
/// p[1] += weight * cos(a) * -r;
/// </summary>
template <typename T>
class EMBER_API HeartVariation : public Variation<T>
{
public:
	HeartVariation(T weight = 1.0) : Variation<T>("heart", VAR_HEART, weight, true, true, false, true) { }

	VARCOPY(HeartVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcSqrtSumSquares * helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = r * sin(a);
		helper.Out.y = (-r) * cos(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t a = precalcSqrtSumSquares * precalcAtanxy;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * sin(a);\n"
		   << "\t\tvOut.y = (-r) * cos(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Disc:
/// nx = tx * M_PI;
/// ny = ty * M_PI;
/// a = atan2(nx, ny);
/// r = sqrt(nx * nx + ny * ny);
/// p[0] += weight * sin(r) * a / M_PI;
/// p[1] += weight * cos(r) * a / M_PI;
/// </summary>
template <typename T>
class EMBER_API DiscVariation : public ParametricVariation<T>
{
public:
	DiscVariation(T weight = 1.0) : ParametricVariation<T>("disc", VAR_DISC, weight, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(DiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T val = T(M_PI) * helper.m_PrecalcSqrtSumSquares;
		T r = m_WeightByPI * helper.m_PrecalcAtanxy;

		helper.Out.x = sin(val) * r;
		helper.Out.y = cos(val) * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weightByPI = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		ss << "\t{\n"
		   << "\t\treal_t val = M_PI * precalcSqrtSumSquares;\n"
		   << "\t\treal_t r = " << weightByPI << " * precalcAtanxy;\n"
		   << "\n"
		   << "\t\tvOut.x = sin(val) * r;\n"
		   << "\t\tvOut.y = cos(val) * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightByPI = m_Weight * T(M_1_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightByPI, prefix + "disc_weight_by_pi"));//Precalcs only, no params.
	}

private:
	T m_WeightByPI;//Precalcs only, no params.
};

/// <summary>
/// Spiral:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty) + 1e-6;
/// p[0] += weight * (cos(a) + sin(r)) / r;
/// p[1] += weight * (sin(a) - cos(r)) / r;
/// </summary>
template <typename T>
class EMBER_API SpiralVariation : public Variation<T>
{
public:
	SpiralVariation(T weight = 1.0) : Variation<T>("spiral", VAR_SPIRAL, weight, true, true, true) { }

	VARCOPY(SpiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = Zeps(helper.m_PrecalcSqrtSumSquares);
		T r1 = m_Weight / r;

		helper.Out.x = r1 * (helper.m_PrecalcCosa + sin(r));
		helper.Out.y = r1 * (helper.m_PrecalcSina - cos(r));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = Zeps(precalcSqrtSumSquares);\n"
		   << "\t\treal_t r1 = xform->m_VariationWeights[" << varIndex << "] / r;\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * (precalcCosa + sin(r));\n"
		   << "\t\tvOut.y = r1 * (precalcSina - cos(r));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Hyperbolic:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty) + 1e-6;
/// p[0] += weight * sin(a) / r;
/// p[1] += weight * cos(a) * r;
/// </summary>
template <typename T>
class EMBER_API HyperbolicVariation : public Variation<T>
{
public:
	HyperbolicVariation(T weight = 1.0) : Variation<T>("hyperbolic", VAR_HYPERBOLIC, weight, true, true, true) { }

	VARCOPY(HyperbolicVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = Zeps(helper.m_PrecalcSqrtSumSquares);

		helper.Out.x = m_Weight * helper.m_PrecalcSina / r;
		helper.Out.y = m_Weight * helper.m_PrecalcCosa * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = Zeps(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * precalcSina / r;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * precalcCosa * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Diamond:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// p[0] += weight * sin(a) * cos(r);
/// p[1] += weight * cos(a) * sin(r);
/// </summary>
template <typename T>
class EMBER_API DiamondVariation : public Variation<T>
{
public:
	DiamondVariation(T weight = 1.0) : Variation<T>("diamond", VAR_DIAMOND, weight, true, true, true) { }

	VARCOPY(DiamondVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.m_PrecalcSina * cos(helper.m_PrecalcSqrtSumSquares);
		helper.Out.y = m_Weight * helper.m_PrecalcCosa * sin(helper.m_PrecalcSqrtSumSquares);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * precalcSina * cos(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * precalcCosa * sin(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Ex:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// n0 = sin(a + r);
/// n1 = cos(a - r);
/// m0 = n0 * n0 * n0 * r;
/// m1 = n1 * n1 * n1 * r;
/// p[0] += weight * (m0 + m1);
/// p[1] += weight * (m0 - m1);
/// </summary>
template <typename T>
class EMBER_API ExVariation : public Variation<T>
{
public:
	ExVariation(T weight = 1.0) : Variation<T>("ex", VAR_EX, weight, true, true, false, true) { }

	VARCOPY(ExVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanxy;
		T r = helper.m_PrecalcSqrtSumSquares;
		T n0 = sin(a + r);
		T n1 = cos(a - r);
		T m0 = n0 * n0 * n0 * r;
		T m1 = n1 * n1 * n1 * r;

		helper.Out.x = m_Weight * (m0 + m1);
		helper.Out.y = m_Weight * (m0 - m1);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanxy;\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t n0 = sin(a + r);\n"
		   << "\t\treal_t n1 = cos(a - r);\n"
		   << "\t\treal_t m0 = n0 * n0 * n0 * r;\n"
		   << "\t\treal_t m1 = n1 * n1 * n1 * r;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (m0 + m1);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (m0 - m1);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Julia:
/// a = atan2(tx, ty)/2.0;
/// if (random bit()) a += M_PI;
/// r = pow(tx*tx + ty*ty, 0.25);
/// nx = r * cos(a);
/// ny = r * sin(a);
/// p[0] += v * nx;
/// p[1] += v * ny;
/// </summary>
template <typename T>
class EMBER_API JuliaVariation : public Variation<T>
{
public:
	JuliaVariation(T weight = 1.0) : Variation<T>("julia", VAR_JULIA, weight, true, true, false, true) { }

	VARCOPY(JuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * sqrt(helper.m_PrecalcSqrtSumSquares);
		T a = T(0.5) * helper.m_PrecalcAtanxy;

		if (rand.RandBit())
			a += T(M_PI);

		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * sqrt(precalcSqrtSumSquares);\n"
		   << "\t\treal_t a = T(0.5) * precalcAtanxy;\n"
		   << "\n"
		   << "\t\tif (MwcNext(mwc) & 1)\n"
		   << "\t\t	a += M_PI;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Bent:
/// nx = tx;
/// ny = ty;
/// if (nx < 0.0) nx = nx * 2.0;
/// if (ny < 0.0) ny = ny / 2.0;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API BentVariation : public Variation<T>
{
public:
	BentVariation(T weight = 1.0) : Variation<T>("bent", VAR_BENT, weight) { }

	VARCOPY(BentVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T nx = helper.In.x < T(0.0) ? helper.In.x * 2 : helper.In.x;
		T ny = helper.In.y < T(0.0) ? helper.In.y / 2 : helper.In.y;

		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t nx = vIn.x < T(0.0) ? (vIn.x * T(2.0)) : vIn.x;\n"
		   << "\t\treal_t ny = vIn.y < T(0.0) ? (vIn.y / T(2.0)) : vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * nx;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * ny;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Waves:
/// dx = coef[2][0];
/// dy = coef[2][1];
/// nx = tx + coef[1][0] * sin(ty / ((dx * dx) + EPS));
/// ny = ty + coef[1][1] * sin(tx / ((dy * dy) + EPS));
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// Special case here, use parametric for precalcs, but no regular params.
/// </summary>
template <typename T>
class EMBER_API WavesVariation : public ParametricVariation<T>
{
public:
	WavesVariation(T weight = 1.0) : ParametricVariation<T>("waves", VAR_WAVES, weight)
	{
		Init();
	}

	PARVARCOPY(WavesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c10 = m_Xform->m_Affine.B();
		T c11 = m_Xform->m_Affine.E();
		T nx = helper.In.x + c10 * sin(helper.In.y * m_Dx2);
		T ny = helper.In.y + c11 * sin(helper.In.x * m_Dy2);

		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string dx2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string dy2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t c10 = xform->m_B;\n"
		   << "\t\treal_t c11 = xform->m_E;\n"
		   << "\t\treal_t nx = vIn.x + c10 * sin(vIn.y * " << dx2 << ");\n"
		   << "\t\treal_t ny = vIn.y + c11 * sin(vIn.x * " << dy2 << ");\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * nx);\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * ny);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Xform)//If this variation exists by itself and hasn't been added to an xform yet, m_Xform will be nullptr.
		{
			T dx = m_Xform->m_Affine.C();
			T dy = m_Xform->m_Affine.F();

			m_Dx2 = 1 / Zeps(dx * dx);
			m_Dy2 = 1 / Zeps(dy * dy);
		}
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Dx2, prefix + "waves_dx2"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_Dy2, prefix + "waves_dy2"));
	}

private:
	T m_Dx2;//Precalcs only, no params.
	T m_Dy2;
};

/// <summary>
/// Fisheye:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// r = 2 * r / (r + 1);
/// nx = r * cos(a);
/// ny = r * sin(a);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API FisheyeVariation : public Variation<T>
{
public:
	FisheyeVariation(T weight = 1.0) : Variation<T>("fisheye", VAR_FISHEYE, weight, true, true) { }

	VARCOPY(FisheyeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 2 * m_Weight / (helper.m_PrecalcSqrtSumSquares + 1);

		helper.Out.x = r * helper.In.y;
		helper.Out.y = r * helper.In.x;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = 2 * xform->m_VariationWeights[" << varIndex << "] / (precalcSqrtSumSquares + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.y;\n"
		   << "\t\tvOut.y = r * vIn.x;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Popcorn:
/// dx = tan(3 * ty);
/// dy = tan(3 * tx);
/// nx = tx + coef[2][0] * sin(dx);
/// ny = ty + coef[2][1] * sin(dy);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API PopcornVariation : public Variation<T>
{
public:
	PopcornVariation(T weight = 1.0) : Variation<T>("popcorn", VAR_POPCORN, weight) { }

	VARCOPY(PopcornVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = SafeTan<T>(3 * helper.In.y);
		T dy = SafeTan<T>(3 * helper.In.x);
		T nx = helper.In.x + m_Xform->m_Affine.C() * sin(dx);
		T ny = helper.In.y + m_Xform->m_Affine.F() * sin(dy);

		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t dx = tan(3 * vIn.y);\n"
		   << "\t\treal_t dy = tan(3 * vIn.x);\n"
		   << "\t\treal_t nx = vIn.x + xform->m_C * sin(dx);\n"
		   << "\t\treal_t ny = vIn.y + xform->m_F * sin(dy);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * nx;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * ny;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Exponential:
/// dx = exp(tx - 1.0);
/// dy = M_PI * ty;
/// nx = cos(dy) * dx;
/// ny = sin(dy) * dx;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API ExponentialVariation : public Variation<T>
{
public:
	ExponentialVariation(T weight = 1.0) : Variation<T>("exponential", VAR_EXPONENTIAL, weight) { }

	VARCOPY(ExponentialVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = m_Weight * exp(helper.In.x - 1);
		T dy = T(M_PI) * helper.In.y;

		helper.Out.x = dx * cos(dy);
		helper.Out.y = dx * sin(dy);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t dx = xform->m_VariationWeights[" << varIndex << "] * exp(vIn.x - T(1.0));\n"
		   << "\t\treal_t dy = M_PI * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.x = dx * cos(dy);\n"
		   << "\t\tvOut.y = dx * sin(dy);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Power:
/// a = atan2(tx, ty);
/// sa = sin(a);
/// r = sqrt(tx * tx + ty * ty);
/// r = pow(r, sa);
/// nx = r * precalc_cosa;
/// ny = r * sa;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API PowerVariation : public Variation<T>
{
public:
	PowerVariation(T weight = 1.0) : Variation<T>("power", VAR_POWER, weight, true, true, true) { }

	VARCOPY(PowerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * pow(helper.m_PrecalcSqrtSumSquares, helper.m_PrecalcSina);

		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSqrtSumSquares, precalcSina);\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cosine:
/// nx = cos(tx * M_PI) * cosh(ty);
/// ny = -sin(tx * M_PI) * sinh(ty);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API CosineVariation : public Variation<T>
{
public:
	CosineVariation(T weight = 1.0) : Variation<T>("cosine", VAR_COSINE, weight) { }

	VARCOPY(CosineVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x * T(M_PI);
		T nx =  cos(a) * cosh(helper.In.y);
		T ny = -sin(a) * sinh(helper.In.y);

		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x * M_PI;\n"
		   << "\t\treal_t nx = cos(a) * cosh(vIn.y);\n"
		   << "\t\treal_t ny = -sin(a) * sinh(vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * nx;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * ny;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Rings:
/// dx = coef[2][0];
/// dx = dx * dx + EPS;
/// r = sqrt(tx * tx + ty * ty);
/// r = fmod(r + dx, 2 * dx) - dx + r * (1 - dx);
/// a = atan2(tx, ty);
/// nx = cos(a) * r;
/// ny = sin(a) * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API RingsVariation : public Variation<T>
{
public:
	RingsVariation(T weight = 1.0) : Variation<T>("rings", VAR_RINGS, weight, true, true, true) { }

	VARCOPY(RingsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = Zeps(m_Xform->m_Affine.C() * m_Xform->m_Affine.C());
		T r = helper.m_PrecalcSqrtSumSquares;

		r = m_Weight * (fmod(r + dx, 2 * dx) - dx + r * (1 - dx));
		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t dx = Zeps(xform->m_C * xform->m_C);\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tr = xform->m_VariationWeights[" << varIndex << "] * (fmod(r + dx, 2 * dx) - dx + r * (1 - dx));\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Fan:
/// dx = coef[2][0];
/// dy = coef[2][1];
/// dx = M_PI * (dx * dx + EPS);
/// dx2 = dx / 2;
/// a = atan(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// a += (fmod(a + dy, dx) > dx2) ? -dx2 : dx2;
/// nx = cos(a) * r;
/// ny = sin(a) * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API FanVariation : public Variation<T>
{
public:
	FanVariation(T weight = 1.0) : Variation<T>("fan", VAR_FAN, weight, true, true, false, true) { }

	VARCOPY(FanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = T(M_PI) * Zeps(m_Xform->m_Affine.C() * m_Xform->m_Affine.C());
		T dy = m_Xform->m_Affine.F();
		T dx2 = T(0.5) * dx;
		T a = helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;

		a += (fmod(a + dy, dx) > dx2) ? -dx2 : dx2;
		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t dx = M_PI * Zeps(xform->m_C * xform->m_C);\n"
		   << "\t\treal_t dy = xform->m_F;\n"
		   << "\t\treal_t dx2 = T(0.5) * dx;\n"
		   << "\t\treal_t a = precalcAtanxy + ((fmod(precalcAtanxy + dy, dx) > dx2) ? -dx2 : dx2);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Blob:
/// a = atan2(tx, ty);
/// r = sqrt(tx * tx + ty * ty);
/// r = r * (bloblow + (blobhigh - bloblow) * (0.5 + 0.5 * sin(blobwaves * a)));
/// nx = sin(a) * r;
/// ny = cos(a) * r;
///
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API BlobVariation : public ParametricVariation<T>
{
public:
	BlobVariation(T weight = 1.0) : ParametricVariation<T>("blob", VAR_BLOB, weight, true, true, true, true)
	{
		Init();
	}

	PARVARCOPY(BlobVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares * (m_BlobLow + m_BlobDiff * (T(0.5) + T(0.5) * sin(m_BlobWaves * helper.m_PrecalcAtanxy)));

		helper.Out.x = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.y = m_Weight * helper.m_PrecalcCosa * r;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string blobLow   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobHigh  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobWaves = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobDiff  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares * (" << blobLow << " + " << blobDiff << " * (T(0.5) + T(0.5) * sin(" << blobWaves << " * precalcAtanxy)));\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * precalcSina * r);\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * precalcCosa * r);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_BlobDiff = m_BlobHigh - m_BlobLow;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_BlobLow   = T(0.2) + T(0.5) * rand.Frand01<T>();
		m_BlobHigh  = T(0.8) + T(0.4) * rand.Frand01<T>();
		m_BlobWaves = T(int(2 + 5 * rand.Frand01<T>()));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlobLow,   prefix + "blob_low"));
		m_Params.push_back(ParamWithName<T>(&m_BlobHigh,  prefix + "blob_high", 1));
		m_Params.push_back(ParamWithName<T>(&m_BlobWaves, prefix + "blob_waves", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlobDiff, prefix + "blob_diff"));//Precalc.
	}

private:
	T m_BlobLow;
	T m_BlobHigh;
	T m_BlobWaves;
	T m_BlobDiff;//Precalc.
};

/// <summary>
/// Pdj:
/// nx1 = cos(pdjb * tx);
/// nx2 = sin(pdjc * tx);
/// ny1 = sin(pdja * ty);
/// ny2 = cos(pdjd * ty);
///
/// p[0] += weight * (ny1 - nx1);
/// p[1] += weight * (nx2 - ny2);
/// </summary>
template <typename T>
class EMBER_API PdjVariation : public ParametricVariation<T>
{
public:
	PdjVariation(T weight = 1.0) : ParametricVariation<T>("pdj", VAR_PDJ, weight)
	{
		Init();
	}

	PARVARCOPY(PdjVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T nx1 = cos(m_PdjB * helper.In.x);
		T nx2 = sin(m_PdjC * helper.In.x);
		T ny1 = sin(m_PdjA * helper.In.y);
		T ny2 = cos(m_PdjD * helper.In.y);

		helper.Out.x = m_Weight * (ny1 - nx1);
		helper.Out.y = m_Weight * (nx2 - ny2);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string pdjA = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjB = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjC = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t nx1 = cos(" << pdjB << " * vIn.x)" << ";\n"
		   << "\t\treal_t nx2 = sin(" << pdjC << " * vIn.x)" << ";\n"
		   << "\t\treal_t ny1 = sin(" << pdjA << " * vIn.y)" << ";\n"
		   << "\t\treal_t ny2 = cos(" << pdjD << " * vIn.y)" << ";\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * (ny1 - nx1));\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * (nx2 - ny2));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_PdjA = 3 * rand.Frand11<T>();
		m_PdjB = 3 * rand.Frand11<T>();
		m_PdjC = 3 * rand.Frand11<T>();
		m_PdjD = 3 * rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PdjA, prefix + "pdj_a"));
		m_Params.push_back(ParamWithName<T>(&m_PdjB, prefix + "pdj_b"));
		m_Params.push_back(ParamWithName<T>(&m_PdjC, prefix + "pdj_c"));
		m_Params.push_back(ParamWithName<T>(&m_PdjD, prefix + "pdj_d"));
	}

private:
	T m_PdjA;
	T m_PdjB;
	T m_PdjC;
	T m_PdjD;
};

/// <summary>
/// Fan2:
/// a = precalc_atan;
/// r = precalc_sqrt;
///
/// dy = fan2y;
/// dx = M_PI * (fan2x * fan2x + EPS);
/// dx2 = dx / 2.0;
///
/// t = a + dy - dx * (int)((a + dy) / dx);
///
/// if (t > dx2)
///     a = a - dx2;
/// else
///     a = a + dx2;
///
/// nx = sin(a) * r;
/// ny = cos(a) * r;
///
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API Fan2Variation : public ParametricVariation<T>
{
public:
	Fan2Variation(T weight = 1.0) : ParametricVariation<T>("fan2", VAR_FAN2, weight, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(Fan2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		T t = a + m_Fan2Y - m_Fan2Dx * int((a + m_Fan2Y) / m_Fan2Dx);

		if (t > m_Fan2Dx2)
			a = a - m_Fan2Dx2;
		else
			a = a + m_Fan2Dx2;

		helper.Out.x = r * sin(a);
		helper.Out.y = r * cos(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string fan2X = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fan2Y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanxy;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\t\treal_t t = a + " << fan2Y << " - " << dx << " * (int)((a + " << fan2Y << ") / " << dx << ");\n"
		   << "\n"
		   << "\t\tif (t > " << dx2 << ")\n"
		   << "\t\t	a = a - " << dx2 << ";\n"
		   << "\t\telse\n"
		   << "\t\t	a = a + " << dx2 << ";\n"
		   << "\n"
		   << "\t\tvOut.x = r * sin(a);\n"
		   << "\t\tvOut.y = r * cos(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Fan2Dx = T(M_PI) * Zeps(SQR(m_Fan2X));
		m_Fan2Dx2 = T(0.5) * m_Fan2Dx;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Fan2X = rand.Frand11<T>();
		m_Fan2Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Fan2X, prefix + "fan2_x"));
		m_Params.push_back(ParamWithName<T>(&m_Fan2Y, prefix + "fan2_y"));
		m_Params.push_back(ParamWithName<T>(true, &m_Fan2Dx,  prefix + "fan2_dx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Fan2Dx2, prefix + "fan2_dx2"));
	}

private:
	T m_Fan2X;
	T m_Fan2Y;
	T m_Fan2Dx;//Precalc.
	T m_Fan2Dx2;
};

/// <summary>
/// Rings2:
/// r = precalc_sqrt;
/// dx = rings2val * rings2val + EPS;
/// r += dx - 2.0 * dx * (int)((r + dx)/(2.0 * dx)) - dx + r * (1.0 - dx);
/// nx = precalc_sina * r;
/// ny = precalc_cosa * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class EMBER_API Rings2Variation : public ParametricVariation<T>
{
public:
	Rings2Variation(T weight = 1.0) : ParametricVariation<T>("rings2", VAR_RINGS2, weight, true, true, true)
	{
		Init();
	}

	PARVARCOPY(Rings2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;

		r += -2 * m_Rings2Val2 * int((r + m_Rings2Val2) / (2 * m_Rings2Val2)) + r * (1 - m_Rings2Val2);
		helper.Out.x = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.y = m_Weight * helper.m_PrecalcCosa * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rings2Val  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rings2Val2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tr += -T(2.0) * " << rings2Val2 << " * (int)((r + " << rings2Val2 << ") / (T(2.0) * " << rings2Val2 << ")) + r * (T(1.0) - " << rings2Val2 << ");\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * precalcSina * r);\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * precalcCosa * r);\n"
		   << "\t\tvOut.z =  xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Rings2Val2 = Zeps(SQR(m_Rings2Val));
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rings2Val = 2 * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rings2Val, prefix + "rings2_val", 1));//This differs from the original which used zero. Use 1 instead to avoid getting too close to dividing by zero.
		m_Params.push_back(ParamWithName<T>(true, &m_Rings2Val2, prefix + "rings2_val2"));//Precalc.
	}

private:
	T m_Rings2Val;
	T m_Rings2Val2;//Precalc.
};

/// <summary>
/// Eyefish:
/// r = 2.0 * weight / (precalc_sqrt + 1.0);
/// p[0] += r * tx;
/// p[1] += r * ty;
/// </summary>
template <typename T>
class EMBER_API EyefishVariation : public Variation<T>
{
public:
	EyefishVariation(T weight = 1.0) : Variation<T>("eyefish", VAR_EYEFISH, weight, true, true) { }

	VARCOPY(EyefishVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 2 * m_Weight / (helper.m_PrecalcSqrtSumSquares + 1);

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = (xform->m_VariationWeights[" << varIndex << "] * T(2.0)) / (precalcSqrtSumSquares + T(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Bubble.
/// </summary>
template <typename T>
class EMBER_API BubbleVariation : public Variation<T>
{
public:
	BubbleVariation(T weight = 1.0) : Variation<T>("bubble", VAR_BUBBLE, weight, true) { }

	VARCOPY(BubbleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T denom = T(0.25) * helper.m_PrecalcSumSquares + 1;
		T r = m_Weight / denom;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * (2 / denom - 1);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t denom = T(0.25) * precalcSumSquares + 1;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / denom;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (2 / denom - 1);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cylinder.
/// </summary>
template <typename T>
class EMBER_API CylinderVariation : public Variation<T>
{
public:
	CylinderVariation(T weight = 1.0) : Variation<T>("cylinder", VAR_CYLINDER, weight) { }

	VARCOPY(CylinderVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * sin(helper.In.x);
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * cos(helper.In.x);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * cos(vIn.x);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Perspective.
/// </summary>
template <typename T>
class EMBER_API PerspectiveVariation : public ParametricVariation<T>
{
public:
	PerspectiveVariation(T weight = 1.0) : ParametricVariation<T>("perspective", VAR_PERSPECTIVE, weight)
	{
		Init();
	}

	PARVARCOPY(PerspectiveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = Zeps(m_Dist - helper.In.y * m_Vsin);
		T t = 1 / d;

		helper.Out.x = m_Weight * m_Dist * helper.In.x * t;
		helper.Out.y = m_Weight * m_VfCos * helper.In.y * t;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vSin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string vfCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t d = Zeps(" << dist << " - vIn.y * " << vSin << ");\n"
		   << "\t\treal_t t = T(1.0) / d;\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * " << dist << " * vIn.x * t);\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * " << vfCos << " * vIn.y * t);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T angle = m_Angle * T(M_PI) / 2;

		m_Vsin = sin(angle);
		m_VfCos = m_Dist * cos(angle);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = rand.Frand01<T>();
		m_Dist = 2 * rand.Frand01<T>() + 1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "perspective_angle"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "perspective_dist"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vsin,  prefix + "perspective_vsin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_VfCos, prefix + "perspective_vfcos"));
	}

private:
	T m_Angle;//Params.
	T m_Dist;
	T m_Vsin;//Precalc.
	T m_VfCos;
};

/// <summary>
/// Noise.
/// </summary>
template <typename T>
class EMBER_API NoiseVariation : public Variation<T>
{
public:
	NoiseVariation(T weight = 1.0) : Variation<T>("noise", VAR_NOISE, weight) { }

	VARCOPY(NoiseVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tempr = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * rand.Frand01<T>();

		helper.Out.x = helper.In.x * r * cos(tempr);
		helper.Out.y = helper.In.y * r * sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t tempr = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * cos(tempr);\n"
		   << "\t\tvOut.y = vIn.y * r * sin(tempr);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// JuliaN.
/// </summary>
template <typename T>
class EMBER_API JuliaNGenericVariation : public ParametricVariation<T>
{
public:
	JuliaNGenericVariation(T weight = 1.0) : ParametricVariation<T>("julian", VAR_JULIAN, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaNGenericVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tempr = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(ISAAC_INT(m_Rn))) / m_Power;
		T r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn);

		helper.Out.x = r * cos(tempr);
		helper.Out.y = r * sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint tRnd = (int)(" << rn << " * MwcNext01(mwc));\n"
		   << "\t\treal_t tempr = (precalcAtanyx + M_2PI * tRnd) / " << power << ";\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << ");\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(tempr);\n"
		   << "\t\tvOut.y = r * sin(tempr);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Power = Zeps(m_Power);
		m_Rn = fabs(m_Power);
		m_Cn = m_Dist / m_Power / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Dist = 1;
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "julian_dist", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "julian_power", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "julian_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "julian_cn"));
	}

private:
	T m_Dist;//Params.
	T m_Power;
	T m_Rn;//Precalc.
	T m_Cn;
};

/// <summary>
/// JuliaScope.
/// </summary>
template <typename T>
class EMBER_API JuliaScopeVariation : public ParametricVariation<T>
{
public:
	JuliaScopeVariation(T weight = 1.0) : ParametricVariation<T>("juliascope", VAR_JULIASCOPE, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaScopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int rnd = int(m_Rn * rand.Frand01<T>());
		T tempr, r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn);

		if ((rnd & 1) == 0)
			tempr = (M_2PI * rnd + helper.m_PrecalcAtanyx) / m_Power;
		else
			tempr = (M_2PI * rnd - helper.m_PrecalcAtanyx) / m_Power;

		helper.Out.x = r * cos(tempr);
		helper.Out.y = r * sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;

		//int rnd = (int)(m_Rn * rand.Frand01<T>());
		//T tempr, r;

		//if ((rnd & 1) == 0)
		//	tempr = (2 * T(M_PI) * (int)(m_Rn * rand.Frand01<T>()) + helper.m_PrecalcAtanyx) / m_Power;//Fixed to get new random rather than use rnd from above.//SMOULDER
		//else
		//	tempr = (2 * T(M_PI) * (int)(m_Rn * rand.Frand01<T>()) - helper.m_PrecalcAtanyx) / m_Power;

		//r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn);

		//helper.Out.x = r * cos(tempr);
		//helper.Out.y = r * sin(tempr);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;


		ss << "\t{\n"
			<< "\t\tint rnd = (int)(" << rn << " * MwcNext01(mwc));\n"
			<< "\t\treal_t tempr, r;\n"
			<< "\n"
			<< "\t\tif ((rnd & 1) == 0)\n"
			<< "\t\t	tempr = (M_2PI * rnd + precalcAtanyx) / " << power << ";\n"
			<< "\t\telse\n"
			<< "\t\t	tempr = (M_2PI * rnd - precalcAtanyx) / " << power << ";\n"
			<< "\n"
			<< "\t\tr = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << ");\n"
			<< "\n"
			<< "\t\tvOut.x = r * cos(tempr);\n"
			<< "\t\tvOut.y = r * sin(tempr);\n"
			<< "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
			<< "\t}\n";

		//ss << "\t{\n"
		//   << "\t\tint rnd = (int)(" << rn << " * MwcNext01(mwc));\n"
		//   << "\t\treal_t tempr, r;\n"
		//   << "\n"
		//   << "\t\tif ((rnd & 1) == 0)\n"
		//   << "\t\t	tempr = (2 * M_PI * (int)(" << rn << " * MwcNext01(mwc)) + precalcAtanyx) / " << power << ";\n"//Fixed to get new random rather than use rnd from above.//SMOULDER
		//   << "\t\telse\n"
		//   << "\t\t	tempr = (2 * M_PI * (int)(" << rn << " * MwcNext01(mwc)) - precalcAtanyx) / " << power << ";\n"
		//   << "\n"
		//   << "\t\tr = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << ");\n"
		//   << "\n"
		//   << "\t\tvOut.x = r * cos(tempr);\n"
		//   << "\t\tvOut.y = r * sin(tempr);\n"
		//	 << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		//   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Rn = fabs(m_Power);
		m_Cn = m_Dist / m_Power / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Dist = 1;
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "juliascope_dist", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "juliascope_power", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "juliascope_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "juliascope_cn"));
	}

private:
	T m_Dist;//Params.
	T m_Power;
	T m_Rn;//Precalc.
	T m_Cn;
};

/// <summary>
/// Blur.
/// This is somewhat different than the original functionality in that blur used
/// the code below, but pre_blur used gaussian_blur.
/// If the original pre_blur functionality is needed, use pre_gaussian_blur.
/// </summary>
template <typename T>
class EMBER_API BlurVariation : public Variation<T>
{
public:
	BlurVariation(T weight = 1.0) : Variation<T>("blur", VAR_BLUR, weight) { }

	VARCOPY(BlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tempr = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * rand.Frand01<T>();

		helper.Out.x = r * cos(tempr);
		helper.Out.y = r * sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t tmpr = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(tmpr);\n"
		   << "\t\tvOut.y = r * sin(tmpr);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Gaussian blur.
/// </summary>
template <typename T>
class EMBER_API GaussianBlurVariation : public Variation<T>
{
public:
	GaussianBlurVariation(T weight = 1.0) : Variation<T>("gaussian_blur", VAR_GAUSSIAN_BLUR, weight) { }

	VARCOPY(GaussianBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);

		helper.Out.x = r * cos(angle);
		helper.Out.y = r * sin(angle);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - T(2.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(angle);\n"
		   << "\t\tvOut.y = r * sin(angle);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Radial blur.
/// </summary>
template <typename T>
class EMBER_API RadialBlurVariation : public ParametricVariation<T>
{
public:
	RadialBlurVariation(T weight = 1.0) : ParametricVariation<T>("radial_blur", VAR_RADIAL_BLUR, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(RadialBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Get pseudo-gaussian.
		T rndG = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>()
				+ rand.Frand01<T>() + rand.Frand01<T>() - 2);

		//Calculate angle & zoom.
		T ra = helper.m_PrecalcSqrtSumSquares;
		T tempa = helper.m_PrecalcAtanyx + m_Spin * rndG;
		T rz = m_Zoom * rndG - 1;

		helper.Out.x = ra * cos(tempa) + rz * helper.In.x;
		helper.Out.y = ra * sin(tempa) + rz * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string spin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string zoom  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t rndG = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - T(2.0));\n"
		   << "\t\treal_t ra = precalcSqrtSumSquares;\n"
		   << "\t\treal_t tempa = precalcAtanyx + "<< spin << " * rndG;\n"
		   << "\t\treal_t rz = " << zoom << " * rndG - 1;\n"
		   << "\n"
		   << "\t\tvOut.x = ra * cos(tempa) + rz * vIn.x;\n"
		   << "\t\tvOut.y = ra * sin(tempa) + rz * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Angle * T(M_PI) / 2, &m_Spin, &m_Zoom);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = (2 * rand.Frand01<T>() - 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "radial_blur_angle"));//Params.
		m_Params.push_back(ParamWithName<T>(true, &m_Spin, prefix + "radial_blur_spin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Zoom, prefix + "radial_blur_zoom"));
	}

private:
	T m_Angle;//Params.
	T m_Spin;//Precalc.
	T m_Zoom;
};

/// <summary>
/// Pie.
/// </summary>
template <typename T>
class EMBER_API PieVariation : public ParametricVariation<T>
{
public:
	PieVariation(T weight = 1.0) : ParametricVariation<T>("pie", VAR_PIE, weight)
	{
		Init();
	}

	PARVARCOPY(PieVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int sl = int(rand.Frand01<T>() * m_Slices + T(0.5));
		T a = m_Rotation + M_2PI * (sl + rand.Frand01<T>() * m_Thickness) / m_Slices;
		T r = m_Weight * rand.Frand01<T>();

		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string slices =    "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotation =  "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint sl = (int)(MwcNext01(mwc) * " << slices << " + T(0.5));\n"
		   << "\t\treal_t a = " << rotation << " + M_2PI * (sl + MwcNext01(mwc) * " << thickness << ") / " << slices << ";\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Params[0].Set(10 * rand.Frand01<T>());//Slices.
		m_Params[1].Set(M_2PI * rand.Frand11<T>());//Rotation.
		m_Thickness = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Slices,    prefix + "pie_slices", 6, INTEGER_NONZERO, 1));
		m_Params.push_back(ParamWithName<T>(&m_Rotation,  prefix + "pie_rotation", T(T(0.5)), REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "pie_thickness", T(T(0.5)), REAL, 0, 1));
	}

private:
	T m_Slices;
	T m_Rotation;
	T m_Thickness;
};

/// <summary>
/// Ngon.
/// </summary>
template <typename T>
class EMBER_API NgonVariation : public ParametricVariation<T>
{
public:
	NgonVariation(T weight = 1.0) : ParametricVariation<T>("ngon", VAR_NGON, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(NgonVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rFactor;

		if ((helper.In.x == 0) && (helper.In.y == 0))
			rFactor = 0;
		else
			rFactor = pow(helper.m_PrecalcSumSquares, m_CPower);

		T phi = helper.m_PrecalcAtanyx - m_CSides * Floor<T>(helper.m_PrecalcAtanyx * m_CSidesInv);

		if (phi > T(0.5) * m_CSides)
			phi -= m_CSides;

		T amp = (m_Corners * (1 / cos(phi) - 1) + m_Circle) * m_Weight * rFactor;

		helper.Out.x = amp * helper.In.x;
		helper.Out.y = amp * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string sides     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string circle    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string corners   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string csides    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string csidesinv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cpower    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t rFactor;\n"
		   << "\n"
		   << "\t\tif ((vIn.x == T(0.0)) && (vIn.y == T(0.0)))\n"
		   << "\t\t	rFactor = T(0.0);\n"
		   << "\t\telse\n"
		   << "\t\t	rFactor = pow(precalcSumSquares, " << cpower << ");\n"
		   << "\n"
		   << "\t\treal_t phi = precalcAtanyx - " << csides << " * floor(precalcAtanyx * " << csidesinv << ");\n"
		   << "\n"
		   << "\t\tif (phi > T(0.5) * " << csides << ")\n"
		   << "\t\t	phi -= " << csides << ";\n"
		   << "\n"
		   << "\t\treal_t amp = (" << corners << " * (1 / cos(phi) - 1) + " << circle << ") * xform->m_VariationWeights[" << varIndex << "] * rFactor;\n"
		   << "\n"
		   << "\t\tvOut.x = amp * vIn.x;\n"
		   << "\t\tvOut.y = amp * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_CPower = -T(0.5) * m_Power;
		m_CSides = 2 * T(M_PI) / m_Sides;
		m_CSidesInv = 1 / m_CSides;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Sides = T(int(rand.Frand01<T>() * 10 + 3));
		m_Power = 3 * rand.Frand01<T>() + 1;
		m_Circle = 3 * rand.Frand01<T>();
		m_Corners = 2 * rand.Frand01<T>() * m_Circle;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sides,   prefix + "ngon_sides", 5, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Power,   prefix + "ngon_power", 3));
		m_Params.push_back(ParamWithName<T>(&m_Circle,  prefix + "ngon_circle", 1));
		m_Params.push_back(ParamWithName<T>(&m_Corners, prefix + "ngon_corners", 2));
		m_Params.push_back(ParamWithName<T>(true, &m_CSides,    prefix + "ngon_csides"));
		m_Params.push_back(ParamWithName<T>(true, &m_CSidesInv, prefix + "ngon_csides_inv"));
		m_Params.push_back(ParamWithName<T>(true, &m_CPower,    prefix + "ngon_cpower"));
	}

private:
	T m_Sides;
	T m_Power;
	T m_Circle;
	T m_Corners;
	T m_CSides;
	T m_CSidesInv;
	T m_CPower;
};

/// <summary>
/// Curl.
/// Note that in Apophysis, curl and post_curl differed slightly.
/// Using what post_curl did here gave bad results, so sticking with the original
/// curl code.
/// </summary>
template <typename T>
class EMBER_API CurlVariation : public ParametricVariation<T>
{
public:
	CurlVariation(T weight = 1.0) : ParametricVariation<T>("curl", VAR_CURL, weight)
	{
		Init();
	}

	PARVARCOPY(CurlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T re = 1 + m_C1 * helper.In.x + m_C2 * (SQR(helper.In.x) - SQR(helper.In.y));
		T im = m_C1 * helper.In.y + m_C22 * helper.In.x * helper.In.y;
		T r = m_Weight / Zeps(SQR(re) + SQR(im));

		helper.Out.x = (helper.In.x * re + helper.In.y * im) * r;
		helper.Out.y = (helper.In.y * re - helper.In.x * im) * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string c1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c22 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t re = T(1.0) + " << c1 << " * vIn.x + " << c2 << " * (SQR(vIn.x) - SQR(vIn.y));\n"
		   << "\t\treal_t im = " << c1 << " * vIn.y + " << c22 << " * vIn.x * vIn.y;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / Zeps(SQR(re) + SQR(im));\n"
		   << "\n"
		   << "\t\tvOut.x = (vIn.x * re + vIn.y * im) * r;\n"
		   << "\t\tvOut.y = (vIn.y * re - vIn.x * im) * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C22 = 2 * m_C2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_C1 = rand.Frand01<T>();
		m_C2 = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C1, prefix + "curl_c1", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2, prefix + "curl_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C22, prefix + "curl_c22"));//Precalc.
	}

private:
	T m_C1;
	T m_C2;
	T m_C22;//Precalc.
};

/// <summary>
/// Rectangles.
/// </summary>
template <typename T>
class EMBER_API RectanglesVariation : public ParametricVariation<T>
{
public:
	RectanglesVariation(T weight = 1.0) : ParametricVariation<T>("rectangles", VAR_RECTANGLES, weight)
	{
		Init();
	}

	PARVARCOPY(RectanglesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_X == 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = m_Weight * ((2 * Floor<T>(helper.In.x / m_X) + 1) * m_X - helper.In.x);

		if (m_Y == 0)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = m_Weight * ((2 * Floor<T>(helper.In.y / m_Y) + 1) * m_Y - helper.In.y);

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (" << x << " == 0)\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = (xform->m_VariationWeights[" << varIndex << "] * ((2 * floor(vIn.x / " << x << ") + 1) * " << x << " - vIn.x));\n"
		   << "\n"
		   << "\t\tif (" << y << " == 0)\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = (xform->m_VariationWeights[" << varIndex << "] * ((2 * floor(vIn.y / " << y << ") + 1) * " << y << " - vIn.y));\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand01<T>();
		m_Y = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "rectangles_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "rectangles_y", 1));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// Arch.
/// </summary>
template <typename T>
class EMBER_API ArchVariation : public Variation<T>
{
public:
	ArchVariation(T weight = 1.0) : Variation<T>("arch", VAR_ARCH, weight) { }

	VARCOPY(ArchVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * m_Weight * T(M_PI);
		T sinr, cosr;

		sincos(angle, &sinr, &cosr);
		helper.Out.x = m_Weight * sinr;
		helper.Out.y = m_Weight * (sinr * sinr) / cosr;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * xform->m_VariationWeights[" << varIndex << "] * M_PI;\n"
		   << "\t\treal_t sinr = sin(angle);\n"
		   << "\t\treal_t cosr = cos(angle);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sinr;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (sinr * sinr) / cosr;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Tangent.
/// </summary>
template <typename T>
class EMBER_API TangentVariation : public Variation<T>
{
public:
	TangentVariation(T weight = 1.0) : Variation<T>("tangent", VAR_TANGENT, weight) { }

	VARCOPY(TangentVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * sin(helper.In.x) / cos(helper.In.y);
		helper.Out.y = m_Weight * SafeTan<T>(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x) / cos(vIn.y);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * tan(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Square.
/// </summary>
template <typename T>
class EMBER_API SquareVariation : public Variation<T>
{
public:
	SquareVariation(T weight = 1.0) : Variation<T>("square", VAR_SQUARE, weight) { }

	VARCOPY(SquareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.y = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - T(0.5));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - T(0.5));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Rays.
/// </summary>
template <typename T>
class EMBER_API RaysVariation : public Variation<T>
{
public:
	RaysVariation(T weight = 1.0) : Variation<T>("rays", VAR_RAYS, weight, true) { }

	VARCOPY(RaysVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ang = m_Weight * rand.Frand01<T>() * T(M_PI);
		T r = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		T tanr = m_Weight * SafeTan<T>(ang) * r;

		helper.Out.x = tanr * cos(helper.In.x);
		helper.Out.y = tanr * sin(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t ang = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc) * M_PI;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / Zeps(precalcSumSquares);\n"
		   << "\t\treal_t tanr = xform->m_VariationWeights[" << varIndex << "] * tan(ang) * r;\n"
		   << "\n"
		   << "\t\tvOut.x = tanr * cos(vIn.x);\n"
		   << "\t\tvOut.y = tanr * sin(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Blade.
/// </summary>
template <typename T>
class EMBER_API BladeVariation : public Variation<T>
{
public:
	BladeVariation(T weight = 1.0) : Variation<T>("blade", VAR_BLADE, weight, true, true) { }

	VARCOPY(BladeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = rand.Frand01<T>() * m_Weight * helper.m_PrecalcSqrtSumSquares;
		T sinr, cosr;

		sincos(r, &sinr, &cosr);
		helper.Out.x = m_Weight * helper.In.x * (cosr + sinr);
		helper.Out.y = m_Weight * helper.In.x * (cosr - sinr);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = MwcNext01(mwc) * xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\t\treal_t sinr = sin(r);\n"
		   << "\t\treal_t cosr = cos(r);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x * (cosr + sinr);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.x * (cosr - sinr);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Secant2.
/// </summary>
template <typename T>
class EMBER_API Secant2Variation : public Variation<T>
{
public:
	Secant2Variation(T weight = 1.0) : Variation<T>("secant2", VAR_SECANT2, weight, true, true) { }

	VARCOPY(Secant2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		T cr = cos(r);
		T icr = 1 / cr;

		helper.Out.x = m_Weight * helper.In.x;

		if (cr < 0)
			helper.Out.y = m_Weight * (icr + 1);
		else
			helper.Out.y = m_Weight * (icr - 1);

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\t\treal_t cr = cos(r);\n"
		   << "\t\treal_t icr = T(1.0) / cr;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\n"
		   << "\t\tif (cr < T(0.0))\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (icr + T(1.0));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (icr - T(1.0));\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// TwinTrian.
/// </summary>
template <typename T>
class EMBER_API TwinTrianVariation : public Variation<T>
{
public:
	TwinTrianVariation(T weight = 1.0) : Variation<T>("TwinTrian", VAR_TWINTRIAN, weight, true, true) { }

	VARCOPY(TwinTrianVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = rand.Frand01<T>() * m_Weight * helper.m_PrecalcSqrtSumSquares;
		T sinr, cosr, diff;

		sincos(r, &sinr, &cosr);
		diff = log10(sinr * sinr) + cosr;

		if (BadVal(diff))
			diff = -30.0;

		helper.Out.x = m_Weight * helper.In.x * diff;
		helper.Out.y = m_Weight * helper.In.x * (diff - sinr * T(M_PI));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = MwcNext01(mwc) * xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\t\treal_t sinr = sin(r);\n"
		   << "\t\treal_t cosr = cos(r);\n"
		   << "\t\treal_t diff = log10(sinr * sinr) + cosr;\n"
		   << "\n"
		   << "\t\tif (BadVal(diff))\n"
		   << "\t\t	diff = -T(30.0);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x * diff;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.x * (diff - sinr * M_PI);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cross.
/// </summary>
template <typename T>
class EMBER_API CrossVariation : public Variation<T>
{
public:
	CrossVariation(T weight = 1.0) : Variation<T>("cross", VAR_CROSS, weight) { }

	VARCOPY(CrossVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(fabs((helper.In.x - helper.In.y) * (helper.In.x + helper.In.y)));

		helper.Out.x = helper.In.x * r;
		helper.Out.y = helper.In.y * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] /Zeps(fabs((vIn.x - vIn.y) * (vIn.x + vIn.y)));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r;\n"
		   << "\t\tvOut.y = vIn.y * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Disc2.
/// </summary>
template <typename T>
class EMBER_API Disc2Variation : public ParametricVariation<T>
{
public:
	Disc2Variation(T weight = 1.0) : ParametricVariation<T>("disc2", VAR_DISC2, weight, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Disc2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, t, sinr, cosr;

		t = m_RotTimesPi * (helper.In.x + helper.In.y);
		sincos(t, &sinr, &cosr);
		r = m_Weight * helper.m_PrecalcAtanxy / T(M_PI);
		helper.Out.x = (sinr + m_CosAdd) * r;
		helper.Out.y = (cosr + m_SinAdd) * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rot        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string twist      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinAdd     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cosAdd     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotTimesPi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t = " << rotTimesPi << " * (vIn.x + vIn.y);\n"
		   << "\t\treal_t sinr = sin(t);\n"
		   << "\t\treal_t cosr = cos(t);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * precalcAtanxy / M_PI;\n"
		   << "\n"
		   << "\t\tvOut.x = (sinr + " << cosAdd << ") * r;\n"
		   << "\t\tvOut.y = (cosr + " << sinAdd << ") * r;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T k, add = m_Twist;

		m_RotTimesPi = m_Rot * T(M_PI);
		sincos(add, &m_SinAdd, &m_CosAdd);
		m_CosAdd -= 1;

		if (add > 2 * M_PI)
		{
			k = (1 + add - 2 * T(M_PI));
			m_CosAdd *= k;
			m_SinAdd *= k;
		}

		if (add < -2 * M_PI)
		{
			k = (1 + add + 2 * T(M_PI));
			m_CosAdd *= k;
			m_SinAdd *= k;
		}
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rot   = T(0.5) * rand.Frand01<T>();
		m_Twist = T(0.5) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rot,   prefix + "disc2_rot"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "disc2_twist"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinAdd,     prefix + "disc2_sin_add"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CosAdd,     prefix + "disc2_cos_add"));
		m_Params.push_back(ParamWithName<T>(true, &m_RotTimesPi, prefix + "disc2_rot_times_pi"));
	}

private:
	T m_Rot;//Params.
	T m_Twist;
	T m_SinAdd;//Precalc.
	T m_CosAdd;
	T m_RotTimesPi;
};

/// <summary>
/// SuperShape.
/// </summary>
template <typename T>
class EMBER_API SuperShapeVariation : public ParametricVariation<T>
{
public:
	SuperShapeVariation(T weight = 1.0) : ParametricVariation<T>("super_shape", VAR_SUPER_SHAPE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(SuperShapeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = m_Pm4 * helper.m_PrecalcAtanyx + T(M_PI_4);

		T t1 = fabs(cos(theta));
		t1 = pow(t1, m_N2);

		T t2 = fabs(sin(theta));
		t2 = pow(t2, m_N3);

		T r = m_Weight * ((m_Rnd * rand.Frand01<T>() + (1 - m_Rnd) * helper.m_PrecalcSqrtSumSquares) - m_Holes)
			* pow(t1 + t2, m_PNeg1N1) / helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string m       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string n1      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n3      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rnd     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pm4     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string pNeg1N1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t theta = " << pm4 << " * precalcAtanyx + M_PI_4;\n"
		   << "\t\treal_t t1 = fabs(cos(theta));\n"
		   << "\t\tt1 = pow(t1, " << n2 << ");\n"
		   << "\t\treal_t t2 = fabs(sin(theta));\n"
		   << "\t\tt2 = pow(t2, " << n3 << ");\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * ((" << rnd << " * MwcNext01(mwc) + (T(1.0) - " << rnd << ") * precalcSqrtSumSquares) - " << holes << ") * pow(t1 + t2, " << pNeg1N1 << ") / precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Pm4 = m_M / T(4.0);
		m_PNeg1N1 = T(-1.0) / m_N1;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rnd   = rand.Frand01<T>();
		m_M     = T(int(rand.Frand01<T>() * 6));
		m_N1    = rand.Frand01<T>() * 40;
		m_N2    = rand.Frand01<T>() * 20;
		m_N3    = m_N2;
		m_Holes = 0.0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_M,       prefix + "super_shape_m"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_N1,      prefix + "super_shape_n1", 1));
		m_Params.push_back(ParamWithName<T>(&m_N2,      prefix + "super_shape_n2", 1));
		m_Params.push_back(ParamWithName<T>(&m_N3,      prefix + "super_shape_n3", 1));
		m_Params.push_back(ParamWithName<T>(&m_Rnd,     prefix + "super_shape_rnd"));
		m_Params.push_back(ParamWithName<T>(&m_Holes,   prefix + "super_shape_holes"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pm4,     prefix + "super_shape_pm4"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_PNeg1N1, prefix + "super_shape_pneg1n1"));
	}

private:
	T m_M;//Params.
	T m_N1;
	T m_N2;
	T m_N3;
	T m_Rnd;
	T m_Holes;
	T m_Pm4;//Precalc.
	T m_PNeg1N1;
};

/// <summary>
/// Flower.
/// </summary>
template <typename T>
class EMBER_API FlowerVariation : public ParametricVariation<T>
{
public:
	FlowerVariation(T weight = 1.0) : ParametricVariation<T>("flower", VAR_FLOWER, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(FlowerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = helper.m_PrecalcAtanyx;
		T r = m_Weight * (rand.Frand01<T>() - m_Holes) * cos(m_Petals * theta) / helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string petals = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes =  "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t theta = precalcAtanyx;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - " << holes << ") * cos(" << petals << " * theta) / precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Petals = 4 * rand.Frand01<T>();
		m_Holes  = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Petals, prefix + "flower_petals"));
		m_Params.push_back(ParamWithName<T>(&m_Holes,  prefix + "flower_holes"));
	}

private:
	T m_Petals;
	T m_Holes;
};

/// <summary>
/// Conic.
/// </summary>
template <typename T>
class EMBER_API ConicVariation : public ParametricVariation<T>
{
public:
	ConicVariation(T weight = 1.0) : ParametricVariation<T>("conic", VAR_CONIC, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ConicVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ct = helper.In.x / helper.m_PrecalcSqrtSumSquares;
		T r = m_Weight * (rand.Frand01<T>() - m_Holes) *
					m_Eccentricity / (1 + m_Eccentricity * ct) / helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string eccentricity = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes =        "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t ct = vIn.x / precalcSqrtSumSquares;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - " << holes << ") * " << eccentricity << " / (1 + " << eccentricity << " * ct) / precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Eccentricity = rand.Frand01<T>();
		m_Holes        = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Eccentricity, prefix + "conic_eccentricity", 1));
		m_Params.push_back(ParamWithName<T>(&m_Holes,        prefix + "conic_holes"));
	}

private:
	T m_Eccentricity;
	T m_Holes;
};

/// <summary>
/// Parabola.
/// </summary>
template <typename T>
class EMBER_API ParabolaVariation : public ParametricVariation<T>
{
public:
	ParabolaVariation(T weight = 1.0) : ParametricVariation<T>("parabola", VAR_PARABOLA, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ParabolaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sr, cr;

		sincos(helper.m_PrecalcSqrtSumSquares, &sr, &cr);
		helper.Out.x = m_Height * m_Weight * sr * sr * rand.Frand01<T>();
		helper.Out.y = m_Width * m_Weight * cr * rand.Frand01<T>();
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string height = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t sr = sin(precalcSqrtSumSquares);\n"
		   << "\t\treal_t cr = cos(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = " << height << " * (xform->m_VariationWeights[" << varIndex << "] * sr * sr * MwcNext01(mwc));\n"
		   << "\t\tvOut.y = " << width << " * (xform->m_VariationWeights[" << varIndex << "] * cr * MwcNext01(mwc));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Height = T(0.5) * rand.Frand01<T>();
		m_Width  = T(0.5) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Height, prefix + "parabola_height"));
		m_Params.push_back(ParamWithName<T>(&m_Width,  prefix + "parabola_width"));
	}

private:
	T m_Height;
	T m_Width;
};

/// <summary>
/// Bent2.
/// </summary>
template <typename T>
class EMBER_API Bent2Variation : public ParametricVariation<T>
{
public:
	Bent2Variation(T weight = 1.0) : ParametricVariation<T>("bent2", VAR_BENT2, weight)
	{
		Init();
	}

	PARVARCOPY(Bent2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x >= 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = m_Vx * helper.In.x;

		if (helper.In.y >= 0)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = m_Vy * helper.In.y;

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (vIn.x >= 0)\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << vx << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (vIn.y >= 0)\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\tvOut.y = " << vy << " * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vx = m_X * m_Weight;
		m_Vy = m_Y * m_Weight;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = 3 * (T(-0.5) + rand.Frand01<T>());
		m_Y = 3 * (T(-0.5) + rand.Frand01<T>());
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "bent2_x", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "bent2_y", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Vx, prefix + "bent2_vx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vy, prefix + "bent2_vy"));
	}

private:
	T m_X;//Params.
	T m_Y;
	T m_Vx;//Precalc.
	T m_Vy;
};

/// <summary>
/// Bipolar.
/// </summary>
template <typename T>
class EMBER_API BipolarVariation : public ParametricVariation<T>
{
public:
	BipolarVariation(T weight = 1.0) : ParametricVariation<T>("bipolar", VAR_BIPOLAR, weight, true)
	{
		Init();
	}

	PARVARCOPY(BipolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T x2y2 = helper.m_PrecalcSumSquares;
		const T t = x2y2 + 1;
		const T x2 = 2 * helper.In.x;
		T y = T(0.5) * atan2(2 * helper.In.y, x2y2 - 1) + m_S;

		if (y > T(M_PI_2))
			y = -T(M_PI_2) + fmod(y + T(M_PI_2), T(M_PI));
		else if (y < -T(M_PI_2))
			y = T(M_PI_2) - fmod(T(M_PI_2) - y, T(M_PI));

		const T f = t + x2;
		const T g = t - x2;

		if ((g == 0) || (f / g <= 0))
		{
			if (m_VarType == VARTYPE_REG)
			{
				helper.Out.x = 0;
				helper.Out.y = 0;
				helper.Out.z = 0;
			}
			else
			{
				helper.Out.x = helper.In.x;
				helper.Out.y = helper.In.y;
				helper.Out.z = helper.In.z;
			}
		}
		else
		{
			helper.Out.x = m_V4 * log((t + x2) / (t - x2));
			helper.Out.y = m_V * y;
			helper.Out.z = m_Weight * helper.In.z;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string shift = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v4    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x2y2 = precalcSumSquares;\n"
		   << "\t\treal_t t = x2y2 + 1;\n"
		   << "\t\treal_t x2 = 2 * vIn.x;\n"
		   << "\t\treal_t ps = " << s << ";\n"
		   << "\t\treal_t y = T(0.5) * atan2(T(2.0) * vIn.y, x2y2 - T(1.0)) + ps;\n"
		   << "\n"
		   << "\t\tif (y > M_PI_2)\n"
		   << "\t\t	y = -M_PI_2 + fmod(y + M_PI_2, M_PI);\n"
		   << "\t\telse if (y < -M_PI_2)\n"
		   << "\t\t	y = M_PI_2 - fmod(M_PI_2 - y, M_PI);\n"
		   << "\n"
		   << "\t\treal_t f = t + x2;\n"
		   << "\t\treal_t g = t - x2;\n"
		   << "\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss << "\t\tif ((g == 0) || (f / g <= 0))\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = 0;\n"
			   << "\t\t	vOut.y = 0;\n"
			   << "\t\t	vOut.z = 0;\n"
			   << "\t\t}\n";
		}
		else
		{
			ss << "\t\tif ((g == 0) || (f / g <= 0))\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = vIn.x;\n"
			   << "\t\t	vOut.y = vIn.y;\n"
			   << "\t\t	vOut.z = vIn.z;\n"
			   << "\t\t}\n";
		}

		ss << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = (" << v4 << " * log((t + x2) / (t - x2)));\n"
		   << "\t\t	vOut.y = (" << v << " * y);\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_S = -T(M_PI_2) * m_Shift;;
		m_V = m_Weight * T(M_2_PI);
		m_V4 = m_Weight * T(0.25) * T(M_2_PI);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Shift = 2 * rand.Frand01<T>() - 1;
	}

	virtual bool SetParamVal(const char* name, T val) override
	{
		if (!_stricmp(name, "bipolar_shift"))
		{
			T temp = Fabsmod(T(0.5) * (val + 1));

			m_Shift = 2 * temp - 1;
			Precalc();
			return true;
		}

		return ParametricVariation<T>::SetParamVal(name, val);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Shift, prefix + "bipolar_shift"));//Params.
		m_Params.push_back(ParamWithName<T>(true, &m_S,  prefix + "bipolar_s"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_V,  prefix + "bipolar_v"));
		m_Params.push_back(ParamWithName<T>(true, &m_V4, prefix + "bipolar_v4"));
	}

private:
	T m_Shift;//Params.
	T m_S;//Precalc.
	T m_V;
	T m_V4;
};

/// <summary>
/// Boarders.
/// </summary>
template <typename T>
class EMBER_API BoardersVariation : public Variation<T>
{
public:
	BoardersVariation(T weight = 1.0) : Variation<T>("boarders", VAR_BOARDERS, weight) { }

	VARCOPY(BoardersVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundX = Rint(helper.In.x);
		T roundY = Rint(helper.In.y);
		T offsetX = helper.In.x - roundX;
		T offsetY = helper.In.y - roundY;

		if (rand.Frand01<T>() >= 0.75)
		{
			helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX);
			helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY);
		}
		else
		{
			if (fabs(offsetX) >= fabs(offsetY))
			{
				if (offsetX >= 0.0)
				{
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX + T(0.25));
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY + T(0.25) * offsetY / offsetX);
				}
				else
				{
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX - T(0.25));
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY - T(0.25) * offsetY / offsetX);
				}
			}
			else
			{
				if (offsetY >= 0.0)
				{
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY + T(0.25));
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX + offsetX / offsetY * T(0.25));
				}
				else
				{
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY - T(0.25));
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX - offsetX / offsetY * T(0.25));
				}
			}
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t roundX = Rint(vIn.x);\n"
		   << "\t\treal_t roundY = Rint(vIn.y);\n"
		   << "\t\treal_t offsetX = vIn.x - roundX;\n"
		   << "\t\treal_t offsetY = vIn.y - roundY;\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) >= T(0.75))\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * T(0.5) + roundX);\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * T(0.5) + roundY);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (fabs(offsetX) >= fabs(offsetY))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetX >= T(0.0))\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * T(0.5) + roundX + T(0.25));\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * T(0.5) + roundY + T(0.25) * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * T(0.5) + roundX - T(0.25));\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * T(0.5) + roundY - T(0.25) * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetY >= T(0.0))\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * T(0.5) + roundY + T(0.25));\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * T(0.5) + roundX + offsetX / offsetY * T(0.25));\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * T(0.5) + roundY - T(0.25));\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * T(0.5) + roundX - offsetX / offsetY * T(0.25));\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Butterfly.
/// </summary>
template <typename T>
class EMBER_API ButterflyVariation : public Variation<T>
{
public:
	ButterflyVariation(T weight = 1.0) : Variation<T>("butterfly", VAR_BUTTERFLY, weight) { }

	VARCOPY(ButterflyVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T wx = m_Weight * T(1.3029400317411197908970256609023);//This precision came from the original.
		T y2 = helper.In.y * 2;
		T r = wx * sqrt(fabs(helper.In.y * helper.In.x) / Zeps(SQR(helper.In.x) + SQR(y2)));

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * y2;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t wx = xform->m_VariationWeights[" << varIndex << "] * T(1.3029400317411197908970256609023);\n"
		   << "\t\treal_t y2 = vIn.y * T(2.0);\n"
		   << "\t\treal_t r = wx * sqrt(fabs(vIn.y * vIn.x) / Zeps(SQR(vIn.x) + SQR(y2)));\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * y2;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cell.
/// </summary>
template <typename T>
class EMBER_API CellVariation : public ParametricVariation<T>
{
public:
	CellVariation(T weight = 1.0) : ParametricVariation<T>("cell", VAR_CELL, weight)
	{
		Init();
	}

	PARVARCOPY(CellVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T invCellSize = 1 / m_Size;
		T x = floor(helper.In.x * invCellSize);//Calculate input cell. Note that int cast is omitted here. See below.
		T y = floor(helper.In.y * invCellSize);
		T dx = helper.In.x - x * m_Size;//Offset from cell origin.
		T dy = helper.In.y - y * m_Size;

		//Interleave cells.
		if (y >= 0)
		{
			if (x >= 0)
			{
				y *= 2;
				x *= 2;
			}
			else
			{
				y *= 2;
				x = -(2 * x + 1);
			}
		}
		else
		{
			if (x >= 0)
			{
				y = -(2 * y + 1);
				x *= 2;
			}
			else
			{
				y = -(2 * y + 1);
				x = -(2 * x + 1);
			}
		}

		helper.Out.x = m_Weight * (dx + x * m_Size);
		helper.Out.y = -(m_Weight * (dy + y * m_Size));
		helper.Out.z = m_Weight * helper.In.z;
	}

	/// <summary>
	/// Cell is very strange and will not run using integers.
	/// When using floats, it at least gives some output, however
	/// that output is slightly different than the CPU. But not by enough
	/// to change the shape of the final image.
	/// </summary>
	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t invCellSize = T(1.0) / " << size << ";\n"
		   //Float to int, orig.
		   //<< "\t\tint x = (int)floor(vIn.x * invCellSize);\n"
		   //<< "\t\tint y = (int)floor(vIn.y * invCellSize);\n"

		   //For some reason, OpenCL renders nothing if these are ints, so use floats.
		   //Note that Cuburn also omits the usage of ints.
		   << "\t\treal_t x = floor(vIn.x * invCellSize);\n"
		   << "\t\treal_t y = floor(vIn.y * invCellSize);\n"
		   << "\t\treal_t dx = vIn.x - x * " << size << ";\n"
		   << "\t\treal_t dy = vIn.y - y * " << size << ";\n"
		   << "\n"
		   << "\t\tif (y >= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		y *= 2;\n"
		   << "\t\t		x *= 2;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		y *= 2;\n"
		   << "\t\t		x = -(2 * x + 1);\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		y = -(2 * y + 1);\n"
		   << "\t\t		x *= 2;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		y = -(2 * y + 1);\n"
		   << "\t\t		x = -(2 * x + 1);\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (dx + x * " << size << ");\n"
		   << "\t\tvOut.y = -(xform->m_VariationWeights[" << varIndex << "] * (dy + y * " << size << "));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Size = 2 * rand.Frand01<T>() + T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Size, prefix + "cell_size", 1));
	}

private:
	T m_Size;
};

/// <summary>
/// Cpow.
/// </summary>
template <typename T>
class EMBER_API CpowVariation : public ParametricVariation<T>
{
public:
	CpowVariation(T weight = 1.0) : ParametricVariation<T>("cpow", VAR_CPOW, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(CpowVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T lnr = T(0.5) * log(helper.m_PrecalcSumSquares);
		T angle = m_C * a + m_D * lnr + m_Ang * Floor<T>(m_Power * rand.Frand01<T>());
		T m = m_Weight * exp(m_C * lnr - m_D * a);

		helper.Out.x = m * cos(angle);
		helper.Out.y = m * sin(angle);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string powerR = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powerI = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ang    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t lnr = T(0.5) * log(precalcSumSquares);\n"
		   << "\t\treal_t angle = " << c << " * a + " << d << " * lnr + " << ang << " * floor(" << power << " * MwcNext01(mwc));\n"
		   << "\t\treal_t m = xform->m_VariationWeights[" << varIndex << "] * exp(" << c << " * lnr - " << d << " * a);\n"
		   << "\n"
		   << "\t\tvOut.x = m * cos(angle);\n"
		   << "\t\tvOut.y = m * sin(angle);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C = m_PowerR / m_Power;
		m_D = m_PowerI / m_Power;
		m_Ang = 2 * T(M_PI) / m_Power;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_PowerR = 3 * rand.Frand01<T>();
		m_PowerI = rand.Frand01<T>() - T(0.5);
		m_Params[2].Set(5 * rand.Frand01<T>());//Power.
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PowerR, prefix + "cpow_r", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_PowerI, prefix + "cpow_i"));
		m_Params.push_back(ParamWithName<T>(&m_Power,  prefix + "cpow_power", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_C,   prefix + "cpow_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_D,   prefix + "cpow_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ang, prefix + "cpow_ang"));
	}

private:
	T m_PowerR;//Params.
	T m_PowerI;
	T m_Power;
	T m_C;//Precalc.
	T m_D;
	T m_Ang;
};

/// <summary>
/// Curve.
/// </summary>
template <typename T>
class EMBER_API CurveVariation : public ParametricVariation<T>
{
public:
	CurveVariation(T weight = 1.0) : ParametricVariation<T>("curve", VAR_CURVE, weight)
	{
		Init();
	}

	PARVARCOPY(CurveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x + m_XAmpV * exp(-helper.In.y * helper.In.y * m_XLengthV);
		helper.Out.y = m_Weight * helper.In.y + m_YAmpV * exp(-helper.In.x * helper.In.x * m_YLengthV);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xAmp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yAmp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xLength  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yLength  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xAmpV    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yAmpV    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xLengthV = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yLengthV = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x + " << xAmpV << " * exp(-vIn.y * vIn.y * " << xLengthV << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y + " << yAmpV << " * exp(-vIn.x * vIn.x * " << yLengthV << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XAmpV = m_Weight * m_XAmp;
		m_YAmpV = m_Weight * m_YAmp;
		m_XLengthV = 1 / max(SQR(m_XLength), T(1e-20));
		m_YLengthV = 1 / max(SQR(m_YLength), T(1e-20));
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_XAmp    = 5 * (rand.Frand01<T>() - T(0.5));
		m_YAmp    = 4 * (rand.Frand01<T>() - T(0.5));
		m_XLength = 2 * (rand.Frand01<T>() + T(0.5));
		m_YLength = 2 * (rand.Frand01<T>() + T(0.5));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XAmp,    prefix + "curve_xamp"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_YAmp,    prefix + "curve_yamp"));
		m_Params.push_back(ParamWithName<T>(&m_XLength, prefix + "curve_xlength", 1));
		m_Params.push_back(ParamWithName<T>(&m_YLength, prefix + "curve_ylength", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_XAmpV,    prefix + "curve_xampv"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YAmpV,    prefix + "curve_yampv"));
		m_Params.push_back(ParamWithName<T>(true, &m_XLengthV, prefix + "curve_xlenv"));
		m_Params.push_back(ParamWithName<T>(true, &m_YLengthV, prefix + "curve_ylenv"));
	}

private:
	T m_XAmp;//Params.
	T m_YAmp;
	T m_XLength;
	T m_YLength;
	T m_XAmpV;//Precalc.
	T m_YAmpV;
	T m_XLengthV;
	T m_YLengthV;
};

/// <summary>
/// Edisc.
/// </summary>
template <typename T>
class EMBER_API EdiscVariation : public Variation<T>
{
public:
	EdiscVariation(T weight = 1.0) : Variation<T>("edisc", VAR_EDISC, weight, true) { }

	VARCOPY(EdiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T r1 = sqrt(tmp + tmp2);
		T r2 = sqrt(tmp - tmp2);
		T xmax = (r1 + r2) * T(0.5);
		T a1 = log(xmax + sqrt(xmax - 1));
		T a2 = -acos(Clamp<T>(helper.In.x / xmax, -1, 1));
		T w = m_Weight / T(11.57034632);//This is an interesting magic number.
		T snv, csv, snhu, cshu;

		sincos(a1, &snv, &csv);

		snhu = sinh(a2);
		cshu = cosh(a2);

		if (helper.In.y > 0.0)
			snv = -snv;

		helper.Out.x = w * cshu * csv;
		helper.Out.y = w * snhu * snv;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + T(1.0);\n"
		   << "\t\treal_t tmp2 = T(2.0) * vIn.x;\n"
		   << "\t\treal_t r1 = sqrt(tmp + tmp2);\n"
		   << "\t\treal_t r2 = sqrt(tmp - tmp2);\n"
		   << "\t\treal_t xmax = (r1 + r2) * T(0.5);\n"
		   << "\t\treal_t a1 = log(xmax + sqrt(xmax - T(1.0)));\n"
		   << "\t\treal_t a2 = -acos(Clamp(vIn.x / xmax, -T(1.0), T(1.0)));\n"
		   << "\t\treal_t w = xform->m_VariationWeights[" << varIndex << "] / T(11.57034632);\n"
		   << "\t\treal_t snv = sin(a1);\n"
		   << "\t\treal_t csv = cos(a1);\n"
		   << "\t\treal_t snhu = sinh(a2);\n"
		   << "\t\treal_t cshu = cosh(a2);\n"

		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t	snv = -snv;\n"

		   << "\t\tvOut.x = w * cshu * csv;\n"
		   << "\t\tvOut.y = w * snhu * snv;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Elliptic.
/// </summary>
template <typename T>
class EMBER_API EllipticVariation : public ParametricVariation<T>
{
public:
	EllipticVariation(T weight = 1.0) : ParametricVariation<T>("elliptic", VAR_ELLIPTIC, weight, true)
	{
		Init();
	}

	PARVARCOPY(EllipticVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T x2 = 2 * helper.In.x;
		T xmax = T(0.5) * (sqrt(tmp + x2) + sqrt(tmp - x2));
		T a = helper.In.x / xmax;
		T b = 1 - a * a;
		T ssx = xmax - 1;
		const T w = m_WeightDivPiDiv2;

		if (b < 0)
			b = 0;
		else
			b = sqrt(b);

		if (ssx < 0)
			ssx = 0;
		else
			ssx = sqrt(ssx);

		helper.Out.x = w * atan2(a, b);

		if (helper.In.y > 0)
			helper.Out.y = w * log(xmax + ssx);
		else
			helper.Out.y = -(w * log(xmax + ssx));

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weightDivPiDiv2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + T(1.0);\n"
		   << "\t\treal_t x2 = T(2.0) * vIn.x;\n"
		   << "\t\treal_t xmax = T(0.5) * (sqrt(tmp + x2) + sqrt(tmp - x2));\n"
		   << "\t\treal_t a = vIn.x / xmax;\n"
		   << "\t\treal_t b = T(1.0) - a * a;\n"
		   << "\t\treal_t ssx = xmax - T(1.0);\n"
		   << "\t\tconst real_t w = " << weightDivPiDiv2 << ";\n"
		   << "\n"
		   << "\t\tif (b < 0)\n"
		   << "\t\t	b = 0;\n"
		   << "\t\telse\n"
		   << "\t\t	b = sqrt(b);\n"
		   << "\n"
		   << "\t\tif (ssx < 0)\n"
		   << "\t\t	ssx = 0;\n"
		   << "\t\telse\n"
		   << "\t\t	ssx = sqrt(ssx);\n"
		   << "\n"
		   << "\t\tvOut.x = w * atan2(a, b);\n"
		   << "\n"
		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t	vOut.y = w * log(xmax + ssx);\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = -(w * log(xmax + ssx));\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightDivPiDiv2 = m_Weight / T(M_PI_2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightDivPiDiv2, prefix + "elliptic_weight_div_pi_div_2"));//Precalc.
	}

private:
	T m_WeightDivPiDiv2;//Precalc.
};

/// <summary>
/// Escher.
/// </summary>
template <typename T>
class EMBER_API EscherVariation : public ParametricVariation<T>
{
public:
	EscherVariation(T weight = 1.0) : ParametricVariation<T>("escher", VAR_ESCHER, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(EscherVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T lnr = T(0.5) * log(helper.m_PrecalcSumSquares);
		T m = m_Weight * exp(m_C * lnr - m_D * a);
		T n = m_C * a + m_D * lnr;

		helper.Out.x = m * cos(n);
		helper.Out.y = m * sin(n);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string beta = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t lnr = T(0.5) * log(precalcSumSquares);\n"
		   << "\t\treal_t m = xform->m_VariationWeights[" << varIndex << "] * exp(" << c << " * lnr - " << d << " * a);\n"
		   << "\t\treal_t n = " << c << " * a + " << d << " * lnr;\n"
		   << "\n"
		   << "\t\tvOut.x = m * cos(n);\n"
		   << "\t\tvOut.y = m * sin(n);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Beta, &m_D, &m_C);
		m_C = T(0.5) * (1 + m_C);
		m_D = T(0.5) * m_D;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		SetParamVal("escher_beta", T(M_PI) * rand.Frand01<T>());
	}

	virtual bool SetParamVal(const char* name, T val) override
	{
		if (!_stricmp(name, "escher_beta"))
		{
			m_Beta = Fabsmod((val + T(M_PI)) / (2 * T(M_PI))) * 2 * T(M_PI) - T(M_PI);
			Precalc();
			return true;
		}

		return ParametricVariation<T>::SetParamVal(name, val);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Beta, prefix + "escher_beta"));//Params.
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "escher_beta_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_D, prefix + "escher_beta_d"));
	}

private:
	T m_Beta;//Params.
	T m_C;//Precalc.
	T m_D;
};

/// <summary>
/// Foci.
/// </summary>
template <typename T>
class EMBER_API FociVariation : public Variation<T>
{
public:
	FociVariation(T weight = 1.0) : Variation<T>("foci", VAR_FOCI, weight) { }

	VARCOPY(FociVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expx = exp(helper.In.x) * T(0.5);
		T expnx = T(0.25) / expx;
		T sn, cn, tmp;

		sincos(helper.In.y, &sn, &cn);

		tmp = m_Weight / Zeps(expx + expnx - cn);

		helper.Out.x = tmp * (expx - expnx);
		helper.Out.y = tmp * sn;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t expx = exp(vIn.x) * T(0.5);\n"
		   << "\t\treal_t expnx = T(0.25) / expx;\n"
		   << "\t\treal_t sn = sin(vIn.y);\n"
		   << "\t\treal_t cn = cos(vIn.y);\n"
		   << "\t\treal_t tmp = Zeps(expx + expnx - cn);\n"
		   << "\n"
		   << "\t\ttmp = xform->m_VariationWeights[" << varIndex << "] / tmp;\n"
		   << "\n"
		   << "\t\tvOut.x = tmp * (expx - expnx);\n"
		   << "\t\tvOut.y = tmp * sn;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// LazySusan.
/// </summary>
template <typename T>
class EMBER_API LazySusanVariation : public ParametricVariation<T>
{
public:
	LazySusanVariation(T weight = 1.0) : ParametricVariation<T>("lazysusan", VAR_LAZYSUSAN, weight)
	{
		Init();
	}

	PARVARCOPY(LazySusanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x - m_X;
		T y = helper.In.y + m_Y;
		T r = sqrt(x * x + y * y);

		if (r < m_Weight)
		{
			T a = atan2(y, x) + m_Spin + m_Twist * (m_Weight - r);

			helper.Out.x = m_Weight * (r * cos(a) + m_X);//Fix to make it colapse to 0 when weight is 0.//SMOULDER
			helper.Out.y = m_Weight * (r * sin(a) - m_Y);
		}
		else
		{
			r = 1 + m_Space / Zeps(r);

			helper.Out.x = m_Weight * (r * x + m_X);//Fix to make it colapse to 0 when weight is 0.//SMOULDER
			helper.Out.y = m_Weight * (r * y - m_Y);
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string spin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string space = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x - " << x << ";\n"
		   << "\t\treal_t y = vIn.y + " << y << ";\n"
		   << "\t\treal_t r = sqrt(x * x + y * y);\n"
		   << "\n"
		   << "\t\tif (r < xform->m_VariationWeights[" << varIndex << "])\n"
		   << "\t\t{\n"
		   << "\t\t	real_t a = atan2(y, x) + " << spin << " + " << twist << " * (xform->m_VariationWeights[" << varIndex << "] - r);\n"
		   << "\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (r * cos(a) + " << x << ");\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (r * sin(a) - " << y << ");\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	r = T(1.0) + " << space << " / Zeps(r);\n"
		   << "\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (r * x + " << x << ");\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (r * y - " << y << ");\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual bool SetParamVal(const char* name, T val) override
	{
		if (!_stricmp(name, "lazysusan_spin"))
		{
			m_Spin = Fabsmod(val / T(M_2PI)) * T(M_2PI);
			this->Precalc();
			return true;
		}

		return ParametricVariation<T>::SetParamVal(name, val);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X     = 2       * rand.Frand11<T>();
		m_Y     = 2       * rand.Frand11<T>();
		m_Spin  = T(M_PI) * rand.Frand11<T>();
		m_Space = 2       * rand.Frand11<T>();
		m_Twist = 2       * rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spin,  prefix + "lazysusan_spin", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Space, prefix + "lazysusan_space"));
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "lazysusan_twist"));
		m_Params.push_back(ParamWithName<T>(&m_X,     prefix + "lazysusan_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y,     prefix + "lazysusan_y"));
	}

private:
	T m_Spin;
	T m_Space;
	T m_Twist;
	T m_X;
	T m_Y;
};

/// <summary>
/// Loonie.
/// </summary>
template <typename T>
class EMBER_API LoonieVariation : public ParametricVariation<T>
{
public:
	LoonieVariation(T weight = 1.0) : ParametricVariation<T>("loonie", VAR_LOONIE, weight, true)
	{
		Init();
	}

	PARVARCOPY(LoonieVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.m_PrecalcSumSquares < m_W2 && helper.m_PrecalcSumSquares != 0)
		{
			T r = m_Weight * sqrt((m_W2 / helper.m_PrecalcSumSquares) - 1);

			helper.Out.x = r * helper.In.x;
			helper.Out.y = r * helper.In.y;
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string w2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (precalcSumSquares < " << w2 << " && precalcSumSquares != 0)\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = xform->m_VariationWeights[" << varIndex << "] * sqrt((" << w2 << " / precalcSumSquares) - T(1.0));\n"
		   << "\t\t	vOut.x = r * vIn.x;\n"
		   << "\t\t	vOut.y = r * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_W2 = SQR(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_W2, prefix + "loonie_w2"));//Precalc.
	}

private:
	T m_W2;//Precalc.
};

/// <summary>
/// Modulus.
/// </summary>
template <typename T>
class EMBER_API ModulusVariation : public ParametricVariation<T>
{
public:
	ModulusVariation(T weight = 1.0) : ParametricVariation<T>("modulus", VAR_MODULUS, weight)
	{
		Init();
	}

	PARVARCOPY(ModulusVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x > m_X)
			helper.Out.x = m_Weight * (-m_X + fmod(helper.In.x + m_X, m_XRange));
		else if (helper.In.x < -m_X)
			helper.Out.x = m_Weight * ( m_X - fmod(m_X - helper.In.x, m_XRange));
		else
			helper.Out.x = m_Weight * helper.In.x;

		if (helper.In.y > m_Y)
			helper.Out.y = m_Weight * (-m_Y + fmod(helper.In.y + m_Y, m_YRange));
		else if (helper.In.y < -m_Y)
			helper.Out.y = m_Weight * ( m_Y - fmod(m_Y - helper.In.y, m_YRange));
		else
			helper.Out.y = m_Weight * helper.In.y;

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (vIn.x > " << x << ")\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (-" << x << " + fmod(vIn.x + " << x << ", " << xr << "));\n"
		   << "\t\telse if (vIn.x < -" << x << ")\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * ( " << x << " - fmod(" << x << " - vIn.x, " << xr << "));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\n"
		   << "\t\tif (vIn.y > " << y << ")\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (-" << y << " + fmod(vIn.y + " << y << ", " << yr << "));\n"
		   << "\t\telse if (vIn.y < -" << y << ")\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * ( " << y << " - fmod(" << y << " - vIn.y, " << yr << "));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XRange = 2 * m_X;
		m_YRange = 2 * m_Y;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "modulus_x", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "modulus_y", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_XRange, prefix + "modulus_xrange"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YRange, prefix + "modulus_yrange"));
	}

private:
	T m_X;//Params.
	T m_Y;
	T m_XRange;//Precalc.
	T m_YRange;
};

/// <summary>
/// Oscilloscope.
/// </summary>
template <typename T>
class EMBER_API OscilloscopeVariation : public ParametricVariation<T>
{
public:
	OscilloscopeVariation(T weight = 1.0) : ParametricVariation<T>("oscilloscope", VAR_OSCILLOSCOPE, weight)
	{
		Init();
	}

	PARVARCOPY(OscilloscopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t;

		if (m_Damping == 0.0)
			t = m_Amplitude * cos(m_2PiFreq * helper.In.x) + m_Separation;
		else
			t = m_Amplitude * exp(-fabs(helper.In.x) * m_Damping) * cos(m_2PiFreq * helper.In.x) + m_Separation;

		if (fabs(helper.In.y) <= t)
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = -(m_Weight * helper.In.y);
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string separation = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string frequency  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amplitude  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string damping    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tpf        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t;\n"
		   << "\n"
		   << "\t\tif (" << damping << " == T(0.0))\n"
		   << "\t\t	t = " << amplitude << " * cos(" << tpf << " * vIn.x) + " << separation << ";\n"
		   << "\t\telse\n"
		   << "\t\t	t = " << amplitude << " * exp(-fabs(vIn.x) * " << damping << ") * cos(" << tpf << " * vIn.x) + " << separation << ";\n"
		   << "\n"
		   << "\t\tif (fabs(vIn.y) <= t)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * vIn.y);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_2PiFreq = m_Frequency * T(M_2PI);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Separation = 1 + rand.Frand11<T>();
		m_Frequency  = T(M_PI) * rand.Frand11<T>();
		m_Amplitude  = 1 + 2 * rand.Frand01<T>();
		m_Damping    = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Separation, prefix + "oscilloscope_separation", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Frequency,  prefix + "oscilloscope_frequency", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Amplitude,  prefix + "oscilloscope_amplitude", 1));
		m_Params.push_back(ParamWithName<T>(&m_Damping,    prefix + "oscilloscope_damping"));
		m_Params.push_back(ParamWithName<T>(true, &m_2PiFreq, prefix + "oscilloscope_2pifreq"));//Precalc.
	}

private:
	T m_Separation;//Params.
	T m_Frequency;
	T m_Amplitude;
	T m_Damping;
	T m_2PiFreq;//Precalc.
};

/// <summary>
/// Polar2.
/// </summary>
template <typename T>
class EMBER_API Polar2Variation : public ParametricVariation<T>
{
public:
	Polar2Variation(T weight = 1.0) : ParametricVariation<T>("polar2", VAR_POLAR2, weight, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Polar2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Vvar * helper.m_PrecalcAtanxy;
		helper.Out.y = m_Vvar2 * log(helper.m_PrecalcSumSquares);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string vvar  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = " << vvar << " * precalcAtanxy;\n"
		   << "\t\tvOut.y = " << vvar2 << " * log(precalcSumSquares);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar = m_Weight / T(M_PI);
		m_Vvar2 = m_Vvar * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar, prefix + "polar2_vvar"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "polar2_vvar2"));
	}

private:
	T m_Vvar;
	T m_Vvar2;
};

/// <summary>
/// Popcorn2.
/// </summary>
template <typename T>
class EMBER_API Popcorn2Variation : public ParametricVariation<T>
{
public:
	Popcorn2Variation(T weight = 1.0) : ParametricVariation<T>("popcorn2", VAR_POPCORN2, weight)
	{
		Init();
	}

	PARVARCOPY(Popcorn2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_X * sin(SafeTan<T>(helper.In.y * m_C)));
		helper.Out.y = m_Weight * (helper.In.y + m_Y * sin(SafeTan<T>(helper.In.x * m_C)));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << x << " * sin(tan(vIn.y * " << c << ")));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << y << " * sin(tan(vIn.x * " << c << ")));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = T(0.2) + rand.Frand01<T>();
		m_Y = T(0.2) * rand.Frand01<T>();
		m_C = 5      * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "popcorn2_x", T(T(0.1))));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "popcorn2_y", T(T(0.1))));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "popcorn2_c", 3));
	}

private:
	T m_X;
	T m_Y;
	T m_C;
};

/// <summary>
/// Scry.
/// Note that scry does not multiply by weight, but as the
/// values still approach 0 as the weight approaches 0, it
/// should be ok.
/// </summary>
template <typename T>
class EMBER_API ScryVariation : public ParametricVariation<T>
{
public:
	ScryVariation(T weight = 1.0) : ParametricVariation<T>("scry", VAR_SCRY, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ScryVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = helper.m_PrecalcSumSquares;
		T r = 1 / Zeps(helper.m_PrecalcSqrtSumSquares * (t + m_InvWeight));

		helper.Out.x = helper.In.x * r;
		helper.Out.y = helper.In.y * r;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string invWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t = precalcSumSquares;\n"
		   << "\t\treal_t r = T(1.0) / Zeps(precalcSqrtSumSquares * (t + " << invWeight << "));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r;\n"
		   << "\t\tvOut.y = vIn.y * r;\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_InvWeight = 1 / Zeps(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_InvWeight, prefix + "scry_inv_weight"));//Precalcs only, no params.
	}

private:
	T m_InvWeight;//Precalcs only, no params.
};

/// <summary>
/// Separation.
/// </summary>
template <typename T>
class EMBER_API SeparationVariation : public ParametricVariation<T>
{
public:
	SeparationVariation(T weight = 1.0) : ParametricVariation<T>("separation", VAR_SEPARATION, weight)
	{
		Init();
	}

	PARVARCOPY(SeparationVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x > 0.0)
			helper.Out.x = m_Weight * (sqrt(SQR(helper.In.x) + m_XX) - helper.In.x * m_XInside);
		else
			helper.Out.x = -(m_Weight * (sqrt(SQR(helper.In.x) + m_XX) + helper.In.x * m_XInside));

		if (helper.In.y > 0.0)
			helper.Out.y = m_Weight * (sqrt(SQR(helper.In.y) + m_YY) - helper.In.y * m_YInside);
		else
			helper.Out.y = -(m_Weight * (sqrt(SQR(helper.In.y) + m_YY) + helper.In.y * m_YInside));

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xInside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yInside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (vIn.x > T(0.0))\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (sqrt(vIn.x * vIn.x + " << xx << ") - vIn.x * " << xInside << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * (sqrt(vIn.x * vIn.x + " << xx << ") + vIn.x * " << xInside << "));\n"
		   << "\n"
		   << "\t\tif (vIn.y > T(0.0))\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (sqrt(vIn.y * vIn.y + " << yy << ") - vIn.y * " << yInside << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * (sqrt(vIn.y * vIn.y + " << yy << ") + vIn.y * " << yInside << "));\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XX = SQR(m_X);
		m_YY = SQR(m_Y);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X       = 1 + rand.Frand11<T>();
		m_XInside = 1 + rand.Frand11<T>();
		m_Y       = rand.Frand11<T>();
		m_YInside = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,       prefix + "separation_x", 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_XInside, prefix + "separation_xinside"));
		m_Params.push_back(ParamWithName<T>(&m_Y,       prefix + "separation_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_YInside, prefix + "separation_yinside"));
		m_Params.push_back(ParamWithName<T>(true, &m_XX, prefix + "separation_xx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YY, prefix + "separation_yy"));
	}

private:
	T m_X;//Params.
	T m_XInside;
	T m_Y;
	T m_YInside;
	T m_XX;//Precalc.
	T m_YY;
};

/// <summary>
/// Split.
/// </summary>
template <typename T>
class EMBER_API SplitVariation : public ParametricVariation<T>
{
public:
	SplitVariation(T weight = 1.0) : ParametricVariation<T>("split", VAR_SPLIT, weight)
	{
		Init();
	}

	PARVARCOPY(SplitVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (cos(helper.In.y * m_YAng) >= 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = -(m_Weight * helper.In.x);

		if (cos(helper.In.x * m_XAng) >= 0)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = -(m_Weight * helper.In.y);

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xSize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ySize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xAng  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yAng  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (cos(vIn.y * " << yAng << ") >= 0)\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * vIn.x);\n"
		   << "\n"
		   << "\t\tif (cos(vIn.x * " << xAng << ") >= 0)\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XAng = T(M_PI) * m_XSize;
		m_YAng = T(M_PI) * m_YSize;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_XSize = rand.Frand11<T>();
		m_YSize = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XSize, prefix + "split_xsize", T(T(0.5))));//Params.
		m_Params.push_back(ParamWithName<T>(&m_YSize, prefix + "split_ysize", T(T(0.5))));
		m_Params.push_back(ParamWithName<T>(true, &m_XAng, prefix + "split_xang"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YAng, prefix + "split_yang"));
	}

private:
	T m_XSize;//Params.
	T m_YSize;
	T m_XAng;//Precalc.
	T m_YAng;
};

/// <summary>
/// Splits.
/// </summary>
template <typename T>
class EMBER_API SplitsVariation : public ParametricVariation<T>
{
public:
	SplitsVariation(T weight = 1.0) : ParametricVariation<T>("splits", VAR_SPLITS, weight)
	{
		Init();
	}

	PARVARCOPY(SplitsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x >= 0)
			helper.Out.x = m_Weight * (helper.In.x + m_X);
		else
			helper.Out.x = m_Weight * (helper.In.x - m_X);

		if (helper.In.y >= 0)
			helper.Out.y = m_Weight * (helper.In.y + m_Y);
		else
			helper.Out.y = m_Weight * (helper.In.y - m_Y);

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (vIn.x >= 0)\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << x << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x - " << x << ");\n"
		   << "\n"
		   << "\t\tif (vIn.y >= 0)\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << y << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - " << y << ");\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "splits_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "splits_y"));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// Stripes.
/// </summary>
template <typename T>
class EMBER_API StripesVariation : public ParametricVariation<T>
{
public:
	StripesVariation(T weight = 1.0) : ParametricVariation<T>("stripes", VAR_STRIPES, weight)
	{
		Init();
	}

	PARVARCOPY(StripesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundx = T(int(helper.In.x >= 0 ? (helper.In.x + T(0.5)) : (helper.In.x - T(0.5))));
		T offsetx = helper.In.x - roundx;

		helper.Out.x = m_Weight * (offsetx * (1 - m_Space) + roundx);
		helper.Out.y = m_Weight * (helper.In.y + offsetx * offsetx * m_Warp);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string space = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string warp  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t roundx = (real_t)(int)(vIn.x >= 0 ? (vIn.x + T(0.5)) : (vIn.x - T(0.5)));\n"
		   << "\t\treal_t offsetx = vIn.x - roundx;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetx * (T(1.0) - " << space << ") + roundx);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + offsetx * offsetx * " << warp << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Params[0].Set(rand.Frand01<T>());//Space.
		m_Params[1].Set(5 * rand.Frand01<T>());//Warp.
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Space, prefix + "stripes_space", T(T(0.5)), REAL, T(T(0.5)), 5));
		m_Params.push_back(ParamWithName<T>(&m_Warp,  prefix + "stripes_warp"));
	}

private:
	T m_Space;
	T m_Warp;
};

/// <summary>
/// Wedge.
/// </summary>
template <typename T>
class EMBER_API WedgeVariation : public ParametricVariation<T>
{
public:
	WedgeVariation(T weight = 1.0) : ParametricVariation<T>("wedge", VAR_WEDGE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;
		T a = helper.m_PrecalcAtanyx + m_Swirl * r;
		T c = T(Floor<T>((m_Count * a + T(M_PI)) * T(M_1_PI) * T(0.5)));

		a = a * m_CompFac + c * m_Angle;
		r = m_Weight * (r + m_Hole);
		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hole    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string count   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string swirl   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string compFac = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t a = precalcAtanyx + " << swirl << " * r;\n"
		   << "\t\treal_t c = floor((" << count << " * a + M_PI) * M_1_PI * T(0.5));\n"
		   << "\n"
		   << "\t\ta = a * " << compFac << " + c * " << angle << ";\n"
		   << "\t\tr = xform->m_VariationWeights[" << varIndex << "] * (r + " << hole << ");\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_CompFac = 1 - m_Angle * m_Count * T(M_1_PI) * T(0.5);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = T(M_PI) * rand.Frand01<T>();
		m_Hole  = T(0.5)  * rand.Frand11<T>();
		m_Count = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Swirl = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_angle", T(M_PI_2)));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Hole,  prefix + "wedge_hole"));
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_count", 2, INTEGER, 1));
		m_Params.push_back(ParamWithName<T>(&m_Swirl, prefix + "wedge_swirl"));
		m_Params.push_back(ParamWithName<T>(true, &m_CompFac, prefix + "wedge_compfac"));//Precalc.
	}

private:
	T m_Angle;//Params.
	T m_Hole;
	T m_Count;
	T m_Swirl;
	T m_CompFac;//Precalc.
};

/// <summary>
/// Wedge julia.
/// </summary>
template <typename T>
class EMBER_API WedgeJuliaVariation : public ParametricVariation<T>
{
public:
	WedgeJuliaVariation(T weight = 1.0) : ParametricVariation<T>("wedge_julia", VAR_WEDGE_JULIA, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeJuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn);
		int tRand = int(m_Rn * rand.Frand01<T>());
		T a = (helper.m_PrecalcAtanyx + M_2PI * tRand) / m_Power;
		T c = T(Floor<T>((m_Count * a + T(M_PI)) * T(M_1_PI) * T(0.5)));

		a = a * m_Cf + c * m_Angle;
		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string count = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cf    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\tint tRand = (int)(" << rn << " * MwcNext01(mwc));\n"
		   << "\t\treal_t a = (precalcAtanyx + M_2PI * tRand) / " << power << ";\n"
		   << "\t\treal_t c = floor((" << count << " * a + M_PI) * M_1_PI * T(0.5));\n"
		   << "\n"
		   << "\t\ta = a * " << cf << " + c * " << angle << ";\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Cf = 1 - m_Angle * m_Count * T(M_1_PI) * T(0.5);
		m_Rn = fabs(m_Power);
		m_Cn = m_Dist / m_Power / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
		m_Dist  = 1;
		m_Count = T(int(3 * rand.Frand01<T>() + 1));
		m_Angle = T(M_PI) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_julia_angle"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_julia_count", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "wedge_julia_power", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "wedge_julia_dist"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "wedge_julia_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "wedge_julia_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cf, prefix + "wedge_julia_cf"));
	}

private:
	T m_Angle;//Params.
	T m_Count;
	T m_Power;
	T m_Dist;
	T m_Rn;//Precalc.
	T m_Cn;
	T m_Cf;
};

/// <summary>
/// Wedge sph.
/// </summary>
template <typename T>
class EMBER_API WedgeSphVariation : public ParametricVariation<T>
{
public:
	WedgeSphVariation(T weight = 1.0) : ParametricVariation<T>("wedge_sph", VAR_WEDGE_SPH, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeSphVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 1 / Zeps(helper.m_PrecalcSqrtSumSquares);
		T a = helper.m_PrecalcAtanyx + m_Swirl * r;
		T c = T(Floor<T>((m_Count * a + T(M_PI)) * T(M_1_PI) * T(0.5)));
		T compFac = 1 - m_Angle * m_Count * T(M_1_PI) * T(0.5);

		a = a * compFac + c * m_Angle;
		r = m_Weight * (r + m_Hole);
		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string count = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hole  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string swirl = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = T(1.0) / Zeps(precalcSqrtSumSquares);\n"
		   << "\t\treal_t a = precalcAtanyx + " << swirl << " * r;\n"
		   << "\t\treal_t c = floor((" << count << " * a + M_PI) * M_1_PI * T(0.5));\n"
		   << "\t\treal_t compFac = 1 - " << angle << " * " << count << " * M_1_PI * T(0.5);\n"
		   << "\n"
		   << "\t\ta = a * compFac + c * " << angle << ";\n"
		   << "\t\tr = xform->m_VariationWeights[" << varIndex << "] * (r + " << hole << ");\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = T(M_PI) * rand.Frand01<T>();
		m_Count = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Hole  = T(0.5)  * rand.Frand11<T>();
		m_Swirl = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_sph_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_sph_hole", 1));
		m_Params.push_back(ParamWithName<T>(&m_Hole,  prefix + "wedge_sph_count"));
		m_Params.push_back(ParamWithName<T>(&m_Swirl, prefix + "wedge_sph_swirl"));
	}

private:
	T m_Angle;
	T m_Count;
	T m_Hole;
	T m_Swirl;
};

/// <summary>
/// Whorl.
/// </summary>
template <typename T>
class EMBER_API WhorlVariation : public ParametricVariation<T>
{
public:
	WhorlVariation(T weight = 1.0) : ParametricVariation<T>("whorl", VAR_WHORL, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WhorlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a, r = helper.m_PrecalcSqrtSumSquares;

		if (r < m_Weight)
			a = helper.m_PrecalcAtanyx + m_Inside / (m_Weight - r);
		else
			a = helper.m_PrecalcAtanyx + m_Outside / (m_Weight - r);

		helper.Out.x = m_Weight * r * cos(a);
		helper.Out.y = m_Weight * r * sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string inside  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string outside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a;\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (r < xform->m_VariationWeights[" << varIndex << "])\n"
		   << "\t\t	a = precalcAtanyx + " << inside << " / (xform->m_VariationWeights[" << varIndex << "] - r);\n"
		   << "\t\telse\n"
		   << "\t\t	a = precalcAtanyx + " << outside << " / (xform->m_VariationWeights[" << varIndex << "] - r);\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] * r * cos(a));\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] * r * sin(a));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Inside = rand.Frand01<T>();
		m_Outside = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Inside,  prefix + "whorl_inside", 1));
		m_Params.push_back(ParamWithName<T>(&m_Outside, prefix + "whorl_outside", 1));
	}

private:
	T m_Inside;
	T m_Outside;
};

/// <summary>
/// Waves.
/// </summary>
template <typename T>
class EMBER_API Waves2Variation : public ParametricVariation<T>
{
public:
	Waves2Variation(T weight = 1.0) : ParametricVariation<T>("waves2", VAR_WAVES2, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(Waves2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_ScaleX * sin(helper.In.y * m_FreqX));
		helper.Out.y = m_Weight * (helper.In.y + m_ScaleY * sin(helper.In.x * m_FreqY));
		helper.Out.z = m_Weight * (helper.In.z + m_ScaleZ * sin(helper.m_PrecalcSqrtSumSquares * m_FreqZ));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string freqX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqZ  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleZ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << scaleX << " * sin(vIn.y * " << freqX << "));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << scaleY << " * sin(vIn.x * " << freqY << "));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z + " << scaleZ << " * sin(precalcSqrtSumSquares * " << freqZ << "));\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_FreqX  = 4      * rand.Frand01<T>();
		m_ScaleX = T(0.5) + rand.Frand01<T>();
		m_FreqY  = 4      * rand.Frand01<T>();
		m_ScaleY = T(0.5) + rand.Frand01<T>();
		m_FreqZ = 0;
		m_ScaleZ = 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_FreqX,  prefix + "waves2_freqx", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleX, prefix + "waves2_scalex"));
		m_Params.push_back(ParamWithName<T>(&m_FreqY,  prefix + "waves2_freqy", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleY, prefix + "waves2_scaley"));
		m_Params.push_back(ParamWithName<T>(&m_FreqZ,  prefix + "waves2_freqz"));
		m_Params.push_back(ParamWithName<T>(&m_ScaleZ, prefix + "waves2_scalez"));
	}

private:
	T m_FreqX;
	T m_ScaleX;
	T m_FreqY;
	T m_ScaleY;
	T m_FreqZ;
	T m_ScaleZ;
};

/// <summary>
/// Exp.
/// </summary>
template <typename T>
class EMBER_API ExpVariation : public Variation<T>
{
public:
	ExpVariation(T weight = 1.0) : Variation<T>("exp", VAR_EXP, weight) { }

	VARCOPY(ExpVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expe = m_Weight * exp(helper.In.x);

		helper.Out.x = expe * cos(helper.In.y);
		helper.Out.y = expe * sin(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t expe = xform->m_VariationWeights[" << varIndex << "] * exp(vIn.x);\n"
		   << "\n"
		   << "\t\tvOut.x = expe * cos(vIn.y);\n"
		   << "\t\tvOut.y = expe * sin(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Log.
/// </summary>
template <typename T>
class EMBER_API LogVariation : public ParametricVariation<T>
{
public:
	LogVariation(T weight = 1.0) : ParametricVariation<T>("log", VAR_LOG, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(LogVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * log(helper.m_PrecalcSumSquares) * m_Denom;
		helper.Out.y = m_Weight * helper.m_PrecalcAtanyx;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string base  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string denom = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * log(precalcSumSquares) * " << denom << ";\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * precalcAtanyx;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Denom = T(0.5) / log(m_Base);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Base, prefix + "log_base", T(M_E), REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(true, &m_Denom, prefix + "log_denom"));//Precalc.
	}

private:
	T m_Base;
	T m_Denom;//Precalc.
};

/// <summary>
/// Sine.
/// </summary>
template <typename T>
class EMBER_API SinVariation : public Variation<T>
{
public:
	SinVariation(T weight = 1.0) : Variation<T>("sin", VAR_SIN, weight) { }

	VARCOPY(SinVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * sin(helper.In.x) * cosh(helper.In.y);
		helper.Out.y = m_Weight * cos(helper.In.x) * sinh(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x) * cosh(vIn.y);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * cos(vIn.x) * sinh(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cosine.
/// </summary>
template <typename T>
class EMBER_API CosVariation : public Variation<T>
{
public:
	CosVariation(T weight = 1.0) : Variation<T>("cos", VAR_COS, weight) { }

	VARCOPY(CosVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//clamp fabs x and y to 7.104760e+002 for cosh, and |x| 7.104760e+002 for sinh
		helper.Out.x = m_Weight * cos(helper.In.x) * cosh(helper.In.y);
		helper.Out.y = -(m_Weight * sin(helper.In.x) * sinh(helper.In.y));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cos(vIn.x) * cosh(vIn.y);\n"
		   << "\t\tvOut.y = -(xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x) * sinh(vIn.y));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Tangent.
/// </summary>
template <typename T>
class EMBER_API TanVariation : public Variation<T>
{
public:
	TanVariation(T weight = 1.0) : Variation<T>("tan", VAR_TAN, weight) { }

	VARCOPY(TanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tansin, tancos, tansinh, tancosh, tanden;

		sincos(2 * helper.In.x, &tansin, &tancos);
		tansinh = sinh(2 * helper.In.y);
		tancosh = cosh(2 * helper.In.y);
		tanden = 1 / (tancos + tancosh);
		helper.Out.x = m_Weight * tanden * tansin;
		helper.Out.y = m_Weight * tanden * tansinh;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t tansin = sin(T(2.0) * vIn.x);\n"
		   << "\t\treal_t tancos = cos(T(2.0) * vIn.x);\n"
		   << "\t\treal_t tansinh = sinh(T(2.0) * vIn.y);\n"
		   << "\t\treal_t tancosh = cosh(T(2.0) * vIn.y);\n"
		   << "\t\treal_t tanden = T(1.0) / (tancos + tancosh);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * tanden * tansin;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * tanden * tansinh;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Sec.
/// </summary>
template <typename T>
class EMBER_API SecVariation : public Variation<T>
{
public:
	SecVariation(T weight = 1.0) : Variation<T>("sec", VAR_SEC, weight) { }

	VARCOPY(SecVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T secsin, seccos, secsinh, seccosh, secden;

		sincos(helper.In.x, &secsin, &seccos);
		secsinh = sinh(helper.In.y);
		seccosh = cosh(helper.In.y);
		secden = 2 / (cos(2 * helper.In.x) + cosh(2 * helper.In.y));
		helper.Out.x = m_Weight * secden * seccos * seccosh;
		helper.Out.y = m_Weight * secden * secsin * secsinh;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t secsin = sin(vIn.x);\n"
		   << "\t\treal_t seccos = cos(vIn.x);\n"
		   << "\t\treal_t secsinh = sinh(vIn.y);\n"
		   << "\t\treal_t seccosh = cosh(vIn.y);\n"
		   << "\t\treal_t secden = T(2.0) / (cos(T(2.0) * vIn.x) + cosh(T(2.0) * vIn.y));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * secden * seccos * seccosh;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * secden * secsin * secsinh;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cosecant.
/// </summary>
template <typename T>
class EMBER_API CscVariation : public Variation<T>
{
public:
	CscVariation(T weight = 1.0) : Variation<T>("csc", VAR_CSC, weight) { }

	VARCOPY(CscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T cscsin, csccos, cscsinh, csccosh, cscden;

		sincos(helper.In.x, &cscsin, &csccos);
		cscsinh = sinh(helper.In.y);
		csccosh = cosh(helper.In.y);
		cscden = 2 / (cosh(2 * helper.In.y) - cos(2 * helper.In.x));
		helper.Out.x = m_Weight * cscden * cscsin * csccosh;
		helper.Out.y = -(m_Weight * cscden * csccos * cscsinh);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t cscsin = sin(vIn.x);\n"
		   << "\t\treal_t csccos = cos(vIn.x);\n"
		   << "\t\treal_t cscsinh = sinh(vIn.y);\n"
		   << "\t\treal_t csccosh = cosh(vIn.y);\n"
		   << "\t\treal_t cscden = T(2.0) / (cosh(T(2.0) * vIn.y) - cos(T(2.0) * vIn.x));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cscden * cscsin * csccosh;\n"
		   << "\t\tvOut.y = -(xform->m_VariationWeights[" << varIndex << "] * cscden * csccos * cscsinh);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cotangent.
/// </summary>
template <typename T>
class EMBER_API CotVariation : public Variation<T>
{
public:
	CotVariation(T weight = 1.0) : Variation<T>("cot", VAR_COT, weight) { }

	VARCOPY(CotVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T cotsin, cotcos, cotsinh, cotcosh, cotden;

		sincos(2 * helper.In.x, &cotsin, &cotcos);
		cotsinh = sinh(2 * helper.In.y);
		cotcosh = cosh(2 * helper.In.y);
		cotden = 1 / (cotcosh - cotcos);
		helper.Out.x = m_Weight * cotden * cotsin;
		helper.Out.y = m_Weight * cotden * -1 * cotsinh;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t cotsin = sin(T(2.0) * vIn.x);\n"
		   << "\t\treal_t cotcos = cos(T(2.0) * vIn.x);\n"
		   << "\t\treal_t cotsinh = sinh(T(2.0) * vIn.y);\n"
		   << "\t\treal_t cotcosh = cosh(T(2.0) * vIn.y);\n"
		   << "\t\treal_t cotden = T(1.0) / (cotcosh - cotcos);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cotden * cotsin;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * cotden * -1 * cotsinh;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Sinh.
/// </summary>
template <typename T>
class EMBER_API SinhVariation : public Variation<T>
{
public:
	SinhVariation(T weight = 1.0) : Variation<T>("sinh", VAR_SINH, weight) { }

	VARCOPY(SinhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sinhsin, sinhcos, sinhsinh, sinhcosh;

		sincos(helper.In.y, &sinhsin, &sinhcos);
		sinhsinh = sinh(helper.In.x);
		sinhcosh = cosh(helper.In.x);
		helper.Out.x = m_Weight * sinhsinh * sinhcos;
		helper.Out.y = m_Weight * sinhcosh * sinhsin;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sinhsin = sin(vIn.y);\n"
		   << "\t\treal_t sinhcos = cos(vIn.y);\n"
		   << "\t\treal_t sinhsinh = sinh(vIn.x);\n"
		   << "\t\treal_t sinhcosh = cosh(vIn.x);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sinhsinh * sinhcos;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sinhcosh * sinhsin;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cosh.
/// </summary>
template <typename T>
class EMBER_API CoshVariation : public Variation<T>
{
public:
	CoshVariation(T weight = 1.0) : Variation<T>("cosh", VAR_COSH, weight) { }

	VARCOPY(CoshVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T coshsin,coshcos,coshsinh,coshcosh;

		sincos(helper.In.y, &coshsin, &coshcos);
		coshsinh = sinh(helper.In.x);
		coshcosh = cosh(helper.In.x);
		helper.Out.x = m_Weight * coshcosh * coshcos;
		helper.Out.y = m_Weight * coshsinh * coshsin;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t coshsin = sin(vIn.y);\n"
		   << "\t\treal_t coshcos = cos(vIn.y);\n"
		   << "\t\treal_t coshsinh = sinh(vIn.x);\n"
		   << "\t\treal_t coshcosh = cosh(vIn.x);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * coshcosh * coshcos;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * coshsinh * coshsin;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Tanh.
/// </summary>
template <typename T>
class EMBER_API TanhVariation : public Variation<T>
{
public:
	TanhVariation(T weight = 1.0) : Variation<T>("tanh", VAR_TANH, weight) { }

	VARCOPY(TanhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tanhsin, tanhcos, tanhsinh, tanhcosh, tanhden;

		sincos(2 * helper.In.y, &tanhsin, &tanhcos);
		tanhsinh = sinh(2 * helper.In.x);
		tanhcosh = cosh(2 * helper.In.x);
		tanhden = 1 / (tanhcos + tanhcosh);
		helper.Out.x = m_Weight * tanhden * tanhsinh;
		helper.Out.y = m_Weight * tanhden * tanhsin;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t tanhsin = sin(T(2.0) * vIn.y);\n"
		   << "\t\treal_t tanhcos = cos(T(2.0) * vIn.y);\n"
		   << "\t\treal_t tanhsinh = sinh(T(2.0) * vIn.x);\n"
		   << "\t\treal_t tanhcosh = cosh(T(2.0) * vIn.x);\n"
		   << "\t\treal_t tanhden = T(1.0) / (tanhcos + tanhcosh);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * tanhden * tanhsinh;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * tanhden * tanhsin;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Sech
/// </summary>
template <typename T>
class EMBER_API SechVariation : public Variation<T>
{
public:
	SechVariation(T weight = 1.0) : Variation<T>("sech", VAR_SECH, weight) { }

	VARCOPY(SechVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sechsin, sechcos, sechsinh, sechcosh, sechden;

		sincos(helper.In.y, &sechsin, &sechcos);
		sechsinh = sinh(helper.In.x);
		sechcosh = cosh(helper.In.x);
		sechden = 2 / (cos(2 * helper.In.y) + cosh(2 * helper.In.x));
		helper.Out.x = m_Weight * sechden * sechcos * sechcosh;
		helper.Out.y = -(m_Weight * sechden * sechsin * sechsinh);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sechsin = sin(vIn.y);\n"
		   << "\t\treal_t sechcos = cos(vIn.y);\n"
		   << "\t\treal_t sechsinh = sinh(vIn.x);\n"
		   << "\t\treal_t sechcosh = cosh(vIn.x);\n"
		   << "\t\treal_t sechden = T(2.0) / (cos(T(2.0) * vIn.y) + cosh(T(2.0) * vIn.x));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sechden * sechcos * sechcosh;\n"
		   << "\t\tvOut.y = -(xform->m_VariationWeights[" << varIndex << "] * sechden * sechsin * sechsinh);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Csch.
/// </summary>
template <typename T>
class EMBER_API CschVariation : public Variation<T>
{
public:
	CschVariation(T weight = 1.0) : Variation<T>("csch", VAR_CSCH, weight) { }

	VARCOPY(CschVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T cschsin, cschcos, cschsinh, cschcosh, cschden;

		sincos(helper.In.y, &cschsin, &cschcos);
		cschsinh = sinh(helper.In.x);
		cschcosh = cosh(helper.In.x);
		cschden = 2 / (cosh(2 * helper.In.x) - cos(2 * helper.In.y));
		helper.Out.x = m_Weight * cschden * cschsinh * cschcos;
		helper.Out.y = -(m_Weight * cschden * cschcosh * cschsin);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t cschsin = sin(vIn.y);\n"
		   << "\t\treal_t cschcos = cos(vIn.y);\n"
		   << "\t\treal_t cschsinh = sinh(vIn.x);\n"
		   << "\t\treal_t cschcosh = cosh(vIn.x);\n"
		   << "\t\treal_t cschden = T(2.0) / (cosh(T(2.0) * vIn.x) - cos(T(2.0) * vIn.y));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cschden * cschsinh * cschcos;\n"
		   << "\t\tvOut.y = -(xform->m_VariationWeights[" << varIndex << "] * cschden * cschcosh * cschsin);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Coth.
/// </summary>
template <typename T>
class EMBER_API CothVariation : public Variation<T>
{
public:
	CothVariation(T weight = 1.0) : Variation<T>("coth", VAR_COTH, weight) { }

	VARCOPY(CothVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T cothsin, cothcos, cothsinh, cothcosh, cothden;

		sincos(2 * helper.In.y, &cothsin, &cothcos);
		cothsinh = sinh(2 * helper.In.x);
		cothcosh = cosh(2 * helper.In.x);
		cothden = 1 / (cothcosh - cothcos);
		helper.Out.x = m_Weight * cothden * cothsinh;
		helper.Out.y = m_Weight * cothden * cothsin;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t cothsin = sin(T(2.0) * vIn.y);\n"
		   << "\t\treal_t cothcos = cos(T(2.0) * vIn.y);\n"
		   << "\t\treal_t cothsinh = sinh(T(2.0) * vIn.x);\n"
		   << "\t\treal_t cothcosh = cosh(T(2.0) * vIn.x);\n"
		   << "\t\treal_t cothden = T(1.0) / (cothcosh - cothcos);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cothden * cothsinh;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * cothden * cothsin;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Auger.
/// </summary>
template <typename T>
class EMBER_API AugerVariation : public ParametricVariation<T>
{
public:
	AugerVariation(T weight = 1.0) : ParametricVariation<T>("auger", VAR_AUGER, weight)
	{
		Init();
	}

	PARVARCOPY(AugerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s = sin(m_Freq * helper.In.x);
		T t = sin(m_Freq * helper.In.y);
		T dy = helper.In.y + m_AugerWeight * (m_Scale * s / 2 + fabs(helper.In.y) * s);
		T dx = helper.In.x + m_AugerWeight * (m_Scale * t / 2 + fabs(helper.In.x) * t);

		helper.Out.x = m_Weight * (helper.In.x + m_Symmetry * (dx - helper.In.x));
		helper.Out.y = m_Weight * dy;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string symmetry    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string augerWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t s = sin(" << freq << " * vIn.x);\n"
		   << "\t\treal_t t = sin(" << freq << " * vIn.y);\n"
		   << "\t\treal_t dy = vIn.y + " << augerWeight << " * (" << scale << " * s / T(2.0) + fabs(vIn.y) * s);\n"
		   << "\t\treal_t dx = vIn.x + " << augerWeight << " * (" << scale << " * t / T(2.0) + fabs(vIn.x) * t);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << symmetry << " * (dx - vIn.x));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * dy;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Symmetry    = 0;
		m_AugerWeight = T(0.5) + rand.Frand01<T>() / 2;
		m_Freq        = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Scale       = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Symmetry,    prefix + "auger_sym"));
		m_Params.push_back(ParamWithName<T>(&m_AugerWeight, prefix + "auger_weight", T(T(0.5))));
		m_Params.push_back(ParamWithName<T>(&m_Freq,        prefix + "auger_freq", 5));
		m_Params.push_back(ParamWithName<T>(&m_Scale,       prefix + "auger_scale", T(T(0.1))));
	}

private:
	T m_Symmetry;
	T m_AugerWeight;
	T m_Freq;
	T m_Scale;
};

/// <summary>
/// Flux.
/// </summary>
template <typename T>
class EMBER_API FluxVariation : public ParametricVariation<T>
{
public:
	FluxVariation(T weight = 1.0) : ParametricVariation<T>("flux", VAR_FLUX, weight)
	{
		Init();
	}

	PARVARCOPY(FluxVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xpw = helper.In.x + m_Weight;
		T xmw = helper.In.x - m_Weight;
		T yy = SQR(helper.In.y);
		T frac = sqrt(yy + SQR(xmw));

		if (frac == 0)
			frac = 1;

		T avgr = m_Weight * (m_Spr * sqrt(sqrt(yy + SQR(xpw)) / frac));
		T avga = (atan2(helper.In.y, xmw) - atan2(helper.In.y, xpw)) * T(0.5);

		helper.Out.x = avgr * cos(avga);
		helper.Out.y = avgr * sin(avga);
		helper.Out.z = helper.In.z;
		//helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spr    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t xpw = vIn.x + xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t\treal_t xmw = vIn.x - xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t\treal_t yy = SQR(vIn.y);\n"
		   << "\t\treal_t frac = sqrt(yy + SQR(xmw));\n"
		   << "\n"
		   << "\t\tif (frac == T(0.0))\n"
		   << "\t\t	frac = T(1.0);\n"
		   << "\n"
		   << "\t\treal_t avgr = xform->m_VariationWeights[" << varIndex << "] * (" << spr << " * sqrt(sqrt(yy + SQR(xpw)) / frac));\n"
		   << "\t\treal_t avga = (atan2(vIn.y, xmw) - atan2(vIn.y, xpw)) * T(0.5);\n"
		   << "\n"
		   << "\t\tvOut.x = avgr * cos(avga);\n"
		   << "\t\tvOut.y = avgr * sin(avga);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Spr = 2 + m_Spread;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Spread  = T(0.5) + rand.Frand01<T>() / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "flux_spread"));//Params.
		m_Params.push_back(ParamWithName<T>(true, &m_Spr, prefix + "flux_spr"));//Precalc.
	}

private:
	T m_Spread;//Params.
	T m_Spr;//Precalc.
};

MAKEPREPOSTVAR(Linear, linear, LINEAR)
MAKEPREPOSTVAR(Sinusoidal, sinusoidal, SINUSOIDAL)
MAKEPREPOSTVAR(Spherical, spherical, SPHERICAL)
MAKEPREPOSTVAR(Swirl, swirl, SWIRL)
MAKEPREPOSTVAR(Horseshoe, horseshoe, HORSESHOE)
MAKEPREPOSTVAR(Polar, polar, POLAR)
MAKEPREPOSTVAR(Handkerchief, handkerchief, HANDKERCHIEF)
MAKEPREPOSTVAR(Heart, heart, HEART)
MAKEPREPOSTPARVAR(Disc, disc, DISC)
MAKEPREPOSTVAR(Spiral, spiral, SPIRAL)
MAKEPREPOSTVAR(Hyperbolic, hyperbolic, HYPERBOLIC)
MAKEPREPOSTVAR(Diamond, diamond, DIAMOND)
MAKEPREPOSTVAR(Ex, ex, EX)
MAKEPREPOSTVAR(Julia, julia, JULIA)
MAKEPREPOSTVAR(Bent, bent, BENT)
MAKEPREPOSTPARVAR(Waves, waves, WAVES)
MAKEPREPOSTVAR(Fisheye, fisheye, FISHEYE)
MAKEPREPOSTVAR(Popcorn, popcorn, POPCORN)
MAKEPREPOSTVAR(Exponential, exponential, EXPONENTIAL)
MAKEPREPOSTVAR(Power, power, POWER)
MAKEPREPOSTVAR(Cosine, cosine, COSINE)
MAKEPREPOSTVAR(Rings, rings, RINGS)
MAKEPREPOSTVAR(Fan, fan, FAN)
MAKEPREPOSTPARVAR(Blob, blob, BLOB)
MAKEPREPOSTPARVAR(Pdj, pdj, PDJ)
MAKEPREPOSTPARVAR(Fan2, fan2, FAN2)
MAKEPREPOSTPARVAR(Rings2, rings2, RINGS2)
MAKEPREPOSTVAR(Eyefish, eyefish, EYEFISH)
MAKEPREPOSTVAR(Bubble, bubble, BUBBLE)
MAKEPREPOSTVAR(Cylinder, cylinder, CYLINDER)
MAKEPREPOSTPARVAR(Perspective, perspective, PERSPECTIVE)
MAKEPREPOSTVAR(Noise, noise, NOISE)
MAKEPREPOSTPARVAR(JuliaNGeneric, julian, JULIAN)
MAKEPREPOSTPARVAR(JuliaScope, juliascope, JULIASCOPE)
MAKEPREPOSTVARASSIGN(Blur, blur, BLUR, ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(GaussianBlur, gaussian_blur, GAUSSIAN_BLUR, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(RadialBlur, radial_blur, RADIAL_BLUR)
MAKEPREPOSTPARVARASSIGN(Pie, pie, PIE, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Ngon, ngon, NGON)
MAKEPREPOSTPARVAR(Curl, curl, CURL)
MAKEPREPOSTPARVAR(Rectangles, rectangles, RECTANGLES)
MAKEPREPOSTVARASSIGN(Arch, arch, ARCH, ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Tangent, tangent, TANGENT)
MAKEPREPOSTVARASSIGN(Square, square, SQUARE, ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Rays, rays, RAYS)
MAKEPREPOSTVAR(Blade, blade, BLADE)
MAKEPREPOSTVAR(Secant2, secant2, SECANT2)
MAKEPREPOSTVAR(TwinTrian, TwinTrian, TWINTRIAN)
MAKEPREPOSTVAR(Cross, cross, CROSS)
MAKEPREPOSTPARVAR(Disc2, disc2, DISC2)
MAKEPREPOSTPARVAR(SuperShape, super_shape, SUPER_SHAPE)
MAKEPREPOSTPARVAR(Flower, flower, FLOWER)
MAKEPREPOSTPARVAR(Conic, conic, CONIC)
MAKEPREPOSTPARVAR(Parabola, parabola, PARABOLA)
MAKEPREPOSTPARVAR(Bent2, bent2, BENT2)
MAKEPREPOSTPARVAR(Bipolar, bipolar, BIPOLAR)
MAKEPREPOSTVAR(Boarders, boarders, BOARDERS)
MAKEPREPOSTVAR(Butterfly, butterfly, BUTTERFLY)
MAKEPREPOSTPARVAR(Cell, cell, CELL)
MAKEPREPOSTPARVAR(Cpow, cpow, CPOW)
MAKEPREPOSTPARVAR(Curve, curve, CURVE)
MAKEPREPOSTVAR(Edisc, edisc, EDISC)
MAKEPREPOSTPARVAR(Elliptic, elliptic, ELLIPTIC)
MAKEPREPOSTPARVAR(Escher, escher, ESCHER)
MAKEPREPOSTVAR(Foci, foci, FOCI)
MAKEPREPOSTPARVAR(LazySusan, lazysusan, LAZYSUSAN)
MAKEPREPOSTPARVAR(Loonie, loonie, LOONIE)
MAKEPREPOSTPARVAR(Modulus, modulus, MODULUS)
MAKEPREPOSTPARVAR(Oscilloscope, oscilloscope, OSCILLOSCOPE)
MAKEPREPOSTPARVAR(Polar2, polar2, POLAR2)
MAKEPREPOSTPARVAR(Popcorn2, popcorn2, POPCORN2)
MAKEPREPOSTPARVAR(Scry, scry, SCRY)
MAKEPREPOSTPARVAR(Separation, separation, SEPARATION)
MAKEPREPOSTPARVAR(Split, split, SPLIT)
MAKEPREPOSTPARVAR(Splits, splits, SPLITS)
MAKEPREPOSTPARVAR(Stripes, stripes, STRIPES)
MAKEPREPOSTPARVAR(Wedge, wedge, WEDGE)
MAKEPREPOSTPARVAR(WedgeJulia, wedge_julia, WEDGE_JULIA)
MAKEPREPOSTPARVAR(WedgeSph, wedge_sph, WEDGE_SPH)
MAKEPREPOSTPARVAR(Whorl, whorl, WHORL)
MAKEPREPOSTPARVAR(Waves2, waves2, WAVES2)
MAKEPREPOSTVAR(Exp, exp, EXP)
MAKEPREPOSTPARVAR(Log, log, LOG)
MAKEPREPOSTVAR(Sin, sin, SIN)
MAKEPREPOSTVAR(Cos, cos, COS)
MAKEPREPOSTVAR(Tan, tan, TAN)
MAKEPREPOSTVAR(Sec, sec, SEC)
MAKEPREPOSTVAR(Csc, csc, CSC)
MAKEPREPOSTVAR(Cot, cot, COT)
MAKEPREPOSTVAR(Sinh, sinh, SINH)
MAKEPREPOSTVAR(Cosh, cosh, COSH)
MAKEPREPOSTVAR(Tanh, tanh, TANH)
MAKEPREPOSTVAR(Sech, sech, SECH)
MAKEPREPOSTVAR(Csch, csch, CSCH)
MAKEPREPOSTVAR(Coth, coth, COTH)
MAKEPREPOSTPARVAR(Auger, auger, AUGER)
MAKEPREPOSTPARVAR(Flux, flux, FLUX)
}
