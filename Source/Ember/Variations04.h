#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// eSwirl.
/// </summary>
template <typename T>
class EMBER_API ESwirlVariation : public ParametricVariation<T>
{
public:
	ESwirlVariation(T weight = 1.0) : ParametricVariation<T>("eSwirl", VAR_ESWIRL, weight, true)
	{
		Init();
	}

	PARVARCOPY(ESwirlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * T(0.5);

		ClampGteRef<T>(xmax, -1);

		T mu = acosh(xmax);
		T nu = acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		nu = nu + mu * m_Out + m_In / mu;

		helper.Out.x = m_Weight * cosh(mu) * cos(nu);
		helper.Out.y = m_Weight * sinh(mu) * sin(nu);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string in  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string out = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * 0.5;\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(Clamp(vIn.x / xmax, -1.0, 1.0));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tnu = nu + mu * " << out << " + " << in << " / mu;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_In,  prefix + "eSwirl_in"));
		m_Params.push_back(ParamWithName<T>(&m_Out, prefix + "eSwirl_out"));
	}

private:
	T m_In;
	T m_Out;
};

/// <summary>
/// lazyTravis.
/// </summary>
template <typename T>
class EMBER_API LazyTravisVariation : public ParametricVariation<T>
{
public:
	LazyTravisVariation(T weight = 1.0) : ParametricVariation<T>("lazyTravis", VAR_LAZY_TRAVIS, weight)
	{
		Init();
	}

	PARVARCOPY(LazyTravisVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = fabs(helper.In.x);
		T y = fabs(helper.In.y);
		T s;
		T p;
		T x2, y2;

		if (x > m_Weight || y > m_Weight)
		{
			if (x > y)
			{
				s = x;

				if (helper.In.x > 0)
					p = s + helper.In.y + s * m_Out4;
				else
					p = 5 * s - helper.In.y + s * m_Out4;
			}
			else
			{
				s = y;

				if (helper.In.y > 0)
					p = 3 * s - helper.In.x + s * m_Out4;
				else
					p = 7 * s + helper.In.x + s * m_Out4;
			}

			p = fmod(p, s * 8);

			if (p <= 2 * s)
			{
				x2 = s + m_Space;
				y2 = -(1 * s - p);
				y2 = y2 + y2 / s * m_Space;
			}
			else if (p <= 4 * s)
			{
				y2 = s + m_Space;
				x2 = (3 * s - p);
				x2 = x2 + x2 / s * m_Space;
			}
			else if (p <= 6 * s)
			{
				x2 = -(s + m_Space);
				y2 = (5 * s - p);
				y2 = y2 + y2 / s * m_Space;
			}
			else
			{
				y2 = -(s + m_Space);
				x2 = -(7 * s - p);
				x2 = x2 + x2 / s * m_Space;
			}

			helper.Out.x = m_Weight * x2;
			helper.Out.y = m_Weight * y2;
		}
		else
		{
			if (x > y)
			{
				s = x;

				if (helper.In.x > 0)
					p = s + helper.In.y + s * m_In4;
				else
					p = 5 * s - helper.In.y + s * m_In4;
			}
			else
			{
				s = y;

				if (helper.In.y > 0)
					p = 3 * s - helper.In.x + s * m_In4;
				else
					p = 7 * s + helper.In.x + s * m_In4;
			}

			p = fmod(p, s * 8);

			if (p <= 2 * s)
			{
				helper.Out.x = m_Weight * s;
				helper.Out.y = -(m_Weight * (s - p));
			}
			else if (p <= 4 * s)
			{
				helper.Out.x = m_Weight * (3 * s - p);
				helper.Out.y = m_Weight * s;
			}
			else if (p <= 6 * s)
			{
				helper.Out.x = -(m_Weight * s);
				helper.Out.y = m_Weight * (5 * s - p);
			}
			else
			{
				helper.Out.x = -(m_Weight * (7 * s - p));
				helper.Out.y = -(m_Weight * s);
			}
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string spinIn  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spinOut = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string space   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string in4     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string out4    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = fabs(vIn.x);\n"
		   << "\t\treal_t y = fabs(vIn.y);\n"
		   << "\t\treal_t s;\n"
		   << "\t\treal_t p;\n"
		   << "\t\treal_t x2, y2;\n"
		   << "\n"
		   << "\t\tif (x > xform->m_VariationWeights[" << varIndex << "] || y > xform->m_VariationWeights[" << varIndex << "])\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > y)\n"
		   << "\t\t	{\n"
		   << "\t\t		s = x;\n"
		   << "\n"
		   << "\t\t		if (vIn.x > 0)\n"
		   << "\t\t			p = s + vIn.y + s * " << out4 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			p = 5 * s - vIn.y + s * " << out4 << ";\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		s = y;\n"
		   << "\n"
		   << "\t\t		if (vIn.y > 0)\n"
		   << "\t\t			p = 3 * s - vIn.x + s * " << out4 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			p = 7 * s + vIn.x + s * " << out4 << ";\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	p = fmod(p, s * 8);\n"
		   << "\n"
		   << "\t\t	if (p <= 2 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		x2 = s + " << space << ";\n"
		   << "\t\t		y2 = -(1 * s - p);\n"
		   << "\t\t		y2 = y2 + y2 / s * " << space << ";\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (p <= 4 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		y2 = s + " << space << ";\n"
		   << "\t\t		x2 = (3 * s - p);\n"
		   << "\t\t		x2 = x2 + x2 / s * " << space << ";\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (p <= 6 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		x2 = -(s + " << space << ");\n"
		   << "\t\t		y2 = (5 * s - p);\n"
		   << "\t\t		y2 = y2 + y2 / s * " << space << ";\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		y2 = -(s + " << space << ");\n"
		   << "\t\t		x2 = -(7 * s - p);\n"
		   << "\t\t		x2 = x2 + x2 / s * " << space << ";\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * x2;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * y2;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > y)\n"
		   << "\t\t	{\n"
		   << "\t\t		s = x;\n"
		   << "\n"
		   << "\t\t		if (vIn.x > 0)\n"
		   << "\t\t			p = s + vIn.y + s * " << in4 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			p = 5 * s - vIn.y + s * " << in4 << ";\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		s = y;\n"
		   << "\n"
		   << "\t\t		if (vIn.y > 0)\n"
		   << "\t\t			p = 3 * s - vIn.x + s * " << in4 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			p = 7 * s + vIn.x + s * " << in4 << ";\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	p = fmod(p, s * 8);\n"
		   << "\n"
		   << "\t\t	if (p <= 2 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * s;\n"
		   << "\t\t		vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * (s - p));\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (p <= 4 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (3 * s - p);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * s;\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (p <= 6 * s)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * s);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * (5 * s - p);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * (7 * s - p));\n"
		   << "\t\t		vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * s);\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_In4 = 4 * m_SpinIn;
		m_Out4 = 4 * m_SpinOut;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_SpinIn,  prefix + "lazyTravis_spin_in",  1, REAL_CYCLIC, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_SpinOut, prefix + "lazyTravis_spin_out", 0, REAL_CYCLIC, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_Space,   prefix + "lazyTravis_space"));
		m_Params.push_back(ParamWithName<T>(true, &m_In4,  prefix + "lazyTravis_in4"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Out4, prefix + "lazyTravis_out4"));
	}

private:
	T m_SpinIn;
	T m_SpinOut;
	T m_Space;
	T m_In4;//Precalc.
	T m_Out4;
};

/// <summary>
/// squish.
/// </summary>
template <typename T>
class EMBER_API SquishVariation : public ParametricVariation<T>
{
public:
	SquishVariation(T weight = 1.0) : ParametricVariation<T>("squish", VAR_SQUISH, weight)
	{
		Init();
	}

	PARVARCOPY(SquishVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = fabs(helper.In.x);
		T y = fabs(helper.In.y);
		T s;
		T p;

		if (x > y)
		{
			s = x;

			if (helper.In.x > 0)
				p = helper.In.y;
			else
				p = 4 * s - helper.In.y;
		}
		else
		{
			s = y;

			if (helper.In.y > 0)
				p = 2 * s - helper.In.x;
			else
				p = 6 * s + helper.In.x;
		}

		p = m_InvPower * (p + 8 * s * Floor<T>(m_Power * rand.Frand01<T>()));

		if (p <= s)
		{
			helper.Out.x = m_Weight * s;
			helper.Out.y = m_Weight * p;
		}
		else if (p <= 3 * s)
		{
			helper.Out.x = m_Weight * (2 * s - p);
			helper.Out.y = m_Weight * s;
		}
		else if (p <= 5 * s)
		{
			helper.Out.x = -(m_Weight * s);
			helper.Out.y = m_Weight * (4 * s - p);
		}
		else if (p <= 7 * s)
		{
			helper.Out.x = -(m_Weight * (6 * s - p));
			helper.Out.y = -(m_Weight * s);
		}
		else
		{
			helper.Out.x = m_Weight * s;
			helper.Out.y = (m_Weight * (8 * s - p));
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = fabs(vIn.x);\n"
		   << "\t\treal_t y = fabs(vIn.y);\n"
		   << "\t\treal_t s;\n"
		   << "\t\treal_t p;\n"
		   << "\n"
		   << "\t\tif (x > y)\n"
		   << "\t\t{\n"
		   << "\t\t	s = x;\n"
		   << "\n"
		   << "\t\t	if (vIn.x > 0)\n"
		   << "\t\t		p = vIn.y;\n"
		   << "\t\t	else\n"
		   << "\t\t		p = 4 * s - vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	s = y;\n"
		   << "\n"
		   << "\t\t	if (vIn.y > 0)\n"
		   << "\t\t		p = 2 * s - vIn.x;\n"
		   << "\t\t	else\n"
		   << "\t\t		p = 6 * s + vIn.x;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tp = " << invPower << " * (p + 8 * s * floor(" << power << " * MwcNext01(mwc)));\n"
		   << "\n"
		   << "\t\tif (p <= s)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * s;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * p;\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 3 * s)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (2 * s - p);\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * s;\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 5 * s)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * s);\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (4 * s - p);\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 7 * s)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * (6 * s - p));\n"
		   << "\t\t	vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * s);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * s;\n"
		   << "\t\t	vOut.y = -(xform->m_VariationWeights[" << varIndex << "] * (8 * s - p));\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_InvPower = 1 / m_Power;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "squish_power", 2, INTEGER, 2, T(INT_MAX)));
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower, prefix + "squish_inv_power"));//Precalc.
	}

private:
	T m_Power;
	T m_InvPower;//Precalc.
};

/// <summary>
/// circus.
/// </summary>
template <typename T>
class EMBER_API CircusVariation : public ParametricVariation<T>
{
public:
	CircusVariation(T weight = 1.0) : ParametricVariation<T>("circus", VAR_CIRCUS, weight, true, true, true)
	{
		Init();
	}

	PARVARCOPY(CircusVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;

		if (r <= 1)
			r *= m_Scale;
		else
			r *= m_InvScale;

		helper.Out.x = m_Weight * r * helper.m_PrecalcCosa;
		helper.Out.y = m_Weight * r * helper.m_PrecalcSina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string scale    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invScale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (r <= 1)\n"
		   << "\t\t	r *= " << scale << ";\n"
		   << "\t\telse\n"
		   << "\t\t	r *= " << invScale << ";\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * r * precalcCosa;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * r * precalcSina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_InvScale = 1 / m_Scale;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "circus_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_InvScale, prefix + "circus_inv_power"));//Precalc.
	}

private:
	T m_Scale;
	T m_InvScale;//Precalc.
};

/// <summary>
/// tancos.
/// </summary>
template <typename T>
class EMBER_API TancosVariation : public Variation<T>
{
public:
	TancosVariation(T weight = 1.0) : Variation<T>("tancos", VAR_TANCOS, weight, true) { }

	VARCOPY(TancosVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = Zeps(helper.m_PrecalcSumSquares);

		helper.Out.x = (m_Weight / d) * (tanh(d) * (2 * helper.In.x));
		helper.Out.y = (m_Weight / d) * (cos(d)  * (2 * helper.In.y));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t d = Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] / d) * (tanh(d) * (2.0 * vIn.x));\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] / d) * (cos(d)  * (2.0 * vIn.y));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// rippled.
/// </summary>
template <typename T>
class EMBER_API RippledVariation : public Variation<T>
{
public:
	RippledVariation(T weight = 1.0) : Variation<T>("rippled", VAR_RIPPLED, weight, true) { }

	VARCOPY(RippledVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = Zeps(helper.m_PrecalcSumSquares);

		helper.Out.x = (m_Weight / 2) * (tanh(d) * (2 * helper.In.x));
		helper.Out.y = (m_Weight / 2) * (cos(d)  * (2 * helper.In.y));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t d = Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = (xform->m_VariationWeights[" << varIndex << "] / 2.0) * (tanh(d) * (2.0 * vIn.x));\n"
		   << "\t\tvOut.y = (xform->m_VariationWeights[" << varIndex << "] / 2.0) * (cos(d)  * (2.0 * vIn.y));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// RotateX.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API RotateXVariation : public ParametricVariation<T>
{
public:
	RotateXVariation(T weight = 1.0) : ParametricVariation<T>("rotate_x", VAR_ROTATE_X, weight)
	{
		Init();
	}

	PARVARCOPY(RotateXVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T z = m_RxCos * helper.In.z - m_RxSin * helper.In.y;

		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.x = 0;
			outPoint.m_X = helper.In.x;
		}
		else
		{
			helper.Out.x = helper.In.x;
		}

		helper.Out.y = m_RxSin * helper.In.z + m_RxCos * helper.In.y;
		helper.Out.z = z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rxSin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string rxCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t z = " << rxCos << " * vIn.z - " << rxSin << " * vIn.y;\n"
		   << "\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.x = 0;\n"
			"\t\toutPoint->m_X = vIn.x;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.x = vIn.x;\n";
		}

		ss << "\t\tvOut.y = " << rxSin << " * vIn.z + " << rxCos << " * vIn.y;\n"
		   << "\t\tvOut.z = z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_RxSin = sin(m_Weight * T(M_PI_2));
		m_RxCos = cos(m_Weight * T(M_PI_2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_RxSin, prefix + "rotate_x_sin"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_RxCos, prefix + "rotate_x_cos"));//Original used a prefix of rx_, which is incompatible with Ember's design.
	}

private:
	T m_RxSin;
	T m_RxCos;
};

/// <summary>
/// RotateY.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API RotateYVariation : public ParametricVariation<T>
{
public:
	RotateYVariation(T weight = 1.0) : ParametricVariation<T>("rotate_y", VAR_ROTATE_Y, weight)
	{
		Init();
	}

	PARVARCOPY(RotateYVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_RyCos * helper.In.x - m_RySin * helper.In.z;

		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.y = 0;
			outPoint.m_Y = helper.In.y;
		}
		else
		{
			helper.Out.y = helper.In.y;
		}

		helper.Out.z = m_RySin * helper.In.x + m_RyCos * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rySin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string ryCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = " << ryCos << " * vIn.x - " << rySin << " * vIn.z;\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.y = 0;\n"
			"\t\toutPoint->m_Y = vIn.y;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.y = vIn.y;\n";
		}

		ss << "\t\tvOut.z = " << rySin << " * vIn.x + " << ryCos << " * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_RySin = sin(m_Weight * T(M_PI_2));
		m_RyCos = cos(m_Weight * T(M_PI_2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_RySin, prefix + "rotate_y_sin"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_RyCos, prefix + "rotate_y_cos"));//Original used a prefix of ry_, which is incompatible with Ember's design.
	}

private:
	T m_RySin;
	T m_RyCos;
};

/// <summary>
/// RotateZ.
/// This was originally pre and post spin_z, consolidated here to be consistent with the other rotate variations.
/// </summary>
template <typename T>
class EMBER_API RotateZVariation : public ParametricVariation<T>
{
public:
	RotateZVariation(T weight = 1.0) : ParametricVariation<T>("rotate_z", VAR_ROTATE_Z, weight)
	{
		Init();
	}

	PARVARCOPY(RotateZVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_RzSin * helper.In.y + m_RzCos * helper.In.x;
		helper.Out.y = m_RzCos * helper.In.y - m_RzSin * helper.In.x;

		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.z = 0;
			outPoint.m_Z = helper.In.z;
		}
		else
		{
			helper.Out.z = helper.In.z;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rzSin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string rzCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = " << rzSin << " * vIn.y + " << rzCos << " * vIn.x;\n"
		   << "\t\tvOut.y = " << rzCos << " * vIn.y - " << rzSin << " * vIn.x;\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.z = 0;\n"
			"\t\toutPoint->m_Z = vIn.z;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.z = vIn.z;\n";
		}

		ss << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_RzSin = sin(m_Weight * T(M_PI_2));
		m_RzCos = cos(m_Weight * T(M_PI_2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_RzSin, prefix + "rotate_z_sin"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_RzCos, prefix + "rotate_z_cos"));
	}

private:
	T m_RzSin;
	T m_RzCos;
};

