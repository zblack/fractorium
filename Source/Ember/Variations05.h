#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// bubble2.
/// </summary>
template <typename T>
class EMBER_API Bubble2Variation : public ParametricVariation<T>
{
public:
	Bubble2Variation(T weight = 1.0) : ParametricVariation<T>("bubble2", VAR_BUBBLE2, weight, true)
	{
		Init();
	}

	PARVARCOPY(Bubble2Variation)
	
	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T t = T(0.25) * (helper.m_PrecalcSumSquares + SQR(helper.In.z)) + 1;
		T r = m_Weight / t;

		helper.Out.x = helper.In.x * r * m_X;
		helper.Out.y = helper.In.y * r * m_Y;

		if (helper.In.z >= 0)
			helper.Out.z = m_Weight * (helper.In.z + m_Z);
		else
			helper.Out.z = m_Weight * (helper.In.z - m_Z);

		helper.Out.z += helper.In.z * r * m_Z;//The += is intentional.
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t = 0.25 * (precalcSumSquares + SQR(vIn.z)) + 1;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / t;\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * " << x << ";\n"
		   << "\t\tvOut.y = vIn.y * r * " << y << ";\n"
		   << "\n"
		   << "\t\tif (vIn.z >= 0)\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z + " << z << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z - " << z << ");\n"
		   << "\n"
		   << "\t\tvOut.z += vIn.z * r * " << z << ";\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "bubble2_x", 1));//Original used a prefix of bubble_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "bubble2_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "bubble2_z"));
	}

private:
	T m_X;
	T m_Y;
	T m_Z;
};

/// <summary>
/// CircleLinear.
/// </summary>
template <typename T>
class EMBER_API CircleLinearVariation : public ParametricVariation<T>
{
public:
	CircleLinearVariation(T weight = 1.0) : ParametricVariation<T>("CircleLinear", VAR_CIRCLELINEAR, weight)
	{
		Init();
	}

	PARVARCOPY(CircleLinearVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int m = Floor<T>(T(0.5) * helper.In.x / m_Sc);
		int n = Floor<T>(T(0.5) * helper.In.y / m_Sc);
		T x = helper.In.x - (m * 2 + 1) * m_Sc;
		T y = helper.In.y - (n * 2 + 1) * m_Sc;
		T u = Hypot(x, y);
		T v = (T(0.3) + T(0.7) * DiscreteNoise2(m + 10, n + 3)) * m_Sc;
		T z1 = DiscreteNoise2(int(m + m_Seed), n);

		if ((z1 < m_Dens1) && (u < v))
		{
			if (m_Reverse > 0)
			{
				if (z1 < m_Dens1 * m_Dens2)
				{
					x *= m_K;
					y *= m_K;
				}
				else
				{
					T z = v / u * (1 - m_K) + m_K;

					x *= z;
					y *= z;
				}
			}
			else
			{
				if (z1 > m_Dens1 * m_Dens2)
				{
					x *= m_K;
					y *= m_K;
				}
				else
				{
					T z = v / u * (1 - m_K) + m_K;

					x *= z;
					y *= z;
				}
			}
		}

		helper.Out.x = m_Weight * (x + (m * 2 + 1) * m_Sc);
		helper.Out.y = m_Weight * (y + (n * 2 + 1) * m_Sc);
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string sc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string k       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dens1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dens2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reverse = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\tint m = (int)floor(0.5 * vIn.x / " << sc << ");\n"
		   << "\t\tint n = (int)floor(0.5 * vIn.y / " << sc << ");\n"
		   << "\t\treal_t x = vIn.x - (m * 2 + 1) * " << sc << ";\n"
		   << "\t\treal_t y = vIn.y - (n * 2 + 1) * " << sc << ";\n"
		   << "\t\treal_t u = Hypot(x, y);\n"
		   << "\t\treal_t v = (0.3 + 0.7 * CircleLinearDiscreteNoise2(m + 10, n + 3)) * " << sc << ";\n"
		   << "\t\treal_t z1 = CircleLinearDiscreteNoise2((int)(m + " << seed << "), n);\n"
		   << "\n"
		   << "\t\tif ((z1 < " << dens1 << ") && (u < v))\n"
		   << "\t\t{\n"
		   << "\t\t	if (" << reverse << " > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (z1 < " << dens1 << " * " << dens2 << ")\n"
		   << "\t\t		{\n"
		   << "\t\t			x *= " << k << ";\n"
		   << "\t\t			y *= " << k << ";\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			real_t z = v / u * (1 - " << k << ") + " << k << ";\n"
		   << "\n"
		   << "\t\t			x *= z;\n"
		   << "\t\t			y *= z;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (z1 > " << dens1 << " * " << dens2 << ")\n"
		   << "\t\t		{\n"
		   << "\t\t			x *= " << k << ";\n"
		   << "\t\t			y *= " << k << ";\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			real_t z = v / u * (1 - " << k << ") + " << k << ";\n"
		   << "\n"
		   << "\t\t			x *= z;\n"
		   << "\t\t			y *= z;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (x + (m * 2 + 1) * " << sc << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (y + (n * 2 + 1) * " << sc << ");\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"real_t CircleLinearDiscreteNoise2(int x, int y)\n"
			"{\n"
			"	const real_t im = 2147483647;\n"
			"	const real_t am = 1 / im;\n"
			"\n"
			"	int n = x + y * 57;\n"
			"	n = (n << 13) ^ n;\n"
			"	return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;\n"
			"}\n"
			"\n";
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sc,      prefix + "CircleLinear_Sc", 1));
		m_Params.push_back(ParamWithName<T>(&m_K,       prefix + "CircleLinear_K", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Dens1,   prefix + "CircleLinear_Dens1", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Dens2,   prefix + "CircleLinear_Dens2", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Reverse, prefix + "CircleLinear_Reverse", 1));
		m_Params.push_back(ParamWithName<T>(&m_X,       prefix + "CircleLinear_X", 10));
		m_Params.push_back(ParamWithName<T>(&m_Y,       prefix + "CircleLinear_Y", 10));
		m_Params.push_back(ParamWithName<T>(&m_Seed,    prefix + "CircleLinear_Seed", 0, INTEGER));
	}
	
private:
	T DiscreteNoise2(int x, int y)
	{
		const T im = T(2147483647);
		const T am = (1 / im);

		int n = x + y * 57;
		n = (n << 13) ^ n;
		return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;
	}

	T m_Sc;
	T m_K;
	T m_Dens1;
	T m_Dens2;
	T m_Reverse;
	T m_X;
	T m_Y;
	T m_Seed;
};

/// <summary>
/// CircleRand.
/// The original would loop infinitely as x and y approached zero, so put a check for a max of 10 iters.
/// </summary>
template <typename T>
class EMBER_API CircleRandVariation : public ParametricVariation<T>
{
public:
	CircleRandVariation(T weight = 1.0) : ParametricVariation<T>("CircleRand", VAR_CIRCLERAND, weight)
	{
		Init();
	}

	PARVARCOPY(CircleRandVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int m, n, iters = 0;
		T x, y, u;

		do
		{
			x = m_X * (1 - 2 * rand.Frand01<T>());
			y = m_Y * (1 - 2 * rand.Frand01<T>());
			m = Floor<T>(T(0.5) * x / m_Sc);
			n = Floor<T>(T(0.5) * y / m_Sc);
			x -= (m * 2 + 1) * m_Sc;
			y -= (n * 2 + 1) * m_Sc;
			u = Hypot(x, y);

			if (++iters > 10)
				break;
		}
		while ((DiscreteNoise2((int)(m + m_Seed), n) > m_Dens) || (u > (T(0.3) + T(0.7) * DiscreteNoise2(m + 10, n + 3)) * m_Sc));

		helper.Out.x = m_Weight * (x + (m * 2 + 1) * m_Sc);
		helper.Out.y = m_Weight * (y + (n * 2 + 1) * m_Sc);
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string sc   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dens = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\tint m, n, iters = 0;\n"
		   << "\t\treal_t x, y, u;\n"
		   << "\n"
		   << "\t\tdo\n"
		   << "\t\t{\n"
		   << "\t\t	x = " << x << " * (1 - 2 * MwcNext01(mwc));\n"
		   << "\t\t	y = " << y << " * (1 - 2 * MwcNext01(mwc));\n"
		   << "\t\t	m = (int)floor(0.5 * x / " << sc << ");\n"
		   << "\t\t	n = (int)floor(0.5 * y / " << sc << ");\n"
		   << "\t\t	x = x - (m * 2 + 1) * " << sc << ";\n"
		   << "\t\t	y = y - (n * 2 + 1) * " << sc << ";\n"
		   << "\t\t	u = Hypot(x, y);\n"
		   << "\n"
		   << "\t\t	if (++iters > 10)\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\t\twhile ((CircleRandDiscreteNoise2((int)(m + " << seed << "), n) > " << dens << ") || (u > (0.3 + 0.7 * CircleRandDiscreteNoise2(m + 10, n + 3)) * " << sc << "));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (x + (m * 2 + 1) * " << sc << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (y + (n * 2 + 1) * " << sc << ");\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"real_t CircleRandDiscreteNoise2(int x, int y)\n"
			"{\n"
			"	const real_t im = 2147483647;\n"
			"	const real_t am = 1 / im;\n"
			"\n"
			"	int n = x + y * 57;\n"
			"	n = (n << 13) ^ n;\n"
			"	return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;\n"
			"}\n"
			"\n";
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sc,   prefix + "CircleRand_Sc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dens, prefix + "CircleRand_Dens", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_X,    prefix + "CircleRand_X", 10));
		m_Params.push_back(ParamWithName<T>(&m_Y,    prefix + "CircleRand_Y", 10));
		m_Params.push_back(ParamWithName<T>(&m_Seed, prefix + "CircleRand_Seed", 0, INTEGER));
	}
	
private:
	T DiscreteNoise2(int x, int y)
	{
		const T im = T(2147483647);
		const T am = (1 / im);

		int n = x + y * 57;
		n = (n << 13) ^ n;
		return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;
	}

	T m_Sc;
	T m_Dens;
	T m_X;
	T m_Y;
	T m_Seed;
};

/// <summary>
/// CircleTrans1.
/// The original would loop infinitely as x and y approached zero, so put a check for a max of 10 iters.
/// </summary>
template <typename T>
class EMBER_API CircleTrans1Variation : public ParametricVariation<T>
{
public:
	CircleTrans1Variation(T weight = 1.0) : ParametricVariation<T>("CircleTrans1", VAR_CIRCLETRANS1, weight)
	{
		Init();
	}

