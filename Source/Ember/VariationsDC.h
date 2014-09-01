#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// DC Bubble.
/// This accesses the summed output point in a rare and different way
/// and therefore cannot be made into pre and post variations.
/// </summary>
template <typename T>
class EMBER_API DCBubbleVariation : public ParametricVariation<T>
{
public:
	DCBubbleVariation(T weight = 1.0) : ParametricVariation<T>("dc_bubble", VAR_DC_BUBBLE, weight, true)
	{
		Init();
	}

	PARVARCOPY(DCBubbleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSumSquares;
		T r4_1 = Zeps(r / 4 + 1);
		r4_1 = m_Weight / r4_1;

		helper.Out.x = r4_1 * helper.In.x;
		helper.Out.y = r4_1 * helper.In.y;
		helper.Out.z = m_Weight * (2 / r4_1 - 1);

		T tempX = helper.Out.x + outPoint.m_X;
		T tempY = helper.Out.y + outPoint.m_Y;

		outPoint.m_ColorX = fmod(fabs(m_Bdcs * (Sqr<T>(tempX + m_CenterX) + Sqr<T>(tempY + m_CenterY))), T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string scale   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string centerX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bdcs    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSumSquares;\n"
		   << "\t\treal_t r4_1 = Zeps(r / 4 + 1);\n"
		   << "\t\tr4_1 = xform->m_VariationWeights[" << varIndex << "] / r4_1;\n"
		   << "\n"
		   << "\t\tvOut.x = r4_1 * vIn.x;\n"
		   << "\t\tvOut.y = r4_1 * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (2 / r4_1 - 1);\n"
		   << "\n"
		   << "\t\treal_t tempX = vOut.x + outPoint->m_X;\n"
		   << "\t\treal_t tempY = vOut.y + outPoint->m_Y;\n"
		   << "\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(" << bdcs << " * (Sqr(tempX + " << centerX << ") + Sqr(tempY + " << centerY << "))), 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Bdcs = 1 / (m_Scale == 0 ? T(10E-6) : m_Scale);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_CenterX, prefix + "dc_bubble_centerx"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_CenterY, prefix + "dc_bubble_centery"));
		m_Params.push_back(ParamWithName<T>(&m_Scale,   prefix + "dc_bubble_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Bdcs, prefix + "dc_bubble_bdcs"));//Precalc.
	}

private:
	T m_CenterX;//Params.
	T m_CenterY;
	T m_Scale;
	T m_Bdcs;//Precalc.
};

/// <summary>
/// DC Carpet.
/// </summary>
template <typename T>
class EMBER_API DCCarpetVariation : public ParametricVariation<T>
{
public:
	DCCarpetVariation(T weight = 1.0) : ParametricVariation<T>("dc_carpet", VAR_DC_CARPET, weight)
	{
		Init();
	}

	PARVARCOPY(DCCarpetVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int x0 = rand.RandBit() ? -1 : 1;
		int y0 = rand.RandBit() ? -1 : 1;
		T x = helper.In.x + x0;
		T y = helper.In.y + y0;
		T x0_xor_y0 = T(x0 ^ y0);
		T h = -m_H + (1 - x0_xor_y0) * m_H;

		helper.Out.x = m_Weight * (m_Xform->m_Affine.A() * x + m_Xform->m_Affine.B() * y + m_Xform->m_Affine.E());
		helper.Out.y = m_Weight * (m_Xform->m_Affine.C() * x + m_Xform->m_Affine.D() * y + m_Xform->m_Affine.F());
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
		outPoint.m_ColorX = fmod(fabs(outPoint.m_ColorX * T(0.5) * (1 + h) + x0_xor_y0 * (1 - h) * T(0.5)), T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string origin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string h      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.

		ss << "\t{\n"
		   << "\t\tint x0 = (MwcNext(mwc) & 1) ? -1 : 1;\n"
		   << "\t\tint y0 = (MwcNext(mwc) & 1) ? -1 : 1;\n"
		   << "\t\treal_t x = vIn.x + x0;\n"
		   << "\t\treal_t y = vIn.y + y0;\n"
		   << "\t\treal_t x0_xor_y0 = (real_t)(x0 ^ y0);\n"
		   << "\t\treal_t h = -" << h << " + (1 - x0_xor_y0) * " << h << ";\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (xform->m_A * x + xform->m_B * y + xform->m_E);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (xform->m_C * x + xform->m_D * y + xform->m_F);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(outPoint->m_ColorX * 0.5 * (1 + h) + x0_xor_y0 * (1 - h) * 0.5), 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_H = T(0.1) * m_Origin;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Origin, prefix + "dc_carpet_origin"));//Params.
		m_Params.push_back(ParamWithName<T>(true, &m_H, prefix + "dc_carpet_h"));//Precalc.
	}

private:
	T m_Origin;//Params.
	T m_H;//Precalc.
};

/// <summary>
/// DC Cube.
/// </summary>
template <typename T>
class EMBER_API DCCubeVariation : public ParametricVariation<T>
{
public:
	DCCubeVariation(T weight = 1.0) : ParametricVariation<T>("dc_cube", VAR_DC_CUBE, weight)
	{
		Init();
	}

	PARVARCOPY(DCCubeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y, z;
		T p = 2 * rand.Frand01<T>() - 1;
		T q = 2 * rand.Frand01<T>() - 1;
		unsigned int i = rand.Rand(3);
		unsigned int j = rand.RandBit();

		switch (i)
		{
			case 0:
				x = m_Weight * (j ? -1 : 1);
				y = m_Weight * p;
				z = m_Weight * q;

				if (j)
					outPoint.m_ColorX = m_ClampC1;
				else
					outPoint.m_ColorX = m_ClampC2;

				break;
			case 1:
				x = m_Weight * p;
				y = m_Weight * (j ? -1 : 1);
				z = m_Weight * q;

				if (j)
					outPoint.m_ColorX = m_ClampC3;
				else
					outPoint.m_ColorX = m_ClampC4;

				break;
			case 2:
				x = m_Weight * p;
				y = m_Weight * q;
				z = m_Weight * (j ? -1 : 1);

				if (j)
					outPoint.m_ColorX = m_ClampC5;
				else
					outPoint.m_ColorX = m_ClampC6;

				break;
		}

		helper.Out.x = x * m_DcCubeX;
		helper.Out.y = y * m_DcCubeY;
		helper.Out.z = z * m_DcCubeZ;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string cubeC1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string cubeC2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC3  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC4  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC5  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC6  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeZ   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string clampC2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC4 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC5 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC6 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x, y, z;\n"
		   << "\t\treal_t p = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\treal_t q = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\tuint i = MwcNext(mwc) & 3;\n"
		   << "\t\tuint j = MwcNext(mwc) & 1;\n"
		   << "\n"
		   << "\t\tswitch (i)\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		x = xform->m_VariationWeights[" << varIndex << "] * (j ? -1 : 1);\n"
		   << "\t\t		y = xform->m_VariationWeights[" << varIndex << "] * p;\n"
		   << "\t\t		z = xform->m_VariationWeights[" << varIndex << "] * q;\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC1 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC2 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		x =xform->m_VariationWeights[" << varIndex << "] * p;\n"
		   << "\t\t		y =xform->m_VariationWeights[" << varIndex << "] * (j ? -1 : 1);\n"
		   << "\t\t		z =xform->m_VariationWeights[" << varIndex << "] * q;\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC3 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC4 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t		x = xform->m_VariationWeights[" << varIndex << "] * p;\n"
		   << "\t\t		y = xform->m_VariationWeights[" << varIndex << "] * q;\n"
		   << "\t\t		z = xform->m_VariationWeights[" << varIndex << "] * (j ? -1 : 1);\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC5 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC6 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = x * " << cubeX << ";\n"
		   << "\t\tvOut.y = y * " << cubeY << ";\n"
		   << "\t\tvOut.z = z * " << cubeZ << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_ClampC1 = Clamp<T>(m_DcCubeC1, 0, 1);
		m_ClampC2 = Clamp<T>(m_DcCubeC2, 0, 1);
		m_ClampC3 = Clamp<T>(m_DcCubeC3, 0, 1);
		m_ClampC4 = Clamp<T>(m_DcCubeC4, 0, 1);
		m_ClampC5 = Clamp<T>(m_DcCubeC5, 0, 1);
		m_ClampC6 = Clamp<T>(m_DcCubeC6, 0, 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC1, prefix + "dc_cube_c1"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC2, prefix + "dc_cube_c2"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC3, prefix + "dc_cube_c3"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC4, prefix + "dc_cube_c4"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC5, prefix + "dc_cube_c5"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC6, prefix + "dc_cube_c6"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeX,  prefix + "dc_cube_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeY,  prefix + "dc_cube_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeZ,  prefix + "dc_cube_z", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC1, prefix + "dc_cube_clamp_c1"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC2, prefix + "dc_cube_clamp_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC3, prefix + "dc_cube_clamp_c3"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC4, prefix + "dc_cube_clamp_c4"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC5, prefix + "dc_cube_clamp_c5"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC6, prefix + "dc_cube_clamp_c6"));
	}

private:
	T m_DcCubeC1;//Params.
	T m_DcCubeC2;
	T m_DcCubeC3;
	T m_DcCubeC4;
	T m_DcCubeC5;
	T m_DcCubeC6;
	T m_DcCubeX;
	T m_DcCubeY;
	T m_DcCubeZ;
	T m_ClampC1;//Precalc.
	T m_ClampC2;
	T m_ClampC3;
	T m_ClampC4;
	T m_ClampC5;
	T m_ClampC6;
};

/// <summary>
/// DC Cylinder.
/// This accesses the summed output point in a rare and different way
/// and therefore cannot be made into pre and post variations.
/// </summary>
template <typename T>
class EMBER_API DCCylinderVariation : public ParametricVariation<T>
{
public:
	DCCylinderVariation(T weight = 1.0) : ParametricVariation<T>("dc_cylinder", VAR_DC_CYLINDER, weight)
	{
		Init();
	}

	PARVARCOPY(DCCylinderVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = rand.Frand01<T>() * M_2PI;
		T sr = sin(temp);
		T cr = cos(temp);
		T r = m_Blur * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);

		helper.Out.x = m_Weight * sin(helper.In.x + r * sr) * m_X;
		helper.Out.y = r + helper.In.y * m_Y;
		helper.Out.z = m_Weight * cos(helper.In.x + r * cr);

		T tempX = helper.Out.x + outPoint.m_X;
		T tempY = helper.Out.y + outPoint.m_Y;

		outPoint.m_ColorX = fmod(fabs(T(0.5) * (m_Ldcs * ((m_Cosa * tempX + m_Sina * tempY + m_Offset)) + 1)), T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string offset = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string angle  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cosa   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldcs   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldca   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t temp = MwcNext(mwc) * M_2PI;\n"
		   << "\t\treal_t sr = sin(temp);\n"
		   << "\t\treal_t cr = cos(temp);\n"
		   << "\t\treal_t r = " << blur << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - 2);\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x + r * sr)* " << x << ";\n"
		   << "\t\tvOut.y = r + vIn.y * " << y << ";\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * cos(vIn.x + r * cr);\n"
		   << "\n"
		   << "\t\treal_t tempX = vOut.x + outPoint->m_X;\n"
		   << "\t\treal_t tempY = vOut.y + outPoint->m_Y;\n"
		   << "\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(0.5 * (" << ldcs << " * ((" << cosa << " * tempX + " << sina << " * tempY + " << offset << ")) + 1.0)), 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Angle, &m_Sina, &m_Cosa);
		m_Ldcs = 1 / (m_Scale == 0.0 ? T(10E-6) : m_Scale);
		m_Ldca = m_Offset * T(M_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Offset, prefix + "dc_cylinder_offset"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Angle,  prefix + "dc_cylinder_angle"));//Original used a prefix of dc_cyl_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Scale,  prefix + "dc_cylinder_scale", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_X,      prefix + "dc_cylinder_x", T(0.125)));//Original used a prefix of cyl_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Y,      prefix + "dc_cylinder_y", T(0.125)));
		m_Params.push_back(ParamWithName<T>(&m_Blur,   prefix + "dc_cylinder_blur", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "dc_cylinder_sina"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "dc_cylinder_cosa"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldcs, prefix + "dc_cylinder_ldcs"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldca, prefix + "dc_cylinder_ldca"));
	}

private:
	T m_Offset;//Params.
	T m_Angle;
	T m_Scale;
	T m_X;
	T m_Y;
	T m_Blur;
	T m_Sina;//Precalc.
	T m_Cosa;
	T m_Ldcs;
	T m_Ldca;
};

/// <summary>
/// DC GridOut.
/// </summary>
template <typename T>
class EMBER_API DCGridOutVariation : public Variation<T>
{
public:
	DCGridOutVariation(T weight = 1.0) : Variation<T>("dc_gridout", VAR_DC_GRIDOUT, weight) { }

	VARCOPY(DCGridOutVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = LRint(helper.In.x);
		T y = LRint(helper.In.y);
		T c = outPoint.m_ColorX;

		if (y <= 0)
		{
			if (x > 0)
			{
				if (-y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
					c += T(0.75);
				}
			}
			else
			{
				if (y <= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
					c += T(0.75);
				}
			}
		}
		else
		{
			if (x > 0)
			{
				if (y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
					c += T(0.75);
				}
			}
			else
			{
				if (y > -x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
					c += T(0.75);
				}
			}
		}

		helper.Out.z = m_Weight * helper.In.z;
		outPoint.m_ColorX = fmod(c, T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		int varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t x = LRint(vIn.x);\n"
		   << "\t\treal_t y = LRint(vIn.y);\n"
		   << "\t\treal_t c = outPoint->m_ColorX;\n"
		   << "\n"
		   << "\t\tif (y <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (-y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t			c += 0.25;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + 1);\n"
		   << "\t\t			c += 0.75;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y <= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t			c += 0.25;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - 1);\n"
		   << "\t\t			c += 0.75;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t			c += 0.25;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + 1);\n"
		   << "\t\t			c += 0.75;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y > -x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t			c += 0.25;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - 1);\n"
		   << "\t\t			c += 0.75;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\toutPoint->m_ColorX = fmod(c, 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// DC Linear.
/// This accesses the summed output point in a rare and different way
/// and therefore cannot be made into pre and post variations.
/// </summary>
template <typename T>
class EMBER_API DCLinearVariation : public ParametricVariation<T>
{
public:
	DCLinearVariation(T weight = 1.0) : ParametricVariation<T>("dc_linear", VAR_DC_LINEAR, weight)
	{
		Init();
	}

	PARVARCOPY(DCLinearVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;

		T tempX = helper.Out.x + outPoint.m_X;
		T tempY = helper.Out.y + outPoint.m_Y;

		outPoint.m_ColorX = fmod(fabs(T(0.5) * (m_Ldcs * ((m_Cosa * tempX + m_Sina * tempY + m_Offset)) + T(1.0))), T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string offset = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string angle  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldcs   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string ldca   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosa   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\n"
		   << "\t\treal_t tempX = vOut.x + outPoint->m_X;\n"
		   << "\t\treal_t tempY = vOut.y + outPoint->m_Y;\n"
		   << "\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(0.5 * (" << ldcs << " * ((" << cosa << " * tempX + " << sina << " * tempY + " << offset << ")) + 1.0)), 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ldcs = 1 / (m_Scale == 0 ? T(10E-6) : m_Scale);
		m_Ldca = m_Offset * T(M_PI);
		sincos(m_Angle, &m_Sina, &m_Cosa);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Offset, prefix + "dc_linear_offset"));//Params.
		m_Params.push_back(ParamWithName<T>(&m_Angle,  prefix + "dc_linear_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Scale,  prefix + "dc_linear_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldcs, prefix + "dc_linear_ldcs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Ldca, prefix + "dc_linear_ldca"));
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "dc_linear_sina"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "dc_linear_cosa"));
	}

private:
	T m_Offset;//Params.
	T m_Angle;
	T m_Scale;
	T m_Ldcs;//Precalc.
	T m_Ldca;
	T m_Sina;
	T m_Cosa;
};

/// <summary>
/// DC Triangle.
/// </summary>
template <typename T>
class EMBER_API DCTriangleVariation : public ParametricVariation<T>
{
public:
	DCTriangleVariation(T weight = 1.0) : ParametricVariation<T>("dc_triangle", VAR_DC_TRIANGLE, weight)
	{
		Init();
	}

	PARVARCOPY(DCTriangleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		// set up triangle
		const T
		xx = m_Xform->m_Affine.A(), xy = m_Xform->m_Affine.B(),  // X
		yx = m_Xform->m_Affine.C() * -1, yy = m_Xform->m_Affine.D() * -1,  // Y
		ox = m_Xform->m_Affine.E(), oy = m_Xform->m_Affine.F(),  // O
		px = helper.In.x - ox, py = helper.In.y - oy; // P

		// calculate dot products
		const T dot00 = xx * xx + xy * xy; // X * X
		const T dot01 = xx * yx + xy * yy; // X * Y
		const T dot02 = xx * px + xy * py; // X * P
		const T dot11 = yx * yx + yy * yy; // Y * Y
		const T dot12 = yx * px + yy * py; // Y * P

		// calculate barycentric coordinates
		const T denom = (dot00 * dot11 - dot01 * dot01);
		const T num_u = (dot11 * dot02 - dot01 * dot12);
		const T num_v = (dot00 * dot12 - dot01 * dot02);

		// u, v must not be constant
		T u = num_u / denom;
		T v = num_v / denom;
		int inside = 0, f = 1;

		// case A - point escapes edge XY
		if (u + v > 1)
		{
			f = -1;

			if (u > v)
			{
				ClampLteRef<T>(u, 1);
				v = 1 - u;
			}
			else
			{
				ClampLteRef<T>(v, 1);
				u = 1 - v;
			}
		}
		else if ((u < 0) || (v < 0))// case B - point escapes either edge OX or OY
		{
			ClampRef<T>(u, 0, 1);
			ClampRef<T>(v, 0, 1);
		}
		else
		{
			inside = 1;// case C - point is in triangle
		}

		// handle outside points
		if (m_ZeroEdges && !inside)
		{
			u = v = 0;
		}
		else if (!inside)
		{
			u = (u + rand.Frand01<T>() * m_A * f);
			v = (v + rand.Frand01<T>() * m_A * f);

			ClampRef<T>(u, -1, 1);
			ClampRef<T>(v, -1, 1);

			if ((u + v > 1) && (m_A > 0))
			{
				if (u > v)
				{
					ClampLteRef<T>(u, 1);
					v = 1 - u;
				}
				else
				{
					ClampLteRef<T>(v, 1);
					u = 1 - v;
				}
			}
		}

		// set output
		helper.Out.x = m_Weight * (ox + u * xx + v * yx);
		helper.Out.y = m_Weight * (oy + u * xy + v * yy);
		helper.Out.z = m_Weight * helper.In.z;
		outPoint.m_ColorX = fmod(fabs(u + v), T(1.0));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string scatterArea = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string zeroEdges   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.

		ss << "\t{\n"
		   << "\t\tconst real_t\n"
		   << "\t\txx = xform->m_A, xy = xform->m_B,\n"
		   << "\t\tyx = xform->m_C * -1, yy = xform->m_D * -1,\n"
		   << "\t\tox = xform->m_E, oy = xform->m_F,\n"
		   << "\t\tpx = vIn.x - ox, py = vIn.y - oy;\n"
		   << "\n"
		   << "\t\tconst real_t dot00 = xx * xx + xy * xy;\n"
		   << "\t\tconst real_t dot01 = xx * yx + xy * yy;\n"
		   << "\t\tconst real_t dot02 = xx * px + xy * py;\n"
		   << "\t\tconst real_t dot11 = yx * yx + yy * yy;\n"
		   << "\t\tconst real_t dot12 = yx * px + yy * py;\n"
		   << "\n"
		   << "\t\tconst real_t denom = (dot00 * dot11 - dot01 * dot01);\n"
		   << "\t\tconst real_t num_u = (dot11 * dot02 - dot01 * dot12);\n"
		   << "\t\tconst real_t num_v = (dot00 * dot12 - dot01 * dot02);\n"
		   << "\n"
		   << "\t\treal_t u = num_u / denom;\n"
		   << "\t\treal_t v = num_v / denom;\n"
		   << "\t\tint inside = 0, f = 1;\n"
		   << "\n"
		   << "\t\tif (u + v > 1)\n"
		   << "\t\t{\n"
		   << "\t\t	f = -1;\n"
		   << "\n"
		   << "\t\t	if (u > v)\n"
		   << "\t\t	{\n"
		   << "\t\t		u = u > 1 ? 1 : u;\n"
		   << "\t\t		v = 1 - u;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		v = v > 1 ? 1 : v;\n"
		   << "\t\t		u = 1 - v;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse if ((u < 0) || (v < 0))\n"
		   << "\t\t{\n"
		   << "\t\t	u = u < 0 ? 0 : u > 1 ? 1 : u;\n"
		   << "\t\t	v = v < 0 ? 0 : v > 1 ? 1 : v;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	inside = 1;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tif (" << zeroEdges << " && !inside)\n"
		   << "\t\t{\n"
		   << "\t\t	u = v = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse if (!inside)\n"
		   << "\t\t{\n"
		   << "\t\t	u = (u + MwcNext01(mwc) * " << a << " * f);\n"
		   << "\t\t	v = (v + MwcNext01(mwc) * " << a << " * f);\n"
		   << "\t\t	u = u < -1 ? -1 : u > 1 ? 1 : u;\n"
		   << "\t\t	v = v < -1 ? -1 : v > 1 ? 1 : v;\n"
		   << "\n"
		   << "\t\t	if ((u + v > 1) && (" << a << " > 0))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (u > v)\n"
		   << "\t\t		{\n"
		   << "\t\t			u = u > 1 ? 1 : u;\n"
		   << "\t\t			v = 1 - u;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			v = v > 1 ? 1 : v;\n"
		   << "\t\t			u = 1 - v;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (ox + u * xx + v * yx);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (oy + u * xy + v * yy);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(u + v), 1.0);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_A = Clamp<T>(m_ScatterArea, -1, 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ScatterArea, prefix + "dc_triangle_scatter_area", 0, REAL, -1, 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_ZeroEdges,   prefix + "dc_triangle_zero_edges", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_A,     prefix + "dc_triangle_a"));//Precalc.
	}

private:
	T m_ScatterArea;//Params.
	T m_ZeroEdges;
	T m_A;//Precalc.
};

/// <summary>
/// DC Transl.
/// The original used dc_ztransl and post_dcztransl incompatible with Ember's design.
/// These will follow the same naming convention as all other variations.
/// </summary>
template <typename T>
class EMBER_API DCZTranslVariation : public ParametricVariation<T>
{
public:
	DCZTranslVariation(T weight = 1.0) : ParametricVariation<T>("dc_ztransl", VAR_DC_ZTRANSL, weight)
	{
		Init();
	}

	PARVARCOPY(DCZTranslVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T zf = m_Factor * (outPoint.m_ColorX - m_X0_) / m_X1_m_x0;

		if (m_Clamp != 0)
			ClampRef<T>(zf, 0, 1);

		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;

		if (m_Overwrite == 0)
			helper.Out.z = m_Weight * helper.In.z * zf;
		else
			helper.Out.z = m_Weight * zf;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x0        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Params.
		string x1        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string factor    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string overwrite = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string clamp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0_       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_m_x0   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t zf = " << factor << " * (outPoint->m_ColorX - " << x0_ << ") / " << x1_m_x0 << ";\n"
		   << "\n"
		   << "\t\tif (" << clamp << " != 0)\n"
		   << "\t\t	zf = zf < 0 ? 0 : zf > 1 ? 1 : zf;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\n"
		   << "\t\tif (" << overwrite << " == 0)\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z * zf;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * zf;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_X0_ = m_X0 < m_X1 ? m_X0 : m_X1;
		m_X1_ = m_X0 > m_X1 ? m_X0 : m_X1;
		m_X1_m_x0 = Zeps(m_X1_ - m_X0_);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X0,        prefix + "dc_ztransl_x0", 0, REAL, 0, 1));//Params.
		m_Params.push_back(ParamWithName<T>(&m_X1,        prefix + "dc_ztransl_x1", 1, REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Factor,    prefix + "dc_ztransl_factor", 1));
		m_Params.push_back(ParamWithName<T>(&m_Overwrite, prefix + "dc_ztransl_overwrite", 1, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Clamp,     prefix + "dc_ztransl_clamp", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_X0_,     prefix + "dc_ztransl_x0_"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_X1_,     prefix + "dc_ztransl_x1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_X1_m_x0, prefix + "dc_ztransl_x1_m_x0"));
	}

private:
	T m_X0;//Params.
	T m_X1;
	T m_Factor;
	T m_Overwrite;
	T m_Clamp;
	T m_X0_;//Precalc.
	T m_X1_;
	T m_X1_m_x0;
};

MAKEPREPOSTPARVAR(DCCarpet, dc_carpet, DC_CARPET)
MAKEPREPOSTPARVARASSIGN(DCCube, dc_cube, DC_CUBE, ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(DCGridOut, dc_gridout, DC_GRIDOUT)
MAKEPREPOSTPARVAR(DCTriangle, dc_triangle, DC_TRIANGLE)
MAKEPREPOSTPARVAR(DCZTransl, dc_ztransl, DC_ZTRANSL)
}