/// <summary>
/// MirrorX.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API MirrorXVariation : public Variation<T>
{
public:
	MirrorXVariation(T weight = 1.0) : Variation<T>("mirror_x", VAR_MIRROR_X, weight) { }

	VARCOPY(MirrorXVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.x = fabs(outPoint.m_X);

			if (rand.RandBit())
				helper.Out.x = -helper.Out.x;

			helper.Out.y = 0;
			helper.Out.z = 0;
			outPoint.m_X = 0;//Flipped x will be added.
		}
		else
		{
			helper.Out.x = fabs(helper.In.x);

			if (rand.RandBit())
				helper.Out.x = -helper.Out.x;

			helper.Out.y = helper.In.y;
			helper.Out.z = helper.In.z;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;

		ss << "\t{\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.x = fabs(outPoint->m_X);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.x = -vOut.x;\n"
			"\n"
			"\t\tvOut.y = 0;\n"
			"\t\tvOut.z = 0;\n"
			"\t\toutPoint->m_X = 0;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.x = fabs(vIn.x);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.x = -vOut.x;\n"
			"\n"
			"\t\tvOut.y = vIn.y;\n"
			"\t\tvOut.z = vIn.z;\n";
		}

		ss << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// MirrorY.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API MirrorYVariation : public Variation<T>
{
public:
	MirrorYVariation(T weight = 1.0) : Variation<T>("mirror_y", VAR_MIRROR_Y, weight) { }

	VARCOPY(MirrorYVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.y = fabs(outPoint.m_Y);

			if (rand.RandBit())
				helper.Out.y = -helper.Out.y;

			helper.Out.x = 0;
			helper.Out.z = 0;
			outPoint.m_Y = 0;//Flipped y will be added.
		}
		else
		{
			helper.Out.y = fabs(helper.In.y);

			if (rand.RandBit())
				helper.Out.y = -helper.Out.y;

			helper.Out.x = helper.In.x;
			helper.Out.z = helper.In.z;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;

		ss << "\t{\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.y = fabs(outPoint->m_Y);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.y = -vOut.y;\n"
			"\n"
			"\t\tvOut.x = 0;\n"
			"\t\tvOut.z = 0;\n"
			"\t\toutPoint->m_Y = 0;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.y = fabs(vIn.y);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.y = -vOut.y;\n"
			"\n"
			"\t\tvOut.x = vIn.x;\n"
			"\t\tvOut.z = vIn.z;\n";
		}

		ss << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// MirrorZ.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API MirrorZVariation : public Variation<T>
{
public:
	MirrorZVariation(T weight = 1.0) : Variation<T>("mirror_z", VAR_MIRROR_Z, weight) { }

	VARCOPY(MirrorZVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.z = fabs(outPoint.m_Z);

			if (rand.RandBit())
				helper.Out.z = -helper.Out.z;

			helper.Out.x = 0;
			helper.Out.y = 0;
			outPoint.m_Z = 0;//Flipped z will be added.
		}
		else
		{
			helper.Out.z = fabs(helper.In.z);

			if (rand.RandBit())
				helper.Out.z = -helper.Out.z;

			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;

		ss << "\t{\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss <<
			"\t\tvOut.z = fabs(outPoint->m_Z);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.z = -vOut.z;\n"
			"\n"
			"\t\tvOut.x = 0;\n"
			"\t\tvOut.y = 0;\n"
			"\t\toutPoint->m_Z = 0;\n";
		}
		else
		{
			ss <<
			"\t\tvOut.z = fabs(vIn.z);\n"
			"\n"
			"\t\tif (MwcNext(mwc) & 1)\n"
			"\t\t	vOut.z = -vOut.z;\n"
			"\n"
			"\t\tvOut.x = vIn.x;\n"
			"\t\tvOut.y = vIn.y;\n";
		}

		ss << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// RBlur.
/// </summary>
template <typename T>
class EMBER_API RBlurVariation : public ParametricVariation<T>
{
public:
	RBlurVariation(T weight = 1.0) : ParametricVariation<T>("rblur", VAR_RBLUR, weight)
	{
		Init();
	}

	PARVARCOPY(RBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sx = helper.In.x - m_CenterX;
		T sy = helper.In.y - m_CenterY;
		T r = sqrt(SQR(sx) + SQR(sy)) - m_Offset;

		r = r < 0 ? 0 : r;
		r *= m_S2;

		helper.Out.x = m_Weight * (helper.In.x + (rand.Frand01<T>() - T(0.5)) * r);
		helper.Out.y = m_Weight * (helper.In.y + (rand.Frand01<T>() - T(0.5)) * r);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string strength = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string offset   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string centerX  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string centerY  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string s2       = "parVars[" + ToUpper(m_Params[i++].Name())  + index;

		ss << "\t{\n"
		   << "\t\treal_t sx = vIn.x - " << centerX << ";\n"
		   << "\t\treal_t sy = vIn.y - " << centerY << ";\n"
		   << "\t\treal_t r = sqrt(SQR(sx) + SQR(sy)) - " << offset << ";\n"
		   << "\n"
		   << "\t\tr = r < 0 ? 0 : r;\n"
		   << "\t\tr *= " << s2 << ";\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + (MwcNext01(mwc) - 0.5) * r);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + (MwcNext01(mwc) - 0.5) * r);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_S2 = 2 * m_Strength;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Strength, prefix + "rblur_strength", 1));
		m_Params.push_back(ParamWithName<T>(&m_Offset,   prefix + "rblur_offset", 1));
		m_Params.push_back(ParamWithName<T>(&m_CenterX,  prefix + "rblur_center_x"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY,  prefix + "rblur_center_y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2, prefix + "rblur_s2"));//Precalc.
	}

private:
	T m_Strength;
	T m_Offset;
	T m_CenterX;
	T m_CenterY;
	T m_S2;//Precalc.
};

/// <summary>
/// JuliaNab.
/// </summary>
template <typename T>
class EMBER_API JuliaNabVariation : public ParametricVariation<T>
{
public:
	JuliaNabVariation(T weight = 1.0) : ParametricVariation<T>("juliaNab", VAR_JULIANAB, weight, true)
	{
		Init();
	}

	PARVARCOPY(JuliaNabVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T jun = Zeps(fabs(m_N));

		T a = (atan2(helper.In.y, pow(fabs(helper.In.x), m_Sep)) + M_2PI * Floor<T>(rand.Frand01<T>() * m_AbsN)) / jun;
		T r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn * m_A);

		helper.Out.x = r * cos(a) + m_B;
		helper.Out.y = r * sin(a) + m_B;
		helper.Out.z = m_Weight * helper.In.z;//Original did not multiply by weight. Do it here to be consistent with others.
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string n    = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string a    = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string b    = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string sep  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string absN = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string cn   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;

		ss << "\t{\n"
		   << "\t\treal_t jun = Zeps(fabs(" << n << "));\n"
		   << "\n"
		   << "\t\treal_t a = (atan2(vIn.y, pow(fabs(vIn.x), " << sep << ")) + M_2PI * floor(MwcNext01(mwc) * " << absN << ")) / jun;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << " * " << a << ");\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a) + " << b << ";\n"
		   << "\t\tvOut.y = r * sin(a) + " << b << ";\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T jun = Zeps(fabs(m_N));

		m_AbsN = abs(m_N);
		m_Cn = 1 / jun / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N,   prefix + "juliaNab_n", 1));
		m_Params.push_back(ParamWithName<T>(&m_A,   prefix + "juliaNab_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B,   prefix + "juliaNab_b", 1));
		m_Params.push_back(ParamWithName<T>(&m_Sep, prefix + "juliaNab_separ", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "juliaNab_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn,   prefix + "juliaNab_cn"));
	}

private:
	T m_N;
	T m_A;
	T m_B;
	T m_Sep;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// Sintrange.
/// </summary>
template <typename T>
class EMBER_API SintrangeVariation : public ParametricVariation<T>
{
public:
	SintrangeVariation(T weight = 1.0) : ParametricVariation<T>("sintrange", VAR_SINTRANGE, weight)
	{
		Init();
	}

	PARVARCOPY(SintrangeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sqX = SQR(helper.In.x);
		T sqY = SQR(helper.In.y);
		T v = (sqX + sqY) * m_W;//Do not use precalcSumSquares here because its components are needed below.

		helper.Out.x = m_Weight * sin(helper.In.x) * (sqX + m_W - v);
		helper.Out.y = m_Weight * sin(helper.In.y) * (sqY + m_W - v);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string w = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t sqX = SQR(vIn.x);\n"
		   << "\t\treal_t sqY = SQR(vIn.y);\n"
		   << "\t\treal_t v = (sqX + sqY) * " << w << ";\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x) * (sqX + " << w << " - v);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.y) * (sqY + " << w << " - v);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_W, prefix + "sintrange_w", 1));
	}

private:
	T m_W;
};

/// <summary>
/// Voron.
/// </summary>
template <typename T>
class EMBER_API VoronVariation : public ParametricVariation<T>
{
public:
	VoronVariation(T weight = 1.0) : ParametricVariation<T>("Voron", VAR_VORON, weight)
	{
		Init();
	}

	PARVARCOPY(VoronVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int i, j, l, k, m, m1, n, n1;
		T r, rMin, offsetX, offsetY, x0 = 0, y0 = 0, x, y;

		rMin = 20;
		m = Floor<T>(helper.In.x / m_Step);
		n = Floor<T>(helper.In.y / m_Step);

		for (i = -1; i < 2; i++)
		{
			m1 = m + i;

			for (j = -1; j < 2; j++)
			{
				n1 = n + j;
				k = 1 + Floor<T>(m_Num * DiscreteNoise(int(19 * m1 + 257 * n1 + m_XSeed)));

				for (l = 0; l < k; l++)
				{
					x = T(DiscreteNoise(int(l + 64 * m1 + 15 * n1 + m_XSeed)) + m1) * m_Step;
					y = T(DiscreteNoise(int(l + 21 * m1 + 33 * n1 + m_YSeed)) + n1) * m_Step;
					offsetX = helper.In.x - x;
					offsetY = helper.In.y - y;
					r = sqrt(SQR(offsetX) + SQR(offsetY));

					if (r < rMin)
					{
						rMin = r;
						x0 = x;
						y0 = y;
					}
				}
			}
		}

		helper.Out.x = m_Weight * (m_K * (helper.In.x - x0) + x0);
		helper.Out.y = m_Weight * (m_K * (helper.In.y - y0) + y0);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string m_k   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string step  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string num   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xSeed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ySeed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint i, j, l, k, m, m1, n, n1;\n"
		   << "\t\treal_t r, rMin, offsetX, offsetY, x0 = 0.0, y0 = 0.0, x, y;\n"
		   << "\n"
		   << "\t\trMin = 20;\n"
		   << "\t\tm = (int)floor(vIn.x / " << step << ");\n"
		   << "\t\tn = (int)floor(vIn.y / " << step << ");\n"
		   << "\n"
		   << "\t\tfor (i = -1; i < 2; i++)\n"
		   << "\t\t{\n"
		   << "\t\t	m1 = m + i;\n"
		   << "\n"
		   << "\t\t	for (j = -1; j < 2; j++)\n"
		   << "\t\t	{\n"
		   << "\t\t		n1 = n + j;\n"
		   << "\t\t		k = 1 + (int)floor(" << num << " * VoronDiscreteNoise((int)(19 * m1 + 257 * n1 + " << xSeed << ")));\n"
		   << "\n"
		   << "\t\t		for (l = 0; l < k; l++)\n"
		   << "\t\t		{\n"
		   << "\t\t			x = (real_t)(VoronDiscreteNoise((int)(l + 64 * m1 + 15 * n1 + " << xSeed << ")) + m1) * " << step << ";\n"
		   << "\t\t			y = (real_t)(VoronDiscreteNoise((int)(l + 21 * m1 + 33 * n1 + " << ySeed << ")) + n1) * " << step << ";\n"
		   << "\t\t			offsetX = vIn.x - x;\n"
		   << "\t\t			offsetY = vIn.y - y;\n"
		   << "\t\t			r = sqrt(SQR(offsetX) + SQR(offsetY));\n"
		   << "\n"
		   << "\t\t			if (r < rMin)\n"
		   << "\t\t			{\n"
		   << "\t\t				rMin = r;\n"
		   << "\t\t				x0 = x;\n"
		   << "\t\t				y0 = y;\n"
		   << "\t\t			}\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (" << m_k << " * (vIn.x - x0) + x0);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (" << m_k << " * (vIn.y - y0) + y0);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"real_t VoronDiscreteNoise(int x)\n"
			"{\n"
			"	const real_t im = 2147483647;\n"
			"	const real_t am = (1 / im);\n"
			"\n"
			"	int n = x;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_K,     prefix + "Voron_K", T(0.99)));
		m_Params.push_back(ParamWithName<T>(&m_Step,  prefix + "Voron_Step", T(0.25), REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Num,   prefix + "Voron_Num", 1, INTEGER, 1, 25));
		m_Params.push_back(ParamWithName<T>(&m_XSeed, prefix + "Voron_XSeed", 3, INTEGER));
		m_Params.push_back(ParamWithName<T>(&m_YSeed, prefix + "Voron_YSeed", 7, INTEGER));
	}

private:
	T DiscreteNoise(int x)
	{
		const T im = T(2147483647);
		const T am = (1 / im);

		int n = x;
		n = (n << 13) ^ n;
		return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * am;
	}

	T m_K;//Params.
	T m_Step;
	T m_Num;
	T m_XSeed;
	T m_YSeed;
};

/// <summary>
/// Waffle.
/// </summary>
template <typename T>
class EMBER_API WaffleVariation : public ParametricVariation<T>
{
public:
	WaffleVariation(T weight = 1.0) : ParametricVariation<T>("waffle", VAR_WAFFLE, weight)
	{
		Init();
	}

	PARVARCOPY(WaffleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = 0, r = 0;

		switch (rand.Rand(5))
		{
			case 0:
				a = (rand.Rand(ISAAC_INT(m_Slices)) + rand.Frand01<T>() * m_XThickness) / m_Slices;
				r = (rand.Rand(ISAAC_INT(m_Slices)) + rand.Frand01<T>() * m_YThickness) / m_Slices;
				break;
			case 1:
				a = (rand.Rand(ISAAC_INT(m_Slices)) + rand.Frand01<T>()) / m_Slices;
				r = (rand.Rand(ISAAC_INT(m_Slices)) + m_YThickness)      / m_Slices;
				break;
			case 2:
				a = (rand.Rand(ISAAC_INT(m_Slices)) + m_XThickness)      / m_Slices;
				r = (rand.Rand(ISAAC_INT(m_Slices)) + rand.Frand01<T>()) / m_Slices;
				break;
			case 3:
				a = rand.Frand01<T>();
				r = (rand.Rand(ISAAC_INT(m_Slices)) + m_YThickness + rand.Frand01<T>() * (1 - m_YThickness)) / m_Slices;
				break;
			case 4:
			default:
				a = (rand.Rand(ISAAC_INT(m_Slices)) + m_XThickness + rand.Frand01<T>() * (1 - m_XThickness)) / m_Slices;
				r = rand.Frand01<T>();
				break;
		}

		helper.Out.x = m_CosR * a + m_SinR * r;
		helper.Out.y = -m_SinR * a + m_CosR * r;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string slices     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xThickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yThickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotation   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinr       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosr       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = 0, r = 0;\n"
		   << "\n"
		   << "\t\tswitch (MwcNextRange(mwc, 5))\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		a = (MwcNextRange(mwc, (int)" << slices << ") + MwcNext01(mwc) * " << xThickness << ") / " << slices << ";\n"
		   << "\t\t		r = (MwcNextRange(mwc, (int)" << slices << ") + MwcNext01(mwc) * " << yThickness << ") / " << slices << ";\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		a = (MwcNextRange(mwc, (int)" << slices << ") + MwcNext01(mwc)) / " << slices << ";\n"
		   << "\t\t		r = (MwcNextRange(mwc, (int)" << slices << ") + " << yThickness << ") / " << slices << ";\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t		a = (MwcNextRange(mwc, (int)" << slices << ") + " << xThickness << ") / " << slices << ";\n"
		   << "\t\t		r = (MwcNextRange(mwc, (int)" << slices << ") + MwcNext01(mwc)) / " << slices << ";\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 3:\n"
		   << "\t\t		a = MwcNext01(mwc);\n"
		   << "\t\t		r = (MwcNextRange(mwc, (int)" << slices << ") + " << yThickness << " + MwcNext01(mwc) * (1 - " << yThickness << ")) / " << slices << ";\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 4:\n"
		   << "\t\t		a = (MwcNextRange(mwc, (int)" << slices << ") + " << xThickness << " + MwcNext01(mwc) * (1 - " << xThickness << ")) / " << slices << ";\n"
		   << "\t\t		r = MwcNext01(mwc);\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << cosr << " * a + " << sinr << " * r;\n"
		   << "\t\tvOut.y = -" << sinr << " * a + " << cosr << " * r;\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SinR = sin(m_Rotation);
		m_CosR = cos(m_Rotation);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Slices,     prefix + "waffle_slices", 6, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_XThickness, prefix + "waffle_xthickness", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_YThickness, prefix + "waffle_ythickness", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Rotation,   prefix + "waffle_rotation"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinR, prefix + "waffle_sinr"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosR, prefix + "waffle_cosr"));
	}

private:
	T m_Slices;
	T m_XThickness;
	T m_YThickness;
	T m_Rotation;
	T m_SinR;//Precalc.
	T m_CosR;
};

/// <summary>
/// Square3D.
/// </summary>
template <typename T>
class EMBER_API Square3DVariation : public Variation<T>
{
public:
	Square3DVariation(T weight = 1.0) : Variation<T>("square3D", VAR_SQUARE3D, weight) { }

	VARCOPY(Square3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.y = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.z = m_Weight * (rand.Frand01<T>() - T(0.5));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - 0.5);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - 0.5);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) - 0.5);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// SuperShape3D.
/// </summary>
template <typename T>
class EMBER_API SuperShape3DVariation : public ParametricVariation<T>
{
public:
	SuperShape3DVariation(T weight = 1.0) : ParametricVariation<T>("SuperShape3D", VAR_SUPER_SHAPE3D, weight)
	{
		Init();
	}

	PARVARCOPY(SuperShape3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T pr1, r1, pr2, r2, rho1, phi1, sinr, sinp, cosr, cosp, msinr, msinp, mcosr, mcosp, temp;

		rho1 = rand.Frand01<T>() * m_Rho2Pi;
		phi1 = rand.Frand01<T>() * m_Phi2Pi;

		if (rand.RandBit())
			phi1 = -phi1;

		sinr = sin(rho1);
		cosr = cos(rho1);

		sinp = sin(phi1);
		cosp = cos(phi1);

		temp = m_M4_1 * rho1;
		msinr = sin(temp);
		mcosr = cos(temp);

		temp = m_M4_2 * phi1;
		msinp = sin(temp);
		mcosp = cos(temp);

		pr1 = m_An2_1 * pow(fabs(mcosr), m_N2_1) + m_Bn3_1 * pow(fabs(msinr), m_N3_1);
		pr2 = m_An2_2 * pow(fabs(mcosp), m_N2_2) + m_Bn3_2 * pow(fabs(msinp), m_N3_2);
		r1 = pow(fabs(pr1), m_N1_1) + m_Spiral * rho1;
		r2 = pow(fabs(pr2), m_N1_2);

		if (int(m_Toroidmap) == 1)
		{
			helper.Out.x = m_Weight * cosr * (r1 + r2 * cosp);
			helper.Out.y = m_Weight * sinr * (r1 + r2 * cosp);
			helper.Out.z = m_Weight * r2 * sinp;
		}
		else
		{
			helper.Out.x = m_Weight * r1 * cosr * r2 * cosp;
			helper.Out.y = m_Weight * r1 * sinr * r2 * cosp;
			helper.Out.z = m_Weight * r2 * sinp;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rho    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string m1     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string m2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a1     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b1     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n1_1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n1_2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n2_1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n2_2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n3_1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n3_2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spiral = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string toroid = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n1n_1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n1n_2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string an2_1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string an2_2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bn3_1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bn3_2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string m4_1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string m4_2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rho2pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi2pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t pr1, r1, pr2, r2, rho1, phi1, sinr, sinp, cosr, cosp, msinr, msinp, mcosr, mcosp, temp;\n"
		   << "\n"
		   << "\t\trho1 = MwcNext01(mwc) * " << rho2pi << ";\n"
		   << "\t\tphi1 = MwcNext01(mwc) * " << phi2pi<< ";\n"
		   << "\n"
		   << "\t\tif (MwcNext(mwc) & 1)\n"
		   << "\t\t	phi1 = -phi1;\n"
		   << "\n"
		   << "\t\tsinr = sin(rho1);\n"
		   << "\t\tcosr = cos(rho1);\n"
		   << "\n"
		   << "\t\tsinp = sin(phi1);\n"
		   << "\t\tcosp = cos(phi1);\n"
		   << "\n"
		   << "\t\ttemp = " << m4_1<< " * rho1;\n"
		   << "\t\tmsinr = sin(temp);\n"
		   << "\t\tmcosr = cos(temp);\n"
		   << "\n"
		   << "\t\ttemp = " << m4_2 << " * phi1;\n"
		   << "\t\tmsinp = sin(temp);\n"
		   << "\t\tmcosp = cos(temp);\n"
		   << "\n"
		   << "\t\tpr1 = " << an2_1 << " * pow(fabs(mcosr), " << n2_1 << ") + " << bn3_1 << " * pow(fabs(msinr), " << n3_1 << ");\n"
		   << "\t\tpr2 = " << an2_2 << " * pow(fabs(mcosp), " << n2_2 << ") + " << bn3_2 << " * pow(fabs(msinp), " << n3_2 << ");\n"
		   << "\t\tr1 = pow(fabs(pr1), " << n1_1 << ") + " << spiral << " * rho1;\n"
		   << "\t\tr2 = pow(fabs(pr2), " << n1_2 << ");\n"
		   << "\n"
		   << "\t\tif ((int)" << toroid << " == 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * cosr * (r1 + r2 * cosp);\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * sinr * (r1 + r2 * cosp);\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * r2 * sinp;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * r1 * cosr * r2 * cosp;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * r1 * sinr * r2 * cosp;\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * r2 * sinp;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_N1n_1 = (-1 / m_N1_1);
		m_N1n_2 = (-1 / m_N1_2);
		m_An2_1 = pow(fabs(1 / m_A1), m_N2_1);
		m_An2_2 = pow(fabs(1 / m_A2), m_N2_2);
		m_Bn3_1 = pow(fabs(1 / m_B1), m_N3_1);
		m_Bn3_2 = pow(fabs(1 / m_B2), m_N3_2);
		m_M4_1 = m_M1 / 4;
		m_M4_2 = m_M2 / 4;
		m_Rho2Pi = m_Rho * T(M_2_PI);
		m_Phi2Pi = m_Phi * T(M_2_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rho,       prefix + "SuperShape3D_rho", T(9.9)));
		m_Params.push_back(ParamWithName<T>(&m_Phi,       prefix + "SuperShape3D_phi", T(2.5)));
		m_Params.push_back(ParamWithName<T>(&m_M1,        prefix + "SuperShape3D_m1", 6));
		m_Params.push_back(ParamWithName<T>(&m_M2,        prefix + "SuperShape3D_m2", 3));
		m_Params.push_back(ParamWithName<T>(&m_A1,        prefix + "SuperShape3D_a1", 1));
		m_Params.push_back(ParamWithName<T>(&m_A2,        prefix + "SuperShape3D_a2", 1));
		m_Params.push_back(ParamWithName<T>(&m_B1,        prefix + "SuperShape3D_b1", 1));
		m_Params.push_back(ParamWithName<T>(&m_B2,        prefix + "SuperShape3D_b2", 1));
		m_Params.push_back(ParamWithName<T>(&m_N1_1,      prefix + "SuperShape3D_n1_1", 1));
		m_Params.push_back(ParamWithName<T>(&m_N1_2,      prefix + "SuperShape3D_n1_2", 1));
		m_Params.push_back(ParamWithName<T>(&m_N2_1,      prefix + "SuperShape3D_n2_1", 1));
		m_Params.push_back(ParamWithName<T>(&m_N2_2,      prefix + "SuperShape3D_n2_2", 1));
		m_Params.push_back(ParamWithName<T>(&m_N3_1,      prefix + "SuperShape3D_n3_1", 1));
		m_Params.push_back(ParamWithName<T>(&m_N3_2,      prefix + "SuperShape3D_n3_2", 1));
		m_Params.push_back(ParamWithName<T>(&m_Spiral,    prefix + "SuperShape3D_spiral"));
		m_Params.push_back(ParamWithName<T>(&m_Toroidmap, prefix + "SuperShape3D_toroidmap", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_N1n_1,  prefix + "SuperShape3D_n1n1"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_N1n_2,  prefix + "SuperShape3D_n1n2"));
		m_Params.push_back(ParamWithName<T>(true, &m_An2_1,  prefix + "SuperShape3D_an21"));
		m_Params.push_back(ParamWithName<T>(true, &m_An2_2,  prefix + "SuperShape3D_an22"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bn3_1,  prefix + "SuperShape3D_bn31"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bn3_2,  prefix + "SuperShape3D_bn32"));
		m_Params.push_back(ParamWithName<T>(true, &m_M4_1,   prefix + "SuperShape3D_m41"));
		m_Params.push_back(ParamWithName<T>(true, &m_M4_2,   prefix + "SuperShape3D_m42"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rho2Pi, prefix + "SuperShape3D_rho2pi"));
		m_Params.push_back(ParamWithName<T>(true, &m_Phi2Pi, prefix + "SuperShape3D_phi2pi"));
	}

private:
	T m_Rho;
	T m_Phi;
	T m_M1;
	T m_M2;
	T m_A1;
	T m_A2;
	T m_B1;
	T m_B2;
	T m_N1_1;
	T m_N1_2;
	T m_N2_1;
	T m_N2_2;
	T m_N3_1;
	T m_N3_2;
	T m_Spiral;
	T m_Toroidmap;
	T m_N1n_1;//Precalc.
	T m_N1n_2;
	T m_An2_1;
	T m_An2_2;
	T m_Bn3_1;
	T m_Bn3_2;
	T m_M4_1;
	T m_M4_2;
	T m_Rho2Pi;
	T m_Phi2Pi;
};

/// <summary>
/// sphyp3D.
/// </summary>
template <typename T>
class EMBER_API Sphyp3DVariation : public ParametricVariation<T>
{
public:
	Sphyp3DVariation(T weight = 1.0) : ParametricVariation<T>("sphyp3D", VAR_SPHYP3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Sphyp3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t, rX, rY, rZ;

		t  = Zeps(helper.m_PrecalcSumSquares + SQR(helper.In.z));
		rX = m_Weight / pow(t, m_StretchX);
		rY = m_Weight / pow(t, m_StretchY);

		helper.Out.x = helper.In.x * rX;
		helper.Out.y = helper.In.y * rY;

		//Optional 3D calculation.
		if (int(m_ZOn) == 1)
		{
			rZ = m_Weight / pow(t, m_StretchZ);
			helper.Out.z = helper.In.z * rZ;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string stretchX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string stretchY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string stretchZ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zOn      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t, rX, rY, rZ;\n"
		   << "\n"
		   << "\t\tt  = Zeps(precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\trX = xform->m_VariationWeights[" << varIndex << "] / pow(t, " << stretchX << ");\n"
		   << "\t\trY = xform->m_VariationWeights[" << varIndex << "] / pow(t, " << stretchY << ");\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * rX;\n"
		   << "\t\tvOut.y = vIn.y * rY;\n"
		   << "\n"
		   << "\t\tif ((int)" << zOn << " == 1)\n"
		   << "\t\t{\n"
		   << "\t\trZ = xform->m_VariationWeights[" << varIndex << "] / pow(t, " << stretchZ << ");\n"
		   << "\n"
		   << "\t\tvOut.z = vIn.z * rZ;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_StretchX, prefix + "sphyp3D_stretchX", 1));
		m_Params.push_back(ParamWithName<T>(&m_StretchY, prefix + "sphyp3D_stretchY", 1));
		m_Params.push_back(ParamWithName<T>(&m_StretchZ, prefix + "sphyp3D_stretchZ", 1));
		m_Params.push_back(ParamWithName<T>(&m_ZOn,      prefix + "sphyp3D_zOn", 1, INTEGER, 0, 1));
	}

private:
	T m_StretchX;
	T m_StretchY;
	T m_StretchZ;
	T m_ZOn;
};

/// <summary>
/// circlecrop.
/// </summary>
template <typename T>
class EMBER_API CirclecropVariation : public ParametricVariation<T>
{
public:
	CirclecropVariation(T weight = 1.0) : ParametricVariation<T>("circlecrop", VAR_CIRCLECROP, weight)
	{
		Init();
	}

	PARVARCOPY(CirclecropVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xi = helper.In.x - m_X;//Original altered the input pointed to for reg, but not for pre/post. Don't do that here.
		T yi = helper.In.y - m_Y;

		const T rad = sqrt(SQR(xi) + SQR(yi));
		const T ang = atan2(yi, xi);
		const T rdc = m_Radius + (rand.Frand01<T>() * T(0.5) * m_Ca);
		const T s = sin(ang);
		const T c = cos(ang);

		const int esc = rad > m_Radius;
		const int cr0 = int(m_Zero);

		if (cr0 &&  esc)
		{
			if (m_VarType == VARTYPE_PRE)
				helper.m_TransX = helper.m_TransY = 0;
			else
				outPoint.m_X = outPoint.m_Y = 0;

			helper.Out.x = helper.Out.y = 0;
			helper.Out.z = m_Weight * helper.In.z;
		}
		else if (cr0 && !esc)
		{
			helper.Out.x = m_Weight * xi + m_X;
			helper.Out.y = m_Weight * yi + m_Y;
			helper.Out.z = m_Weight * helper.In.z;
		}
		else if (!cr0 &&  esc)
		{
			helper.Out.x = m_Weight * rdc * c + m_X;
			helper.Out.y = m_Weight * rdc * s + m_Y;
			helper.Out.z = m_Weight * helper.In.z;
		}
		else if (!cr0 && !esc)
		{
			helper.Out.x = m_Weight * xi + m_X;
			helper.Out.y = m_Weight * yi + m_Y;
			helper.Out.z = m_Weight * helper.In.z;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string radius      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scatterArea = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zero        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ca          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
			<< "\t\treal_t xi = vIn.x - " << x << ";\n"
			<< "\t\treal_t yi = vIn.y - " << y << ";\n"
			<< "\n"
			<< "\t\tconst real_t rad = sqrt(SQR(xi) + SQR(yi));\n"
			<< "\t\tconst real_t ang = atan2(yi, xi);\n"
			<< "\t\tconst real_t rdc = " << radius << " + (MwcNext01(mwc) * 0.5 * " << ca << "); \n"
			<< "\t\tconst real_t s = sin(ang);\n"
			<< "\t\tconst real_t c = cos(ang);\n"
			<< "\n"
			<< "\t\tconst int esc = rad > " << radius << ";\n"
			<< "\t\tconst int cr0 = (int)" << zero << ";\n"
			<< "\n"
			<< "\t\tif (cr0 &&  esc)\n"
			<< "\t\t{\n";

		if (m_VarType == VARTYPE_PRE)
			ss << "\t\t	transX = transY = 0;\n";
		else
			ss << "\t\t	outPoint->m_X = outPoint->m_Y = 0;\n";

		   ss
		   << "\t\t	vOut.x = vOut.y = 0;\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t\telse if (cr0 && !esc)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * xi + " << x << ";\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * yi + " << y << ";\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t\telse if (!cr0 &&  esc)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * rdc * c + " << x << ";\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * rdc * s + " << y << ";\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t\telse if (!cr0 && !esc)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * xi + " << x << ";\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * yi + " << y << ";\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ca = Clamp<T>(m_ScatterArea, -1, 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius,      prefix + "circlecrop_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_X,           prefix + "circlecrop_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y,           prefix + "circlecrop_y"));
		m_Params.push_back(ParamWithName<T>(&m_ScatterArea, prefix + "circlecrop_scatter_area"));
		m_Params.push_back(ParamWithName<T>(&m_Zero,        prefix + "circlecrop_zero", 1, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ca,    prefix + "circlecrop_ca"));
	}

private:
	T m_Radius;
	T m_X;
	T m_Y;
	T m_ScatterArea;
	T m_Zero;
	T m_Ca;//Precalc.
};

/// <summary>
/// julian3Dx.
/// </summary>
template <typename T>
class EMBER_API Julian3DxVariation : public ParametricVariation<T>
{
public:
	Julian3DxVariation(T weight = 1.0) : ParametricVariation<T>("julian3Dx", VAR_JULIAN3DX, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(Julian3DxVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T z = helper.In.z / m_AbsN;
		const T radiusOut = m_Weight * pow(helper.m_PrecalcSumSquares + z * z, m_Cn);
		const T x = m_A * helper.In.x + m_B * helper.In.y + m_E;
		const T y = m_C * helper.In.x + m_D * helper.In.y + m_F;
		const T tempRand = T(int(rand.Frand01<T>() * m_AbsN));
		const T alpha = (atan2(y, x) + M_2PI * tempRand) / m_Power;
		const T gamma = radiusOut * helper.m_PrecalcSqrtSumSquares;

		helper.Out.x = gamma * cos(alpha);
		helper.Out.y = gamma * sin(alpha);
		helper.Out.z = radiusOut * z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string e     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst real_t z = vIn.z / " << absn << ";\n"
		   << "\t\tconst real_t radiusOut = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares + z * z, " << cn << ");\n"
		   << "\t\tconst real_t x = " << a << " * vIn.x + " << b << " * vIn.y + " << e << ";\n"
		   << "\t\tconst real_t y = " << c << " * vIn.x + " << d << " * vIn.y + " << f << ";\n"
		   << "\t\tconst real_t rand = (int)(MwcNext01(mwc) * " << absn << ");\n"
		   << "\t\tconst real_t alpha = (atan2(y, x) + M_2PI * rand) / " << power << ";\n"
		   << "\t\tconst real_t gamma = radiusOut * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = gamma * cos(alpha);\n"
		   << "\t\tvOut.y = gamma * sin(alpha);\n"
		   << "\t\tvOut.z = radiusOut * z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_AbsN = fabs(m_Power);
		m_Cn = (m_Dist / m_Power - 1) / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "julian3Dx_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "julian3Dx_power", 2, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_A,     prefix + "julian3Dx_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B,     prefix + "julian3Dx_b"));
		m_Params.push_back(ParamWithName<T>(&m_C,     prefix + "julian3Dx_c"));
		m_Params.push_back(ParamWithName<T>(&m_D,     prefix + "julian3Dx_d", 1));
		m_Params.push_back(ParamWithName<T>(&m_E,     prefix + "julian3Dx_e"));
		m_Params.push_back(ParamWithName<T>(&m_F,     prefix + "julian3Dx_f"));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "julian3Dx_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn,   prefix + "julian3Dx_cn"));
	}

private:
	T m_Dist;//Params.
	T m_Power;
	T m_A;
	T m_B;
	T m_C;
	T m_D;
	T m_E;
	T m_F;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// fourth.
/// </summary>
template <typename T>
class EMBER_API FourthVariation : public ParametricVariation<T>
{
public:
	FourthVariation(T weight = 1.0) : ParametricVariation<T>("fourth", VAR_FOURTH, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(FourthVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x > 0 && helper.In.y > 0)//Quadrant IV: spherical.
		{
			T r = 1 / helper.m_PrecalcSqrtSumSquares;

			helper.Out.x = m_Weight * r * cos(helper.m_PrecalcAtanyx);
			helper.Out.y = m_Weight * r * sin(helper.m_PrecalcAtanyx);
		}
		else if (helper.In.x > 0 && helper.In.y < 0)//Quadrant I: loonie.
		{
			T r2 = helper.m_PrecalcSumSquares;

			if (r2 < m_SqrWeight)
			{
				T r = m_Weight * sqrt(m_SqrWeight / r2 - 1);

				helper.Out.x = r * helper.In.x;
				helper.Out.y = r * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}
		else if (helper.In.x < 0 && helper.In.y > 0)//Quadrant III: susan.
		{
			T x = helper.In.x - m_X;
			T y = helper.In.y + m_Y;
			T r = sqrt(SQR(x) + SQR(y));

			if (r < m_Weight)
			{
				T a = atan2(y, x) + m_Spin + m_Twist * (m_Weight - r);

				r *= m_Weight;
				helper.Out.x = r * cos(a) + m_X;
				helper.Out.y = r * sin(a) - m_Y;
			}
			else
			{
				r = m_Weight * (1 + m_Space / Zeps(r));
				helper.Out.x = r * x + m_X;
				helper.Out.y = r * y - m_Y;
			}
		}
		else//Quadrant II: Linear.
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}

		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string spin      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string space     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twist     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sqrWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
			<< "\t\tif (vIn.x > 0 && vIn.y > 0)\n"
			<< "\t\t{\n"
			<< "\t\t	real_t r = 1 / precalcSqrtSumSquares;\n"
			<< "\n"
			<< "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * r * cos(precalcAtanyx);\n"
			<< "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * r * sin(precalcAtanyx);\n"
			<< "\t\t}\n"
			<< "\t\telse if (vIn.x > 0 && vIn.y < 0)\n"
			<< "\t\t{\n"
			<< "\t\t	real_t r2 = precalcSumSquares;\n"
			<< "\n"
			<< "\t\t	if (r2 < " << sqrWeight << ")\n"
			<< "\t\t	{\n"
			<< "\t\t		real_t r = xform->m_VariationWeights[" << varIndex << "] * sqrt(" << sqrWeight << " / r2 - 1);\n"
			<< "\n"
			<< "\t\t		vOut.x = r * vIn.x;\n"
			<< "\t\t		vOut.y = r * vIn.y;\n"
			<< "\t\t	}\n"
			<< "\t\t	else\n"
			<< "\t\t	{\n"
			<< "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
			<< "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
			<< "\t\t	}\n"
			<< "\t\t}\n"
			<< "\t\telse if (vIn.x < 0 && vIn.y > 0)\n"
			<< "\t\t{\n"
			<< "\t\t	real_t x = vIn.x - " << x << ";\n"
			<< "\t\t	real_t y = vIn.y + " << y << ";\n"
			<< "\t\t	real_t r = sqrt(SQR(x) + SQR(y));\n"
			<< "\n"
			<< "\t\t	if (r < xform->m_VariationWeights[" << varIndex << "])\n"
			<< "\t\t	{\n"
			<< "\t\t		real_t a = atan2(y, x) + " << spin << " + " << twist << " * (xform->m_VariationWeights[" << varIndex << "] - r);\n"
			<< "\n"
			<< "\t\t		r *= xform->m_VariationWeights[" << varIndex << "];\n"
			<< "\t\t		vOut.x = r * cos(a) + " << x << ";\n"
			<< "\t\t		vOut.y = r * sin(a) - " << y << ";\n"
			<< "\t\t	}\n"
			<< "\t\t	else\n"
			<< "\t\t	{\n"
			<< "\t\t		r = xform->m_VariationWeights[" << varIndex << "] * (1 + " << space << " / Zeps(r));\n"
			<< "\t\t		vOut.x = r * x + " << x << ";\n"
			<< "\t\t		vOut.y = r * y - " << y << ";\n"
			<< "\t\t	}\n"
			<< "\t\t}\n"
			<< "\t\telse\n"
			<< "\t\t{\n"
			<< "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
			<< "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
			<< "\t\t}\n"
			<< "\n"
			<< "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
			<< "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SqrWeight = SQR(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spin,  prefix + "fourth_spin", T(M_PI), REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Space, prefix + "fourth_space"));
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "fourth_twist"));
		m_Params.push_back(ParamWithName<T>(&m_X,     prefix + "fourth_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y,     prefix + "fourth_y"));
		m_Params.push_back(ParamWithName<T>(true, &m_SqrWeight, prefix + "fourth_sqr_weight"));//Precalc.
	}

private:
	T m_Spin;
	T m_Space;
	T m_Twist;
	T m_X;
	T m_Y;
	T m_SqrWeight;//Precalc.
};

/// <summary>
/// mobiq.
/// </summary>
template <typename T>
class EMBER_API MobiqVariation : public ParametricVariation<T>
{
public:
	MobiqVariation(T weight = 1.0) : ParametricVariation<T>("mobiq", VAR_MOBIQ, weight)
	{
		Init();
	}

	PARVARCOPY(MobiqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T t1 = m_At;
		const T t2 = helper.In.x;
		const T t3 = m_Bt;
		const T t4 = m_Ct;
		const T t5 = m_Dt;
		const T x1 = m_Ax;
		const T x2 = helper.In.y;
		const T x3 = m_Bx;
		const T x4 = m_Cx;
		const T x5 = m_Dx;
		const T y1 = m_Ay;
		const T y2 = helper.In.z;
		const T y3 = m_By;
		const T y4 = m_Cy;
		const T y5 = m_Dy;
		const T z1 = m_Az;
		const T z3 = m_Bz;
		const T z4 = m_Cz;
		const T z5 = m_Dz;

		T nt = t1 * t2 - x1 * x2 - y1 * y2 + t3;
		T nx = t1 * x2 + x1 * t2 - z1 * y2 + x3;
		T ny = t1 * y2 + y1 * t2 + z1 * x2 + y3;
		T nz = z1 * t2 + x1 * y2 - y1 * x2 + z3;
		T dt = t4 * t2 - x4 * x2 - y4 * y2 + t5;
		T dx = t4 * x2 + x4 * t2 - z4 * y2 + x5;
		T dy = t4 * y2 + y4 * t2 + z4 * x2 + y5;
		T dz = z4 * t2 + x4 * y2 - y4 * x2 + z5;
		T ni = m_Weight / (SQR(dt) + SQR(dx) + SQR(dy) + SQR(dz));

		helper.Out.x = (nt * dt + nx * dx + ny * dy + nz * dz) * ni;
		helper.Out.y = (nx * dt - nt * dx - ny * dz + nz * dy) * ni;
		helper.Out.z = (ny * dt - nt * dy - nz * dx + nx * dz) * ni;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string at = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string az = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bt = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string by = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bz = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ct = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dt = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dz = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst real_t t1 = " << at << ";\n"
		   << "\t\tconst real_t t2 = vIn.x;\n"
		   << "\t\tconst real_t t3 = " << bt << ";\n"
		   << "\t\tconst real_t t4 = " << ct << ";\n"
		   << "\t\tconst real_t t5 = " << dt << ";\n"
		   << "\t\tconst real_t x1 = " << ax << ";\n"
		   << "\t\tconst real_t x2 = vIn.y;\n"
		   << "\t\tconst real_t x3 = " << bx << ";\n"
		   << "\t\tconst real_t x4 = " << cx << ";\n"
		   << "\t\tconst real_t x5 = " << dx << ";\n"
		   << "\t\tconst real_t y1 = " << ay << ";\n"
		   << "\t\tconst real_t y2 = vIn.z;\n"
		   << "\t\tconst real_t y3 = " << by << ";\n"
		   << "\t\tconst real_t y4 = " << cy << ";\n"
		   << "\t\tconst real_t y5 = " << dy << ";\n"
		   << "\t\tconst real_t z1 = " << az << ";\n"
		   << "\t\tconst real_t z3 = " << bz << ";\n"
		   << "\t\tconst real_t z4 = " << cz << ";\n"
		   << "\t\tconst real_t z5 = " << dz << ";\n"
		   << "\n"
		   << "\t\treal_t nt = t1 * t2 - x1 * x2 - y1 * y2 + t3;\n"
		   << "\t\treal_t nx = t1 * x2 + x1 * t2 - z1 * y2 + x3;\n"
		   << "\t\treal_t ny = t1 * y2 + y1 * t2 + z1 * x2 + y3;\n"
		   << "\t\treal_t nz = z1 * t2 + x1 * y2 - y1 * x2 + z3;\n"
		   << "\t\treal_t dt = t4 * t2 - x4 * x2 - y4 * y2 + t5;\n"
		   << "\t\treal_t dx = t4 * x2 + x4 * t2 - z4 * y2 + x5;\n"
		   << "\t\treal_t dy = t4 * y2 + y4 * t2 + z4 * x2 + y5;\n"
		   << "\t\treal_t dz = z4 * t2 + x4 * y2 - y4 * x2 + z5;\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (SQR(dt) + SQR(dx) + SQR(dy) + SQR(dz));\n"
		   << "\n"
		   << "\t\tvOut.x = (nt * dt + nx * dx + ny * dy + nz * dz) * ni;\n"
		   << "\t\tvOut.y = (nx * dt - nt * dx - ny * dz + nz * dy) * ni;\n"
		   << "\t\tvOut.z = (ny * dt - nt * dy - nz * dx + nx * dz) * ni;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_At, prefix + "mobiq_at", 1));
		m_Params.push_back(ParamWithName<T>(&m_Ax, prefix + "mobiq_ax"));
		m_Params.push_back(ParamWithName<T>(&m_Ay, prefix + "mobiq_ay"));
		m_Params.push_back(ParamWithName<T>(&m_Az, prefix + "mobiq_az"));
		m_Params.push_back(ParamWithName<T>(&m_Bt, prefix + "mobiq_bt"));
		m_Params.push_back(ParamWithName<T>(&m_Bx, prefix + "mobiq_bx"));
		m_Params.push_back(ParamWithName<T>(&m_By, prefix + "mobiq_by"));
		m_Params.push_back(ParamWithName<T>(&m_Bz, prefix + "mobiq_bz"));
		m_Params.push_back(ParamWithName<T>(&m_Ct, prefix + "mobiq_ct"));
		m_Params.push_back(ParamWithName<T>(&m_Cx, prefix + "mobiq_cx"));
		m_Params.push_back(ParamWithName<T>(&m_Cy, prefix + "mobiq_cy"));
		m_Params.push_back(ParamWithName<T>(&m_Cz, prefix + "mobiq_cz"));
		m_Params.push_back(ParamWithName<T>(&m_Dt, prefix + "mobiq_dt", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dx, prefix + "mobiq_dx"));
		m_Params.push_back(ParamWithName<T>(&m_Dy, prefix + "mobiq_dy"));
		m_Params.push_back(ParamWithName<T>(&m_Dz, prefix + "mobiq_dz"));
	}

private:
	T m_At;
	T m_Ax;
	T m_Ay;
	T m_Az;
	T m_Bt;
	T m_Bx;
	T m_By;
	T m_Bz;
	T m_Ct;
	T m_Cx;
	T m_Cy;
	T m_Cz;
	T m_Dt;
	T m_Dx;
	T m_Dy;
	T m_Dz;
};

/// <summary>
/// spherivoid.
/// </summary>
template <typename T>
class EMBER_API SpherivoidVariation : public ParametricVariation<T>
{
public:
	SpherivoidVariation(T weight = 1.0) : ParametricVariation<T>("spherivoid", VAR_SPHERIVOID, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(SpherivoidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T zr = Hypot<T>(helper.In.z, helper.m_PrecalcSqrtSumSquares);
		const T phi = acos(Clamp<T>(helper.In.z / zr, -1, 1));
		const T ps = sin(phi);
		const T pc = cos(phi);

		helper.Out.x = m_Weight * cos(helper.m_PrecalcAtanyx) * ps * (zr + m_Radius);
		helper.Out.y = m_Weight * sin(helper.m_PrecalcAtanyx) * ps * (zr + m_Radius);
		helper.Out.z = m_Weight * pc * (zr + m_Radius);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst real_t zr = Hypot(vIn.z, precalcSqrtSumSquares);\n"
		   << "\t\tconst real_t phi = acos(Clamp(vIn.z / zr, -1.0, 1.0));\n"
		   << "\t\tconst real_t ps = sin(phi);\n"
		   << "\t\tconst real_t pc = cos(phi);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * cos(precalcAtanyx) * ps * (zr + " << radius << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * sin(precalcAtanyx) * ps * (zr + " << radius << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * pc * (zr + " << radius << ");\n"
			<< "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "spherivoid_radius"));
	}

private:
	T m_Radius;
};

/// <summary>
/// farblur.
/// </summary>
template <typename T>
class EMBER_API FarblurVariation : public ParametricVariation<T>
{
public:
	FarblurVariation(T weight = 1.0) : ParametricVariation<T>("farblur", VAR_FARBLUR, weight)
	{
		Init();
	}

	PARVARCOPY(FarblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * (Sqr(helper.In.x - m_XOrigin) +
			Sqr(helper.In.y - m_YOrigin) +
			Sqr(helper.In.z - m_ZOrigin)) *
			(rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		T u = rand.Frand01<T>() * M_2PI;
		T su = sin(u);
		T cu = cos(u);
		T v = rand.Frand01<T>() * M_2PI;
		T sv = sin(v);
		T cv = cos(v);

		helper.Out.x = m_X * r * sv * cu;
		helper.Out.y = m_Y * r * sv * su;
		helper.Out.z = m_Z * r * cv;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xOrigin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yOrigin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zOrigin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (Sqr(vIn.x - " << xOrigin << ") + \n"
		   << "\t\t	Sqr(vIn.y - " << yOrigin << ") + \n"
		   << "\t\t	Sqr(vIn.z - " << zOrigin << ")) *\n"
		   << "\t\t	(MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - 2);\n"
		   << "\t\treal_t u = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t su = sin(u);\n"
		   << "\t\treal_t cu = cos(u);\n"
		   << "\t\treal_t v = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t sv = sin(v);\n"
		   << "\t\treal_t cv = cos(v);\n"
		   << "\n"
		   << "\t\tvOut.x = " << x << " * r * sv * cu;\n"
		   << "\t\tvOut.y = " << y << " * r * sv * su;\n"
		   << "\t\tvOut.z = " << z << " * r * cv;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "farblur_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "farblur_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "farblur_z", 1));
		m_Params.push_back(ParamWithName<T>(&m_XOrigin, prefix + "farblur_x_origin"));
		m_Params.push_back(ParamWithName<T>(&m_YOrigin, prefix + "farblur_y_origin"));
		m_Params.push_back(ParamWithName<T>(&m_ZOrigin, prefix + "farblur_z_origin"));
	}

private:
	T m_X;
	T m_Y;
	T m_Z;
	T m_XOrigin;
	T m_YOrigin;
	T m_ZOrigin;
};

/// <summary>
/// curl_sp.
/// </summary>
template <typename T>
class EMBER_API CurlSPVariation : public ParametricVariation<T>
{
public:
	CurlSPVariation(T weight = 1.0) : ParametricVariation<T>("curl_sp", VAR_CURL_SP, weight)
	{
		Init();
	}

	PARVARCOPY(CurlSPVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T x = Powq4c(helper.In.x, m_Power);
		const T y = Powq4c(helper.In.y, m_Power);
		const T z = Powq4c(helper.In.z, m_Power);
		const T d = SQR(x) - SQR(y);
		const T re = Spread(m_C1 * x + m_C2 * d, m_Sx) + 1;
		const T im = Spread(m_C1 * y + m_C2x2 * x * y, m_Sy);
		T c = Zeps(Powq4c(SQR(re) + SQR(im), m_PowerInv));
		const T r = m_Weight / c;

		helper.Out.x = (x * re + y * im) * r;
		helper.Out.y = (y * re - x * im) * r;
		helper.Out.z = (z * m_Weight) / c;
		outPoint.m_ColorX = Clamp<T>(outPoint.m_ColorX + m_DcAdjust * c, 0, 1);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sx       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sy       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dc       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dcAdjust = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powerInv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tconst real_t x = Powq4c(vIn.x, " << power << ");\n"
		   << "\t\tconst real_t y = Powq4c(vIn.y, " << power << ");\n"
		   << "\t\tconst real_t z = Powq4c(vIn.z, " << power << ");\n"
		   << "\t\tconst real_t d = SQR(x) - SQR(y);\n"
		   << "\t\tconst real_t re = Spread(" << c1 << " * x + " << c2 << " * d, " << sx << ") + 1.0;\n"
		   << "\t\tconst real_t im = Spread(" << c1 << " * y + " << c2x2 << " * x * y, " << sy << ");\n"
		   << "\t\treal_t c = Zeps(Powq4c(SQR(re) + SQR(im), " << powerInv << "));\n"
		   << "\n"
		   << "\t\tconst real_t r = xform->m_VariationWeights[" << varIndex << "] / c;\n"
		   << "\n"
		   << "\t\tvOut.x = (x * re + y * im) * r;\n"
		   << "\t\tvOut.y = (y * re - x * im) * r;\n"
		   << "\t\tvOut.z = (z * xform->m_VariationWeights[" << varIndex << "]) / c;\n"
		   << "\t\toutPoint->m_ColorX = Clamp(outPoint->m_ColorX + " << dcAdjust << " * c, 0.0, 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C2x2 = 2 * m_C2;
		m_DcAdjust = T(0.1) * m_Dc;
		m_Power = Zeps(m_Power);
		m_PowerInv = 1 / m_Power;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "curl_sp_pow", 1, REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_C1,    prefix + "curl_sp_c1"));
		m_Params.push_back(ParamWithName<T>(&m_C2,    prefix + "curl_sp_c2"));
		m_Params.push_back(ParamWithName<T>(&m_Sx,    prefix + "curl_sp_sx"));
		m_Params.push_back(ParamWithName<T>(&m_Sy,    prefix + "curl_sp_sy"));
		m_Params.push_back(ParamWithName<T>(&m_Dc,    prefix + "curl_sp_dc"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x2,     prefix + "curl_sp_c2_x2"));
		m_Params.push_back(ParamWithName<T>(true, &m_DcAdjust, prefix + "curl_sp_dc_adjust"));
		m_Params.push_back(ParamWithName<T>(true, &m_PowerInv, prefix + "curl_sp_power_inv"));
	}

private:
	T m_Power;
	T m_C1;
	T m_C2;
	T m_Sx;
	T m_Sy;
	T m_Dc;
	T m_C2x2;//Precalc.
	T m_DcAdjust;
	T m_PowerInv;
};

/// <summary>
/// heat.
/// </summary>
template <typename T>
class EMBER_API HeatVariation : public ParametricVariation<T>
{
public:
	HeatVariation(T weight = 1.0) : ParametricVariation<T>("heat", VAR_HEAT, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(HeatVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = sqrt(fabs(helper.m_PrecalcSumSquares + helper.In.z));

		r += m_Ar * sin(fma(m_Br, r, m_Cr));

		if (r == 0)
			r = EPS;

		T temp = fma(m_At, sin(fma(m_Bt, r, m_Ct)), helper.m_PrecalcAtanyx);
		T st = sin(temp);
		T ct = cos(temp);

		temp = fma(m_Ap, sin(fma(m_Bp, r, m_Cp)), acos(Clamp<T>(helper.In.z / r, -1, 1)));

		T sp = sin(temp);
		T cp = cos(temp);

		helper.Out.x = r * ct * sp;
		helper.Out.y = r * st * sp;
		helper.Out.z = r * cp;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string thetaPeriod = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thetaPhase  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thetaAmp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phiPeriod   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phiPhase    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phiAmp      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rperiod     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rphase      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ramp        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string at          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bt          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ct          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ap          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bp          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cp          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ar          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string br          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cr          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = sqrt(fabs(precalcSumSquares + vIn.z));\n"
		   << "\n"
		   << "\t\tr += " << ar << " * sin(fma(" << br << ", r, " << cr << "));\n"
		   << "\n"
		   << "\t\tif (r == 0)\n"
		   << "\t\t	r = EPS;\n"
		   << "\n"
		   << "\t\treal_t temp = fma(" << at << ", sin(fma(" << bt << ", r, " << ct << ")), precalcAtanyx);\n"
		   << "\t\treal_t st = sin(temp);\n"
		   << "\t\treal_t ct = cos(temp);\n"
		   << "\n"
		   << "\t\ttemp = fma(" << ap << ", sin(fma(" << bp << ", r, " << cp << ")), acos(Clamp(vIn.z / r, -1.0, 1.0)));\n"
		   << "\n"
		   << "\t\treal_t sp = sin(temp);\n"
		   << "\t\treal_t cp = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = r * ct * sp;\n"
		   << "\t\tvOut.y = r * st * sp;\n"
		   << "\t\tvOut.z = r * cp;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T tx = m_ThetaPeriod == 0 ? 0 : (1 / m_ThetaPeriod);
		T px = m_PhiPeriod == 0 ? 0 : (1 / m_PhiPeriod);
		T rx = m_Rperiod == 0 ? 0 : (1 / m_Rperiod);

		m_At = m_Weight * m_ThetaAmp;
		m_Bt = M_2PI * tx;
		m_Ct = m_ThetaPhase * tx;
		m_Ap = m_Weight * m_PhiAmp;
		m_Bp = M_2PI * px;
		m_Cp = m_PhiPhase * px;
		m_Ar = m_Weight * m_Ramp;
		m_Br = M_2PI * rx;
		m_Cr = m_Rphase * rx;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ThetaPeriod, prefix + "heat_theta_period", 1));
		m_Params.push_back(ParamWithName<T>(&m_ThetaPhase,  prefix + "heat_theta_phase"));
		m_Params.push_back(ParamWithName<T>(&m_ThetaAmp,    prefix + "heat_theta_amp", 1));
		m_Params.push_back(ParamWithName<T>(&m_PhiPeriod,   prefix + "heat_phi_period", 1));
		m_Params.push_back(ParamWithName<T>(&m_PhiPhase,    prefix + "heat_phi_phase"));
		m_Params.push_back(ParamWithName<T>(&m_PhiAmp,      prefix + "heat_phi_amp"));
		m_Params.push_back(ParamWithName<T>(&m_Rperiod,     prefix + "heat_r_period", 1));
		m_Params.push_back(ParamWithName<T>(&m_Rphase,      prefix + "heat_r_phase"));
		m_Params.push_back(ParamWithName<T>(&m_Ramp,        prefix + "heat_r_amp"));
		m_Params.push_back(ParamWithName<T>(true, &m_At, prefix + "heat_at"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bt, prefix + "heat_bt"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ct, prefix + "heat_ct"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ap, prefix + "heat_ap"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bp, prefix + "heat_bp"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cp, prefix + "heat_cp"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ar, prefix + "heat_ar"));
		m_Params.push_back(ParamWithName<T>(true, &m_Br, prefix + "heat_br"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cr, prefix + "heat_cr"));
	}

private:
	T m_ThetaPeriod;
	T m_ThetaPhase;
	T m_ThetaAmp;
	T m_PhiPeriod;
	T m_PhiPhase;
	T m_PhiAmp;
	T m_Rperiod;
	T m_Rphase;
	T m_Ramp;
	T m_At;//Precalc.
	T m_Bt;
	T m_Ct;
	T m_Ap;
	T m_Bp;
	T m_Cp;
	T m_Ar;
	T m_Br;
	T m_Cr;
};

/// <summary>
/// interference2.
/// </summary>
template <typename T>
class EMBER_API Interference2Variation : public ParametricVariation<T>
{
public:
	Interference2Variation(T weight = 1.0) : ParametricVariation<T>("interference2", VAR_INTERFERENCE2, weight)
	{
		Init();
	}

	PARVARCOPY(Interference2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T fp1x;
		T fp1y;
		T fp2x;
		T fp2y;

		switch (int(m_T1))
		{
			case 0:
				fp1x = Sine(m_A1, m_B1, m_C1, m_P1, helper.In.x);
				fp1y = Sine(m_A1, m_B1, m_C1, m_P1, helper.In.y);
				break;
			case 1:
				fp1x = Tri(m_A1, m_B1, m_C1, m_P1, helper.In.x);
				fp1y = Tri(m_A1, m_B1, m_C1, m_P1, helper.In.y);
				break;
			case 2:
				fp1x = Squ(m_A1, m_B1, m_C1, m_P1, helper.In.x);
				fp1y = Squ(m_A1, m_B1, m_C1, m_P1, helper.In.y);
				break;
			default:
				fp1x = Sine(m_A1, m_B1, m_C1, m_P1, helper.In.x);
				fp1y = Sine(m_A1, m_B1, m_C1, m_P1, helper.In.y);
				break;
		}

		switch (int(m_T2))
		{
			case 0:
				fp2x = Sine(m_A2, m_B2, m_C2, m_P2, helper.In.x);
				fp2y = Sine(m_A2, m_B2, m_C2, m_P2, helper.In.y);
				break;
			case 1:
				fp2x = Tri(m_A2, m_B2, m_C2, m_P2, helper.In.x);
				fp2y = Tri(m_A2, m_B2, m_C2, m_P2, helper.In.y);
				break;
			case 2:
				fp2x = Squ(m_A2, m_B2, m_C2, m_P2, helper.In.x);
				fp2y = Squ(m_A2, m_B2, m_C2, m_P2, helper.In.y);
				break;
			default:
				fp2x = Sine(m_A2, m_B2, m_C2, m_P2, helper.In.x);
				fp2y = Sine(m_A2, m_B2, m_C2, m_P2, helper.In.y);
				break;
		}

		helper.Out.x = m_Weight * (fp1x + fp2x);
		helper.Out.y = m_Weight * (fp1y + fp2y);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string a1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t fp1x;\n"
		   << "\t\treal_t fp1y;\n"
		   << "\t\treal_t fp2x;\n"
		   << "\t\treal_t fp2y;\n"
		   << "\n"
		   << "\t\tswitch ((int)" << t1 << ")\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		fp1x = Interference2Sine(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.x);\n"
		   << "\t\t		fp1y = Interference2Sine(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		fp1x = Interference2Tri(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.x);\n"
		   << "\t\t		fp1y = Interference2Tri(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t		fp1x = Interference2Squ(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.x);\n"
		   << "\t\t		fp1y = Interference2Squ(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	default:\n"
		   << "\t\t		fp1x = Interference2Sine(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.x);\n"
		   << "\t\t		fp1y = Interference2Sine(" << a1 << ", " << b1 << ", " << c1 << ", " << p1 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tswitch ((int)" << t2 << ")\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		fp2x = Interference2Sine(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.x);\n"
		   << "\t\t		fp2y = Interference2Sine(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		fp2x = Interference2Tri(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.x);\n"
		   << "\t\t		fp2y = Interference2Tri(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t		fp2x = Interference2Squ(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.x);\n"
		   << "\t\t		fp2y = Interference2Squ(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t	default:\n"
		   << "\t\t		fp2x = Interference2Sine(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.x);\n"
		   << "\t\t		fp2y = Interference2Sine(" << a2 << ", " << b2 << ", " << c2 << ", " << p2 << ", vIn.y);\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (fp1x + fp2x);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (fp1y + fp2y);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"real_t Interference2Sine(real_t a, real_t b, real_t c, real_t p, real_t x)\n"
			"{\n"
			"	return a * pow(fabs(sin(b * x + c)), p);\n"
			"}\n"
			"\n"
			"real_t Interference2Tri(real_t a, real_t b, real_t c, real_t p, real_t x)\n"
			"{\n"
			"	return a * 2 * pow(fabs(asin(cos(b * x + c - M_PI_2))) * M_1_PI, p);\n"
			"}\n"
			"\n"
			"real_t Interference2Squ(real_t a, real_t b, real_t c, real_t p, real_t x)\n"
			"{\n"
			"	return a * pow(sin(b * x + c) < 0 ? EPS : 1, p);\n"
			"}\n"
			"\n";
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A1, prefix + "interference2_a1", 1));//Original used a prefix of intrfr2_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_B1, prefix + "interference2_b1", 1));
		m_Params.push_back(ParamWithName<T>(&m_C1, prefix + "interference2_c1"));
		m_Params.push_back(ParamWithName<T>(&m_P1, prefix + "interference2_p1", 1));
		m_Params.push_back(ParamWithName<T>(&m_T1, prefix + "interference2_t1", 0, INTEGER, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_A2, prefix + "interference2_a2", 1));
		m_Params.push_back(ParamWithName<T>(&m_B2, prefix + "interference2_b2", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2, prefix + "interference2_c2"));
		m_Params.push_back(ParamWithName<T>(&m_P2, prefix + "interference2_p2", 1));
		m_Params.push_back(ParamWithName<T>(&m_T2, prefix + "interference2_t2", 0, INTEGER, 0, 2));
	}

private:
	inline static T Sine(T a, T b, T c, T p, T x)
	{
		return a * pow(fabs(sin(b * x + c)), p);//Original did not fabs().
	}

	inline static T Tri(T a, T b, T c, T p, T x)
	{
		return a * 2 * pow(fabs(asin(cos(b * x + c - T(M_PI_2)))) * T(M_1_PI), p);//Original did not fabs().
	}

	inline static T Squ(T a, T b, T c, T p, T x)
	{
		return a * pow(sin(b * x + c) < 0 ? EPS : T(1), p);//Original passed -1 to pow if sin() was < 0. Doing so will return NaN, so EPS is passed instead.
	}

	T m_A1;
	T m_B1;
	T m_C1;
	T m_P1;
	T m_T1;
	T m_A2;
	T m_B2;
	T m_C2;
	T m_P2;
	T m_T2;
};

/// <summary>
/// sinq.
/// </summary>
template <typename T>
class EMBER_API SinqVariation : public Variation<T>
{
public:
	SinqVariation(T weight = 1.0) : Variation<T>("sinq", VAR_SINQ, weight) { }

	VARCOPY(SinqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = m_Weight * c * sh / absV;

		helper.Out.x = m_Weight * s * ch;
		helper.Out.y = d * helper.In.y;
		helper.Out.z = d * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] * c * sh / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * s * ch;\n"
		   << "\t\tvOut.y = d * vIn.y;\n"
		   << "\t\tvOut.z = d * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// sinhq.
/// </summary>
template <typename T>
class EMBER_API SinhqVariation : public Variation<T>
{
public:
	SinhqVariation(T weight = 1.0) : Variation<T>("sinhq", VAR_SINHQ, weight) { }

	VARCOPY(SinhqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = m_Weight * ch * s / absV;

		helper.Out.x = m_Weight * sh * c;
		helper.Out.y = d * helper.In.y;
		helper.Out.z = d * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] * ch * s / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sh * c;\n"
		   << "\t\tvOut.y = d * vIn.y;\n"
		   << "\t\tvOut.z = d * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// secq.
/// </summary>
template <typename T>
class EMBER_API SecqVariation : public Variation<T>
{
public:
	SecqVariation(T weight = 1.0) : Variation<T>("secq", VAR_SECQ, weight, true) { }

	VARCOPY(SecqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T ni = m_Weight / (helper.m_PrecalcSumSquares + SQR(helper.In.z));
		T s = sin(-helper.In.x);
		T c = cos(-helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = ni * s * sh / absV;

		helper.Out.x =   c * ch * ni;
		helper.Out.y = -(d * helper.In.y);
		helper.Out.z = -(d * helper.In.z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\treal_t s = sin(-vIn.x);\n"
		   << "\t\treal_t c = cos(-vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = ni * s * sh / absV;\n"
		   << "\n"
		   << "\t\tvOut.x =   c * ch * ni;\n"
		   << "\t\tvOut.y = -(d * vIn.y);\n"
		   << "\t\tvOut.z = -(d * vIn.z);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// sechq.
/// </summary>
template <typename T>
class EMBER_API SechqVariation : public Variation<T>
{
public:
	SechqVariation(T weight = 1.0) : Variation<T>("sechq", VAR_SECHQ, weight, true) { }

	VARCOPY(SechqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T ni = m_Weight / (helper.m_PrecalcSumSquares + SQR(helper.In.z));
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = ni * sh * s / absV;

		helper.Out.x =  ch * c * ni;
		helper.Out.y = -(d * helper.In.y);
		helper.Out.z = -(d * helper.In.z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = ni * sh * s / absV;\n"
		   << "\n"
		   << "\t\tvOut.x =  ch * c * ni;\n"
		   << "\t\tvOut.y = -(d * vIn.y);\n"
		   << "\t\tvOut.z = -(d * vIn.z);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// tanq.
/// </summary>
template <typename T>
class EMBER_API TanqVariation : public Variation<T>
{
public:
	TanqVariation(T weight = 1.0) : Variation<T>("tanq", VAR_TANQ, weight) { }

	VARCOPY(TanqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sysz = SQR(helper.In.y) + SQR(helper.In.z);
		T absV = sqrt(sysz);
		T ni = m_Weight / (SQR(helper.In.x) + sysz);
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = c * sh / absV;
		T b = -s * sh / absV;
		T stcv = s * ch;
		T nstcv = -stcv;
		T ctcv = c * ch;

		helper.Out.x = (stcv * ctcv + d * b * sysz) * ni;
		helper.Out.y = (nstcv * b * helper.In.y + d * helper.In.y * ctcv) * ni;
		helper.Out.z = (nstcv * b * helper.In.z + d * helper.In.z * ctcv) * ni;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sysz = SQR(vIn.y) + SQR(vIn.z);\n"
		   << "\t\treal_t absV = sqrt(sysz);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (SQR(vIn.x) + sysz);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = c * sh / absV;\n"
		   << "\t\treal_t b = -s * sh / absV;\n"
		   << "\t\treal_t stcv = s * ch;\n"
		   << "\t\treal_t nstcv = -stcv;\n"
		   << "\t\treal_t ctcv = c * ch;\n"
		   << "\n"
		   << "\t\tvOut.x = (stcv * ctcv + d * b * sysz) * ni;\n"
		   << "\t\tvOut.y = (nstcv * b * vIn.y + d * vIn.y * ctcv) * ni;\n"
		   << "\t\tvOut.z = (nstcv * b * vIn.z + d * vIn.z * ctcv) * ni;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// tanhq.
/// </summary>
template <typename T>
class EMBER_API TanhqVariation : public Variation<T>
{
public:
	TanhqVariation(T weight = 1.0) : Variation<T>("tanhq", VAR_TANHQ, weight) { }

	VARCOPY(TanhqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sysz = SQR(helper.In.y) + SQR(helper.In.z);
		T absV = sqrt(sysz);
		T ni = m_Weight / (SQR(helper.In.x) + sysz);
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = ch * s / absV;
		T b = sh * s / absV;
		T stcv = sh * c;
		T nstcv = -stcv;
		T ctcv = c * ch;

		helper.Out.x = (stcv * ctcv + d * b * sysz) * ni;
		helper.Out.y = (nstcv * b * helper.In.y + d * helper.In.y * ctcv) * ni;
		helper.Out.z = (nstcv * b * helper.In.z + d * helper.In.z * ctcv) * ni;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sysz = SQR(vIn.y) + SQR(vIn.z);\n"
		   << "\t\treal_t absV = sqrt(sysz);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (SQR(vIn.x) + sysz);\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = ch * s / absV;\n"
		   << "\t\treal_t b = sh * s / absV;\n"
		   << "\t\treal_t stcv = sh * c;\n"
		   << "\t\treal_t nstcv = -stcv;\n"
		   << "\t\treal_t ctcv = c * ch;\n"
		   << "\n"
		   << "\t\tvOut.x = (stcv * ctcv + d * b * sysz) * ni;\n"
		   << "\t\tvOut.y = (nstcv * b * vIn.y + d * vIn.y * ctcv) * ni;\n"
		   << "\t\tvOut.z = (nstcv * b * vIn.z + d * vIn.z * ctcv) * ni;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// cosq.
/// </summary>
template <typename T>
class EMBER_API CosqVariation : public Variation<T>
{
public:
	CosqVariation(T weight = 1.0) : Variation<T>("cosq", VAR_COSQ, weight) { }

	VARCOPY(CosqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = -m_Weight * s * sh / absV;

		helper.Out.x = m_Weight * c * ch;
		helper.Out.y = d * helper.In.y;
		helper.Out.z = d * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = -xform->m_VariationWeights[" << varIndex << "] * s * sh / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * c * ch;\n"
		   << "\t\tvOut.y = d * vIn.y;\n"
		   << "\t\tvOut.z = d * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// coshq.
/// </summary>
template <typename T>
class EMBER_API CoshqVariation : public Variation<T>
{
public:
	CoshqVariation(T weight = 1.0) : Variation<T>("coshq", VAR_COSHQ, weight) { }

	VARCOPY(CoshqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = -m_Weight * sh * s / absV;

		helper.Out.x = m_Weight * c * ch;
		helper.Out.y = d * helper.In.y;
		helper.Out.z = d * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = -xform->m_VariationWeights[" << varIndex << "] * sh * s / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * c * ch;\n"
		   << "\t\tvOut.y = d * vIn.y;\n"
		   << "\t\tvOut.z = d * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// cotq.
/// </summary>
template <typename T>
class EMBER_API CotqVariation : public Variation<T>
{
public:
	CotqVariation(T weight = 1.0) : Variation<T>("cotq", VAR_COTQ, weight) { }

	VARCOPY(CotqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sysz = SQR(helper.In.y) + SQR(helper.In.z);
		T absV = sqrt(sysz);
		T ni = m_Weight / (SQR(helper.In.x) + sysz);
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = c * sh / absV;
		T b = -s * sh / absV;
		T stcv = s * ch;
		T nstcv = -stcv;
		T ctcv = c * ch;

		helper.Out.x =  (stcv * ctcv + d * b * sysz) * ni;
		helper.Out.y = -(nstcv * b * helper.In.y + d * helper.In.y * ctcv) * ni;
		helper.Out.z = -(nstcv * b * helper.In.z + d * helper.In.z * ctcv) * ni;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sysz = SQR(vIn.y) + SQR(vIn.z);\n"
		   << "\t\treal_t absV = sqrt(sysz);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (SQR(vIn.x) + sysz);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = c * sh / absV;\n"
		   << "\t\treal_t b = -s * sh / absV;\n"
		   << "\t\treal_t stcv = s * ch;\n"
		   << "\t\treal_t nstcv = -stcv;\n"
		   << "\t\treal_t ctcv = c * ch;\n"
		   << "\n"
		   << "\t\tvOut.x =  (stcv * ctcv + d * b * sysz) * ni;\n"
		   << "\t\tvOut.y = -(nstcv * b * vIn.y + d * vIn.y * ctcv) * ni;\n"
		   << "\t\tvOut.z = -(nstcv * b * vIn.z + d * vIn.z * ctcv) * ni;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// cothq.
/// </summary>
template <typename T>
class EMBER_API CothqVariation : public Variation<T>
{
public:
	CothqVariation(T weight = 1.0) : Variation<T>("cothq", VAR_COTHQ, weight) { }

	VARCOPY(CothqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sysz = SQR(helper.In.y) + SQR(helper.In.z);
		T absV = sqrt(sysz);
		T ni = m_Weight / (SQR(helper.In.x) + sysz);
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = ch * s / absV;
		T b = sh * s / absV;
		T stcv = sh * c;
		T nstcv = -stcv;
		T ctcv = ch * c;

		helper.Out.x =  (stcv * ctcv + d * b * sysz) * ni;
		helper.Out.y = -(nstcv * b * helper.In.y + d * helper.In.y * ctcv) * ni;
		helper.Out.z = -(nstcv * b * helper.In.z + d * helper.In.z * ctcv) * ni;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t sysz = SQR(vIn.y) + SQR(vIn.z);\n"
		   << "\t\treal_t absV = sqrt(sysz);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (SQR(vIn.x) + sysz);\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = ch * s / absV;\n"
		   << "\t\treal_t b = sh * s / absV;\n"
		   << "\t\treal_t stcv = sh * c;\n"
		   << "\t\treal_t nstcv = -stcv;\n"
		   << "\t\treal_t ctcv = ch * c;\n"
		   << "\n"
		   << "\t\tvOut.x =  (stcv * ctcv + d * b * sysz) * ni;\n"
		   << "\t\tvOut.y = -(nstcv * b * vIn.y + d * vIn.y * ctcv) * ni;\n"
		   << "\t\tvOut.z = -(nstcv * b * vIn.z + d * vIn.z * ctcv) * ni;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// cscq.
/// </summary>
template <typename T>
class EMBER_API CscqVariation : public Variation<T>
{
public:
	CscqVariation(T weight = 1.0) : Variation<T>("cscq", VAR_CSCQ, weight, true) { }

	VARCOPY(CscqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T ni = m_Weight / (helper.m_PrecalcSumSquares + SQR(helper.In.z));
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);
		T sh = sinh(absV);
		T ch = cosh(absV);
		T d = ni * c * sh / absV;

		helper.Out.x =   s * ch * ni;
		helper.Out.y = -(d * helper.In.y);
		helper.Out.z = -(d * helper.In.z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t sh = sinh(absV);\n"
		   << "\t\treal_t ch = cosh(absV);\n"
		   << "\t\treal_t d = ni * c * sh / absV;\n"
		   << "\n"
		   << "\t\tvOut.x =   s * ch * ni;\n"
		   << "\t\tvOut.y = -(d * vIn.y);\n"
		   << "\t\tvOut.z = -(d * vIn.z);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// cschq.
/// </summary>
template <typename T>
class EMBER_API CschqVariation : public Variation<T>
{
public:
	CschqVariation(T weight = 1.0) : Variation<T>("cschq", VAR_CSCHQ, weight, true) { }

	VARCOPY(CschqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T ni = m_Weight / (helper.m_PrecalcSumSquares + SQR(helper.In.z));
		T s = sin(absV);
		T c = cos(absV);
		T sh = sinh(helper.In.x);
		T ch = cosh(helper.In.x);
		T d = ni * ch * s / absV;

		helper.Out.x =  sh * c * ni;
		helper.Out.y = -(d * helper.In.y);
		helper.Out.z = -(d * helper.In.z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t ni = xform->m_VariationWeights[" << varIndex << "] / (precalcSumSquares + SQR(vIn.z));\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t sh = sinh(vIn.x);\n"
		   << "\t\treal_t ch = cosh(vIn.x);\n"
		   << "\t\treal_t d = ni * ch * s / absV;\n"
		   << "\n"
		   << "\t\tvOut.x =  sh * c * ni;\n"
		   << "\t\tvOut.y = -(d * vIn.y);\n"
		   << "\t\tvOut.z = -(d * vIn.z);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// estiq.
/// </summary>
template <typename T>
class EMBER_API EstiqVariation : public Variation<T>
{
public:
	EstiqVariation(T weight = 1.0) : Variation<T>("estiq", VAR_ESTIQ, weight) { }

	VARCOPY(EstiqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T e = exp(helper.In.x);
		T s = sin(absV);
		T c = cos(absV);
		T a = e * s / absV;

		helper.Out.x = m_Weight * e * c;
		helper.Out.y = m_Weight * a * helper.In.y;
		helper.Out.z = m_Weight * a * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t e = exp(vIn.x);\n"
		   << "\t\treal_t s = sin(absV);\n"
		   << "\t\treal_t c = cos(absV);\n"
		   << "\t\treal_t a = e * s / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * e * c;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * a * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * a * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// loq.
/// </summary>
template <typename T>
class EMBER_API LoqVariation : public ParametricVariation<T>
{
public:
	LoqVariation(T weight = 1.0) : ParametricVariation<T>("loq", VAR_LOQ, weight)
	{
		Init();
	}

	PARVARCOPY(LoqVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T absV = Hypot<T>(helper.In.y, helper.In.z);
		T c = m_Weight * atan2(absV, helper.In.x) / absV;

		helper.Out.x = log(SQR(helper.In.x) + SQR(absV)) * m_Denom;
		helper.Out.y = c * helper.In.y;
		helper.Out.z = c * helper.In.z;
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
		   << "\t\treal_t absV = Hypot(vIn.y, vIn.z);\n"
		   << "\t\treal_t c = xform->m_VariationWeights[" << varIndex << "] * atan2(absV, vIn.x) / absV;\n"
		   << "\n"
		   << "\t\tvOut.x = log(SQR(vIn.x) + SQR(absV)) * " << denom << ";\n"
		   << "\t\tvOut.y = c * vIn.y;\n"
		   << "\t\tvOut.z = c * vIn.z;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_Base, prefix + "loq_base", T(M_E), REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(true, &m_Denom, prefix + "loq_denom"));//Precalc.
	}

private:
	T m_Base;
	T m_Denom;//Precalc.
};

/// <summary>
/// curvature.
/// </summary>
template <typename T>
class EMBER_API CurvatureVariation : public Variation<T>
{
public:
	CurvatureVariation(T weight = 1.0) : Variation<T>("curvature", VAR_CURVATURE, weight, true, true, false, false, true) { }

	VARCOPY(CurvatureVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight / Zeps(helper.m_PrecalcSqrtSumSquares);
		helper.Out.y = helper.m_PrecalcAtanyx;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] / Zeps(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = precalcAtanyx;\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// q_ode.
/// </summary>
template <typename T>
class EMBER_API QodeVariation : public ParametricVariation<T>
{
public:
	QodeVariation(T weight = 1.0) : ParametricVariation<T>("q_ode", VAR_Q_ODE, weight)
	{
		Init();
	}

	PARVARCOPY(QodeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sqx = SQR(helper.In.x);
		T sqy = SQR(helper.In.y);
		T xy = helper.In.x * helper.In.y;

		helper.Out.x = (m_Q01 + m_Weight * m_Q02 * helper.In.x + m_Q03 * sqx) +
					   (m_Q04 * xy + m_Q05 * helper.In.y + m_Q06 * sqy);
		helper.Out.y = (m_Q07 + m_Q08 * helper.In.x + m_Q09 * sqx) +
					   (m_Q10 * xy + m_Weight * m_Q11 * helper.In.y + m_Q12 * sqy);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string q01 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q02 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q03 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q04 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q05 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q06 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q07 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q08 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q09 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q10 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q11 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q12 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t sqx = SQR(vIn.x);\n"
		   << "\t\treal_t sqy = SQR(vIn.y);\n"
		   << "\t\treal_t xy = vIn.x * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.x = (" << q01 << " + xform->m_VariationWeights[" << varIndex << "] * " << q02 << " * vIn.x + " << q03 << " * sqx) + \n"
		   << "\t\t			(" << q04 << " * xy + " << q05 << " * vIn.y + " << q06 << " * sqy);\n"
		   << "\t\tvOut.y = (" << q07 << " + " << q08 << " * vIn.x + " << q09 << " * sqx) + \n"
		   << "\t\t			(" << q10 << " * xy + xform->m_VariationWeights[" << varIndex << "] * " << q11 << " * vIn.y + " << q12 << " * sqy);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Q01, prefix + "q_ode01", 1));
		m_Params.push_back(ParamWithName<T>(&m_Q02, prefix + "q_ode02", -1));
		m_Params.push_back(ParamWithName<T>(&m_Q03, prefix + "q_ode03"));
		m_Params.push_back(ParamWithName<T>(&m_Q04, prefix + "q_ode04"));
		m_Params.push_back(ParamWithName<T>(&m_Q05, prefix + "q_ode05"));
		m_Params.push_back(ParamWithName<T>(&m_Q06, prefix + "q_ode06"));
		m_Params.push_back(ParamWithName<T>(&m_Q07, prefix + "q_ode07", 1));
		m_Params.push_back(ParamWithName<T>(&m_Q08, prefix + "q_ode08"));
		m_Params.push_back(ParamWithName<T>(&m_Q09, prefix + "q_ode09"));
		m_Params.push_back(ParamWithName<T>(&m_Q10, prefix + "q_ode10"));
		m_Params.push_back(ParamWithName<T>(&m_Q11, prefix + "q_ode11"));
		m_Params.push_back(ParamWithName<T>(&m_Q12, prefix + "q_ode12"));
	}

private:
	T m_Q01;
	T m_Q02;
	T m_Q03;
	T m_Q04;
	T m_Q05;
	T m_Q06;
	T m_Q07;
	T m_Q08;
	T m_Q09;
	T m_Q10;
	T m_Q11;
	T m_Q12;
};

/// <summary>
/// blur_heart.
/// </summary>
template <typename T>
class EMBER_API BlurHeartVariation : public ParametricVariation<T>
{
public:
	BlurHeartVariation(T weight = 1.0) : ParametricVariation<T>("blur_heart", VAR_BLUR_HEART, weight)
	{
		Init();
	}

	PARVARCOPY(BlurHeartVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xx = (rand.Frand01<T>() - T(0.5)) * 2;
		T yy = (rand.Frand01<T>() - T(0.5)) * 2;
		T k = SignNz(yy);
		T yymax = ((m_A * pow(fabs(xx), m_P) + k * m_B * sqrt(fabs(1 - SQR(xx)))) - m_A);

		//The function must be in a range 0-1 to work properly.
		yymax /= Zeps(fabs(m_A) + fabs(m_B));

		//Quick and dirty way to force y to be in range without altering the density.
		if (k > 0)
		{
			if (yy > yymax)
				yy = yymax;
		}
		else
		{
			if (yy < yymax)
				yy = yymax;
		}

		helper.Out.x = xx * m_Weight;
		helper.Out.y = yy * m_Weight;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t xx = (MwcNext01(mwc) - 0.5) * 2;\n"
		   << "\t\treal_t yy = (MwcNext01(mwc) - 0.5) * 2;\n"
		   << "\t\treal_t k = SignNz(yy);\n"
		   << "\t\treal_t yymax = ((" << a << " * pow(fabs(xx), " << p << ") + k * " << b << " * sqrt(fabs(1 - SQR(xx)))) - " << a << ");\n"
		   << "\n"
		   << "\t\tyymax /= Zeps(fabs(" << a << ") + fabs(" << b << "));\n"
		   << "\n"
		   << "\t\tif (k > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (yy > yymax)\n"
		   << "\t\t		yy = yymax;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (yy < yymax)\n"
		   << "\t\t		yy = yymax;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xx * xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t\tvOut.y = yy * xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "blur_heart_p", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "blur_heart_a", T(-0.6)));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "blur_heart_b", T(0.7)));
	}

private:
	T m_P;
	T m_A;
	T m_B;
};

/// <summary>
/// Truchet.
/// </summary>
template <typename T>
class EMBER_API TruchetVariation : public ParametricVariation<T>
{
public:
	TruchetVariation(T weight = 1.0) : ParametricVariation<T>("Truchet", VAR_TRUCHET, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int extended = int(m_Extended);
		T seed = m_AbsSeed;
		T r = -m_Rotation;
		T r0 = 0;
		T r1 = 0;
		T tileType = 0;
		T randInt = 0;
		T modBase = 65535;
		T multiplier = 32747;
		T offset = 12345;
		T niter = 0;
		T x = helper.In.x * m_Scale;
		T y = helper.In.y * m_Scale;
		int intx = int(Round(x));
		int inty = int(Round(y));
		int randiter;

		r = x - intx;

		if (r < 0)
			x = 1 + r;
		else
			x = r;

		r = y - inty;

		if (r < 0)
			y = 1 + r;
		else
			y = r;

		//Calculate the tile type.
		if (seed == 0)
			tileType = 0;
		else if (seed == 1)
			tileType = 1;
		else
		{
			if (extended == 0)
			{
				T xrand = Round(helper.In.x);
				T yrand = Round(helper.In.y);

				xrand = xrand * m_Seed2;
				yrand = yrand * m_Seed2;
				niter = xrand + yrand + xrand*yrand;
				randInt = (niter + seed) * m_Seed2 / 2;
				randInt = fmod((randInt * multiplier + offset), modBase);
			}
			else
			{
				int xrand = int(Round(helper.In.x));
				int yrand = int(Round(helper.In.y));

				seed = T(Floor<T>(seed));
				niter = T(abs(xrand + yrand + xrand * yrand));
				randInt = seed + niter;
				randiter = 0;

				while (randiter < niter && randiter < 20)//Allow it to escape.
				{
					randiter++;
					randInt = fmod((randInt * multiplier + offset), modBase);
				}
			}

			tileType = fmod(randInt, T(2));
		}

		//Drawing the points.
		if (extended == 0)//Fast drawmode
		{
			if (tileType < 1)
			{
				r0 = pow((pow(fabs(x    ), m_Exponent) + pow(fabs(y    ), m_Exponent)), m_OneOverEx);
				r1 = pow((pow(fabs(x - 1), m_Exponent) + pow(fabs(y - 1), m_Exponent)), m_OneOverEx);
			}
			else
			{
				r0 = pow((pow(fabs(x - 1), m_Exponent) + pow(fabs(y    ), m_Exponent)), m_OneOverEx);
				r1 = pow((pow(fabs(x    ), m_Exponent) + pow(fabs(y - 1), m_Exponent)), m_OneOverEx);
			}
		}
		else//Slow drawmode
		{
			if (tileType == 1)
			{
				r0 = pow((pow(fabs(x    ), m_Exponent) + pow(fabs(y    ), m_Exponent)), m_OneOverEx);
				r1 = pow((pow(fabs(x - 1), m_Exponent) + pow(fabs(y - 1), m_Exponent)), m_OneOverEx);
			}
			else
			{
				r0 = pow((pow(fabs(x - 1), m_Exponent) + pow(fabs(y    ), m_Exponent)), m_OneOverEx);
				r1 = pow((pow(fabs(x    ), m_Exponent) + pow(fabs(y - 1), m_Exponent)), m_OneOverEx);
			}
		}

		r = fabs(r0 - T(0.5)) * m_OneOverRmax;

		if (r < 1)
		{
			helper.Out.x = m_Size * (x + Floor<T>(helper.In.x));
			helper.Out.y = m_Size * (y + Floor<T>(helper.In.y));
		}
		else
		{
			helper.Out.x = 0;//Needed because of possible sum below.
			helper.Out.y = 0;
		}

		r = fabs(r1 - T(0.5)) * m_OneOverRmax;

		if (r < 1)
		{
			helper.Out.x += m_Size * (x + Floor<T>(helper.In.x));//The += is intended here.
			helper.Out.y += m_Size * (y + Floor<T>(helper.In.y));
		}

		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string extended    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string exponent    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string arcWidth    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotation    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneOverEx   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absSeed     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed2       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneOverRmax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tint extended = (int)" << extended << ";\n"
		   << "\t\treal_t seed = " << absSeed << ";\n"
		   << "\t\treal_t r = -" << rotation << ";\n"
		   << "\t\treal_t r0 = 0;\n"
		   << "\t\treal_t r1 = 0;\n"
		   << "\t\treal_t tileType = 0;\n"
		   << "\t\treal_t randInt = 0;\n"
		   << "\t\treal_t modBase = 65535;\n"
		   << "\t\treal_t multiplier = 32747;\n"
		   << "\t\treal_t offset = 12345;\n"
		   << "\t\treal_t niter = 0;\n"
		   << "\t\treal_t x = vIn.x * " << scale << ";\n"
		   << "\t\treal_t y = vIn.y * " << scale << ";\n"
		   << "\t\tint intx = (int)Round(x);\n"
		   << "\t\tint inty = (int)Round(y);\n"
		   << "\t\tint randiter;\n"
		   << "\n"
		   << "\t\tr = x - intx;\n"
		   << "\n"
		   << "\t\tif (r < 0)\n"
		   << "\t\t	x = 1 + r;\n"
		   << "\t\telse\n"
		   << "\t\t	x = r;\n"
		   << "\n"
		   << "\t\tr = y - inty;\n"
		   << "\n"
		   << "\t\tif (r < 0)\n"
		   << "\t\t	y = 1 + r;\n"
		   << "\t\telse\n"
		   << "\t\t	y = r;\n"
		   << "\n"
		   << "\t\tif (seed == 0)\n"
		   << "\t\t	tileType = 0;\n"
		   << "\t\telse if (seed == 1)\n"
		   << "\t\t	tileType = 1;\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (extended == 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t xrand = Round(vIn.x);\n"
		   << "\t\t		real_t yrand = Round(vIn.y);\n"
		   << "\n"
		   << "\t\t		xrand = xrand * " << seed2 << ";\n"
		   << "\t\t		yrand = yrand * " << seed2 << ";\n"
		   << "\t\t		niter = xrand + yrand + xrand * yrand;\n"
		   << "\t\t		randInt = (niter + seed) * " << seed2 << " / 2;\n"
		   << "\t\t		randInt = fmod((randInt * multiplier + offset), modBase);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		int xrand = (int)Round(vIn.x);\n"
		   << "\t\t		int yrand = (int)Round(vIn.y);\n"
		   << "\n"
		   << "\t\t		seed = floor(seed);\n"
		   << "\t\t		niter = (real_t)abs(xrand + yrand + xrand * yrand);\n"
		   << "\t\t		randInt = seed + niter;\n"
		   << "\t\t		randiter = 0;\n"
		   << "\n"
		   << "\t\t		while (randiter < niter && randiter < 20)\n"
		   << "\t\t		{\n"
		   << "\t\t			randiter++;\n"
		   << "\t\t			randInt = fmod((randInt * multiplier + offset), modBase);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	tileType = fmod(randInt, 2);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tif (extended == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (tileType < 1)\n"
		   << "\t\t	{\n"
		   << "\t\t		r0 = pow((pow(fabs(x    ), " << exponent << ") + pow(fabs(y    ), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t		r1 = pow((pow(fabs(x - 1), " << exponent << ") + pow(fabs(y - 1), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		r0 = pow((pow(fabs(x - 1), " << exponent << ") + pow(fabs(y    ), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t		r1 = pow((pow(fabs(x    ), " << exponent << ") + pow(fabs(y - 1), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (tileType == 1)\n"
		   << "\t\t	{\n"
		   << "\t\t		r0 = pow((pow(fabs(x    ), " << exponent << ") + pow(fabs(y    ), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t		r1 = pow((pow(fabs(x - 1), " << exponent << ") + pow(fabs(y - 1), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		r0 = pow((pow(fabs(x - 1), " << exponent << ") + pow(fabs(y    ), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t		r1 = pow((pow(fabs(x    ), " << exponent << ") + pow(fabs(y - 1), " << exponent << ")), " << oneOverEx << ");\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tr = fabs(r0 - 0.5) * " << oneOverRmax << ";\n"
		   << "\n"
		   << "\t\tif (r < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << size << " * (x + floor(vIn.x));\n"
		   << "\t\t	vOut.y = " << size << " * (y + floor(vIn.y));\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = 0.0;\n"
		   << "\t\t	vOut.y = 0.0;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tr = fabs(r1 - 0.5) * " << oneOverRmax << ";\n"
		   << "\n"
		   << "\t\tif (r < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x += " << size << " * (x + floor(vIn.x));\n"
		   << "\t\t	vOut.y += " << size << " * (y + floor(vIn.y));\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_OneOverEx = 1 / m_Exponent;
		m_AbsSeed = fabs(m_Seed);
		m_Seed2 = sqrt(Zeps(m_AbsSeed + (m_AbsSeed / 2))) / Zeps((m_AbsSeed * T(0.5))) * T(0.25);
		m_OneOverRmax = 1 / (T(0.5) * (pow(T(2), 1 / m_Exponent) - 1) * m_ArcWidth);
		m_Scale = (cos(-m_Rotation) - sin(-m_Rotation)) / m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Extended, prefix + "Truchet_extended",  0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Exponent, prefix + "Truchet_exponent",  2, REAL_CYCLIC, T(0.001), 2));
		m_Params.push_back(ParamWithName<T>(&m_ArcWidth, prefix + "Truchet_arc_width", T(0.5), REAL_CYCLIC, T(0.001), 1));
		m_Params.push_back(ParamWithName<T>(&m_Rotation, prefix + "Truchet_rotation"));
		m_Params.push_back(ParamWithName<T>(&m_Size,     prefix + "Truchet_size", 1, REAL_CYCLIC, T(0.001), 10));
		m_Params.push_back(ParamWithName<T>(&m_Seed,     prefix + "Truchet_seed", 50));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverEx,   prefix + "Truchet_one_over_ex"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_AbsSeed,     prefix + "Truchet_abs_seed"));
		m_Params.push_back(ParamWithName<T>(true, &m_Seed2,       prefix + "Truchet_seed2"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverRmax, prefix + "Truchet_one_over_rmax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale,       prefix + "Truchet_scale"));
	}

private:
	T m_Extended;
	T m_Exponent;
	T m_ArcWidth;
	T m_Rotation;
	T m_Size;
	T m_Seed;
	T m_OneOverEx;//Precalc.
	T m_AbsSeed;
	T m_Seed2;
	T m_OneOverRmax;
	T m_Scale;
};

/// <summary>
/// gdoffs.
/// </summary>
template <typename T>
class EMBER_API GdoffsVariation : public ParametricVariation<T>
{
public:
	GdoffsVariation(T weight = 1.0) : ParametricVariation<T>("gdoffs", VAR_GDOFFS, weight)
	{
		Init();
	}

	PARVARCOPY(GdoffsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T oscX = GdoffsFosc(m_Dx, 1);
		T oscY = GdoffsFosc(m_Dy, 1);
		T inX = helper.In.x + m_Cx;
		T inY = helper.In.y + m_Cy;
		T outX;
		T outY;

		if (m_Square != 0)
		{
			outX = GdoffsFlip(GdoffsFlip(inX, GdoffsFosc(inX, 4), oscX), GdoffsFosc(GdoffsFclp(m_B * inX), 4), oscX);
			outY = GdoffsFlip(GdoffsFlip(inY, GdoffsFosc(inY, 4), oscX), GdoffsFosc(GdoffsFclp(m_B * inY), 4), oscX);
		}
		else
		{
			outX = GdoffsFlip(GdoffsFlip(inX, GdoffsFosc(inX, 4), oscX), GdoffsFosc(GdoffsFclp(m_B * inX), 4), oscX);
			outY = GdoffsFlip(GdoffsFlip(inY, GdoffsFosc(inY, 4), oscY), GdoffsFosc(GdoffsFclp(m_B * inY), 4), oscY);
		}

		helper.Out.x = m_Weight * outX;
		helper.Out.y = m_Weight * outY;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string deltaX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string deltaY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string areaX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string areaY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string gamma   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string square  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t oscX = GdoffsFosc(" << dx << ", 1);\n"
		   << "\t\treal_t oscY = GdoffsFosc(" << dy << ", 1);\n"
		   << "\t\treal_t inX = vIn.x + " << cx << ";\n"
		   << "\t\treal_t inY = vIn.y + " << cy << ";\n"
		   << "\t\treal_t outX;\n"
		   << "\t\treal_t outY;\n"
		   << "\n"
		   << "\t\tif (" << square << " != 0)\n"
		   << "\t\t{\n"
		   << "\t\t	outX = GdoffsFlip(GdoffsFlip(inX, GdoffsFosc(inX, 4), oscX), GdoffsFosc(GdoffsFclp(" << b << " * inX), 4), oscX);\n"
		   << "\t\t	outY = GdoffsFlip(GdoffsFlip(inY, GdoffsFosc(inY, 4), oscX), GdoffsFosc(GdoffsFclp(" << b << " * inY), 4), oscX);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	outX = GdoffsFlip(GdoffsFlip(inX, GdoffsFosc(inX, 4), oscX), GdoffsFosc(GdoffsFclp(" << b << " * inX), 4), oscX);\n"
		   << "\t\t	outY = GdoffsFlip(GdoffsFlip(inY, GdoffsFosc(inY, 4), oscY), GdoffsFosc(GdoffsFclp(" << b << " * inY), 4), oscY);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * outX;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * outY;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual string OpenCLFuncsString()
	{
		return
			"inline real_t GdoffsFcip(real_t a) { return (real_t)((a < 0) ? -((int)(fabs(a)) + 1) : 0) + ((a > 1) ? ((int)(a)) : 0); }\n"
			"inline real_t GdoffsFclp(real_t a) { return ((a < 0) ? -(fmod(fabs(a), 1)) : fmod(fabs(a), 1)); }\n"
			"inline real_t GdoffsFscl(real_t a) { return GdoffsFclp((a + 1) / 2); }\n"
			"inline real_t GdoffsFosc(real_t p, real_t a) { return GdoffsFscl(-1 * cos(p * a * M_2PI)); }\n"
			"inline real_t GdoffsFlip(real_t a, real_t b, real_t c) { return (c * (b - a) + a); }\n"
			"\n";
	}

	virtual void Precalc() override
	{
		const T agdod = T(0.1);
		const T agdoa = 2;
		const T agdoc = 1;

		m_Dx = m_DeltaX * agdod;
		m_Dy = m_DeltaY * agdod;
		m_Ax = ((fabs(m_AreaX) < 0.1) ? T(0.1) : fabs(m_AreaX)) * agdoa;
		m_Ay = ((fabs(m_AreaY) < 0.1) ? T(0.1) : fabs(m_AreaY)) * agdoa;
		m_Cx = m_CenterX * agdoc;
		m_Cy = m_CenterY * agdoc;
		m_B = m_Gamma * agdoa / (max(m_Ax, m_Ay));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_DeltaX,  prefix + "gdoffs_delta_x", 0, REAL, 0, 16));
		m_Params.push_back(ParamWithName<T>(&m_DeltaY,  prefix + "gdoffs_delta_y", 0, REAL, 0, 16));
		m_Params.push_back(ParamWithName<T>(&m_AreaX,   prefix + "gdoffs_area_x", 2));
		m_Params.push_back(ParamWithName<T>(&m_AreaY,   prefix + "gdoffs_area_y", 2));
		m_Params.push_back(ParamWithName<T>(&m_CenterX, prefix + "gdoffs_center_x"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY, prefix + "gdoffs_center_y"));
		m_Params.push_back(ParamWithName<T>(&m_Gamma,   prefix + "gdoffs_gamma",  1, INTEGER, 1, 6));
		m_Params.push_back(ParamWithName<T>(&m_Square,  prefix + "gdoffs_square", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Dx, prefix + "gdoffs_dx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "gdoffs_ax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix + "gdoffs_cx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Dy, prefix + "gdoffs_dyd"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "gdoffs_ay"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cy, prefix + "gdoffs_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_B,  prefix + "gdoffs_b"));
	}

private:
	static inline T GdoffsFcip(T a) { return T((a < 0) ? -(int(fabs(a)) + 1) : 0) + ((a > 1) ? (int(a)) : 0); }
	static inline T GdoffsFclp(T a) { return ((a < 0) ? -(fmod(fabs(a), T(1))) : fmod(fabs(a), T(1))); }
	static inline T GdoffsFscl(T a) { return GdoffsFclp((a + 1) / 2); }
	static inline T GdoffsFosc(T p, T a) { return GdoffsFscl(-1 * cos(p * a * M_2PI)); }
	static inline T GdoffsFlip(T a, T b, T c) { return (c * (b - a) + a); }

	T m_DeltaX;//Params.
	T m_DeltaY;
	T m_AreaX;
	T m_AreaY;
	T m_CenterX;
	T m_CenterY;
	T m_Gamma;
	T m_Square;
	T m_Dx;//Precalc.
	T m_Ax;
	T m_Cx;
	T m_Dy;
	T m_Ay;
	T m_Cy;
	T m_B;
};

/// <summary>
/// octagon.
/// </summary>
template <typename T>
class EMBER_API OctagonVariation : public ParametricVariation<T>
{
public:
	OctagonVariation(T weight = 1.0) : ParametricVariation<T>("octagon", VAR_OCTAGON, weight)
	{
		Init();
	}

	PARVARCOPY(OctagonVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps((SQR(SQR(helper.In.x)) + SQR(helper.In.z) + SQR(SQR(helper.In.y)) + SQR(helper.In.z)));

		if (r < 2)
		{
			helper.Out.x = r * helper.In.x;
			helper.Out.y = r * helper.In.y;
			helper.Out.z = r * helper.In.z;
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
			helper.Out.z = m_Weight * helper.In.z;

			T t = m_Weight / Zeps((sqrt(SQR(helper.In.x)) + sqrt(helper.In.z) + sqrt(SQR(helper.In.y)) + sqrt(helper.In.z)));

			if (r >= 0)
			{
				helper.Out.x = t * helper.In.x;
				helper.Out.y = t * helper.In.y;
				helper.Out.z = t * helper.In.z;
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
				helper.Out.z = m_Weight * helper.In.z;
			}

			if (helper.In.x >= 0)
				helper.Out.x = m_Weight * (helper.In.x + m_X);
			else
				helper.Out.x = m_Weight * (helper.In.x - m_X);

			if (helper.In.y >= 0)
				helper.Out.y = m_Weight * (helper.In.y + m_Y);
			else
				helper.Out.y = m_Weight * (helper.In.y - m_Y);

			if (helper.In.z >= 0)
				helper.Out.z = m_Weight * (helper.In.z + m_Z);
			else
				helper.Out.z = m_Weight * (helper.In.z - m_Z);
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / Zeps((SQR(SQR(vIn.x)) + SQR(vIn.z) + SQR(SQR(vIn.y)) + SQR(vIn.z)));\n"
		   << "\n"
		   << "\t\tif (r < 2)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = r * vIn.x;\n"
		   << "\t\t	vOut.y = r * vIn.y;\n"
		   << "\t\t	vOut.z = r * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\n"
		   << "\t\t	real_t t = xform->m_VariationWeights[" << varIndex << "] / Zeps((sqrt(SQR(vIn.x)) + sqrt(vIn.z) + sqrt(SQR(vIn.y)) + sqrt(vIn.z)));\n"
		   << "\n"
		   << "\t\t	if (r >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = t * vIn.x;\n"
		   << "\t\t		vOut.y = t * vIn.y;\n"
		   << "\t\t		vOut.z = t * vIn.z;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	if (vIn.x >= 0)\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + " << x << ");\n"
		   << "\t\t	else\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x - " << x << ");\n"
		   << "\n"
		   << "\t\t	if (vIn.y >= 0)\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + " << y << ");\n"
		   << "\t\t	else\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - " << y << ");\n"
		   << "\n"
		   << "\t\t	if (vIn.z >= 0)\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z + " << z << ");\n"
		   << "\t\t	else\n"
		   << "\t\t		vOut.z = xform->m_VariationWeights[" << varIndex << "] * (vIn.z - " << z << ");\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "octagon_x"));//Original used a prefix of octa_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "octagon_y"));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "octagon_z"));
	}

private:
	T m_X;
	T m_Y;
	T m_Z;
};

/// <summary>
/// trade.
/// </summary>
template <typename T>
class EMBER_API TradeVariation : public ParametricVariation<T>
{
public:
	TradeVariation(T weight = 1.0) : ParametricVariation<T>("trade", VAR_TRADE, weight)
	{
		Init();
	}

	PARVARCOPY(TradeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, temp, c1mx;

		if (helper.In.x > 0)
		{
			c1mx = m_C1 - helper.In.x;
			r = sqrt(SQR(c1mx) + SQR(helper.In.y));

			if (r <= m_R1)
			{
				r *= m_R2 / m_R1;
				temp = atan2(helper.In.y, c1mx);

				helper.Out.x = m_Weight * (r * cos(temp) - m_C2);
				helper.Out.y = m_Weight *  r * sin(temp);
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}
		else
		{
			c1mx = -m_C2 - helper.In.x;
			r = sqrt(SQR(c1mx) + SQR(helper.In.y));

			if (r <= m_R2)
			{
				r *= m_R1 / m_R2;
				temp = atan2(helper.In.y, c1mx);

				helper.Out.x = m_Weight * (r * cos(temp) + m_C1);
				helper.Out.y = m_Weight *  r * sin(temp);
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}

		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string r1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r, temp, c1mx;\n"
		   << "\n"
		   << "\t\tif (vIn.x > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	c1mx = " << c1 << " - vIn.x;\n"
		   << "\t\t	r = sqrt(SQR(c1mx) + SQR(vIn.y));\n"
		   << "\n"
		   << "\t\t	if (r <= " << r1 << ")\n"
		   << "\t\t	{\n"
		   << "\t\t		r *= " << r2 << " / " << r1 << ";\n"
		   << "\t\t		temp = atan2(vIn.y, c1mx);\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (r * cos(temp) - " << c2 << ");\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] *  r * sin(temp);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	c1mx = -" << c2 << " - vIn.x;\n"
		   << "\t\t	r = sqrt(SQR(c1mx) + SQR(vIn.y));\n"
		   << "\n"
		   << "\t\t	if (r <= " << r2 << ")\n"
		   << "\t\t	{\n"
		   << "\t\t		r *= " << r1 << " / " << r2 << ";\n"
		   << "\t\t		temp = atan2(vIn.y, c1mx);\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (r * cos(temp) + " << c1 << ");\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] *  r * sin(temp);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C1 = m_R1 + m_D1;
		m_C2 = m_R2 + m_D2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R1, prefix + "trade_r1", 1, REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_D1, prefix + "trade_d1", 1, REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_R2, prefix + "trade_r2", 1, REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_D2, prefix + "trade_d2", 1, REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(true, &m_C1, prefix + "trade_c1"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "trade_c2"));
	}

private:
	T m_R1;
	T m_D1;
	T m_R2;
	T m_D2;
	T m_C1;//Precalc.
	T m_C2;
};

/// <summary>
/// Juliac.
/// </summary>
template <typename T>
class EMBER_API JuliacVariation : public ParametricVariation<T>
{
public:
	JuliacVariation(T weight = 1.0) : ParametricVariation<T>("Juliac", VAR_JULIAC, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliacVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T arg = helper.m_PrecalcAtanyx + fmod(T(rand.Rand()), T(1 / m_ReInv)) * M_2PI;
		T lnmod = m_Dist * T(0.5) * log(helper.m_PrecalcSumSquares);
		T temp = arg * m_ReInv + lnmod * m_Im100;
		T mod2 = exp(lnmod * m_ReInv - arg * m_Im100);

		helper.Out.x = m_Weight * mod2 * cos(temp);
		helper.Out.y = m_Weight * mod2 * sin(temp);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string re    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string im    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reInv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string im100 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t arg = precalcAtanyx + fmod((real_t)MwcNext(mwc), (real_t)(1 / " << reInv << ")) * M_2PI;\n"
		   << "\t\treal_t lnmod = " << dist << " * 0.5 * log(precalcSumSquares);\n"
		   << "\t\treal_t temp = arg * " << reInv << " + lnmod * " << im100 << ";\n"
		   << "\t\treal_t mod2 = exp(lnmod * " << reInv << " - arg * " << im100 << ");\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * mod2 * cos(temp);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * mod2 * sin(temp);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_ReInv = 1 / Zeps(m_Re);
		m_Im100 = m_Im * T(0.01);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Re,   prefix + "Juliac_re",   2));
		m_Params.push_back(ParamWithName<T>(&m_Im,   prefix + "Juliac_im",   1));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "Juliac_dist", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_ReInv, prefix + "Juliac_re_inv"));
		m_Params.push_back(ParamWithName<T>(true, &m_Im100, prefix + "Juliac_im100"));
	}

private:
	T m_Re;
	T m_Im;
	T m_Dist;
	T m_ReInv;
	T m_Im100;
};

/// <summary>
/// blade3D.
/// </summary>
template <typename T>
class EMBER_API Blade3DVariation : public Variation<T>
{
public:
	Blade3DVariation(T weight = 1.0) : Variation<T>("blade3D", VAR_BLADE3D, weight, true, true) { }

	VARCOPY(Blade3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = rand.Frand01<T>() * m_Weight * helper.m_PrecalcSqrtSumSquares;
		T sinr, cosr;

		sincos(r, &sinr, &cosr);
		helper.Out.x = m_Weight * helper.In.x * (cosr + sinr);
		helper.Out.y = m_Weight * helper.In.x * (cosr - sinr);
		helper.Out.z = m_Weight * helper.In.z * (sinr - cosr);
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
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z * (sinr - cosr);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Blob3D.
/// </summary>
template <typename T>
class EMBER_API Blob3DVariation : public ParametricVariation<T>
{
public:
	Blob3DVariation(T weight = 1.0) : ParametricVariation<T>("blob3D", VAR_BLOB3D, weight, true, true, true, true)
	{
		Init();
	}

	PARVARCOPY(Blob3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares * (m_BlobLow + m_BlobDiff * (T(0.5) + T(0.5) * sin(m_BlobWaves * helper.m_PrecalcAtanxy)));

		helper.Out.x = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.y = m_Weight * helper.m_PrecalcCosa * r;
		helper.Out.z = m_Weight * sin(m_BlobWaves * helper.m_PrecalcAtanxy) * r;
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
		   << "\t\treal_t r = precalcSqrtSumSquares * (" << blobLow << " + " << blobDiff << " * (0.5 + 0.5 * sin(" << blobWaves << " * precalcAtanxy)));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (precalcSina * r);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (precalcCosa * r);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (sin(" << blobWaves << " * precalcAtanxy) * r);\n"
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
		m_Params.push_back(ParamWithName<T>(&m_BlobLow,   prefix + "blob3D_low"));
		m_Params.push_back(ParamWithName<T>(&m_BlobHigh,  prefix + "blob3D_high", 1));
		m_Params.push_back(ParamWithName<T>(&m_BlobWaves, prefix + "blob3D_waves", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlobDiff, prefix + "blob3D_diff"));//Precalc.
	}

private:
	T m_BlobLow;
	T m_BlobHigh;
	T m_BlobWaves;
	T m_BlobDiff;//Precalc.
};

/// <summary>
/// blocky.
/// </summary>
template <typename T>
class EMBER_API BlockyVariation : public ParametricVariation<T>
{
public:
	BlockyVariation(T weight = 1.0) : ParametricVariation<T>("blocky", VAR_BLOCKY, weight, true)
	{
		Init();
	}

	PARVARCOPY(BlockyVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = Zeps((cos(helper.In.x) + cos(helper.In.y)) / m_Mp + 1);
		T r = m_Weight / t;
		T tmp = helper.m_PrecalcSumSquares + 1;
		T x2 = 2 * helper.In.x;
		T y2 = 2 * helper.In.y;
		T xmax = T(0.5) * (sqrt(tmp + x2) + sqrt(tmp - x2));
		T ymax = T(0.5) * (sqrt(tmp + y2) + sqrt(tmp - y2));
		T a = helper.In.x / Zeps(xmax);
		T b = SafeSqrt(1 - SQR(a));

		helper.Out.x = m_Vx * atan2(a, b) * r;

		a = helper.In.y / Zeps(ymax);
		b = SafeSqrt(1 - SQR(a));

		helper.Out.y = m_Vy * atan2(a, b) * r;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t t = Zeps((cos(vIn.x) + cos(vIn.y)) / " << mp << " + 1);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / t;\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t x2 = 2 * vIn.x;\n"
		   << "\t\treal_t y2 = 2 * vIn.y;\n"
		   << "\t\treal_t xmax = 0.5 * (sqrt(tmp + x2) + sqrt(tmp - x2));\n"
		   << "\t\treal_t ymax = 0.5 * (sqrt(tmp + y2) + sqrt(tmp - y2));\n"
		   << "\t\treal_t a = vIn.x / Zeps(xmax);\n"
		   << "\t\treal_t b = SafeSqrt(1 - SQR(a));\n"
		   << "\n"
		   << "\t\tvOut.x = " << vx << " * atan2(a, b) * r;\n"
		   << "\n"
		   << "\t\ta = vIn.y / Zeps(ymax);\n"
		   << "\t\tb = SafeSqrt(1 - SQR(a));\n"
		   << "\n"
		   << "\t\tvOut.y = " << vy << " * atan2(a, b) * r;\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight / T(M_PI_2);
		m_Vx = m_V * m_X;
		m_Vy = m_V * m_Y;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,  prefix + "blocky_x",  1));
		m_Params.push_back(ParamWithName<T>(&m_Y,  prefix + "blocky_y",  1));
		m_Params.push_back(ParamWithName<T>(&m_Mp, prefix + "blocky_mp", 4, REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_V,  prefix + "blocky_v"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vx, prefix + "blocky_vx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vy, prefix + "blocky_vy"));
	}

private:
	T m_X;
	T m_Y;
	T m_Mp;
	T m_V;//Precalc.
	T m_Vx;
	T m_Vy;
};

MAKEPREPOSTPARVAR(ESwirl, eSwirl, ESWIRL)
MAKEPREPOSTPARVAR(LazyTravis, lazyTravis, LAZY_TRAVIS)
MAKEPREPOSTPARVAR(Squish, squish, SQUISH)
MAKEPREPOSTPARVAR(Circus, circus, CIRCUS)
MAKEPREPOSTVAR(Tancos, tancos, TANCOS)
MAKEPREPOSTVAR(Rippled, rippled, RIPPLED)
MAKEPREPOSTPARVAR(RotateX, rotate_x, ROTATE_X)
MAKEPREPOSTPARVAR(RotateY, rotate_y, ROTATE_Y)
MAKEPREPOSTPARVAR(RotateZ, rotate_z, ROTATE_Z)
MAKEPREPOSTVAR(MirrorX, mirror_x, MIRROR_X)
MAKEPREPOSTVAR(MirrorY, mirror_y, MIRROR_Y)
MAKEPREPOSTVAR(MirrorZ, mirror_z, MIRROR_Z)
MAKEPREPOSTPARVAR(RBlur, rblur, RBLUR)
MAKEPREPOSTPARVAR(JuliaNab, juliaNab, JULIANAB)
MAKEPREPOSTPARVAR(Sintrange, sintrange, SINTRANGE)
MAKEPREPOSTPARVAR(Voron, Voron, VORON)
MAKEPREPOSTPARVARASSIGN(Waffle, waffle, WAFFLE, ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(Square3D, square3D, SQUARE3D, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVARASSIGN(SuperShape3D, SuperShape3D, SUPER_SHAPE3D, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Sphyp3D, sphyp3D, SPHYP3D)
MAKEPREPOSTPARVAR(Circlecrop, circlecrop, CIRCLECROP)
MAKEPREPOSTPARVAR(Julian3Dx, julian3Dx, JULIAN3DX)
MAKEPREPOSTPARVAR(Fourth, fourth, FOURTH)
MAKEPREPOSTPARVAR(Mobiq, mobiq, MOBIQ)
MAKEPREPOSTPARVAR(Spherivoid, spherivoid, SPHERIVOID)
MAKEPREPOSTPARVAR(Farblur, farblur, FARBLUR)
MAKEPREPOSTPARVAR(CurlSP, curl_sp, CURL_SP)
MAKEPREPOSTPARVAR(Heat, heat, HEAT)
MAKEPREPOSTPARVAR(Interference2, interference2, INTERFERENCE2)
MAKEPREPOSTVAR(Sinq, sinq, SINQ)
MAKEPREPOSTVAR(Sinhq, sinhq, SINHQ)
MAKEPREPOSTVAR(Secq, secq, SECQ)
MAKEPREPOSTVAR(Sechq, sechq, SECHQ)
MAKEPREPOSTVAR(Tanq, tanq, TANQ)
MAKEPREPOSTVAR(Tanhq, tanhq, TANHQ)
MAKEPREPOSTVAR(Cosq, cosq, COSQ)
MAKEPREPOSTVAR(Coshq, coshq, COSHQ)
MAKEPREPOSTVAR(Cotq, cotq, COTQ)
MAKEPREPOSTVAR(Cothq, cothq, COTHQ)
MAKEPREPOSTVAR(Cscq, cscq, CSCQ)
MAKEPREPOSTVAR(Cschq, cschq, CSCHQ)
MAKEPREPOSTVAR(Estiq, estiq, ESTIQ)
MAKEPREPOSTPARVAR(Loq, loq, LOQ)
MAKEPREPOSTVAR(Curvature, curvature, CURVATURE)
MAKEPREPOSTPARVAR(Qode, q_ode, Q_ODE)
MAKEPREPOSTPARVARASSIGN(BlurHeart, blur_heart, BLUR_HEART, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Truchet, Truchet, TRUCHET)
MAKEPREPOSTPARVAR(Gdoffs, gdoffs, GDOFFS)
MAKEPREPOSTPARVAR(Octagon, octagon, OCTAGON)
MAKEPREPOSTPARVAR(Trade, trade, TRADE)
MAKEPREPOSTPARVAR(Juliac, Juliac, JULIAC)
MAKEPREPOSTVAR(Blade3D, blade3D, BLADE3D)
MAKEPREPOSTPARVAR(Blob3D, blob3D, BLOB3D)
MAKEPREPOSTPARVAR(Blocky, blocky, BLOCKY)


///// <summary>
///// LinearXZ.
///// </summary>
//template <typename T>
//class EMBER_API LinearXZVariation : public Variation<T>
//{
//public:
//	LinearXZVariation(T weight = 1.0) : Variation<T>("linearxz", VAR_LINEAR_XZ, weight) { }
//
//	VARCOPY(LinearXZVariation)
//
//	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
//	{
//		helper.Out.x = m_Weight * helper.In.x;
//		helper.Out.z = m_Weight * helper.In.z;
//	}
//
//	virtual string OpenCLString() override
//	{
//		ostringstream ss;
//		intmax_t varIndex = IndexInXform();
//
//		ss << "\t{\n"
//		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
//		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
//		   << "\t}\n";
//
//		return ss.str();
//	}
//};
//
///// <summary>
///// LinearYZ.
///// </summary>
//template <typename T>
//class EMBER_API LinearYZVariation : public Variation<T>
//{
//public:
//	LinearYZVariation(T weight = 1.0) : Variation<T>("linearyz", VAR_LINEAR_YZ, weight) { }
//
//	VARCOPY(LinearYZVariation)
//
//	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
//	{
//		helper.Out.y = m_Weight * helper.In.y;
//		helper.Out.z = m_Weight * helper.In.z;
//	}
//
//	virtual string OpenCLString() override
//	{
//		ostringstream ss;
//		intmax_t varIndex = IndexInXform();
//
//		ss << "\t{\n"
//		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
//		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
//		   << "\t}\n";
//
//		return ss.str();
//	}
//};
}