	PARVARCOPY(CircleTrans1Variation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T ux, uy, u, x, y;

		Trans(m_X, m_Y, helper.In.x, helper.In.y, &ux, &uy);

		int m = Floor<T>(T(0.5) * ux / m_Sc);
		int n = Floor<T>(T(0.5) * uy / m_Sc);

		x = ux - (m * 2 + 1) * m_Sc;
		y = uy - (n * 2 + 1) * m_Sc;
		u = Hypot(x, y);

		if ((DiscreteNoise2((int)(m + m_Seed), n) > m_Dens) || (u > (T(0.3) + T(0.7) * DiscreteNoise2(m + 10, n + 3)) * m_Sc))
		{
			ux = ux;
			uy = uy;
		}
		else
		{
			CircleR(&ux, &uy, rand);
		}

		helper.Out.x = m_Weight * ux;
		helper.Out.y = m_Weight * uy;
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string sc   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dens = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t ux, uy, u, x, y;\n"
		   << "\n"
		   << "\t\tCircleTrans1Trans(" << x << ", " << y << ", vIn.x, vIn.y, &ux, &uy);\n"
		   << "\n"
		   << "\t\tint m = (int)floor(0.5 * ux / " << sc << ");\n"
		   << "\t\tint n = (int)floor(0.5 * uy / " << sc << ");\n"
		   << "\n"
		   << "\t\tx = ux - (m * 2 + 1) * " << sc << ";\n"
		   << "\t\ty = uy - (n * 2 + 1) * " << sc << ";\n"
		   << "\t\tu = Hypot(x, y);\n"
		   << "\n"
		   << "\t\tif ((CircleTrans1DiscreteNoise2((int)(m + " << seed << "), n) > " << dens << ") || (u > (0.3 + 0.7 * CircleTrans1DiscreteNoise2(m + 10, n + 3)) * " << sc << "))\n"
		   << "\t\t{\n"
		   << "\t\t	ux = ux;\n"
		   << "\t\t	uy = uy;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	CircleTrans1CircleR(" << x << ", " << y << ", " << sc << ", " << seed << ", " << dens << ", &ux, &uy, mwc);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * ux;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * uy;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"real_t CircleTrans1DiscreteNoise2(int x, int y)\n"
			"{\n"
			"	const real_t im = 2147483647;\n"
			"	const real_t am = 1 / im;\n"
			"\n"
			"	int n = x + y * 57;\n"
			"	n = (n << 13) ^ n;\n"
			"	return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;\n"
			"}\n"
			"\n"
			"void CircleTrans1Trans(real_t a, real_t b, real_t x, real_t y, real_t* x1, real_t* y1)\n"
			"{\n"
			"	*x1 = (x - a) * 0.5 + a;\n"
			"	*y1 = (y - b) * 0.5 + b;\n"
			"}\n"
			"\n"
			"void CircleTrans1CircleR(real_t mx, real_t my, real_t sc, real_t seed, real_t dens, real_t* ux, real_t* vy, uint2* mwc)\n"
			"{\n"
			"	int m, n, iters = 0;\n"
			"	real_t x, y, alpha, u;\n"
			"\n"
			"	do\n"
			"	{\n"
			"		x = fabs(mx) * (1 - 2 * MwcNext01(mwc));\n"
			"		y = fabs(my) * (1 - 2 * MwcNext01(mwc));\n"
			"		m = (int)floor(0.5 * x / sc);\n"
			"		n = (int)floor(0.5 * y / sc);\n"
			"		alpha = M_2PI * MwcNext01(mwc);\n"
			"		u = 0.3 + 0.7 * CircleTrans1DiscreteNoise2(m + 10, n + 3);\n"
			"		x = u * cos(alpha);\n"
			"		y = u * sin(alpha);\n"
			"\n"
			"		if (++iters > 10)\n"
			"			break;\n"
			"	}\n"
			"	while (CircleTrans1DiscreteNoise2((int)(m + seed), n) > dens);\n"
			"\n"
			"	*ux = x + (m * 2 + 1) * sc;\n"
			"	*vy = y + (n * 2 + 1) * sc;\n"
			"}\n"
			"\n";
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sc,   prefix + "CircleTrans1_Sc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dens, prefix + "CircleTrans1_Dens", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_X,    prefix + "CircleTrans1_X", 10));
		m_Params.push_back(ParamWithName<T>(&m_Y,    prefix + "CircleTrans1_Y", 10));
		m_Params.push_back(ParamWithName<T>(&m_Seed, prefix + "CircleTrans1_Seed", 0, INTEGER));
	}
	
private:
	T DiscreteNoise2(int x, int y)
	{
		const T im = T(2147483647);
		const T am = (1 / im);

		int n = x + y * 57;
		n = (n << 13) ^ n;
		return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;
	}

	void Trans(T a, T b, T x, T y, T* x1, T* y1)
	{
		*x1 = (x - a) * T(0.5) + a;
		*y1 = (y - b) * T(0.5) + b;
	}

	void CircleR(T* ux, T* vy, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int m, n, iters = 0;
		T x, y, alpha, u;

		do
		{
			x = fabs(m_X) * (1 - 2 * rand.Frand01<T>());
			y = fabs(m_Y) * (1 - 2 * rand.Frand01<T>());
			m = Floor<T>(T(0.5) * x / m_Sc);
			n = Floor<T>(T(0.5) * y / m_Sc);
			alpha = M_2PI * rand.Frand01<T>();
			u = T(0.3) + T(0.7) * DiscreteNoise2(m + 10, n + 3);
			x = u * cos(alpha);
			y = u * sin(alpha);
			
			if (++iters > 10)
				break;
		}
		while (DiscreteNoise2((int)(m + m_Seed), n) > m_Dens);
				   
		*ux = x + (m * 2 + 1) * m_Sc;
		*vy = y + (n * 2 + 1) * m_Sc;
	}

	T m_Sc;
	T m_Dens;
	T m_X;
	T m_Y;
	T m_Seed;
};

/// <summary>
/// cubic3D.
/// </summary>
template <typename T>
class EMBER_API Cubic3DVariation : public ParametricVariation<T>
{
public:
	Cubic3DVariation(T weight = 1.0) : ParametricVariation<T>("cubic3D", VAR_CUBIC3D, weight)
	{
		Init();
	}

	PARVARCOPY(Cubic3DVariation)
	
	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int useNode = rand.Rand() & 7;//Faster than % 8.
		T exnze, wynze, znxy;
		T lattd = m_Weight * T(0.5);
		T px, py, pz;

		exnze = 1 - (m_SmoothStyle * (1 - (cos(atan2(helper.In.x, helper.In.z)))));
		wynze = 1 - (m_SmoothStyle * (1 - (sin(atan2(helper.In.y, helper.In.z)))));

		if (m_SmoothStyle > 1)
			znxy = 1 - (m_SmoothStyle * (1 - ((exnze + wynze) / 2 * m_SmoothStyle)));
		else
			znxy = 1 - (m_SmoothStyle * (1 - ((exnze + wynze) * T(0.5))));

		if (m_VarType == VARTYPE_PRE)
		{
			px = helper.In.x;
			py = helper.In.y;
			pz = helper.In.z;
		}
		else
		{
			px = outPoint.m_X;
			py = outPoint.m_Y;
			pz = outPoint.m_Z;
		}

		switch (useNode)
		{
			case 0 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) + lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) + lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  + lattd;
				break;
			case 1 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) + lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) - lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  + lattd;
				break;
			case 2 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) + lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) + lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  - lattd;
				break;
			case 3 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) + lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) - lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  - lattd;
				break;
			case 4 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) - lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) + lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  + lattd;
				break;
			case 5 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) - lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) - lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  + lattd;
				break;
			case 6 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) - lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) + lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  - lattd;
				break;
			case 7 :
				helper.Out.x = ((px - (m_Smooth * (1 - m_Fill) * px * exnze)) + (helper.In.x * m_Smooth * m_Fill * exnze)) - lattd;
				helper.Out.y = ((py - (m_Smooth * (1 - m_Fill) * py * wynze)) + (helper.In.y * m_Smooth * m_Fill * wynze)) - lattd;
				helper.Out.z = ((pz - (m_Smooth * (1 - m_Fill) * pz * znxy))  + (helper.In.z * m_Smooth * m_Fill * znxy))  - lattd;
				break;
		}
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xpand        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string style        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fill         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string smooth       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string smoothStyle  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint useNode = MwcNext(mwc) & 7;\n"
		   << "\t\treal_t exnze, wynze, znxy;\n"
		   << "\t\treal_t lattd = xform->m_VariationWeights[" << varIndex << "] * 0.5;\n"
		   << "\t\treal_t px, py, pz;\n"
		   << "\n"
		   << "\t\texnze = 1 - (" << smoothStyle << " * (1 - (cos(atan2(vIn.x, vIn.z)))));\n"
		   << "\t\twynze = 1 - (" << smoothStyle << " * (1 - (sin(atan2(vIn.y, vIn.z)))));\n"
		   << "\n"
		   << "\t\tif (" << smoothStyle << " > 1)\n"
		   << "\t\t	znxy = 1 - (" << smoothStyle << " * (1 - ((exnze + wynze) / 2 * " << smoothStyle << ")));\n"
		   << "\t\telse\n"
		   << "\t\t	znxy = 1 - (" << smoothStyle << " * (1 - ((exnze + wynze) * 0.5)));\n";

		if (m_VarType == VARTYPE_PRE)
		{
			ss << 
			"\t\tpx = vIn.x;\n"
			"\t\tpy = vIn.y;\n"
			"\t\tpz = vIn.z;\n";
		}
		else
		{
			ss <<
			"\t\tpx = outPoint->m_X;\n"
			"\t\tpy = outPoint->m_Y;\n"
			"\t\tpz = outPoint->m_Z;\n";
		}

		ss <<
		"\t\tswitch (useNode)\n"
		"\t\t{\n"
		"\t\t	case 0 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) + lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) + lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  + lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 1 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " *(1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) + lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " *(1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) - lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " *(1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  + lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 2 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) + lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) + lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  - lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 3 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) + lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) - lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  - lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 4 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) - lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) + lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  + lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 5 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) - lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) - lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  + lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 6 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) - lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) + lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  - lattd;\n"
		"\t\t		break;\n"
		"\t\t	case 7 :\n"
		"\t\t		vOut.x = ((px - (" << smooth << " * (1 - " << fill << ") * px * exnze)) + (vIn.x * " << smooth << " * " << fill << " * exnze)) - lattd;\n"
		"\t\t		vOut.y = ((py - (" << smooth << " * (1 - " << fill << ") * py * wynze)) + (vIn.y * " << smooth << " * " << fill << " * wynze)) - lattd;\n"
		"\t\t		vOut.z = ((pz - (" << smooth << " * (1 - " << fill << ") * pz * znxy))  + (vIn.z * " << smooth << " * " << fill << " * znxy))  - lattd;\n"
		"\t\t		break;\n"
		"\t\t}\n"
		"\t}\n";

		return ss.str();
	}
	
	virtual void Precalc()
	{
		if (fabs(m_Xpand) <= 1)
			m_Fill = m_Xpand * T(0.5);
		else
			m_Fill = sqrt(m_Xpand) * T(0.5);

		if (fabs(m_Weight) <= T(0.5))
			m_Smooth = m_Weight * 2;//Causes full effect above m_Weight = 0.5.
		else
			m_Smooth = 1;

		if (fabs(m_Style) <= 1)
		{
			m_SmoothStyle = m_Style;
		}
		else
		{
			if (m_Style > 1)
				m_SmoothStyle = 1 + (m_Style - 1) * T(0.25);
			else
				m_SmoothStyle = (m_Style + 1) * T(0.25) - 1;
		}
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Xpand, prefix + "cubic3D_xpand", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Style, prefix + "cubic3D_style"));
		m_Params.push_back(ParamWithName<T>(true, &m_Fill,        prefix + "cubic3D_fill"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Smooth,      prefix + "cubic3D_smooth"));
		m_Params.push_back(ParamWithName<T>(true, &m_SmoothStyle, prefix + "cubic3D_smooth_style"));
	}

private:
	T m_Xpand;
	T m_Style;
	T m_Fill;//Precalc.
	T m_Smooth;
	T m_SmoothStyle;
};

/// <summary>
/// cubicLattice_3D.
/// </summary>
template <typename T>
class EMBER_API CubicLattice3DVariation : public ParametricVariation<T>
{
public:
	CubicLattice3DVariation(T weight = 1.0) : ParametricVariation<T>("cubicLattice_3D", VAR_CUBIC_LATTICE3D, weight)
	{
		Init();
	}

	PARVARCOPY(CubicLattice3DVariation)
	
	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int useNode = rand.Rand() & 7;//Faster than % 8.
		T exnze, wynze, znxy, px, py, pz, lattd = m_Weight;

		if (m_Style == 2)
		{
			exnze = cos(atan2(helper.In.x, helper.In.z));
			wynze = sin(atan2(helper.In.y, helper.In.z));
			znxy = (exnze + wynze) * T(0.5);
		}	
		else
		{
			exnze = 1;
			wynze = 1;
			znxy = 1;
		}

		if (m_VarType == VARTYPE_PRE)
		{
			px = helper.In.x;
			py = helper.In.y;
			pz = helper.In.z;
		}
		else
		{
			px = outPoint.m_X;
			py = outPoint.m_Y;
			pz = outPoint.m_Z;
		}

		T pxtx = px + helper.In.x;
		T pyty = py + helper.In.y;
		T pztz = pz + helper.In.z;

		switch (useNode)
		{
			case 0 :
				helper.Out.x = pxtx * m_Fill * exnze + lattd;
				helper.Out.y = pyty * m_Fill * wynze + lattd;
				helper.Out.z = pztz * m_Fill * znxy  + lattd;
				break;
			case 1 :
				helper.Out.x = pxtx * m_Fill * exnze + lattd;
				helper.Out.y = pyty * m_Fill * wynze - lattd;
				helper.Out.z = pztz * m_Fill * znxy  + lattd;
				break;
			case 2 :
				helper.Out.x = pxtx * m_Fill * exnze + lattd;
				helper.Out.y = pyty * m_Fill * wynze + lattd;
				helper.Out.z = pztz * m_Fill * znxy  - lattd;
				break;
			case 3 :
				helper.Out.x = pxtx * m_Fill * exnze + lattd;
				helper.Out.y = pyty * m_Fill * wynze - lattd;
				helper.Out.z = pztz * m_Fill * znxy  - lattd;
				break;
			case 4 :
				helper.Out.x = pxtx * m_Fill * exnze - lattd;
				helper.Out.y = pyty * m_Fill * wynze + lattd;
				helper.Out.z = pztz * m_Fill * znxy  + lattd;
				break;
			case 5 :
				helper.Out.x = pxtx * m_Fill * exnze - lattd;
				helper.Out.y = pyty * m_Fill * wynze - lattd;
				helper.Out.z = pztz * m_Fill * znxy  + lattd;
				break;
			case 6 :
				helper.Out.x = pxtx * m_Fill * exnze - lattd;
				helper.Out.y = pyty * m_Fill * wynze + lattd;
				helper.Out.z = pztz * m_Fill * znxy  - lattd;
				break;
			case 7 :
				helper.Out.x = pxtx * m_Fill * exnze - lattd;
				helper.Out.y = pyty * m_Fill * wynze - lattd;
				helper.Out.z = pztz * m_Fill * znxy  - lattd;
				break;
		}
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xpand = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string style = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fill  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint useNode = MwcNext(mwc) & 7;\n"
		   << "\t\treal_t exnze, wynze, znxy, px, py, pz, lattd = xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\n"
		   << "\t\tif (" << style << " == 2)\n"
		   << "\t\t{\n"
		   << "\t\t	exnze = cos(atan2(vIn.x, vIn.z));\n"
		   << "\t\t	wynze = sin(atan2(vIn.y, vIn.z));\n"
		   << "\t\t	znxy = (exnze + wynze) * 0.5;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	exnze = 1;\n"
		   << "\t\t	wynze = 1;\n"
		   << "\t\t	znxy = 1;\n"
		   << "\t\t}\n";

		if (m_VarType == VARTYPE_PRE)
		{
			ss << 
			"\t\tpx = vIn.x;\n"
			"\t\tpy = vIn.y;\n"
			"\t\tpz = vIn.z;\n";
		}
		else
		{
			ss <<
			"\t\tpx = outPoint->m_X;\n"
			"\t\tpy = outPoint->m_Y;\n"
			"\t\tpz = outPoint->m_Z;\n";
		}

		ss << "\t\treal_t pxtx = px + vIn.x;\n"
		   << "\t\treal_t pyty = py + vIn.y;\n"
		   << "\t\treal_t pztz = pz + vIn.z;\n"
		   << "\n"
		   << "\t\tswitch (useNode)\n"
		   << "\t\t{\n"
		   << "\t\t	case 0 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze + lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze + lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  + lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze + lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze - lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  + lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze + lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze + lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  - lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 3 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze + lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze - lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  - lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 4 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze - lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze + lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  + lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 5 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze - lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze - lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  + lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 6 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze - lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze + lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  - lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 7 :\n"
		   << "\t\t		vOut.x = pxtx * " << fill << " * exnze - lattd;\n"
		   << "\t\t		vOut.y = pyty * " << fill << " * wynze - lattd;\n"
		   << "\t\t		vOut.z = pztz * " << fill << " * znxy  - lattd;\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		if (fabs(m_Xpand) <= 1)
			m_Fill = m_Xpand * T(0.5);
		else
			m_Fill = sqrt(m_Xpand) * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Xpand, prefix + "cubicLattice_3D_xpand", T(0.2)));//Original used a prefix of cubic3D_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Style, prefix + "cubicLattice_3D_style", 1, INTEGER, 1, 2));
		m_Params.push_back(ParamWithName<T>(true, &m_Fill, prefix + "cubicLattice_3D_fill"));//Precalc.
	}

private:
	T m_Xpand;
	T m_Style;
	T m_Fill;//Precalc.
};

/// <summary>
/// foci_3D.
/// </summary>
template <typename T>
class EMBER_API Foci3DVariation : public Variation<T>
{
public:
	Foci3DVariation(T weight = 1.0) : Variation<T>("foci_3D", VAR_FOCI3D, weight, false, false, false, false, true) { }

	VARCOPY(Foci3DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T expx = exp(helper.In.x) * T(0.5);
		T expnx = T(0.25) / expx;
		T boot = helper.In.z == 0 ? helper.m_PrecalcAtanyx : helper.In.z;
		T tmp = m_Weight / (expx + expnx - (cos(helper.In.y) * cos(boot)));

		helper.Out.x = (expx - expnx) * tmp;
		helper.Out.y = sin(helper.In.y) * tmp;
		helper.Out.z = sin(boot) * tmp;
	}

	virtual string OpenCLString()
	{
		ostringstream ss;
		int i = 0, varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t expx = exp(vIn.x) * 0.5;\n"
		   << "\t\treal_t expnx = 0.25 / expx;\n"
		   << "\t\treal_t boot = vIn.z == 0 ? precalcAtanyx : vIn.z;\n"
		   << "\t\treal_t tmp = xform->m_VariationWeights[" << varIndex << "] / (expx + expnx - (cos(vIn.y) * cos(boot)));\n"
		   << "\n"
		   << "\t\tvOut.x = (expx - expnx) * tmp;\n"
		   << "\t\tvOut.y = sin(vIn.y) * tmp;\n"
		   << "\t\tvOut.z = sin(boot) * tmp;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// ho.
/// </summary>
template <typename T>
class EMBER_API HoVariation : public ParametricVariation<T>
{
public:
	HoVariation(T weight = 1.0) : ParametricVariation<T>("ho", VAR_HO, weight)
	{
		Init();
	}

	PARVARCOPY(HoVariation)
	
	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T uu = SQR(helper.In.x);
		T vv = SQR(helper.In.y);
		T ww = SQR(helper.In.z);
		T alpha    = (vv + uu + EPS6);
		T beta     = (uu + ww + EPS6);
		T delta    = (vv + ww + EPS6);
		T epsilon  = (vv + uu + ww + EPS6);
		T atOmegaX = atan2(vv, ww);
		T atOmegaY = atan2(uu, ww);
		T atOmegaZ = atan2(vv, uu);
		T su = sin(helper.In.x);
		T cu = cos(helper.In.x);
		T sv = sin(helper.In.y);
		T cv = cos(helper.In.y);
		T cucv = cu * cv;
		T sucv = su * cv;
		T x = pow(ClampGte<T>(cucv, EPS6), m_XPow) + (cucv * m_XPow) + (T(0.25) * atOmegaX);//Must clamp first argument to pow, because negative values will return NaN.
		T y = pow(ClampGte<T>(sucv, EPS6), m_YPow) + (sucv * m_YPow) + (T(0.25) * atOmegaY);//Original did not do this and would frequently return bad values.
		T z = pow(ClampGte<T>(sv, EPS6),   m_ZPow) +  sv   * m_ZPow;

		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
		helper.Out.z = m_Weight * z;
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xpow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ypow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zpow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t uu = SQR(vIn.x);\n"
		   << "\t\treal_t vv = SQR(vIn.y);\n"
		   << "\t\treal_t ww = SQR(vIn.z);\n"
		   << "\t\treal_t alpha    = (vv + uu + EPS6);\n"
		   << "\t\treal_t beta     = (uu + ww + EPS6);\n"
		   << "\t\treal_t delta    = (vv + ww + EPS6);\n"
		   << "\t\treal_t epsilon  = (vv + uu + ww + EPS6);\n"
		   << "\t\treal_t atOmegaX = atan2(vv, ww);\n"
		   << "\t\treal_t atOmegaY = atan2(uu, ww);\n"
		   << "\t\treal_t atOmegaZ = atan2(vv, uu);\n"
		   << "\t\treal_t su = sin(vIn.x);\n"
		   << "\t\treal_t cu = cos(vIn.x);\n"
		   << "\t\treal_t sv = sin(vIn.y);\n"
		   << "\t\treal_t cv = cos(vIn.y);\n"
		   << "\t\treal_t cucv = cu * cv;\n"
		   << "\t\treal_t sucv = su * cv;\n"
		   << "\t\treal_t x = pow(ClampGte(cucv, EPS6), " << xpow << ") + (cucv * " << xpow << ") + (0.25 * atOmegaX);\n"
		   << "\t\treal_t y = pow(ClampGte(sucv, EPS6), " << ypow << ") + (sucv * " << ypow << ") + (0.25 * atOmegaY);\n"
		   << "\t\treal_t z = pow(ClampGte(sv, EPS6), "   << zpow << ") + sv * "    << zpow << ";\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XPow, prefix + "ho_xpow", 3));
		m_Params.push_back(ParamWithName<T>(&m_YPow, prefix + "ho_ypow", 3));
		m_Params.push_back(ParamWithName<T>(&m_ZPow, prefix + "ho_zpow", 3));
	}

private:
	T m_XPow;
	T m_YPow;
	T m_ZPow;
};

/// <summary>
/// Julia3Dq.
/// </summary>
template <typename T>
class EMBER_API Julia3DqVariation : public ParametricVariation<T>
{
public:
	Julia3DqVariation(T weight = 1.0) : ParametricVariation<T>("julia3Dq", VAR_JULIA3DQ, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Julia3DqVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T temp = helper.m_PrecalcAtanyx * m_InvPower + rand.Rand() * m_InvPower2pi;
		T sina = sin(temp);
		T cosa = cos(temp);
		T z = helper.In.z * m_AbsInvPower;
		T r2d = helper.m_PrecalcSumSquares;
		T r = m_Weight * pow(r2d + SQR(z), m_HalfInvPower);
		T rsss = r * helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = rsss * cosa;
		helper.Out.y = rsss * sina;
		helper.Out.z = r * z;
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absInvPower  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfInvPower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower2pi  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t temp = precalcAtanyx * " << invPower << " + MwcNext(mwc) * " << invPower2pi << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\t\treal_t z = vIn.z * " << absInvPower << ";\n"
		   << "\t\treal_t r2d = precalcSumSquares;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(r2d + SQR(z), " << halfInvPower << ");\n"
		   << "\t\treal_t rsss = r * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = rsss * cosa;\n"
		   << "\t\tvOut.y = rsss * sina;\n"
		   << "\t\tvOut.z = r * z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_InvPower = m_Divisor / m_Power;
		m_AbsInvPower = fabs(m_InvPower);
		m_HalfInvPower = T(0.5) * m_InvPower - T(0.5);
		m_InvPower2pi = M_2PI / m_Power;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,   prefix + "julia3Dq_power",   3, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "julia3Dq_divisor", 2, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower,     prefix + "julia3Dq_inv_power"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_AbsInvPower,  prefix + "julia3Dq_abs_inv_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfInvPower, prefix + "julia3Dq_half_inv_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower2pi,  prefix + "julia3Dq_inv_power_2pi"));
	}

private:
	T m_Power;
	T m_Divisor;
	T m_InvPower;//Precalc.
	T m_AbsInvPower;
	T m_HalfInvPower;
	T m_InvPower2pi;
};

/// <summary>
/// line.
/// </summary>
template <typename T>
class EMBER_API LineVariation : public ParametricVariation<T>
{
public:
	LineVariation(T weight = 1.0) : ParametricVariation<T>("line", VAR_LINE, weight)
	{
		Init();
	}

	PARVARCOPY(LineVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T r = rand.Frand01<T>() * m_Weight;

		helper.Out.x = m_Ux * r;
		helper.Out.y = m_Uy * r;
		helper.Out.z = m_Uz * r;
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string delta = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ux    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string uy    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string uz    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t r = MwcNext01(mwc) * xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\n"
		   << "\t\tvOut.x = " << ux << " * r;\n"
		   << "\t\tvOut.y = " << uy << " * r;\n"
		   << "\t\tvOut.z = " << uz << " * r;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		//Unit vector of the line.
		m_Ux = cos(m_Delta * T(M_PI)) * cos(m_Phi * T(M_PI));
		m_Uy = sin(m_Delta * T(M_PI)) * cos(m_Phi * T(M_PI));
		m_Uz = sin(m_Phi * T(M_PI));

		T r = sqrt(SQR(m_Ux) + SQR(m_Uy) + SQR(m_Uz));

		//Normalize.
		m_Ux /= r;
		m_Uy /= r;
		m_Uz /= r;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Delta, prefix + "line_delta"));
		m_Params.push_back(ParamWithName<T>(&m_Phi,   prefix + "line_phi"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ux, prefix + "line_ux"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Uy, prefix + "line_uy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Uz, prefix + "line_uz"));
	}

private:
	T m_Delta;
	T m_Phi;
	T m_Ux;//Precalc.
	T m_Uy;
	T m_Uz;
};

/// <summary>
/// loonie_3D.
/// </summary>
template <typename T>
class EMBER_API Loonie3DVariation : public ParametricVariation<T>
{
public:
	Loonie3DVariation(T weight = 1.0) : ParametricVariation<T>("loonie_3D", VAR_LOONIE3D, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Loonie3DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T rmod = rand.Frand01<T>() * T(0.5) + T(0.125);
		T kikr = helper.m_PrecalcAtanyx;
		T efTez = helper.In.z == 0 ? kikr : helper.In.z;
		T r2 = helper.m_PrecalcSumSquares + SQR(efTez);

		if (r2 < m_Vv)
		{
			T r = m_Weight * sqrt(m_Vv / r2 - 1);

			helper.Out.x = r * helper.In.x;
			helper.Out.y = r * helper.In.y;
			helper.Out.z = r * efTez * T(0.5);
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
			helper.Out.z = m_Weight * efTez * T(0.5);
		}
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string vv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t rmod = MwcNext01(mwc) * 0.5 + 0.125;\n"
		   << "\t\treal_t kikr = precalcAtanyx;\n"
		   << "\t\treal_t efTez = vIn.z == 0 ? kikr : vIn.z;\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(efTez);\n"
		   << "\n"
		   << "\t\tif (r2 < " << vv << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = xform->m_VariationWeights[" << varIndex << "] * sqrt(" << vv << " / r2 - 1);\n"
		   << "\n"
		   << "\t\t	vOut.x = r * vIn.x;\n"
		   << "\t\t	vOut.y = r * vIn.y;\n"
		   << "\t\t	vOut.z = r * efTez * 0.5;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * efTez * 0.5;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_Vv = SQR(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Vv, prefix + "loonie_3D_vv"));//Precalcs only, no params.
	}

private:
	T m_Vv;//Precalcs only, no params.
};

/// <summary>
/// mcarpet.
/// </summary>
template <typename T>
class EMBER_API McarpetVariation : public ParametricVariation<T>
{
public:
	McarpetVariation(T weight = 1.0) : ParametricVariation<T>("mcarpet", VAR_MCARPET, weight, true)
	{
		Init();
	}

	PARVARCOPY(McarpetVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T t = helper.m_PrecalcSumSquares * T(0.25) + 1;
		T r = m_Weight / t;

		helper.Out.x = helper.In.x * r * m_X;
		helper.Out.y = helper.In.y * r * m_Y;
		helper.Out.x += (1 - (m_Twist * SQR(helper.In.x)) + helper.In.y) * m_Weight;//The += is intentional.
		helper.Out.y += m_Tilt * helper.In.x * m_Weight;
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tilt  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t t = precalcSumSquares * 0.25 + 1;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / t;\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * " << x << ";\n"
		   << "\t\tvOut.y = vIn.y * r * " << y << ";\n"
		   << "\t\tvOut.x += (1 - (" << twist << " * SQR(vIn.x)) + vIn.y) * xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t\tvOut.y += " << tilt << " * vIn.x * xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,     prefix + "mcarpet_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y,     prefix + "mcarpet_y"));
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "mcarpet_twist"));
		m_Params.push_back(ParamWithName<T>(&m_Tilt,  prefix + "mcarpet_tilt"));
	}

private:
	T m_X;
	T m_Y;
	T m_Twist;
	T m_Tilt;
};

/// <summary>
/// waves2_3D.
/// Original used a precalc for the input points, but it doesn't
/// work with Ember's design, so it gets calculated on every iter
/// which is slightly slower.
/// </summary>
template <typename T>
class EMBER_API Waves23DVariation : public ParametricVariation<T>
{
public:
	Waves23DVariation(T weight = 1.0) : ParametricVariation<T>("waves2_3D", VAR_WAVES23D, weight)
	{
		Init();
	}

	PARVARCOPY(Waves23DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T avgxy = (helper.In.x + helper.In.y) * T(0.5);

		helper.Out.x = m_Weight * (helper.In.x + m_Scale * sin(helper.In.y * m_Freq));
		helper.Out.y = m_Weight * (helper.In.y + m_Scale * sin(helper.In.x * m_Freq));
		helper.Out.z = m_Weight * (helper.In.z + m_Scale * sin(avgxy * m_Freq));//Averages the XY to get Z.
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string freq  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\treal_t avgxy = (vIn.x + vIn.y) * 0.5;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << scale << " * sin(vIn.y * " << freq << "));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << scale << " * sin(vIn.x * " << freq << "));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z + " << scale << " * sin(avgxy * " << freq << "));\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Freq,  prefix + "waves2_3D_freq",  2));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "waves2_3D_scale", 1));
	}

private:
	T m_Freq;
	T m_Scale;
};

/// <summary>
/// Pie3D.
/// </summary>
template <typename T>
class EMBER_API Pie3DVariation : public ParametricVariation<T>
{
public:
	Pie3DVariation(T weight = 1.0) : ParametricVariation<T>("pie3D", VAR_PIE3D, weight)
	{
		Init();
	}

	PARVARCOPY(Pie3DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int sl = (int)(rand.Frand01<T>() * m_Slices + T(0.5));
		T a = m_Rotation + M_2PI * (sl + rand.Frand01<T>() * m_Thickness) / m_Slices;
		T r = m_Weight * rand.Frand01<T>();

		helper.Out.x = r * cos(a);
		helper.Out.y = r * sin(a);
		helper.Out.z = m_Weight * sin(r);
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string slices =    "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotation =  "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint sl = (int)(MwcNext01(mwc) * " << slices << " + 0.5);\n"
		   << "\t\treal_t a = " << rotation << " + M_2PI * (sl + MwcNext01(mwc) * " << thickness << ") / " << slices << ";\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * sin(r);\n"
		   << "\t}\n";

		return ss.str();
	}
	
	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		m_Params[0].Set((int)10 * rand.Frand01<T>());//Slices.
		m_Params[1].Set(M_2PI * rand.Frand11<T>());//Rotation.
		m_Thickness = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Slices,    prefix + "pie3D_slices", 6, INTEGER_NONZERO, 1));
		m_Params.push_back(ParamWithName<T>(&m_Rotation,  prefix + "pie3D_rotation", T(0.5), REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "pie3D_thickness", T(0.5), REAL, 0, 1));
	}

private:
	T m_Slices;
	T m_Rotation;
	T m_Thickness;
};

/// <summary>
/// popcorn2_3D.
/// </summary>
template <typename T>
class EMBER_API Popcorn23DVariation : public ParametricVariation<T>
{
public:
	Popcorn23DVariation(T weight = 1.0) : ParametricVariation<T>("popcorn2_3D", VAR_POPCORN23D, weight, false, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Popcorn23DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T otherZ, tempPZ = 0;
		T tempTZ = helper.In.z == 0 ? m_Vv * m_SinTanC * helper.m_PrecalcAtanyx : helper.In.z;

		if (m_VarType == VARTYPE_PRE)
			otherZ = helper.In.z;
		else
			otherZ = outPoint.m_Z;

		if (otherZ == 0)
			tempPZ = m_Vv * m_SinTanC * helper.m_PrecalcAtanyx; 		

		helper.Out.x = m_HalfWeight * (helper.In.x + m_X * sin(tan(m_C * helper.In.y)));
		helper.Out.y = m_HalfWeight * (helper.In.y + m_Y * sin(tan(m_C * helper.In.x)));
		helper.Out.z = tempPZ + m_Vv * (m_Z * m_SinTanC * tempTZ);
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string stc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hw  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vv  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t otherZ, tempPZ = 0;\n"
		   << "\t\treal_t tempTZ = vIn.z == 0 ? " << vv << " * " << stc << " * precalcAtanyx : vIn.z;\n";

		if (m_VarType == VARTYPE_PRE)
			ss << "\t\totherZ = vIn.z;\n";
		else
			ss << "\t\totherZ = outPoint->m_Z;\n";

		ss << "\t\tif (otherZ == 0)\n"
		   << "\t\t	tempPZ = " << vv << " * " << stc << " * precalcAtanyx;\n"
		   << "\n"
		   << "\t\tvOut.x = " << hw << " * (vIn.x + " << x << " * sin(tan(" << c << " * vIn.y)));\n"
		   << "\t\tvOut.y = " << hw << " * (vIn.y + " << y << " * sin(tan(" << c << " * vIn.x)));\n"
		   << "\t\tvOut.z = tempPZ + " << vv << " * (" << z << " * " << stc << " * tempTZ);\n"
		   << "\t}\n";

		return ss.str();
	}
	
	virtual void Precalc()
	{
		m_SinTanC = sin(tan(m_C));
		m_HalfWeight = m_Weight * T(0.5);

		if (fabs(m_Weight) <= 1)
			m_Vv = fabs(m_Weight) * m_Weight;//Sqr(m_Weight) value retaining sign.
		else
			m_Vv = m_Weight;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		m_X = T(0.2) + rand.Frand01<T>();
		m_Y = T(0.2) * rand.Frand01<T>();
		m_Z = T(0.2) * rand.Frand01<T>();
		m_C = 5      * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "popcorn2_3D_x", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "popcorn2_3D_y", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "popcorn2_3D_z", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "popcorn2_3D_c", 3));
		m_Params.push_back(ParamWithName<T>(true, &m_SinTanC,    prefix + "popcorn2_3D_sintanc"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfWeight, prefix + "popcorn2_3D_half_weight"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vv,         prefix + "popcorn2_3D_vv"));
	}
	
private:
	T m_X;
	T m_Y;
	T m_Z;
	T m_C;
	T m_SinTanC;//Precalcs.
	T m_HalfWeight;
	T m_Vv;
};

/// <summary>
/// sinusoidal3d.
/// </summary>
template <typename T>
class EMBER_API Sinusoidal3DVariation : public Variation<T>
{
public:
	Sinusoidal3DVariation(T weight = 1.0) : Variation<T>("sinusoidal3D", VAR_SINUSOIDAL3D, weight) { }

	VARCOPY(Sinusoidal3DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		helper.Out.x = m_Weight * sin(helper.In.x);
		helper.Out.y = m_Weight * sin(helper.In.y);
		helper.Out.z = m_Weight * (atan2(SQR(helper.In.x), SQR(helper.In.y)) * cos(helper.In.z));
	}

	virtual string OpenCLString()
	{
		ostringstream ss;
		int i = 0, varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (atan2(SQR(vIn.x), SQR(vIn.y)) * cos(vIn.z));\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// scry_3D.
/// </summary>
template <typename T>
class EMBER_API Scry3DVariation : public ParametricVariation<T>
{
public:
	Scry3DVariation(T weight = 1.0) : ParametricVariation<T>("scry_3D", VAR_SCRY3D, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Scry3DVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T t = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T r = 1 / (sqrt(t) * (t + m_InvWeight));
		T z = helper.In.z == 0 ? helper.m_PrecalcAtanyx : helper.In.z;

		helper.Out.x = helper.In.x * r;
		helper.Out.y = helper.In.y * r;
		helper.Out.z = z * r;
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string invWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t r = 1 / (sqrt(t) * (t + " << invWeight << "));\n"
		   << "\t\treal_t z = vIn.z == 0 ? precalcAtanyx : vIn.z;\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r;\n"
		   << "\t\tvOut.y = vIn.y * r;\n"
		   << "\t\tvOut.z = z * r;\n"
		   << "\t}\n";

		return ss.str();
	}
	
	virtual void Precalc()
	{
		m_InvWeight = 1 / (m_Weight + EPS6);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_InvWeight, prefix + "scry_3D_inv_weight"));//Precalcs only, no params.
	}
	
private:
	T m_InvWeight;//Precalcs only, no params.
};

/// <summary>
/// shredlin.
/// </summary>
template <typename T>
class EMBER_API ShredlinVariation : public ParametricVariation<T>
{
public:
	ShredlinVariation(T weight = 1.0) : ParametricVariation<T>("shredlin", VAR_SHRED_LIN, weight)
	{
		Init();
	}

	PARVARCOPY(ShredlinVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		const int xpos = helper.In.x < 0;
		const int ypos = helper.In.y < 0;
		const T xrng = helper.In.x / m_XDistance;
		const T yrng = helper.In.y / m_YDistance;
	
		helper.Out.x = m_Xw * ((xrng - (int)xrng) * m_XWidth + (int)xrng + (T(0.5) - xpos) * m_1mX);
		helper.Out.y = m_Yw * ((yrng - (int)yrng) * m_YWidth + (int)yrng + (T(0.5) - ypos) * m_1mY);
		helper.Out.z = m_Weight * helper.In.z;
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string xdist     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xwidth    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ydist     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ywidth    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xw        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yw        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onemx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onemy     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst int xpos = vIn.x < 0;\n"
		   << "\t\tconst int ypos = vIn.y < 0;\n"
		   << "\t\tconst real_t xrng = vIn.x / " << xdist << ";\n"
		   << "\t\tconst real_t yrng = vIn.y / " << ydist << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << xw << " * ((xrng - (int)xrng) * " << xwidth << " + (int)xrng + (0.5 - xpos) * " << onemx << ");\n"
		   << "\t\tvOut.y = " << yw << " * ((yrng - (int)yrng) * " << ywidth << " + (int)yrng + (0.5 - ypos) * " << onemy << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_Xw = m_Weight * m_XDistance;
		m_Yw = m_Weight * m_YDistance;
		m_1mX = 1 - m_XWidth;
		m_1mY = 1 - m_YWidth;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XDistance, prefix + "shredlin_xdistance", 1, REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_XWidth,    prefix + "shredlin_xwidth", T(0.5), REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_YDistance, prefix + "shredlin_ydistance", 1, REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_YWidth,    prefix + "shredlin_ywidth", T(0.5), REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Xw,  prefix + "shredlin_xw"));
		m_Params.push_back(ParamWithName<T>(true, &m_Yw,  prefix + "shredlin_yw"));
		m_Params.push_back(ParamWithName<T>(true, &m_1mX, prefix + "shredlin_1mx"));
		m_Params.push_back(ParamWithName<T>(true, &m_1mY, prefix + "shredlin_1my"));
	}

private:
	T m_XDistance;
	T m_XWidth;
	T m_YDistance;
	T m_YWidth;
	T m_Xw;//Precalc.
	T m_Yw;
	T m_1mX;
	T m_1mY;
};

/// <summary>
/// splitbrdr.
/// </summary>
template <typename T>
class EMBER_API SplitBrdrVariation : public ParametricVariation<T>
{
public:
	SplitBrdrVariation(T weight = 1.0) : ParametricVariation<T>("SplitBrdr", VAR_SPLIT_BRDR, weight, true)
	{
		Init();
	}

	PARVARCOPY(SplitBrdrVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T b = m_Weight / (helper.m_PrecalcSumSquares * T(0.25) + 1);
		T roundX = Rint(helper.In.x);
		T roundY = Rint(helper.In.y);
		T offsetX = helper.In.x - roundX;
		T offsetY = helper.In.y - roundY;

		helper.Out.x = helper.In.x * b;
		helper.Out.y = helper.In.y * b;
	
		if (rand.Frand01<T>() >= T(0.75))
		{
			helper.Out.x += m_Weight * (offsetX * T(0.5) + roundX);
			helper.Out.y += m_Weight * (offsetY * T(0.5) + roundY);
		}
		else
		{
			if (fabs(offsetX) >= fabs(offsetY))
			{
				if (offsetX >= 0)
				{
					helper.Out.x += m_Weight * (offsetX * T(0.5) + roundX + m_X);
					helper.Out.y += m_Weight * (offsetY * T(0.5) + roundY + m_Y * offsetY / offsetX);
				}
				else
				{
					helper.Out.x += m_Weight * (offsetX * T(0.5) + roundX - m_Y);
					helper.Out.y += m_Weight * (offsetY * T(0.5) + roundY - m_Y * offsetY / offsetX);  
				}
			}
			else
			{
				if (offsetY >= 0)
				{
					helper.Out.y += m_Weight * (offsetY * T(0.5) + roundY + m_Y);
					helper.Out.x += m_Weight * (offsetX * T(0.5) + roundX + offsetX / offsetY * m_Y);
				}
				else
				{
					helper.Out.y += m_Weight * (offsetY * T(0.5) + roundY - m_Y);
					helper.Out.x += m_Weight * (offsetX * T(0.5) + roundX - offsetX / offsetY * m_X);
				}
			}
		}

		helper.Out.x += helper.In.x * m_Px;
		helper.Out.y += helper.In.y * m_Py;
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string px     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string py    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t b = xform->m_VariationWeights[" << varIndex << "] / (precalcSumSquares * 0.25 + 1);\n"
		   << "\t\treal_t roundX = rint(vIn.x);\n"
		   << "\t\treal_t roundY = rint(vIn.y);\n"
		   << "\t\treal_t offsetX = vIn.x - roundX;\n"
		   << "\t\treal_t offsetY = vIn.y - roundY;\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * b;\n"
		   << "\t\tvOut.y = vIn.y * b;\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) >= 0.75)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x += xform->m_VariationWeights[" << varIndex << "] * (offsetX * 0.5 + roundX);\n"
		   << "\t\t	vOut.y += xform->m_VariationWeights[" << varIndex << "] * (offsetY * 0.5 + roundY);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (fabs(offsetX) >= fabs(offsetY))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetX >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x += xform->m_VariationWeights[" << varIndex << "] * (offsetX * 0.5 + roundX + " << x << ");\n"
		   << "\t\t			vOut.y += xform->m_VariationWeights[" << varIndex << "] * (offsetY * 0.5 + roundY + " << y << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x += xform->m_VariationWeights[" << varIndex << "] * (offsetX * 0.5 + roundX - " << y << ");\n"
		   << "\t\t			vOut.y += xform->m_VariationWeights[" << varIndex << "] * (offsetY * 0.5 + roundY - " << y << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetY >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y += xform->m_VariationWeights[" << varIndex << "] * (offsetY * 0.5 + roundY + " << y << ");\n"
		   << "\t\t			vOut.x += xform->m_VariationWeights[" << varIndex << "] * (offsetX * 0.5 + roundX + offsetX / offsetY * " << y << ");\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y += xform->m_VariationWeights[" << varIndex << "] * (offsetY * 0.5 + roundY - " << y << ");\n"
		   << "\t\t			vOut.x += xform->m_VariationWeights[" << varIndex << "] * (offsetX * 0.5 + roundX - offsetX / offsetY * " << x << ");\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x += vIn.x * " << px << ";\n"
		   << "\t\tvOut.y += vIn.y * " << py << ";\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,  prefix + "SplitBrdr_x",  T(0.25)));//Original used a prefix of splitb_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Y,  prefix + "SplitBrdr_y",  T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Px, prefix + "SplitBrdr_px"));
		m_Params.push_back(ParamWithName<T>(&m_Py, prefix + "SplitBrdr_py"));
	}

private:
	T m_X;
	T m_Y;
	T m_Px;
	T m_Py;
};

/// <summary>
/// wdisc.
/// </summary>
template <typename T>
class EMBER_API WdiscVariation : public Variation<T>
{
public:
	WdiscVariation(T weight = 1.0) : Variation<T>("wdisc", VAR_WDISC, weight, true, true, false, false, true) { }

	VARCOPY(WdiscVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T a = T(M_PI) / (helper.m_PrecalcSqrtSumSquares + 1);
		T r = helper.m_PrecalcAtanyx * T(M_1_PI);
	
		if (r > 0)
			a = T(M_PI) - a;

		helper.Out.x = m_Weight * r * cos(a);
		helper.Out.y = m_Weight * r * sin(a);
	}

	virtual string OpenCLString()
	{
		ostringstream ss;
		int i = 0, varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t a = M_PI / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t r = precalcAtanyx * M_1_PI;\n"
		   << "\n"
		   << "\t\tif (r > 0)\n"
		   << "\t\t	a = M_PI - a;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * r * cos(a);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * r * sin(a);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// falloff.
/// </summary>
template <typename T>
class EMBER_API FalloffVariation : public ParametricVariation<T>
{
public:
	FalloffVariation(T weight = 1.0) : ParametricVariation<T>("falloff", VAR_FALLOFF, weight, false, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(FalloffVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		const T ax = rand.Frand11<T>();
		const T ay = rand.Frand11<T>();
		const T az = rand.Frand11<T>();
		const T r = sqrt(Sqr(helper.In.x - m_X0) + Sqr(helper.In.y - m_Y0) + Sqr(helper.In.z - m_Z0));
		const T rc = ((m_Invert != 0 ? min<T>(1 - r, 0) : min<T>(r, 0)) - m_MinDist) * m_InternalScatter;
		const T rs = min<T>(rc, 0);

		T sigma, phi, rad, sigmas, sigmac, phis, phic;
		T scale, denom;

		switch ((int)m_Type)
		{
			case 0://Linear.
				helper.Out.x = m_Weight * (helper.In.x + m_MulX * ax * rs);
				helper.Out.y = m_Weight * (helper.In.y + m_MulY * ay * rs);
				helper.Out.z = m_Weight * (helper.In.z + m_MulZ * az * rs);
				break;
			case 1://Radial.
				sigma = asin(r == 0 ? 0 : helper.In.z / r) + m_MulZ * az * rs;
				phi = helper.m_PrecalcAtanyx + m_MulY * ay * rs;
				rad = r + m_MulX * ax * rs;

				sigmas = sin(sigma);
				sigmac = cos(sigma);
				phis = sin(phi);
				phic = cos(phi);

				helper.Out.x = m_Weight * (rad * sigmac * phic);
				helper.Out.y = m_Weight * (rad * sigmac * phis);
				helper.Out.z = m_Weight * (rad * sigmas);
				break;
			case 2://Box.
				scale = Clamp<T>(rs, 0, T(0.9)) + T(0.1);
				denom = 1 / scale;
				helper.Out.x = m_Weight * Lerp<T>(helper.In.x, floor(helper.In.x * denom) + scale * ax, m_MulX * rs) + m_MulX * pow(ax, m_BoxPow) * rs * denom;
				helper.Out.y = m_Weight * Lerp<T>(helper.In.y, floor(helper.In.y * denom) + scale * ay, m_MulY * rs) + m_MulY * pow(ay, m_BoxPow) * rs * denom;
				helper.Out.z = m_Weight * Lerp<T>(helper.In.z, floor(helper.In.z * denom) + scale * az, m_MulZ * rs) + m_MulZ * pow(az, m_BoxPow) * rs * denom;
				break;
		}
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string scatter         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string minDist         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulX            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulY            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulZ            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0              = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0              = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z0              = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invert          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string type            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string boxPow          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string internalScatter = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst real_t ax = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t ay = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t az = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t r = sqrt(Sqr(vIn.x - " << x0 << ") + Sqr(vIn.y - " << y0 << ") + Sqr(vIn.z - " << z0 << "));\n"
		   << "\t\tconst real_t rc = ((" << invert << " != 0 ? min(1 - r, 0.0) : min(r, 0.0)) - " << minDist << ") * " << internalScatter << ";\n"
		   << "\t\tconst real_t rs = min(rc, 0.0);\n"
		   << "\n"
		   << "\t\treal_t sigma, phi, rad, sigmas, sigmac, phis, phic;\n"
		   << "\t\treal_t scale, denom;\n"
		   << "\n"
		   << "\t\tswitch ((int)" << type << ")\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << mulX << " * ax * rs);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << mulY << " * ay * rs);\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z + " << mulZ << " * az * rs);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		sigma = asin(r == 0 ? 0 : vIn.z / r) + " << mulZ << " * az * rs;\n"
		   << "\t\t		phi = precalcAtanyx + " << mulY << " * ay * rs;\n"
		   << "\t\t		rad = r + " << mulX << " * ax * rs;\n"
		   << "\n"
		   << "\t\t		sigmas = sin(sigma);\n"
		   << "\t\t		sigmac = cos(sigma);\n"
		   << "\t\t		phis = sin(phi);\n"
		   << "\t\t		phic = cos(phi);\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (rad * sigmac * phic);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * (rad * sigmac * phis);\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * (rad * sigmas);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t		scale = Clamp(rs, 0, 0.9) + 0.1;\n"
		   << "\t\t		denom = 1 / scale;\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * Lerp(vIn.x, floor(vIn.x * denom) + scale * ax, " << mulX << " * rs) + " << mulX << " * pow(ax, " << boxPow << ") * rs * denom;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * Lerp(vIn.y, floor(vIn.y * denom) + scale * ay, " << mulY << " * rs) + " << mulY << " * pow(ay, " << boxPow << ") * rs * denom;\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * Lerp(vIn.z, floor(vIn.z * denom) + scale * az, " << mulZ << " * rs) + " << mulZ << " * pow(az, " << boxPow << ") * rs * denom;\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_InternalScatter = T(0.04) * m_Scatter;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scatter, prefix + "falloff_scatter", 1, REAL, EPS6, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_MinDist, prefix + "falloff_mindist", T(0.5), REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_MulX,    prefix + "falloff_mul_x", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,    prefix + "falloff_mul_y", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulZ,    prefix + "falloff_mul_z", 0, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,      prefix + "falloff_x0"));
		m_Params.push_back(ParamWithName<T>(&m_Y0,      prefix + "falloff_y0"));
		m_Params.push_back(ParamWithName<T>(&m_Z0,      prefix + "falloff_z0"));
		m_Params.push_back(ParamWithName<T>(&m_Invert,  prefix + "falloff_invert", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Type,    prefix + "falloff_type", 0, INTEGER, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_BoxPow,  prefix + "falloff_boxpow", 2, INTEGER, 2, 32));//Original defaulted this to 0 which directly contradicts the specified range of 2-32.
		m_Params.push_back(ParamWithName<T>(true, &m_InternalScatter, prefix + "falloff_internal_scatter"));
	}

private:
	T m_Scatter;
	T m_MinDist;
	T m_MulX;
	T m_MulY;
	T m_MulZ;
	T m_X0;
	T m_Y0;
	T m_Z0;
	T m_Invert;
	T m_Type;
	T m_BoxPow;
	T m_InternalScatter;
};

/// <summary>
/// falloff2.
/// </summary>
template <typename T>
class EMBER_API Falloff2Variation : public ParametricVariation<T>
{
public:
	Falloff2Variation(T weight = 1.0) : ParametricVariation<T>("falloff2", VAR_FALLOFF2, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Falloff2Variation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		const v4T random(rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		const T distA = sqrt(Sqr(helper.In.x - m_X0) + Sqr(helper.In.y - m_Y0) + Sqr(helper.In.z - m_Z0));
		const T distB = m_Invert != 0 ? min<T>(1 - distA, 0) : min<T>(distA, 0);
		const T dist = min<T>((distB - m_MinDist) * m_RMax, 0);

		switch ((int)m_Type)
		{
			case 0://Linear.
				{
					helper.Out.x = helper.In.x + m_MulX * random.x * dist;
					helper.Out.y = helper.In.y + m_MulY * random.y * dist;
					helper.Out.z = helper.In.z + m_MulZ * random.z * dist;
					outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + m_MulC * random.w * dist, 1));
				}
				break;
			case 1://Radial.
				if (helper.In.x == 0 && helper.In.y == 0 && helper.In.z == 0)
				{
					helper.Out.x = helper.In.x;
					helper.Out.y = helper.In.y;
					helper.Out.z = helper.In.z;
				}
				else
				{
					const T rIn = sqrt(helper.m_PrecalcSumSquares + SQR(helper.In.z));
					const T sigma = asin(helper.In.z / rIn) + m_MulZ * random.z * dist;
					const T phi = helper.m_PrecalcAtanyx + m_MulY * random.y * dist;
					const T r = rIn + m_MulX * random.x * dist;
					const T sigmas = sin(sigma);
					const T sigmac = cos(sigma);
					const T phis = sin(phi);
					const T phic = cos(phi);

					helper.Out.x = r * sigmac * phic;
					helper.Out.y = r * sigmac * phis;
					helper.Out.z = r * sigmas;
					outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + m_MulC * random.w * dist, 1));
				}
				break;
			case 2://Gaussian.
				{
					const T sigma = dist * random.y * M_2PI;
					const T phi = dist * random.z * T(M_PI);
					const T rad = dist * random.x;
					const T sigmas = sin(sigma);
					const T sigmac = cos(sigma);
					const T phis = sin(phi);
					const T phic = cos(phi);

					helper.Out.x = helper.In.x + m_MulX * rad * sigmac * phic;
					helper.Out.y = helper.In.y + m_MulY * rad * sigmac * phis;
					helper.Out.z = helper.In.z + m_MulZ * rad * sigmas;
					outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + m_MulC * random.w * dist, 1));
				}
				break;
		}
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string scatter = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string minDist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulX    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulY    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulZ    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulC    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z0      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invert  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string type    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rMax    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"

		   << "\t\tconst real_t randx = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t randy = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t randz = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t randc = MwcNextNeg1Pos1(mwc);\n"
		   << "\t\tconst real_t distA = sqrt(Sqr(vIn.x - " << x0 << ") + Sqr(vIn.y - " << y0 << ") + Sqr(vIn.z - " << z0 << "));\n"
		   << "\t\tconst real_t distB = " << invert << " != 0 ? min(1 - distA, 0.0) : min(distA, 0.0);\n"
		   << "\t\tconst real_t dist = min((distB - " << minDist << ") * " << rMax<< ", 0.0);\n"
		   << "\n"
		   << "\t\tswitch ((int)" << type << ")\n"
		   << "\t\t{\n"
		   << "\t\t   case 0:\n"
		   << "\t\t	   vOut.x = vIn.x + " << mulX << " * randx * dist;\n"
		   << "\t\t	   vOut.y = vIn.y + " << mulY << " * randy * dist;\n"
		   << "\t\t	   vOut.z = vIn.z + " << mulZ << " * randz * dist;\n"
		   << "\t\t	   outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + " << mulC << " * randc * dist, 1.0));\n"
		   << "\t\t	   break;\n"
		   << "\t\t   case 1:\n"
		   << "\t\t	   if (vIn.x == 0 && vIn.y == 0 && vIn.z == 0)\n"
		   << "\t\t	   {\n"
		   << "\t\t		   vOut.x = vIn.x;\n"
		   << "\t\t		   vOut.y = vIn.y;\n"
		   << "\t\t		   vOut.z = vIn.z;\n"
		   << "\t\t	   }\n"
		   << "\t\t	   else\n"
		   << "\t\t	   {\n"
		   << "\t\t		   real_t rIn = sqrt(precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\t		   real_t sigma = asin(vIn.z / rIn) + " << mulZ << " * randz * dist;\n"
		   << "\t\t		   real_t phi = precalcAtanyx + " << mulY << " * randy * dist;\n"
		   << "\t\t		   real_t r = rIn + " << mulX << " * randx * dist;\n"
		   << "\t\t		   real_t sigmas = sin(sigma);\n"
		   << "\t\t		   real_t sigmac = cos(sigma);\n"
		   << "\t\t		   real_t phis = sin(phi);\n"
		   << "\t\t		   real_t phic = cos(phi);\n"
		   << "\n"
		   << "\t\t		   vOut.x = r * sigmac * phic;\n"
		   << "\t\t		   vOut.y = r * sigmac * phis;\n"
		   << "\t\t		   vOut.z = r * sigmas;\n"
		   << "\t\t		   outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + " << mulC << " * randc * dist, 1.0));\n"
		   << "\t\t	   }\n"
		   << "\t\t	   break;\n"
		   << "\t\t   case 2:\n"
		   << "\t\t	  {\n"
		   << "\t\t	   real_t sigma = dist * randy * M_2PI;\n"
		   << "\t\t	   real_t phi = dist * randz * M_PI;\n"
		   << "\t\t	   real_t rad = dist * randx;\n"
		   << "\t\t	   real_t sigmas = sin(sigma);\n"
		   << "\t\t	   real_t sigmac = cos(sigma);\n"
		   << "\t\t	   real_t phis = sin(phi);\n"
		   << "\t\t	   real_t phic = cos(phi);\n"
		   << "\n"
		   << "\t\t	   vOut.x = vIn.x + " << mulX << " * rad * sigmac * phic;\n"
		   << "\t\t	   vOut.y = vIn.y + " << mulY << " * rad * sigmac * phis;\n"
		   << "\t\t	   vOut.z = vIn.z + " << mulZ << " * rad * sigmas;\n"
		   << "\t\t	   outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + " << mulC << " * randc * dist, 1.0));\n"
		   << "\t\t	   break;\n"
		   << "\t\t	  }\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_RMax = T(0.04) * m_Scatter;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scatter, prefix + "falloff2_scatter", 1, REAL, EPS6, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_MinDist, prefix + "falloff2_mindist", T(0.5), REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_MulX,    prefix + "falloff2_mul_x", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,    prefix + "falloff2_mul_y", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulZ,    prefix + "falloff2_mul_z", 0, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulC,    prefix + "falloff2_mul_c", 0, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,      prefix + "falloff2_x0"));
		m_Params.push_back(ParamWithName<T>(&m_Y0,      prefix + "falloff2_y0"));
		m_Params.push_back(ParamWithName<T>(&m_Z0,      prefix + "falloff2_z0"));
		m_Params.push_back(ParamWithName<T>(&m_Invert,  prefix + "falloff2_invert", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Type,    prefix + "falloff2_type", 0, INTEGER, 0, 2));
		m_Params.push_back(ParamWithName<T>(true, &m_RMax, prefix + "falloff2_rmax"));
	}

private:
	T m_Scatter;
	T m_MinDist;
	T m_MulX;
	T m_MulY;
	T m_MulZ;
	T m_MulC;
	T m_X0;
	T m_Y0;
	T m_Z0;
	T m_Invert;
	T m_Type;
	T m_RMax;
};

/// <summary>
/// falloff3.
/// </summary>
template <typename T>
class EMBER_API Falloff3Variation : public ParametricVariation<T>
{
public:
	Falloff3Variation(T weight = 1.0) : ParametricVariation<T>("falloff3", VAR_FALLOFF3, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Falloff3Variation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		const v4T random(rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		T radius;

		switch ((int)m_BlurShape)
		{
			case 0://Circle.
				radius = sqrt(Sqr(helper.In.x - m_CenterX) + Sqr(helper.In.y - m_CenterY) + Sqr(helper.In.z - m_CenterZ));
				break;
			case 1://Square.
				radius = min(fabs(helper.In.x - m_CenterX), min(fabs(helper.In.y - m_CenterY), (fabs(helper.In.z - m_CenterZ))));
				break;
		}

		const T dist = min<T>(((m_InvertDistance != 0 ? min<T>(1 - radius, 0) : min<T>(radius, 0)) - m_MinDistance) * m_RMax, 0);

		switch ((int)m_BlurType)
		{
		case 0://Gaussian.
			{
				const T sigma = dist * random.y * M_2PI;
				const T phi = dist * random.z * T(M_PI);
				const T rad = dist * random.x;
				const T sigmas = sin(sigma);
				const T sigmac = cos(sigma);
				const T phis = sin(phi);
				const T phic = cos(phi);

				helper.Out.x = helper.In.x + m_MulX * rad * sigmac * phic;
				helper.Out.y = helper.In.y + m_MulY * rad * sigmac * phis;
				helper.Out.z = helper.In.z + m_MulZ * rad * sigmas;
				outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + m_MulC * random.w * dist, 1));
			}
			break;
		case 1://Radial.
			if (helper.In.x == 0 && helper.In.y == 0 && helper.In.z == 0)
			{
				helper.Out.x = helper.In.x;
				helper.Out.y = helper.In.y;
				helper.Out.z = helper.In.z;
			}
			else
			{
				const T rIn = sqrt(helper.m_PrecalcSumSquares + SQR(helper.In.z));
				const T sigma = asin(helper.In.z / rIn) + m_MulZ * random.z * dist;
				const T phi = helper.m_PrecalcAtanyx + m_MulY * random.y * dist;
				const T r = rIn + m_MulX * random.x * dist;
				const T sigmas = sin(sigma);
				const T sigmac = cos(sigma);
				const T phis = sin(phi);
				const T phic = cos(phi);

				helper.Out.x = r * sigmac * phic;
				helper.Out.y = r * sigmac * phis;
				helper.Out.z = r * sigmas;
				outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + m_MulC * random.w * dist, 1));
			}
			break;
		case 2://Log.
			{
				const T coeff = m_RMax <= EPS6 ? dist : dist + m_Alpha * (LogMap(dist) - dist);

				helper.Out.x = helper.In.x + LogMap(m_MulX) * LogScale(random.x) * coeff,
				helper.Out.y = helper.In.y + LogMap(m_MulY) * LogScale(random.y) * coeff,
				helper.Out.z = helper.In.z + LogMap(m_MulZ) * LogScale(random.z) * coeff,
				outPoint.m_ColorX = fabs(fmod(outPoint.m_ColorX + LogMap(m_MulC) * LogScale(random.w) * coeff, 1));
			}
			break;
		}
	}

	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string blurType     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurShape    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurStrength = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string minDist      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invertDist   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulX         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulY         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulZ         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulC         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerX      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerY      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerZ      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rMax         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		    << "\t\tconst real_t randx = MwcNextNeg1Pos1(mwc);\n"
		    << "\t\tconst real_t randy = MwcNextNeg1Pos1(mwc);\n"
		    << "\t\tconst real_t randz = MwcNextNeg1Pos1(mwc);\n"
		    << "\t\tconst real_t randc = MwcNextNeg1Pos1(mwc);\n"
		    << "\t\treal_t radius;\n"
		    << "\n"
		    << "\t\tswitch ((int)" << blurShape << ")\n"
		    << "\t\t{\n"
		    << "\t\t	case 0:\n"
		    << "\t\t		radius = sqrt(Sqr(vIn.x - " << centerX << ") + Sqr(vIn.y - " << centerY << ") + Sqr(vIn.z - " << centerZ << "));\n"
		    << "\t\t		break;\n"
		    << "\t\t	case 1:\n"
		    << "\t\t		radius = min(fabs(vIn.x - " << centerX << "), min(fabs(vIn.y - " << centerY << "), (fabs(vIn.z - " << centerZ << "))));\n"
		    << "\t\t		break;\n"
		    << "\t\t}\n"
		    << "\n"
		    << "\t\tconst real_t dist = min(((" << invertDist << " != 0 ? min(1 - radius, 0.0) : min(radius, 0.0)) - " << minDist << ") * " << rMax << ", 0.0);\n"
		    << "\n"
		    << "\t\tswitch ((int)" << blurType << ")\n"
		    << "\t\t{\n"
		    << "\t\tcase 0:\n"
		    << "\t\t	{\n"
		    << "\t\t		real_t sigma = dist * randy * M_2PI;\n"
		    << "\t\t		real_t phi = dist * randz * M_PI;\n"
		    << "\t\t		real_t rad = dist * randx;\n"
		    << "\t\t		real_t sigmas = sin(sigma);\n"
		    << "\t\t		real_t sigmac = cos(sigma);\n"
		    << "\t\t		real_t phis = sin(phi);\n"
		    << "\t\t		real_t phic = cos(phi);\n"
		    << "\n"
		    << "\t\t		vOut.x = vIn.x + " << mulX << " * rad * sigmac * phic;\n"
		    << "\t\t		vOut.y = vIn.y + " << mulY << " * rad * sigmac * phis;\n"
		    << "\t\t		vOut.z = vIn.z + " << mulZ << " * rad * sigmas;\n"
		    << "\t\t		outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + " << mulC << " * randc * dist, 1.0));\n"
		    << "\t\t	}\n"
		    << "\t\t	break;\n"
		    << "\t\tcase 1:\n"
		    << "\t\t	if (vIn.x == 0 && vIn.y == 0 && vIn.z == 0)\n"
		    << "\t\t	{\n"
		    << "\t\t		vOut.x = vIn.x;\n"
		    << "\t\t		vOut.y = vIn.y;\n"
		    << "\t\t		vOut.z = vIn.z;\n"
		    << "\t\t	}\n"
		    << "\t\t	else\n"
		    << "\t\t	{\n"
		    << "\t\t		real_t rIn = sqrt(precalcSumSquares + SQR(vIn.z));\n"
		    << "\t\t		real_t sigma = asin(vIn.z / rIn) + " << mulZ << " * randz * dist;\n"
		    << "\t\t		real_t phi = precalcAtanyx + " << mulY << " * randy * dist;\n"
		    << "\t\t		real_t r = rIn + " << mulX << " * randx * dist;\n"
		    << "\t\t		real_t sigmas = sin(sigma);\n"
		    << "\t\t		real_t sigmac = cos(sigma);\n"
		    << "\t\t		real_t phis = sin(phi);\n"
		    << "\t\t		real_t phic = cos(phi);\n"
		    << "\n"
		    << "\t\t		vOut.x = r * sigmac * phic;\n"
		    << "\t\t		vOut.y = r * sigmac * phis;\n"
		    << "\t\t		vOut.z = r * sigmas;\n"
		    << "\t\t		outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + " << mulC << " * randc * dist, 1.0));\n"
		    << "\t\t	}\n"
		    << "\t\t	break;\n"
		    << "\t\tcase 2:\n"
		    << "\t\t	{\n"
		    << "\t\t		real_t coeff = " << rMax << " <= EPS6 ? dist : dist + " << alpha << " * (LogMap(dist) - dist);\n"
		    << "\n"
		    << "\t\t		vOut.x = vIn.x + LogMap(" << mulX << ") * LogScale(randx) * coeff,\n"
		    << "\t\t		vOut.y = vIn.y + LogMap(" << mulY << ") * LogScale(randy) * coeff,\n"
		    << "\t\t		vOut.z = vIn.z + LogMap(" << mulZ << ") * LogScale(randz) * coeff,\n"
		    << "\t\t		outPoint->m_ColorX = fabs(fmod(outPoint->m_ColorX + LogMap(" << mulC << ") * LogScale(randc) * coeff, 1.0));\n"
		    << "\t\t	}\n"
		    << "\t\t	break;\n"
		    << "\t\t}\n"
			<< "\t}\n";

		return ss.str();
	}

	virtual void Precalc()
	{
		m_RMax = T(0.04) * m_BlurStrength;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurType,       prefix + "falloff3_blur_type", 0, INTEGER, 0, 3));
		m_Params.push_back(ParamWithName<T>(&m_BlurShape,      prefix + "falloff3_blur_shape", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_BlurStrength,   prefix + "falloff3_blur_strength", 1, REAL, EPS6, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_MinDistance,    prefix + "falloff3_min_distance", T(0.5), REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_InvertDistance, prefix + "falloff3_invert_distance", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulX,           prefix + "falloff3_mul_x", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,           prefix + "falloff3_mul_y", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulZ,           prefix + "falloff3_mul_z", 0, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_MulC,           prefix + "falloff3_mul_c", 0, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_CenterX,        prefix + "falloff3_center_x"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY,        prefix + "falloff3_center_y"));
		m_Params.push_back(ParamWithName<T>(&m_CenterZ,        prefix + "falloff3_center_z"));
		m_Params.push_back(ParamWithName<T>(&m_Alpha,          prefix + "falloff3_alpha"));
		m_Params.push_back(ParamWithName<T>(true, &m_RMax, prefix + "falloff3_rmax"));
	}

private:
	T m_BlurType;
	T m_BlurShape;
	T m_BlurStrength;
	T m_MinDistance;
	T m_InvertDistance;
	T m_MulX;
	T m_MulY;
	T m_MulZ;
	T m_MulC;
	T m_CenterX;
	T m_CenterY;
	T m_CenterZ;
	T m_Alpha;
	T m_RMax;
};

/// <summary>
/// xtrb.
/// </summary>
template <typename T>
class EMBER_API XtrbVariation : public ParametricVariation<T>
{
public:
	XtrbVariation(T weight = 1.0) : ParametricVariation<T>("xtrb", VAR_XTRB, weight)
	{
		Init();
	}

	PARVARCOPY(XtrbVariation)

	void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		int m, n;
		T alpha, beta, offsetAl, offsetBe, offsetGa, x, y;

		//Transfer to trilinear coordinates, normalized to real distances from triangle sides.
		DirectTrilinear(helper.In.x, helper.In.y, alpha, beta);

		m = Floor<T>(alpha / m_S2a);
		offsetAl = alpha - m * m_S2a;
		n = Floor<T>(beta / m_S2b);
		offsetBe = beta - n * m_S2b;
		offsetGa = m_S2c - m_Ac * offsetAl - m_Bc * offsetBe;

		if (offsetGa > 0)
		{
			Hex(offsetAl, offsetBe, offsetGa, alpha, beta, rand);
		}
		else
		{
			offsetAl = m_S2a - offsetAl;
			offsetBe = m_S2b - offsetBe;
			offsetGa = -offsetGa;
			Hex(offsetAl, offsetBe, offsetGa, alpha, beta, rand);
			alpha = m_S2a - alpha;
			beta  = m_S2b - beta;
		}

		alpha += m * m_S2a;
		beta  += n * m_S2b;

		InverseTrilinear(alpha, beta, x, y, rand);
		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
	}
	
	virtual string OpenCLString()
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinC   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosC   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ha     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hb     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hc     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ab     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ac     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ba     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bc     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ca     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cb     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2a    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2b    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2c    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2ab   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2ac   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2bc   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absN   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		
		ss << "\t{\n"
		   << "\t\tint m, n;\n"
		   << "\t\treal_t alpha, beta, offsetAl, offsetBe, offsetGa, x, y;\n"
		   << "\n"
		   << "\t\t{\n"//DirectTrilinear function extracted out here.
		   << "\t\t	alpha = vIn.y + " << radius << ";\n"
		   << "\t\t	beta = vIn.x * " << sinC << " - vIn.y * " << cosC << " + " << radius << ";\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tm = floor(alpha / " << s2a << ");\n"
		   << "\t\toffsetAl = alpha - m * " << s2a << ";\n"
		   << "\t\tn = floor(beta / " << s2b << ");\n"
		   << "\t\toffsetBe = beta - n * " << s2b << ";\n"
		   << "\t\toffsetGa = " << s2c << " - " << ac << " * offsetAl - " << bc << " * offsetBe;\n"
		   << "\n"
		   << "\t\tif (offsetGa > 0)\n"
		   << "\t\t{\n"
		   << "\n"
		   << "\t\t	Hex(offsetAl, offsetBe, offsetGa,\n"
		   << "\t\t		" << width << ",\n"
		   << "\t\t		" << ha << ",\n"
		   << "\t\t		" << hb << ",\n"
		   << "\t\t		" << hc << ",\n"
		   << "\t\t		" << ab << ",\n"
		   << "\t\t		" << ba << ",\n"
		   << "\t\t		" << bc << ",\n"
		   << "\t\t		" << ca << ",\n"
		   << "\t\t		" << cb << ",\n"
		   << "\t\t		" << s2a << ",\n"
		   << "\t\t		" << s2b << ",\n"
		   << "\t\t		" << s2ab << ",\n"
		   << "\t\t		" << s2ac << ",\n"
		   << "\t\t		" << s2bc << ",\n"
		   << "\t\t		" << width1 << ",\n"
		   << "\t\t		" << width2 << ",\n"
		   << "\t\t		" << width3 << ",\n"
		   << "\t\t		&alpha, &beta, mwc);\n"
		   << "\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	offsetAl = " << s2a << " - offsetAl;\n"
		   << "\t\t	offsetBe = " << s2b << " - offsetBe;\n"
		   << "\t\t	offsetGa = -offsetGa;\n"
		   << "\n"
		   << "\t\t	Hex(offsetAl, offsetBe, offsetGa,\n"
		   << "\t\t		" << width << ",\n"
		   << "\t\t		" << ha << ",\n"
		   << "\t\t		" << hb << ",\n"
		   << "\t\t		" << hc << ",\n"
		   << "\t\t		" << ab << ",\n"
		   << "\t\t		" << ba << ",\n"
		   << "\t\t		" << bc << ",\n"
		   << "\t\t		" << ca << ",\n"
		   << "\t\t		" << cb << ",\n"
		   << "\t\t		" << s2a << ",\n"
		   << "\t\t		" << s2b << ",\n"
		   << "\t\t		" << s2ab << ",\n"
		   << "\t\t		" << s2ac << ",\n"
		   << "\t\t		" << s2bc << ",\n"
		   << "\t\t		" << width1 << ",\n"
		   << "\t\t		" << width2 << ",\n"
		   << "\t\t		" << width3 << ",\n"
		   << "\t\t		&alpha, &beta, mwc);\n"
		   << "\n"
		   << "\t\t	alpha = " << s2a << " - alpha;\n"
		   << "\t\t	beta  = " << s2b << " - beta;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\talpha += m * " << s2a << ";\n"
		   << "\t\tbeta  += n * " << s2b << ";\n"
		   << "\n"
		   << "\t\t{\n"//InverseTrilinear function extracted out here.
		   << "\t\t	real_t inx = (beta - " << radius << " + (alpha - " << radius << ") * " << cosC << ") / " << sinC << ";\n"
		   << "\t\t	real_t iny = alpha - " << radius << ";\n"
		   << "\t\t	real_t angle = (atan2(iny, inx) + M_2PI * MwcNextRange(mwc, (int)" << absN << ")) / " << power << ";\n"
		   << "\t\t	real_t r = xform->m_VariationWeights[" << varIndex << "] * pow(SQR(inx) + SQR(iny), " << cn << ");\n"
		   << "\n"
		   << "\t\t	x = r * cos(angle);\n"
		   << "\t\t	y = r * sin(angle);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"\n"
			"void Hex(real_t al, real_t be, real_t ga,\n"
			"	real_t width,\n"
			"	real_t ha,\n"
			"	real_t hb,\n"
			"	real_t hc,\n"
			"	real_t ab,\n"
			"	real_t ba,\n"
			"	real_t bc,\n"
			"	real_t ca,\n"
			"	real_t cb,\n"
			"	real_t s2a,\n"
			"	real_t s2b,\n"
			"	real_t s2ab,\n"
			"	real_t s2ac,\n"
			"	real_t s2bc,\n"
			"	real_t width1,\n"
			"	real_t width2,\n"
			"	real_t width3,\n"
			"	real_t* al1, real_t* be1, uint2* mwc)\n"
			"{\n"
			"	real_t ga1, de1, r = MwcNext01(mwc);\n"
			"\n"
			"	if (be < al)\n"
			"	{\n"
			"		if (ga < be)\n"
			"		{\n"
			"			if (r >= width3)\n"
			"			{\n"
			"				de1 = width * be;\n"
			"				ga1 = width * ga;\n"
			"			}\n"
			"			else\n"
			"			{\n"
			"				ga1 = width1 * ga + width2 * hc * ga / be;\n"
			"				de1 = width1 * be + width2 * s2ab * (3 - ga / be);\n"
			"			}\n"
			"\n"
			"			*al1 = s2a - ba * de1 - ca * ga1;\n"
			"			*be1 = de1;\n"
			"		}\n"
			"		else\n"
			"		{\n"
			"			if (ga < al)\n"
			"			{\n"
			"				if (r >= width3)\n"
			"				{\n"
			"					ga1 = width * ga;\n"
			"					de1 = width * be;\n"
			"				}\n"
			"				else\n"
			"				{\n"
			"					de1 = width1 * be + width2 * hb * be / ga;\n"
			"					ga1 = width1 * ga + width2 * s2ac * (3 - be / ga);\n"
			"				}\n"
			"\n"
			"				*al1 = s2a - ba * de1 - ca * ga1;\n"
			"				*be1 = de1;\n"
			"			}\n"
			"			else\n"
			"			{\n"
			"				if (r >= width3)\n"
			"				{\n"
			"					*al1 = width * al;\n"
			"					*be1 = width * be;\n"
			"				}\n"
			"				else\n"
			"				{\n"
			"					*be1 = width1 * be + width2 * hb * be / al;\n"
			"					*al1 = width1 * al + width2 * s2ac * (3 - be / al);\n"
			"				}\n"
			"			}\n"
			"		}\n"
			"	}\n"
			"	else\n"
			"	{\n"
			"		if (ga < al)\n"
			"		{\n"
			"			if (r >= width3)\n"
			"			{\n"
			"				de1 = width * al;\n"
			"				ga1 = width * ga;\n"
			"			}\n"
			"			else\n"
			"			{\n"
			"				ga1 = width1 * ga + width2 * hc * ga / al;\n"
			"				de1 = width1 * al + width2 * s2ab * (3 - ga / al);\n"
			"			}\n"
			"\n"
			"			*be1 = s2b - ab * de1 - cb * ga1;\n"
			"			*al1 = de1;\n"
			"		}\n"
			"		else\n"
			"		{\n"
			"			if (ga < be)\n"
			"			{\n"
			"				if (r >= width3)\n"
			"				{\n"
			"					ga1 = width * ga;\n"
			"					de1 = width * al;\n"
			"				}\n"
			"				else\n"
			"				{\n"
			"					de1 = width1 * al + width2 * ha * al / ga;\n"
			"					ga1 = width1 * ga + width2 * s2bc * (3 - al / ga);\n"
			"				}\n"
			"\n"
			"				*be1 = s2b - ab * de1 - cb * ga1;\n"
			"				*al1 = de1;\n"
			"			}\n"
			"			else\n"
			"			{\n"
			"				if (r >= width3)\n"
			"				{\n"
			"					*be1 = width * be;\n"
			"					*al1 = width * al;\n"
			"				}\n"
			"				else\n"
			"				{\n"
			"					*al1 = width1 * al + width2 * ha * al / be;\n"
			"					*be1 = width1 * be + width2 * s2bc * (3 - al / be);\n"
			"				}\n"
			"			}\n"
			"		}\n"
			"	}\n"
			"}\n"
			"\n"
			;
	}

	virtual void Precalc()
	{
		T s2, sinA2, cosA2, sinB2, cosB2, sinC2, cosC2;
		T br = T(0.047) + m_A;
		T cr = T(0.047) + m_B;
		T ar = T(M_PI) - br - cr;
		T temp = ar / 2;

		sincos(temp, &sinA2, &cosA2);
		temp = br / 2;
		sincos(temp, &sinB2, &cosB2);
		temp = cr / 2;
		sincos(temp, &sinC2, &cosC2);
		sincos(cr, &m_SinC, &m_CosC);

		T a = m_Radius * (sinC2 / cosC2 + sinB2 / cosB2);
		T b = m_Radius * (sinC2 / cosC2 + sinA2 / cosA2);
		T c = m_Radius * (sinB2 / cosB2 + sinA2 / cosA2);

		m_Width1 = 1 - m_Width;
		m_Width2 = 2 * m_Width;
		m_Width3 = 1 - m_Width * m_Width;
		s2 = m_Radius * (a + b + c);

		m_Ha = s2 / a / 6;
		m_Hb = s2 / b / 6;
		m_Hc = s2 / c / 6;

		m_Ab = a / b;// a div on b
		m_Ac = a / c;
		m_Ba = b / a;
		m_Bc = b / c;
		m_Ca = c / a;
		m_Cb = c / b;
		m_S2a = 6 * m_Ha;
		m_S2b = 6 * m_Hb;
		m_S2c = 6 * m_Hc;
		m_S2bc = s2 / (b + c) / 6;
		m_S2ab = s2 / (a + b) / 6;
		m_S2ac = s2 / (a + c) / 6;

		if (m_Power == 0)
			m_Power = 2;

		m_AbsN = T((int)fabs(m_Power));
		m_Cn = m_Dist / m_Power / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,  prefix + "xtrb_power", 2, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "xtrb_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Width,  prefix + "xtrb_width", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Dist,   prefix + "xtrb_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_A,      prefix + "xtrb_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B,      prefix + "xtrb_b", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_SinC, prefix   + "xtrb_sinc"));//Precalcs.
		m_Params.push_back(ParamWithName<T>(true, &m_CosC, prefix   + "xtrb_cosc"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ha, prefix     + "xtrb_ha"));
		m_Params.push_back(ParamWithName<T>(true, &m_Hb, prefix     + "xtrb_hb"));
		m_Params.push_back(ParamWithName<T>(true, &m_Hc, prefix     + "xtrb_hc"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ab, prefix     + "xtrb_ab"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ac, prefix     + "xtrb_ac"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ba, prefix     + "xtrb_ba"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bc, prefix     + "xtrb_bc"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ca, prefix     + "xtrb_ca"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cb, prefix     + "xtrb_cb"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2a, prefix    + "xtrb_s2a"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2b, prefix    + "xtrb_s2b"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2c, prefix    + "xtrb_s2c"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2ab, prefix   + "xtrb_s2ab"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2ac, prefix   + "xtrb_s2ac"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2bc, prefix   + "xtrb_s2bc"));
		m_Params.push_back(ParamWithName<T>(true, &m_Width1, prefix + "xtrb_width1"));
		m_Params.push_back(ParamWithName<T>(true, &m_Width2, prefix + "xtrb_width2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Width3, prefix + "xtrb_width3"));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix   + "xtrb_absn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix     + "xtrb_cn"));
	}
	
private:
	inline void DirectTrilinear(T x, T y, T& al, T& be)
	{
		al = y + m_Radius;
		be = x * m_SinC - y * m_CosC + m_Radius;
	}

	inline void InverseTrilinear(T al, T be, T& x, T& y, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T inx = (be - m_Radius + (al - m_Radius) * m_CosC) / m_SinC;
		T iny = al - m_Radius;
		T angle = (atan2(iny, inx) + M_2PI * (rand.Rand((int)m_AbsN))) / m_Power;
		T r = m_Weight * pow(SQR(inx) + SQR(iny), m_Cn);

		x = r * cos(angle);
		y = r * sin(angle);
	}
	
	void Hex(T al, T be, T ga, T& al1, T& be1, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T ga1, de1, r = rand.Frand01<T>();

		if (be < al)
		{
			if (ga < be)
			{
				if (r >= m_Width3)
				{
					de1 = m_Width * be;
					ga1 = m_Width * ga;
				}
				else
				{
					ga1 = m_Width1 * ga + m_Width2 * m_Hc * ga / be;
					de1 = m_Width1 * be + m_Width2 * m_S2ab * (3 - ga / be);
				}

				al1 = m_S2a - m_Ba * de1 - m_Ca * ga1;
				be1 = de1;
			}
			else
			{
				if (ga < al)
				{
					if (r >= m_Width3)
					{
						ga1 = m_Width * ga;
						de1 = m_Width * be;
					}
					else
					{
						de1 = m_Width1 * be + m_Width2 * m_Hb * be / ga;
						ga1 = m_Width1 * ga + m_Width2 * m_S2ac * (3 - be / ga);
					}
			
					al1 = m_S2a - m_Ba * de1 - m_Ca * ga1;
					be1 = de1;
				}
				else
				{
					if (r >= m_Width3)
					{
						al1 = m_Width * al;
						be1 = m_Width * be;
					}
					else
					{
						be1 = m_Width1 * be + m_Width2 * m_Hb * be / al;
						al1 = m_Width1 * al + m_Width2 * m_S2ac * (3 - be / al);
					}
				}
			}
		}
		else
		{
			if (ga < al)
			{
				if (r >= m_Width3)
				{
					de1 = m_Width * al;
					ga1 = m_Width * ga;
				}
				else
				{
					ga1 = m_Width1 * ga + m_Width2 * m_Hc * ga / al;
					de1 = m_Width1 * al + m_Width2 * m_S2ab * (3 - ga / al);
				}
		
				be1 = m_S2b - m_Ab * de1 - m_Cb * ga1;
				al1 = de1;
			}
			else
			{
				if (ga < be)
				{
					if (r >= m_Width3)
					{
						ga1 = m_Width * ga;
						de1 = m_Width * al;
					}
					else
					{
						de1 = m_Width1 * al + m_Width2 * m_Ha * al / ga;
						ga1 = m_Width1 * ga + m_Width2 * m_S2bc * (3 - al / ga);
					}
		
					be1 = m_S2b - m_Ab * de1 - m_Cb * ga1;
					al1 = de1;
				}
				else
				{
					if (r >= m_Width3)
					{
						be1 = m_Width * be;
						al1 = m_Width * al;
					}
					else
					{
						al1 = m_Width1 * al + m_Width2 * m_Ha * al / be;
						be1 = m_Width1 * be + m_Width2 * m_S2bc * (3 - al / be);
					}
				}
			}
		}
	}

	T m_Power;
	T m_Radius;
	T m_Width;
	T m_Dist;
	T m_A;
	T m_B;
	T m_SinC;//Precalcs.
	T m_CosC;
	T m_Ha;
	T m_Hb;
	T m_Hc;
	T m_Ab;
	T m_Ac;
	T m_Ba;
	T m_Bc;
	T m_Ca;
	T m_Cb;
	T m_S2a;
	T m_S2b;
	T m_S2c;
	T m_S2ab;
	T m_S2ac;
	T m_S2bc;
	T m_Width1;
	T m_Width2;
	T m_Width3;
	T m_AbsN;
	T m_Cn;
};

MAKEPREPOSTPARVAR(Bubble2, bubble2, BUBBLE2)
MAKEPREPOSTPARVAR(CircleLinear, CircleLinear, CIRCLELINEAR)
MAKEPREPOSTPARVARASSIGN(CircleRand, CircleRand, CIRCLERAND, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(CircleTrans1, CircleTrans1, CIRCLETRANS1)
MAKEPREPOSTPARVAR(Cubic3D, cubic3D, CUBIC3D)
MAKEPREPOSTPARVAR(CubicLattice3D, cubicLattice_3D, CUBIC_LATTICE3D)
MAKEPREPOSTVAR(Foci3D, foci_3D, FOCI3D)
MAKEPREPOSTPARVAR(Ho, ho, HO)
MAKEPREPOSTPARVAR(Julia3Dq, julia3Dq, JULIA3DQ)
MAKEPREPOSTPARVARASSIGN(Line, line, LINE, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Loonie3D, loonie_3D, LOONIE3D)
MAKEPREPOSTPARVAR(Mcarpet, mcarpet, MCARPET)
MAKEPREPOSTPARVAR(Waves23D, waves2_3D, WAVES23D)
MAKEPREPOSTPARVARASSIGN(Pie3D, pie3D, PIE3D, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Popcorn23D, popcorn2_3D, POPCORN23D)
MAKEPREPOSTVAR(Sinusoidal3D, sinusoidal3D, SINUSOIDAL3D)
MAKEPREPOSTPARVAR(Scry3D, scry_3D, SCRY3D)
MAKEPREPOSTPARVAR(Shredlin, shredlin, SHRED_LIN)
MAKEPREPOSTPARVAR(SplitBrdr, SplitBrdr, SPLIT_BRDR)
MAKEPREPOSTVAR(Wdisc, wdisc, WDISC)
MAKEPREPOSTPARVAR(Falloff, falloff, FALLOFF)
MAKEPREPOSTPARVAR(Falloff2, falloff2, FALLOFF2)
MAKEPREPOSTPARVAR(Falloff3, falloff3, FALLOFF3)
MAKEPREPOSTPARVAR(Xtrb, xtrb, XTRB)
}
