#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// Hemisphere.
/// </summary>
template <typename T>
class EMBER_API HemisphereVariation : public Variation<T>
{
public:
	HemisphereVariation(T weight = 1.0) : Variation<T>("hemisphere", VAR_HEMISPHERE, weight, true) { }

	VARCOPY(HemisphereVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = m_Weight / sqrt(helper.m_PrecalcSumSquares + 1);

		helper.Out.x = helper.In.x * t;
		helper.Out.y = helper.In.y * t;
		helper.Out.z = t;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t t = xform->m_VariationWeights[" << varIndex << "] / sqrt(precalcSumSquares + (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * t;\n"
		   << "\t\tvOut.y = vIn.y * t;\n"
		   << "\t\tvOut.z = t;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Epispiral.
/// </summary>
template <typename T>
class EMBER_API EpispiralVariation : public ParametricVariation<T>
{
public:
	EpispiralVariation(T weight = 1.0) : ParametricVariation<T>("epispiral", VAR_EPISPIRAL, weight, false, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(EpispiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = helper.m_PrecalcAtanyx;
		T t = (rand.Frand01<T>() * m_Thickness) * (1 / cos(m_N * theta)) - m_Holes;

		if (fabs(t) != 0)
		{
			helper.Out.x = m_Weight * t * cos(theta);
			helper.Out.y = m_Weight * t * sin(theta);
			helper.Out.z = 0;
		}
		else
		{
			helper.Out.x = 0;
			helper.Out.y = 0;
			helper.Out.z = 0;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string n         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t theta = precalcAtanyx;\n"
		   << "\t\treal_t t = (MwcNext01(mwc) * " << thickness << ") * (1 / cos(" << n << " * theta)) - " << holes << ";\n"
		   << "\n"
		   << "\t\tif (fabs(t) != 0)\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * t * cos(theta);\n"
		   << "\t\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * t * sin(theta);\n"
		   << "\t\t\tvOut.z = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = 0;\n"
		   << "\t\t\tvOut.y = 0;\n"
		   << "\t\t\tvOut.z = 0;\n"
		   << "\t\t}\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N,         prefix + "epispiral_n", 6));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "epispiral_thickness"));
		m_Params.push_back(ParamWithName<T>(&m_Holes,     prefix + "epispiral_holes", 1));
	}

private:
	T m_N;
	T m_Thickness;
	T m_Holes;
};

/// <summary>
/// Bwraps.
/// Note, this is the same as bwraps2.
/// </summary>
template <typename T>
class EMBER_API BwrapsVariation : public ParametricVariation<T>
{
public:
	BwrapsVariation(T weight = 1.0) : ParametricVariation<T>("bwraps", VAR_BWRAPS, weight)
	{
		Init();
	}

	PARVARCOPY(BwrapsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_BwrapsCellsize == 0)
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}
		else
		{
			T vx = helper.In.x;
			T vy = helper.In.y;
			T cx = (Floor<T>(vx / m_BwrapsCellsize) + T(0.5)) * m_BwrapsCellsize;
			T cy = (Floor<T>(vy / m_BwrapsCellsize) + T(0.5)) * m_BwrapsCellsize;
			T lx = vx - cx;
			T ly = vy - cy;

			if ((SQR(lx) + SQR(ly)) > m_R2)
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				lx *= m_G2;
				ly *= m_G2;

				T r = m_Rfactor / ((SQR(lx) + SQR(ly)) / 4 + 1);

				lx *= r;
				ly *= r;
				r = (SQR(lx) + SQR(ly)) / m_R2;

				T theta = m_BwrapsInnerTwist * (1 - r) + m_BwrapsOuterTwist * r;
				T s = sin(theta);
				T c = cos(theta);

				vx = cx + c * lx + s * ly;
				vy = cy - s * lx + c * ly;

				helper.Out.x = m_Weight * vx;
				helper.Out.y = m_Weight * vy;
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
		string bwrapsCellsize   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsSpace      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsGain       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsInnerTwist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsOuterTwist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string g2               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r2               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rfactor          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif (" << bwrapsCellsize << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t vx = vIn.x;\n"
		   << "\t\t	real_t vy = vIn.y;\n"
		   << "\t\t	real_t cx = (floor(vx / " << bwrapsCellsize << ") + (real_t)(0.5)) * " << bwrapsCellsize << ";\n"
		   << "\t\t	real_t cy = (floor(vy / " << bwrapsCellsize << ") + (real_t)(0.5)) * " << bwrapsCellsize << ";\n"
		   << "\t\t	real_t lx = vx - cx;\n"
		   << "\t\t	real_t ly = vy - cy;\n"
		   << "\n"
		   << "\t\t	if ((SQR(lx) + SQR(ly)) > " << r2 << ")\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		lx *= " << g2 << ";\n"
		   << "\t\t		ly *= " << g2 << ";\n"
		   << "\n"
		   << "\t\t		real_t r = " << rfactor << " / ((SQR(lx) + SQR(ly)) / 4 + 1);\n"
		   << "\n"
		   << "\t\t		lx *= r;\n"
		   << "\t\t		ly *= r;\n"
		   << "\t\t		r = (SQR(lx) + SQR(ly)) / " << r2 << ";\n"
		   << "\n"
		   << "\t\t		real_t theta = " << bwrapsInnerTwist << " * (1 - r) + " << bwrapsOuterTwist << " * r;\n"
		   << "\t\t		real_t s = sin(theta);\n"
		   << "\t\t		real_t c = cos(theta);\n"
		   << "\n"
		   << "\t\t		vx = cx + c * lx + s * ly;\n"
		   << "\t\t		vy = cy - s * lx + c * ly;\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * vx;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * vy;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T radius = T(0.5) * (m_BwrapsCellsize / (1 + SQR(m_BwrapsSpace)));
		m_G2 = Zeps(SQR(m_BwrapsGain) / Zeps(radius));
		T maxBubble = m_G2 * radius;

		if (maxBubble > 2)
			maxBubble = 1;
		else
			maxBubble *= (1 / (SQR(maxBubble) / 4 + 1));

		m_R2 = SQR(radius);
		m_Rfactor = radius / maxBubble;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BwrapsCellsize,   prefix + "bwraps_cellsize", 1));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsSpace,      prefix + "bwraps_space"));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsGain,       prefix + "bwraps_gain", 1));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsInnerTwist, prefix + "bwraps_inner_twist"));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsOuterTwist, prefix + "bwraps_outer_twist"));
		m_Params.push_back(ParamWithName<T>(true, &m_G2,      prefix + "bwraps_g2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R2,      prefix + "bwraps_r2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rfactor, prefix + "bwraps_rfactor"));
	}

private:
	T m_BwrapsCellsize;
	T m_BwrapsSpace;
	T m_BwrapsGain;
	T m_BwrapsInnerTwist;
	T m_BwrapsOuterTwist;
	T m_G2;//Precalc.
	T m_R2;
	T m_Rfactor;
};

/// <summary>
/// BlurCircle.
/// </summary>
template <typename T>
class EMBER_API BlurCircleVariation : public Variation<T>
{
public:
	BlurCircleVariation(T weight = 1.0) : Variation<T>("blur_circle", VAR_BLUR_CIRCLE, weight) { }

	VARCOPY(BlurCircleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = 2 * rand.Frand01<T>() - 1;
		T y = 2 * rand.Frand01<T>() - 1;
		T absx = x;
		T absy = y;
		T side, perimeter;

		if (absx < 0)
			absx = absx * -1;

		if (absy < 0)
			absy = absy * -1;

		if (absx >= absy)
		{
			if (x >= absy)
				perimeter = absx + y;
			else
				perimeter = 5 * absx - y;

			side = absx;
		}
		else
		{
			if (y >= absx)
				perimeter = 3 * absy - x;
			else
				perimeter = 7 * absy + x;

			side = absy;
		}

		T r = m_Weight * side;
		T val = T(M_PI_4) * perimeter / side - T(M_PI_4);
		T sina = sin(val);
		T cosa = cos(val);

		helper.Out.x = r * cosa;
		helper.Out.y = r * sina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();

		ss << "\t{\n"
		   << "\t\treal_t x = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\treal_t y = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\treal_t absx = x;\n"
		   << "\t\treal_t absy = y;\n"
		   << "\t\treal_t side, perimeter;\n"
		   << "\t\t\n"
		   << "\t\tif (absx < 0)\n"
		   << "\t\t	absx = absx * -1;\n"
		   << "\n"
		   << "\t\tif (absy < 0)\n"
		   << "\t\t	absy = absy * -1;\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= absy)\n"
		   << "\t\t		perimeter = absx + y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 5 * absx - y;\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (y >= absx)\n"
		   << "\t\t		perimeter = 3 * absy - x;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 7 * absy + x;\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * side;\n"
		   << "\t\treal_t val = M_PI_4 * perimeter / side - M_PI_4;\n"
		   << "\t\treal_t sina = sin(val);\n"
		   << "\t\treal_t cosa = cos(val);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cosa;\n"
		   << "\t\tvOut.y = r * sina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// BlurZoom.
/// </summary>
template <typename T>
class EMBER_API BlurZoomVariation : public ParametricVariation<T>
{
public:
	BlurZoomVariation(T weight = 1.0) : ParametricVariation<T>("blur_zoom", VAR_BLUR_ZOOM, weight)
	{
		Init();
	}

	PARVARCOPY(BlurZoomVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T z = 1 + m_BlurZoomLength * rand.Frand01<T>();

		helper.Out.x = m_Weight * ((helper.In.x - m_BlurZoomX) * z + m_BlurZoomX);
		helper.Out.y = m_Weight * ((helper.In.y - m_BlurZoomY) * z - m_BlurZoomY);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string blurZoomLength = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurZoomX      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurZoomY      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t z = 1 + " << blurZoomLength << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * ((vIn.x - " << blurZoomX << ") * z + " << blurZoomX << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * ((vIn.y - " << blurZoomY << ") * z - " << blurZoomY << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomLength, prefix + "blur_zoom_length"));
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomX,      prefix + "blur_zoom_x"));
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomY,      prefix + "blur_zoom_y"));
	}

private:
	T m_BlurZoomLength;
	T m_BlurZoomX;
	T m_BlurZoomY;
};

/// <summary>
/// BlurPixelize.
/// </summary>
template <typename T>
class EMBER_API BlurPixelizeVariation : public ParametricVariation<T>
{
public:
	BlurPixelizeVariation(T weight = 1.0) : ParametricVariation<T>("blur_pixelize", VAR_BLUR_PIXELIZE, weight)
	{
		Init();
	}

	PARVARCOPY(BlurPixelizeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = T(Floor<T>(helper.In.x * m_InvSize));
		T y = T(Floor<T>(helper.In.y * m_InvSize));

		helper.Out.x = m_V * (x + m_BlurPixelizeScale * (rand.Frand01<T>() - T(0.5)) + T(0.5));
		helper.Out.y = m_V * (y + m_BlurPixelizeScale * (rand.Frand01<T>() - T(0.5)) + T(0.5));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string blurPixelizeSize  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurPixelizeScale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v                 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invSize           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = floor(vIn.x * " << invSize << ");\n"
		   << "\t\treal_t y = floor(vIn.y * " << invSize << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << v << " * (x + " << blurPixelizeScale << " * (MwcNext01(mwc) - (real_t)(0.5)) + (real_t)(0.5));\n"
		   << "\t\tvOut.y = " << v << " * (y + " << blurPixelizeScale << " * (MwcNext01(mwc) - (real_t)(0.5)) + (real_t)(0.5));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight * m_BlurPixelizeSize;
		m_InvSize = 1 / m_BlurPixelizeSize;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurPixelizeSize,  prefix + "blur_pixelize_size", T(0.1), REAL, EPS));
		m_Params.push_back(ParamWithName<T>(&m_BlurPixelizeScale, prefix + "blur_pixelize_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_V,	      prefix + "blur_pixelize_v"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvSize, prefix + "blur_pixelize_inv_size"));
	}

private:
	T m_BlurPixelizeSize;
	T m_BlurPixelizeScale;
	T m_V;//Precalc.
	T m_InvSize;
};

/// <summary>
/// Crop.
/// </summary>
template <typename T>
class EMBER_API CropVariation : public ParametricVariation<T>
{
public:
	CropVariation(T weight = 1.0) : ParametricVariation<T>("crop", VAR_CROP, weight)
	{
		Init();
	}

	PARVARCOPY(CropVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x;
		T y = helper.In.y;

		if (((x < m_X0_) || (x > m_X1_) || (y < m_Y0_) || (y > m_Y1_)) && m_Z != 0)
		{
			x = 0;
			y = 0;
		}
		else
		{
			if (x < m_X0_)
				x = m_X0_ + rand.Frand01<T>() * m_W;
			else if (x > m_X1_)
				x = m_X1_ - rand.Frand01<T>() * m_W;

			if (y < m_Y0_)
				y = m_Y0_ + rand.Frand01<T>() * m_H;
			else if (y > m_Y1_)
				y = m_Y1_ - rand.Frand01<T>() * m_H;
		}

		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x0  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y1  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y1_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string w   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string h   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x;\n"
		   << "\t\treal_t y = vIn.y;\n"
			<< "\n"
		   << "\t\tif (((x < " << x0_ << ") || (x > " << x1_ << ") || (y < " << y0_ << ") || (y > " << y1_ << ")) && " << z << " != 0)\n"
		   << "\t\t{\n"
		   << "\t\t	x = 0;\n"
		   << "\t\t	y = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x < " << x0_ << ")\n"
		   << "\t\t		x = " << x0_ << " + MwcNext01(mwc) * " << w << ";\n"
		   << "\t\t	else if (x > " << x1_ << ")\n"
		   << "\t\t		x = " << x1_ << " - MwcNext01(mwc) * " << w << ";\n"
		   << "\t\t\n"
		   << "\t\t	if (y < " << y0_ << ")\n"
		   << "\t\t		y = " << y0_ << " + MwcNext01(mwc) * " << h << ";\n"
		   << "\t\t	else if (y > " << y1_ << ")\n"
		   << "\t\t		y = " << y1_ << " - MwcNext01(mwc) * " << h << ";\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_X0 < m_X1)
		{
			m_X0_ = m_X0;
			m_X1_ = m_X1;
		}
		else
		{
			m_X0_ = m_X1;
			m_X1_ = m_X0;
		}

		if (m_Y0 < m_Y1)
		{
			m_Y0_ = m_Y0;
			m_Y1_ = m_Y1;
		}
		else
		{
			m_Y0_ = m_Y1;
			m_Y1_ = m_Y0;
		}

		m_W = (m_X1_ - m_X0_) * T(0.5) * m_S;
		m_H = (m_Y1_ - m_Y0_) * T(0.5) * m_S;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X0, prefix + "crop_left", -1));
		m_Params.push_back(ParamWithName<T>(&m_Y0, prefix + "crop_top", -1));
		m_Params.push_back(ParamWithName<T>(&m_X1, prefix + "crop_right", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y1, prefix + "crop_bottom", 1));
		m_Params.push_back(ParamWithName<T>(&m_S,  prefix + "crop_scatter_area", 0, REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Z,  prefix + "crop_zero", 0, INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_X0_, prefix + "crop_x0_"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Y0_, prefix + "crop_y0_"));
		m_Params.push_back(ParamWithName<T>(true, &m_X1_, prefix + "crop_x1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_Y1_, prefix + "crop_y1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_W,   prefix + "crop_w"));
		m_Params.push_back(ParamWithName<T>(true, &m_H,   prefix + "crop_h"));
	}

private:
	T m_X0;
	T m_Y0;
	T m_X1;
	T m_Y1;
	T m_S;
	T m_Z;
	T m_X0_;//Precalc.
	T m_Y0_;
	T m_X1_;
	T m_Y1_;
	T m_W;
	T m_H;
};

/// <summary>
/// BCircle.
/// </summary>
template <typename T>
class EMBER_API BCircleVariation : public ParametricVariation<T>
{
public:
	BCircleVariation(T weight = 1.0) : ParametricVariation<T>("bcircle", VAR_BCIRCLE, weight)
	{
		Init();
	}

	PARVARCOPY(BCircleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if ((helper.In.x == 0) && (helper.In.y == 0))
			return;

		T x = helper.In.x * m_Scale;
		T y = helper.In.y * m_Scale;
		T r = sqrt(SQR(x) + SQR(y));

		if (r <= 1)
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			if (m_Bcbw != 0)
			{
				T ang = atan2(y, x);
				T omega = (T(0.2) * m_Bcbw * rand.Frand01<T>()) + 1;
				T px = omega * cos(ang);
				T py = omega * sin(ang);
				helper.Out.x = m_Weight * px;
				helper.Out.y = m_Weight * py;
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
		string scale        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string borderWidth  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bcbw         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tif ((vIn.x == 0) && (vIn.y == 0))\n"
		   << "\t\t	return;\n"
		   << "\n"
		   << "\t\treal_t x = vIn.x * " << scale << ";\n"
		   << "\t\treal_t y = vIn.y * " << scale << ";\n"
		   << "\t\treal_t r = sqrt(SQR(x) + SQR(y));\n"
		   << "\n"
		   << "\t\tif (r <= 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (" << bcbw << " != 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t ang = atan2(y, x);\n"
		   << "\t\t		real_t omega = ((real_t)(0.2) * " << bcbw << " * MwcNext01(mwc)) + 1;\n"
		   << "\t\t		real_t px = omega * cos(ang);\n"
		   << "\t\t		real_t py = omega * sin(ang);\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * px;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * py;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Bcbw = fabs(m_BorderWidth);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scale,	    prefix + "bcircle_scale", 1));
		m_Params.push_back(ParamWithName<T>(&m_BorderWidth, prefix + "bcircle_borderwidth"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bcbw, prefix + "bcircle_bcbw"));//Precalc.
	}

private:
	T m_Scale;
	T m_BorderWidth;
	T m_Bcbw;//Precalc.
};

/// <summary>
/// BlurLinear.
/// </summary>
template <typename T>
class EMBER_API BlurLinearVariation : public ParametricVariation<T>
{
public:
	BlurLinearVariation(T weight = 1.0) : ParametricVariation<T>("blur_linear", VAR_BLUR_LINEAR, weight)
	{
		Init();
	}

	PARVARCOPY(BlurLinearVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_BlurLinearLength * rand.Frand01<T>();

		helper.Out.x = m_Weight * (helper.In.x + r * m_C);
		helper.Out.y = m_Weight * (helper.In.y + r * m_S);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string blurLinearLength = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurLinearAngle  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s                = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c                = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = " << blurLinearLength << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + r * " << c << ");\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + r * " << s << ");\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_BlurLinearAngle, &m_S, &m_C);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurLinearLength, prefix + "blur_linear_length"));
		m_Params.push_back(ParamWithName<T>(&m_BlurLinearAngle,  prefix + "blur_linear_angle", 0, REAL_CYCLIC, 0, T(M_2PI)));
		m_Params.push_back(ParamWithName<T>(true, &m_S, prefix + "blur_linear_s"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "blur_linear_c"));
	}

private:
	T m_BlurLinearLength;
	T m_BlurLinearAngle;
	T m_S;//Precalc.
	T m_C;
};

/// <summary>
/// BlurSquare.
/// </summary>
template <typename T>
class EMBER_API BlurSquareVariation : public ParametricVariation<T>
{
public:
	BlurSquareVariation(T weight = 1.0) : ParametricVariation<T>("blur_square", VAR_BLUR_SQUARE, weight)
	{
		Init();
	}

	PARVARCOPY(BlurSquareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_V * (rand.Frand01<T>() - T(0.5));
		helper.Out.y = m_V * (rand.Frand01<T>() - T(0.5));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		ss << "\t{\n"
		   << "\t\tvOut.x = " << v << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.y = " << v << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight * 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "blur_square_v"));//Precalcs only, no params.
	}

private:
	T m_V;
};

/// <summary>
/// Flatten.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API FlattenVariation : public Variation<T>
{
public:
	FlattenVariation(T weight = 1.0) : Variation<T>("flatten", VAR_FLATTEN, weight) { }

	VARCOPY(FlattenVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)//Rare and different usage of in/out.
		{
			helper.Out.x = helper.Out.y = helper.Out.z = 0;
			outPoint.m_Z = 0;
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
			helper.Out.z = 0;
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;

		if (m_VarType == VARTYPE_REG)
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = 0;\n"
			   << "\t\tvOut.y = 0;\n"
			   << "\t\tvOut.z = 0;\n"
			   << "\t\toutPoint->m_Z = 0;\n"
			   << "\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n"
			   << "\t\tvOut.z = 0;\n"
			   << "\t}\n";
		}

		return ss.str();
	}
};

/// <summary>
/// Zblur.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API ZblurVariation : public Variation<T>
{
public:
	ZblurVariation(T weight = 1.0) : Variation<T>("zblur", VAR_ZBLUR, weight) { }

	VARCOPY(ZblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// ZScale.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API ZScaleVariation : public Variation<T>
{
public:
	ZScaleVariation(T weight = 1.0) : Variation<T>("zscale", VAR_ZSCALE, weight) { }

	VARCOPY(ZScaleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// ZTranslate.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API ZTranslateVariation : public Variation<T>
{
public:
	ZTranslateVariation(T weight = 1.0) : Variation<T>("ztranslate", VAR_ZTRANSLATE, weight) { }

	VARCOPY(ZTranslateVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// zcone.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class EMBER_API ZConeVariation : public Variation<T>
{
public:
	ZConeVariation(T weight = 1.0) : Variation<T>("zcone", VAR_ZCONE, weight, true, true) { }

	VARCOPY(ZConeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)//Rare and different usage of in/out.
		{
			helper.Out.x = helper.Out.y = 0;
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
		}

		helper.Out.z = m_Weight * helper.m_PrecalcSqrtSumSquares;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n";

		if (m_VarType == VARTYPE_REG)
		{
			ss << "\t\tvOut.x = vOut.y = 0;\n";
		}
		else
		{
			ss << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n";
		}

		ss << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Blur3D.
/// </summary>
template <typename T>
class EMBER_API Blur3DVariation : public Variation<T>
{
public:
	Blur3DVariation(T weight = 1.0) : Variation<T>("blur3D", VAR_BLUR3D, weight) { }

	VARCOPY(Blur3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		T angle2 = rand.Frand01<T>() * T(M_PI);
		T sina = sin(angle);
		T cosa = cos(angle);
		T sinb = sin(angle2);
		T cosb = cos(angle2);

		helper.Out.x = r * sinb * cosa;
		helper.Out.y = r * sinb * sina;
		helper.Out.z = r * cosb;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\t\treal_t angle2 = MwcNext01(mwc) * M_PI;\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t sinb = sin(angle2);\n"
		   << "\t\treal_t cosb = cos(angle2);\n"
		   << "\n"
		   << "\t\tvOut.x = r * sinb * cosa;\n"
		   << "\t\tvOut.y = r * sinb * sina;\n"
		   << "\t\tvOut.z = r * cosb;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Spherical3D.
/// </summary>
template <typename T>
class EMBER_API Spherical3DVariation : public Variation<T>
{
public:
	Spherical3DVariation(T weight = 1.0) : Variation<T>("Spherical3D", VAR_SPHERICAL3D, weight, true) { }

	VARCOPY(Spherical3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = m_Weight / Zeps(helper.m_PrecalcSumSquares + SQR(helper.In.z));

		helper.Out.x = r2 * helper.In.x;
		helper.Out.y = r2 * helper.In.y;
		helper.Out.z = r2 * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r2 = xform->m_VariationWeights[" << varIndex << "] / Zeps(precalcSumSquares + SQR(vIn.z));\n"
		   << "\n"
		   << "\t\tvOut.x = r2 * vIn.x;\n"
		   << "\t\tvOut.y = r2 * vIn.y;\n"
		   << "\t\tvOut.z = r2 * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Curl3D.
/// </summary>
template <typename T>
class EMBER_API Curl3DVariation : public ParametricVariation<T>
{
public:
	Curl3DVariation(T weight = 1.0) : ParametricVariation<T>("curl3D", VAR_CURL3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Curl3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T r = m_Weight / Zeps(r2 * m_C2 + m_C2x * helper.In.x - m_C2y * helper.In.y + m_C2z * helper.In.z + 1);

		helper.Out.x = r * (helper.In.x + m_Cx * r2);
		helper.Out.y = r * (helper.In.y - m_Cy * r2);
		helper.Out.z = r * (helper.In.z + m_Cz * r2);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] / Zeps(r2 * " << c2 << " + " << c2x << " * vIn.x - " << c2y << " * vIn.y + " << c2z << " * vIn.z + (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * (vIn.x + " << cx << " * r2);\n"
		   << "\t\tvOut.y = r * (vIn.y - " << cy << " * r2);\n"
		   << "\t\tvOut.z = r * (vIn.z + " << cz << " * r2);\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_C2z = 2 * m_Cz;
		m_C2 = SQR(m_Cx) + SQR(m_Cy) + SQR(m_Cz);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Cx, prefix + "curl3D_cx"));
		m_Params.push_back(ParamWithName<T>(&m_Cy, prefix + "curl3D_cy"));
		m_Params.push_back(ParamWithName<T>(&m_Cz, prefix + "curl3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2,  prefix + "curl3D_c2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "curl3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "curl3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "curl3D_c2z"));
	}

private:
	T m_Cx;
	T m_Cy;
	T m_Cz;
	T m_C2;//Precalc.
	T m_C2x;
	T m_C2y;
	T m_C2z;
};

/// <summary>
/// Disc3D.
/// </summary>
template <typename T>
class EMBER_API Disc3DVariation : public ParametricVariation<T>
{
public:
	Disc3DVariation(T weight = 1.0) : ParametricVariation<T>("disc3d", VAR_DISC3D, weight, true, true, false, true, false)
	{
		Init();
	}

	PARVARCOPY(Disc3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;
		T temp = r * m_Pi;
		T sr = sin(temp);
		T cr = cos(temp);
		T vv = m_Weight * helper.m_PrecalcAtanxy / Zeps(m_Pi);

		helper.Out.x = vv * sr;
		helper.Out.y = vv * cr;
		helper.Out.z = vv * (r * cos(helper.In.z));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string pi  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t temp = r * " << pi << ";\n"
		   << "\t\treal_t sr = sin(temp);\n"
		   << "\t\treal_t cr = cos(temp);\n"
		   << "\t\treal_t vv = xform->m_VariationWeights[" << varIndex << "] * precalcAtanxy / Zeps(" << pi << ");\n"
		   << "\n"
		   << "\t\tvOut.x = vv * sr;\n"
		   << "\t\tvOut.y = vv * cr;\n"
		   << "\t\tvOut.z = vv * (r * cos(vIn.z));\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Pi, prefix + "disc3d_pi", T(M_PI)));
	}

private:
	T m_Pi;
};

/// <summary>
/// Boarders2.
/// </summary>
template <typename T>
class EMBER_API Boarders2Variation : public ParametricVariation<T>
{
public:
	Boarders2Variation(T weight = 1.0) : ParametricVariation<T>("boarders2", VAR_BOARDERS2, weight)
	{
		Init();
	}

	PARVARCOPY(Boarders2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundX = T(int(helper.In.x >= 0 ? int(helper.In.x + T(0.5)) : int(helper.In.x - T(0.5))));
		T roundY = T(int(helper.In.y >= 0 ? int(helper.In.y + T(0.5)) : int(helper.In.y - T(0.5))));
		T offsetX = helper.In.x - roundX;
		T offsetY = helper.In.y - roundY;

		if (rand.Frand01<T>() >= m_Cr)
		{
			helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX);
			helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY);
		}
		else
		{
			if (fabs(offsetX) >= fabs(offsetY))
			{
				if (offsetX >= 0)
				{
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX + m_Cl);
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY + m_Cl * offsetY / offsetX);
				}
				else
				{
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX - m_Cl);
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY - m_Cl * offsetY / offsetX);
				}
			}
			else
			{
				if(offsetY >= 0)
				{
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY + m_Cl);
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX + offsetX / offsetY * m_Cl);
				}
				else
				{
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY - m_Cl);
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX - offsetX / offsetY * m_Cl);
				}
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
		string c    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string l    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cl   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cr   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t roundX = (real_t)(int)(vIn.x >= 0 ? (int)(vIn.x + (real_t)(0.5)) : (int)(vIn.x - (real_t)(0.5)));\n"
		   << "\t\treal_t roundY = (real_t)(int)(vIn.y >= 0 ? (int)(vIn.y + (real_t)(0.5)) : (int)(vIn.y - (real_t)(0.5)));\n"
		   << "\t\treal_t offsetX = vIn.x - roundX;\n"
		   << "\t\treal_t offsetY = vIn.y - roundY;\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) >= " << cr << ")\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * " << absc << " + roundX);\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * " << absc << " + roundY);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (fabs(offsetX) >= fabs(offsetY))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetX >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * " << absc << " + roundX + " << cl << ");\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * " << absc << " + roundY + " << cl << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * " << absc << " + roundX - " << cl << ");\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * " << absc << " + roundY - " << cl << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if(offsetY >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * " << absc << " + roundY + " << cl << ");\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * " << absc << " + roundX + offsetX / offsetY * " << cl << ");\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (offsetY * " << absc << " + roundY - " << cl << ");\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (offsetX * " << absc << " + roundX - offsetX / offsetY * " << cl << ");\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T c =  Zeps(fabs(m_C));
		T cl = Zeps(fabs(m_Left));
		T cr = Zeps(fabs(m_Right));

		m_AbsC = c;
		m_Cl = c * cl;
		m_Cr = c + (c * cr);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C,     prefix + "boarders2_c", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Left,  prefix + "boarders2_left", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Right, prefix + "boarders2_right", T(0.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsC, prefix + "boarders2_cabs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cl,   prefix + "boarders2_cl"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cr,   prefix + "boarders2_cr"));
	}

private:
	T m_C;
	T m_Left;
	T m_Right;
	T m_AbsC;//Precalc.
	T m_Cl;
	T m_Cr;
};

/// <summary>
/// Cardioid.
/// </summary>
template <typename T>
class EMBER_API CardioidVariation : public ParametricVariation<T>
{
public:
	CardioidVariation(T weight = 1.0) : ParametricVariation<T>("cardioid", VAR_CARDIOID, weight, true, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(CardioidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * sqrt(helper.m_PrecalcSumSquares + sin(helper.m_PrecalcAtanyx * m_A) + 1);

		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * sqrt(precalcSumSquares + sin(precalcAtanyx * " << a << ") + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "cardioid_a", 1));
	}

private:
	T m_A;
};

/// <summary>
/// Checks.
/// </summary>
template <typename T>
class EMBER_API ChecksVariation : public ParametricVariation<T>
{
public:
	ChecksVariation(T weight = 1.0) : ParametricVariation<T>("checks", VAR_CHECKS, weight)
	{
		Init();
	}

	PARVARCOPY(ChecksVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx, dy;
		T rnx = m_Rnd * rand.Frand01<T>();
		T rny = m_Rnd * rand.Frand01<T>();

		int isXY = int(LRint(helper.In.x * m_Cs) + LRint(helper.In.y * m_Cs));

		if (isXY % 2)
		{
			dx = m_Ncx + rnx;
			dy = m_Ncy;
		}
		else
		{
			dx = m_Cx;
			dy = m_Cy + rny;
		}

		helper.Out.x = m_Weight * (helper.In.x + dx);
		helper.Out.y = m_Weight * (helper.In.y + dy);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string x    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rnd  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cs   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ncx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ncy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t dx, dy;\n"
		   << "\t\treal_t rnx = " << rnd << " * MwcNext01(mwc);\n"
		   << "\t\treal_t rny = " << rnd << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tint isXY = (int)(LRint(vIn.x * " << cs << ") + LRint(vIn.y * " << cs << "));\n"
		   << "\n"
		   << "\t\tif (isXY % 2)\n"
		   << "\t\t{\n"
		   << "\t\t	dx = " << ncx << " + rnx;\n"
		   << "\t\t	dy = " << ncy << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	dx = " << cx << ";\n"
		   << "\t\t	dy = " << cy << " + rny;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + dx);\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + dy);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Cs = 1 / Zeps(m_Size);
		m_Cx = m_X;
		m_Cy = m_Y;
		m_Ncx = -m_X;
		m_Ncy = -m_Y;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,    prefix + "checks_x", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Y,    prefix + "checks_y", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Size, prefix + "checks_size", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Rnd,  prefix + "checks_rnd"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cs,  prefix + "checks_cs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cx,  prefix + "checks_cx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cy,  prefix + "checks_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ncx, prefix + "checks_ncx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ncy, prefix + "checks_ncy"));
	}

private:
	T m_X;
	T m_Y;
	T m_Size;
	T m_Rnd;
	T m_Cs;//Precalc.
	T m_Cx;
	T m_Cy;
	T m_Ncx;
	T m_Ncy;
};

/// <summary>
/// Circlize.
/// </summary>
template <typename T>
class EMBER_API CirclizeVariation : public ParametricVariation<T>
{
public:
	CirclizeVariation(T weight = 1.0) : ParametricVariation<T>("circlize", VAR_CIRCLIZE, weight)
	{
		Init();
	}

	PARVARCOPY(CirclizeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T side;
		T perimeter;
		T r, val;
		T absx = fabs(helper.In.x);
		T absy = fabs(helper.In.y);

		if (absx >= absy)
		{
			if (helper.In.x >= absy)
				perimeter = absx + helper.In.y;
			else
				perimeter = 5 * absx - helper.In.y;

			side = absx;
		}
		else
		{
			if (helper.In.y >= absx)
				perimeter = 3 * absy - helper.In.x;
			else
				perimeter = 7 * absy + helper.In.x;

			side = absy;
		}

		r = m_Vvar4Pi * side + m_Hole;
		val = T(M_PI_4) * perimeter / side - T(M_PI_4);
		helper.Out.x = r * cos(val);
		helper.Out.y = r * sin(val);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string hole    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar4pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t side;\n"
		   << "\t\treal_t perimeter;\n"
		   << "\t\treal_t absx = fabs(vIn.x);\n"
		   << "\t\treal_t absy = fabs(vIn.y);\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.x >= absy)\n"
		   << "\t\t		perimeter = absx + vIn.y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 5 * absx - vIn.y;\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.y >= absx)\n"
		   << "\t\t		perimeter = 3 * absy - vIn.x;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 7 * absy + vIn.x;\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = " << vvar4pi << " * side + " << hole << ";\n"
		   << "\t\treal_t val = M_PI_4 * perimeter / side - M_PI_4;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(val);\n"
		   << "\t\tvOut.y = r * sin(val);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar4Pi = m_Weight / T(M_PI_4);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "circlize_hole"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar4Pi, prefix + "circlize_vvar4pi"));//Precalc.
	}

private:
	T m_Hole;
	T m_Vvar4Pi;//Precalc.
};

/// <summary>
/// Circlize2.
/// </summary>
template <typename T>
class EMBER_API Circlize2Variation : public ParametricVariation<T>
{
public:
	Circlize2Variation(T weight = 1.0) : ParametricVariation<T>("circlize2", VAR_CIRCLIZE2, weight)
	{
		Init();
	}

	PARVARCOPY(Circlize2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T side;
		T perimeter;
		T absx = fabs(helper.In.x);
		T absy = fabs(helper.In.y);

		if (absx >= absy)
		{
			if (helper.In.x >= absy)
				perimeter = absx + helper.In.y;
			else
				perimeter = 5 * absx - helper.In.y;

			side = absx;
		}
		else
		{
			if (helper.In.y >= absx)
				perimeter = 3 * absy - helper.In.x;
			else
				perimeter = 7 * absy + helper.In.x;

			side = absy;
		}

		T r = m_Weight * (side + m_Hole);
		T val = T(M_PI_4) * perimeter / side - T(M_PI_4);

		helper.Out.x = r * cos(val);
		helper.Out.y = r * sin(val);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string hole    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t side;\n"
		   << "\t\treal_t perimeter;\n"
		   << "\t\treal_t absx = fabs(vIn.x);\n"
		   << "\t\treal_t absy = fabs(vIn.y);\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.x >= absy)\n"
		   << "\t\t		perimeter = absx + vIn.y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 5 * absx - vIn.y;\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.y >= absx)\n"
		   << "\t\t		perimeter = 3 * absy - vIn.x;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = 7 * absy + vIn.x;\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * (side + " << hole << ");\n"
		   << "\t\treal_t val = M_PI_4 * perimeter / side - M_PI_4;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(val);\n"
		   << "\t\tvOut.y = r * sin(val);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "circlize2_hole"));
	}

private:
	T m_Hole;
};

/// <summary>
/// CosWrap.
/// </summary>
template <typename T>
class EMBER_API CosWrapVariation : public ParametricVariation<T>
{
public:
	CosWrapVariation(T weight = 1.0) : ParametricVariation<T>("coswrap", VAR_COS_WRAP, weight)
	{
		Init();
	}

	PARVARCOPY(CosWrapVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = T(0.5) * helper.In.x + T(0.5);
		T y = T(0.5) * helper.In.y + T(0.5);
		T bx = Fabsmod<T>(m_Fr * x);
		T by = Fabsmod<T>(m_Fr * y);
		T oscnapx = Foscn<T>(m_AmountX, m_Px);
		T oscnapy = Foscn<T>(m_AmountY, m_Py);

		helper.Out.x = -1 + m_Vv2 * Lerp<T>(Lerp(x, Fosc(x, T(4), m_Px), oscnapx), Fosc(bx, T(4), m_Px), oscnapx);//Original did a direct assignment to outPoint, which is incompatible with Ember's design.
		helper.Out.y = -1 + m_Vv2 * Lerp<T>(Lerp(y, Fosc(y, T(4), m_Py), oscnapy), Fosc(by, T(4), m_Py), oscnapy);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string repeat  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amountX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amountY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string px      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string py      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fr      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vv2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = (real_t)(0.5) * vIn.x + (real_t)(0.5);\n"
		   << "\t\treal_t y = (real_t)(0.5) * vIn.y + (real_t)(0.5);\n"
		   << "\t\treal_t bx = Fabsmod(" << fr << " * x);\n"
		   << "\t\treal_t by = Fabsmod(" << fr << " * y);\n"
		   << "\t\treal_t oscnapx = Foscn(" << amountX << ", " << px << ");\n"
		   << "\t\treal_t oscnapy = Foscn(" << amountY << ", " << py << ");\n"
		   << "\n"
		   << "\t\tvOut.x = -1 + " << vv2 << " * Lerp(Lerp(x, Fosc(x, 4, " << px << "), oscnapx), Fosc(bx, 4, " << px << "), oscnapx);\n"
		   << "\t\tvOut.y = -1 + " << vv2 << " * Lerp(Lerp(y, Fosc(y, 4, " << py << "), oscnapy), Fosc(by, 4, " << py << "), oscnapy);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ax  = M_2PI * fabs(m_AmountX);
		m_Ay  = M_2PI * fabs(m_AmountY);
		m_Px  = T(M_PI)  * m_PhaseX;
		m_Py  = T(M_PI)  * m_PhaseY;
		m_Fr  = fabs(m_Repeat);
		m_Vv2 = 2 * m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Repeat,  prefix + "coswrap_repeat", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_AmountX, prefix + "coswrap_amount_x"));
		m_Params.push_back(ParamWithName<T>(&m_AmountY, prefix + "coswrap_amount_y"));
		m_Params.push_back(ParamWithName<T>(&m_PhaseX,  prefix + "coswrap_phase_x", 0, REAL_CYCLIC, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_PhaseY,  prefix + "coswrap_phase_y", 0, REAL_CYCLIC, -1, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax,  prefix + "coswrap_ax"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Ay,  prefix + "coswrap_ay"));
		m_Params.push_back(ParamWithName<T>(true, &m_Px,  prefix + "coswrap_px"));
		m_Params.push_back(ParamWithName<T>(true, &m_Py,  prefix + "coswrap_py"));
		m_Params.push_back(ParamWithName<T>(true, &m_Fr,  prefix + "coswrap_fr"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vv2, prefix + "coswrap_vv2"));
	}

private:
	T m_Repeat;
	T m_AmountX;
	T m_AmountY;
	T m_PhaseX;
	T m_PhaseY;
	T m_Ax;//Precalc.
	T m_Ay;
	T m_Px;
	T m_Py;
	T m_Fr;
	T m_Vv2;
};

/// <summary>
/// DeltaA.
/// The original in deltaA.c in Apophysis used a precalc variable named v, but
/// was unused in the calculation. So this remains a non-parametric variation with
/// that precalc variable omitted.
/// </summary>
template <typename T>
class EMBER_API DeltaAVariation : public Variation<T>
{
public:
	DeltaAVariation(T weight = 1.0) : Variation<T>("deltaa", VAR_DELTA_A, weight) { }

	VARCOPY(DeltaAVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s, c;
		T avgr = m_Weight * (sqrt(SQR(helper.In.y) + SQR(helper.In.x + 1)) / sqrt(SQR(helper.In.y) + SQR(helper.In.x - 1)));
		T avga = (atan2(helper.In.y, helper.In.x - 1) - atan2(helper.In.y, helper.In.x + 1)) / 2;

		sincos(avga, &s, &c);
		helper.Out.x = avgr * c;
		helper.Out.y = avgr * s;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t avgr = xform->m_VariationWeights[" << varIndex << "] * (sqrt(SQR(vIn.y) + SQR(vIn.x + 1)) / sqrt(SQR(vIn.y) + SQR(vIn.x - 1)));\n"
		   << "\t\treal_t avga = (atan2(vIn.y, vIn.x - 1) - atan2(vIn.y, vIn.x + 1)) / 2;\n"
		   << "\t\treal_t s = sin(avga);\n"
		   << "\t\treal_t c = cos(avga);\n"
		   << "\n"
		   << "\t\tvOut.x = avgr * c;\n"
		   << "\t\tvOut.y = avgr * s;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Expo.
/// </summary>
template <typename T>
class EMBER_API ExpoVariation : public ParametricVariation<T>
{
public:
	ExpoVariation(T weight = 1.0) : ParametricVariation<T>("expo", VAR_EXPO, weight)
	{
		Init();
	}

	PARVARCOPY(ExpoVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expor = exp(helper.In.x * m_K - helper.In.y * m_T);
		T temp = helper.In.x * m_T + helper.In.y * m_K;
		T snv = sin(temp);
		T csv = cos(temp);

		helper.Out.x = m_Weight * expor * csv;
		helper.Out.y = m_Weight * expor * snv;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string real = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imag = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string k    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t expor = exp(vIn.x * " << k << " - vIn.y * " << t << ");\n"
		   << "\t\treal_t temp = vIn.x * " << t << " + vIn.y * " << k << ";\n"
		   << "\t\treal_t snv = sin(temp);\n"
		   << "\t\treal_t csv = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * expor * csv;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * expor * snv;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_K = T(0.5) * log(Zeps(SQR(m_Real) + SQR(m_Imag)));//Original used 1e-300, which isn't representable with a float.
		m_T = atan2(m_Imag, m_Real);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Real, prefix + "expo_real", -1));
		m_Params.push_back(ParamWithName<T>(&m_Imag, prefix + "expo_imaginary", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_K, prefix + "expo_k"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_T, prefix + "expo_t"));
	}

private:
	T m_Real;
	T m_Imag;
	T m_K;//Precalc.
	T m_T;
};

/// <summary>
/// Extrude.
/// </summary>
template <typename T>
class EMBER_API ExtrudeVariation : public ParametricVariation<T>
{
public:
	ExtrudeVariation(T weight = 1.0) : ParametricVariation<T>("extrude", VAR_EXTRUDE, weight)
	{
		Init();
	}

	PARVARCOPY(ExtrudeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == VARTYPE_REG)
		{
			helper.Out.x = helper.Out.y = helper.Out.z = 0;

			if (rand.Frand01<T>() < m_RootFace)
				outPoint.m_Z = ClampGte0(m_Weight);
			else
				outPoint.m_Z = m_Weight * rand.Frand01<T>();
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;

			if (rand.Frand01<T>() < m_RootFace)
				helper.Out.z = ClampGte0(m_Weight);
			else
				helper.Out.z = m_Weight * rand.Frand01<T>();
		}
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string rootFace = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		if (m_VarType == VARTYPE_REG)
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vOut.y = vOut.z = 0;\n"
			   << "\n"
			   << "\t\tif (MwcNext01(mwc) < " << rootFace << ")\n"
			   << "\t\t	outPoint->m_Z = max(xform->m_VariationWeights[" << varIndex << "], (real_t)(0.0));\n"
			   << "\t\telse\n"
			   << "\t\t	outPoint->m_Z = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
			   << "\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n"
			   << "\n"
			   << "\t\tif (MwcNext01(mwc) < " << rootFace << ")\n"
			   << "\t\t	vOut.z = max(xform->m_VariationWeights[" << varIndex << "], (real_t)(0.0));\n"
			   << "\t\telse\n"
			   << "\t\t	vOut.z = xform->m_VariationWeights[" << varIndex << "] * MwcNext01(mwc);\n"
			   << "\t}\n";
		}

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_RootFace, prefix + "extrude_root_face", T(0.5)));
	}

private:
	T m_RootFace;
};

/// <summary>
/// fdisc.
/// </summary>
template <typename T>
class EMBER_API FDiscVariation : public Variation<T>
{
public:
	FDiscVariation(T weight = 1.0) : Variation<T>("fdisc", VAR_FDISC, weight, true, true, false, false, true) { }

	VARCOPY(FDiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c, s;
		T a = M_2PI / (helper.m_PrecalcSqrtSumSquares + 1);
		T r = (helper.m_PrecalcAtanyx * T(M_1_PI) + 1) * T(0.5);

		sincos(a, &s, &c);
		helper.Out.x = m_Weight * r * c;
		helper.Out.y = m_Weight * r * s;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t a = M_2PI / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t r = (precalcAtanyx * M_1_PI + 1) * (real_t)(0.5);\n"
		   << "\t\treal_t s = sin(a);\n"
		   << "\t\treal_t c = cos(a);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * r * c;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * r * s;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Fibonacci.
/// </summary>
template <typename T>
class EMBER_API FibonacciVariation : public ParametricVariation<T>
{
public:
	FibonacciVariation(T weight = 1.0) : ParametricVariation<T>("fibonacci", VAR_FIBONACCI, weight)
	{
		Init();
	}

	PARVARCOPY(FibonacciVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snum1, cnum1, snum2, cnum2;
		T temp = helper.In.y * m_NatLog;

		sincos(temp, &snum1, &cnum1);
		temp = (helper.In.x * T(M_PI) + helper.In.y * m_NatLog) * -1;
		sincos(temp, &snum2, &cnum2);

		T eradius1 = exp(helper.In.x * m_NatLog);
		T eradius2 = exp((helper.In.x * m_NatLog - helper.In.y * T(M_PI)) * -1);

		helper.Out.x = m_Weight * (eradius1 * cnum1 - eradius2 * cnum2) * m_Five;
		helper.Out.y = m_Weight * (eradius1 * snum1 - eradius2 * snum2) * m_Five;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string five   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string natLog = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t temp = vIn.y * " << natLog << ";\n"
		   << "\t\treal_t snum1 = sin(temp);\n"
		   << "\t\treal_t cnum1 = cos(temp);\n"
		   << "\t\ttemp = (vIn.x * M_PI + vIn.y * " << natLog << ") * -(real_t)(1.0);\n"
		   << "\t\treal_t snum2 = sin(temp);\n"
		   << "\t\treal_t cnum2 = cos(temp);\n"
		   << "\t\treal_t eradius1 = exp(vIn.x * " << natLog << ");\n"
		   << "\t\treal_t eradius2 = exp((vIn.x * " << natLog << " - vIn.y * M_PI) * -(real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (eradius1 * cnum1 - eradius2 * cnum2) * " << five << ";\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (eradius1 * snum1 - eradius2 * snum2) * " << five << ";\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Five = 1 / SQRT5;
		m_NatLog = log(M_PHI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Five,   prefix + "fibonacci_five"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_NatLog, prefix + "fibonacci_nat_log"));
	}

private:
	T m_Five;//Precalcs only, no params.
	T m_NatLog;
};

/// <summary>
/// Fibonacci2.
/// </summary>
template <typename T>
class EMBER_API Fibonacci2Variation : public ParametricVariation<T>
{
public:
	Fibonacci2Variation(T weight = 1.0) : ParametricVariation<T>("fibonacci2", VAR_FIBONACCI2, weight)
	{
		Init();
	}

	PARVARCOPY(Fibonacci2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snum1, cnum1, snum2, cnum2;
		T temp = helper.In.y * m_NatLog;

		sincos(temp, &snum1, &cnum1);
		temp = (helper.In.x * T(M_PI) + helper.In.y * m_NatLog) * -1;
		sincos(temp, &snum2, &cnum2);

		T eradius1 = m_Sc * exp(m_Sc2 * (helper.In.x * m_NatLog));
		T eradius2 = m_Sc * exp(m_Sc2 * ((helper.In.x * m_NatLog - helper.In.y * T(M_PI)) * -1));

		helper.Out.x = m_Weight * (eradius1 * cnum1 - eradius2 * cnum2) * m_Five;
		helper.Out.y = m_Weight * (eradius1 * snum1 - eradius2 * snum2) * m_Five;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string sc     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sc2    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string five   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string natLog = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t temp = vIn.y * " << natLog << ";\n"
		   << "\t\treal_t snum1 = sin(temp);\n"
		   << "\t\treal_t cnum1 = cos(temp);\n"
		   << "\t\ttemp = (vIn.x * M_PI + vIn.y * " << natLog << ") * -1;\n"
		   << "\t\treal_t snum2 = sin(temp);\n"
		   << "\t\treal_t cnum2 = cos(temp);\n"
		   << "\t\treal_t eradius1 = " << sc << " * exp(" << sc2 << " * (vIn.x * " << natLog << "));\n"
		   << "\t\treal_t eradius2 = " << sc << " * exp(" << sc2 << " * ((vIn.x * " << natLog << " - vIn.y * M_PI) * -1));\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (eradius1 * cnum1 - eradius2 * cnum2) * " << five << ";\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (eradius1 * snum1 - eradius2 * snum2) * " << five << ";\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Five = 1 / SQRT5;
		m_NatLog = log(M_PHI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sc,  prefix + "fibonacci2_sc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Sc2, prefix + "fibonacci2_sc2", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Five,   prefix + "fibonacci2_five"));//Precalcs.
		m_Params.push_back(ParamWithName<T>(true, &m_NatLog, prefix + "fibonacci2_nat_log"));
	}

private:
	T m_Sc;
	T m_Sc2;
	T m_Five;//Precalcs.
	T m_NatLog;
};

/// <summary>
/// Glynnia.
/// </summary>
template <typename T>
class EMBER_API GlynniaVariation : public ParametricVariation<T>
{
public:
	GlynniaVariation(T weight = 1.0) : ParametricVariation<T>("glynnia", VAR_GLYNNIA, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynniaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d, r = helper.m_PrecalcSqrtSumSquares;

		if (r > 1)
		{
			if (rand.Frand01<T>() > T(0.5))
			{
				d = sqrt(r + helper.In.x);
				helper.Out.x = m_V2 * d;
				helper.Out.y = -(m_V2 / d * helper.In.y);
			}
			else
			{
				d = r + helper.In.x;
				r = m_Weight / sqrt(r * (SQR(helper.In.y) + SQR(d)));
				helper.Out.x = r * d;
				helper.Out.y = r * helper.In.y;
			}
		}
		else
		{
			if (rand.Frand01<T>() > T(0.5))
			{
				d = sqrt(r + helper.In.x);
				helper.Out.x = -(m_V2 * d);
				helper.Out.y = -(m_V2 / d * helper.In.y);
			}
			else
			{
				d = r + helper.In.x;
				r = m_Weight / sqrt(r * (SQR(helper.In.y) + SQR(d)));
				helper.Out.x = -(r * d);
				helper.Out.y = r * helper.In.y;
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
		string v2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		ss << "\t{\n"
		   << "\t\treal_t d, r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (r > 1)\n"
		   << "\t\t{\n"
		   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
		   << "\t\t	{\n"
		   << "\t\t		d = sqrt(r + vIn.x);\n"
		   << "\t\t		vOut.x = " << v2 << " * d;\n"
		   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		d = r + vIn.x;\n"
		   << "\t\t		r = xform->m_VariationWeights[" << varIndex << "] / sqrt(r * (SQR(vIn.y) + SQR(d)));\n"
		   << "\t\t		vOut.x = r * d;\n"
		   << "\t\t		vOut.y = r * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
		   << "\t\t	{\n"
		   << "\t\t		d = sqrt(r + vIn.x);\n"
		   << "\t\t		vOut.x = -(" << v2 << " * d);\n"
		   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		d = r + vIn.x;\n"
		   << "\t\t		r = xform->m_VariationWeights[" << varIndex << "] / sqrt(r * (SQR(vIn.y) + SQR(d)));\n"
		   << "\t\t		vOut.x = -(r * d);\n"
		   << "\t\t		vOut.y = r * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V2 = m_Weight * sqrt(T(2)) / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_V2, prefix + "glynnia_v2"));//Precalcs only, no params.
	}

private:
	T m_V2;//Precalcs only, no params.
};

/// <summary>
/// GridOut.
/// </summary>
template <typename T>
class EMBER_API GridOutVariation : public Variation<T>
{
public:
	GridOutVariation(T weight = 1.0) : Variation<T>("gridout", VAR_GRIDOUT, weight) { }

	VARCOPY(GridOutVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = LRint(helper.In.x);
		T y = LRint(helper.In.y);

		if (y <= 0)
		{
			if (x > 0)
			{
				if (-y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
				}
			}
			else
			{
				if (y <= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
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
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
				}
			}
			else
			{
				if (y > -x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
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
		   << "\t\treal_t x = LRint(vIn.x);\n"
		   << "\t\treal_t y = LRint(vIn.y);\n"
		   << "\n"
		   << "\t\tif (y <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (-y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + 1);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y <= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - 1);\n"
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
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y + 1);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y > -x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x;\n"
		   << "\t\t			vOut.y = xform->m_VariationWeights[" << varIndex << "] * (vIn.y - 1);\n"
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
/// Hole.
/// </summary>
template <typename T>
class EMBER_API HoleVariation : public ParametricVariation<T>
{
public:
	HoleVariation(T weight = 1.0) : ParametricVariation<T>("hole", VAR_HOLE, weight, true, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(HoleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, delta = pow(helper.m_PrecalcAtanyx / T(M_PI) + 1, m_A);

		if (m_Inside != 0)
			r = m_Weight * delta / (helper.m_PrecalcSqrtSumSquares + delta);
		else
			r = m_Weight * helper.m_PrecalcSqrtSumSquares + delta;

		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string a      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string inside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r, delta = pow(precalcAtanyx / M_PI + 1, " << a << ");\n"
		   << "\n"
		   << "\t\tif (" << inside << " != 0)\n"
		   << "\t\t	r = xform->m_VariationWeights[" << varIndex << "] * delta / (precalcSqrtSumSquares + delta);\n"
		   << "\t\telse\n"
		   << "\t\t	r = xform->m_VariationWeights[" << varIndex << "] * precalcSqrtSumSquares + delta;\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "hole_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_Inside, prefix + "hole_inside", 0, INTEGER, 0, 1));
	}

private:
	T m_A;
	T m_Inside;
};

/// <summary>
/// Hypertile.
/// </summary>
template <typename T>
class EMBER_API HypertileVariation : public ParametricVariation<T>
{
public:
	HypertileVariation(T weight = 1.0) : ParametricVariation<T>("hypertile", VAR_HYPERTILE, weight)
	{
		Init();
	}

	PARVARCOPY(HypertileVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x + m_Real;
		T b = helper.In.y - m_Imag;
		T c = m_Real * helper.In.x - m_Imag * helper.In.y + 1;
		T d = m_Real * helper.In.y + m_Imag * helper.In.x;
		T vr = m_Weight / (SQR(c) + SQR(d));

		helper.Out.x = vr * (a * c + b * d);
		helper.Out.y = vr * (b * c - a * d);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string real = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imag = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x + " << real << ";\n"
		   << "\t\treal_t b = vIn.y - " << imag << ";\n"
		   << "\t\treal_t c = " << real << " * vIn.x - " << imag << " * vIn.y + 1;\n"
		   << "\t\treal_t d = " << real << " * vIn.y + " << imag << " * vIn.x;\n"
		   << "\t\treal_t vr = xform->m_VariationWeights[" << varIndex << "] / (SQR(c) + SQR(d));\n"
		   << "\n"
		   << "\t\tvOut.x = vr * (a * c + b * d);\n"
		   << "\t\tvOut.y = vr * (b * c - a * d);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = 2 * T(M_PI) / m_P;
		T qa = 2 * T(M_PI) / m_Q;
		T r = (1 - cos(pa)) / (cos(pa) + cos(qa)) + 1;
		T a = m_N * pa;

		if (r > 0)
			r = 1 / sqrt(r);
		else
			r = 1;

		m_Real = r * cos(a);
		m_Imag = r * sin(a);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "hypertile_n", 0, INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_Real, prefix + "hypertile_real"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Imag, prefix + "hypertile_imag"));
	}

private:
	T m_P;
	T m_Q;
	T m_N;
	T m_Real;//Precalc.
	T m_Imag;
};

/// <summary>
/// Hypertile1.
/// </summary>
template <typename T>
class EMBER_API Hypertile1Variation : public ParametricVariation<T>
{
public:
	Hypertile1Variation(T weight = 1.0) : ParametricVariation<T>("hypertile1", VAR_HYPERTILE1, weight)
	{
		Init();
	}

	PARVARCOPY(Hypertile1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = rand.Rand() * m_Pa;
		T sina = sin(temp);
		T cosa = cos(temp);
		T re = m_R * cosa;
		T im = m_R * sina;
		T a = helper.In.x + re;
		T b = helper.In.y - im;
		T c = re * helper.In.x - im * helper.In.y + 1;
		T d = re * helper.In.y + im * helper.In.x;
		T vr = m_Weight / (SQR(c) + SQR(d));

		helper.Out.x = vr * (a * c + b * d);
		helper.Out.y = vr * (b * c - a * d);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t temp = MwcNext(mwc) * " << pa << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\t\treal_t re = " << r << " * cosa;\n"
		   << "\t\treal_t im = " << r << " * sina;\n"
		   << "\t\treal_t a = vIn.x + re;\n"
		   << "\t\treal_t b = vIn.y - im;\n"
		   << "\t\treal_t c = re * vIn.x - im * vIn.y + 1;\n"
		   << "\t\treal_t d = re * vIn.y + im * vIn.x;\n"
		   << "\t\treal_t vr = xform->m_VariationWeights[" << varIndex << "] / (SQR(c) + SQR(d));\n"
		   << "\n"
		   << "\t\tvOut.x = vr * (a * c + b * d);\n"
		   << "\t\tvOut.y = vr * (b * c - a * d);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T r2 = 1 - (cos(2 * T(M_PI) / m_P) - 1) /
			(cos(2 * T(M_PI) / m_P) + cos(2 * T(M_PI) / m_Q));

		if (r2 > 0)
			m_R = 1 / sqrt(r2);
		else
			m_R = 1;

		m_Pa = 2 * T(M_PI) / m_P;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile1_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile1_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile1_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R, prefix  + "hypertile1_r"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
};

/// <summary>
/// Hypertile2.
/// </summary>
template <typename T>
class EMBER_API Hypertile2Variation : public ParametricVariation<T>
{
public:
	Hypertile2Variation(T weight = 1.0) : ParametricVariation<T>("hypertile2", VAR_HYPERTILE2, weight)
	{
		Init();
	}

	PARVARCOPY(Hypertile2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x + m_R;
		T b = helper.In.y;
		T c = m_R * helper.In.x + 1;
		T d = m_R * helper.In.y;
		T x = (a * c + b * d);
		T y = (b * c - a * d);
		T vr = m_Weight / (SQR(c) + SQR(d));
		T temp = rand.Rand() * m_Pa;
		T sina = sin(temp);
		T cosa = cos(temp);

		helper.Out.x = vr * (x * cosa + y * sina);
		helper.Out.y = vr * (y * cosa - x * sina);
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x + " << r << ";\n"
		   << "\t\treal_t b = vIn.y;\n"
		   << "\t\treal_t c = " << r << " * vIn.x + 1;\n"
		   << "\t\treal_t d = " << r << " * vIn.y;\n"
		   << "\t\treal_t x = (a * c + b * d);\n"
		   << "\t\treal_t y = (b * c - a * d);\n"
		   << "\t\treal_t vr = xform->m_VariationWeights[" << varIndex << "] / (SQR(c) + SQR(d));\n"
		   << "\t\treal_t temp = MwcNext(mwc) * " << pa << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = vr * (x * cosa + y * sina);\n"
		   << "\t\tvOut.y = vr * (y * cosa - x * sina);\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T r2 = 1 - (cos(2 * T(M_PI) / m_P) - 1) /
			(cos(2 * T(M_PI) / m_P) + cos(2 * T(M_PI) / m_Q));

		if (r2 > 0)
			m_R = 1 / sqrt(r2);
		else
			m_R = 1;

		m_Pa = 2 * T(M_PI) / m_P;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile2_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile2_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile2_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R, prefix  + "hypertile2_r"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
};

/// <summary>
/// Hypertile3D.
/// </summary>
template <typename T>
class EMBER_API Hypertile3DVariation : public ParametricVariation<T>
{
public:
	Hypertile3DVariation(T weight = 1.0) : ParametricVariation<T>("hypertile3D", VAR_HYPERTILE3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + helper.In.z;
		T x2cx = m_C2x * helper.In.x;
		T y2cy = m_C2y * helper.In.y;
		T d = m_Weight / (m_C2 * r2 + x2cx - y2cy + 1);

		helper.Out.x = d * (helper.In.x * m_S2x - m_Cx * ( y2cy - r2 - 1));
		helper.Out.y = d * (helper.In.y * m_S2y + m_Cy * (-x2cx - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + vIn.z;\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t y2cy = " << c2y << " * vIn.y;\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] / (" << c2 << " * r2 + x2cx - y2cy + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = d * (vIn.x * " << s2x << " - " << cx << "* ( y2cy - r2 - 1));\n"
		   << "\t\tvOut.y = d * (vIn.y * " << s2y << " + " << cy << "* (-x2cx - r2 - 1));\n"
		   << "\t\tvOut.z = d * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = 2 * T(M_PI) / m_P;
		T qa = 2 * T(M_PI) / m_Q;
		T r  = -(cos(pa) - 1) / (cos(pa) + cos(qa));
		T na = m_N * pa;

		if (r > 0)
			r = 1 / sqrt(1 + r);
		else
			r = 1;

		m_Cx = r * cos(na);
		m_Cy = r * sin(na);
		m_C2 = SQR(m_Cx) + SQR(m_Cy);
		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_S2x = 1 + SQR(m_Cx) - SQR(m_Cy);
		m_S2y = 1 + SQR(m_Cy) - SQR(m_Cx);
		m_S2z = 1 - SQR(m_Cy) - SQR(m_Cx);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "hypertile3D_n", 0, INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx,  prefix + "hypertile3D_cx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cy,  prefix + "hypertile3D_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cz,  prefix + "hypertile3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "hypertile3D_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "hypertile3D_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D_s2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "hypertile3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "hypertile3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "hypertile3D_c2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2,  prefix + "hypertile3D_c2"));
	}

private:
	T m_P;
	T m_Q;
	T m_N;
	T m_Cx;//Precalc.
	T m_Cy;
	T m_Cz;
	T m_S2x;
	T m_S2y;
	T m_S2z;
	T m_C2x;
	T m_C2y;
	T m_C2z;
	T m_C2;
};

/// <summary>
/// Hypertile3D1.
/// </summary>
template <typename T>
class EMBER_API Hypertile3D1Variation : public ParametricVariation<T>
{
public:
	Hypertile3D1Variation(T weight = 1.0) : ParametricVariation<T>("hypertile3D1", VAR_HYPERTILE3D1, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3D1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = rand.Rand() * m_Pa;
		T cx = m_R * cos(temp);
		T cy = m_R * sin(temp);
		T s2x = 1 + SQR(cx) - SQR(cy);
		T s2y = 1 + SQR(cy) - SQR(cx);
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = 2 * cx * helper.In.x;
		T y2cy = 2 * cy * helper.In.x;
		T d = m_Weight / (m_C2 * r2 + x2cx - y2cy + 1);

		helper.Out.x = d * (helper.In.x * s2x - cx * ( y2cy - r2 - 1));
		helper.Out.y = d * (helper.In.y * s2y + cy * (-x2cx - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t temp = MwcNext(mwc) * " << pa << ";\n"
		   << "\t\treal_t cx = " << r << " * cos(temp);\n"
		   << "\t\treal_t cy = " << r << " * sin(temp);\n"
		   << "\t\treal_t s2x = 1 + SQR(cx) - SQR(cy);\n"
		   << "\t\treal_t s2y = 1 + SQR(cy) - SQR(cx);\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t x2cx = 2 * cx * vIn.x;\n"
		   << "\t\treal_t y2cy = 2 * cy * vIn.x;\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] / (" << c2 << " * r2 + x2cx - y2cy + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = d * (vIn.x * s2x - cx * ( y2cy - r2 - 1));\n"
		   << "\t\tvOut.y = d * (vIn.y * s2y + cy * (-x2cx - r2 - 1));\n"
		   << "\t\tvOut.z = d * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = M_2PI / m_P;
		T qa = M_2PI / m_Q;
		T r = -(cos(pa) - 1) / (cos(pa) + cos(qa));

		if (r > 0)
			r = 1 / sqrt(1 + r);
		else
			r = 1;

		m_Pa = pa;
		m_R = r;
		m_C2 = SQR(r);
		m_S2z = 1 - m_C2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D1_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D1_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa,  prefix + "hypertile3D1_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R,   prefix + "hypertile3D1_r"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2,  prefix + "hypertile3D1_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D1_s2z"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
	T m_C2;
	T m_S2z;
};

/// <summary>
/// Hypertile3D2.
/// </summary>
template <typename T>
class EMBER_API Hypertile3D2Variation : public ParametricVariation<T>
{
public:
	Hypertile3D2Variation(T weight = 1.0) : ParametricVariation<T>("hypertile3D2", VAR_HYPERTILE3D2, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3D2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = m_C2x * helper.In.x;
		T x = helper.In.x * m_S2x - m_Cx * (-r2 - 1);
		T y = helper.In.y * m_S2y;
		T vr = m_Weight / (m_C2 * r2 + x2cx + 1);
		T temp = rand.Rand() * m_Pa;
		T sina = sin(temp);
		T cosa = cos(temp);

		helper.Out.x = vr * (x * cosa + y * sina);
		helper.Out.y = vr * (y * cosa - x * sina);
		helper.Out.z = vr * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t x = vIn.x * " << s2x << " - " << cx << " * (-r2 - 1);\n"
		   << "\t\treal_t y = vIn.y * " << s2y << ";\n"
		   << "\t\treal_t vr = xform->m_VariationWeights[" << varIndex << "] / (" << c2 << " * r2 + x2cx + 1);\n"
		   << "\t\treal_t temp = MwcNext(mwc) * " << pa << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = vr * (x * cosa + y * sina);\n"
		   << "\t\tvOut.y = vr * (y * cosa - x * sina);\n"
		   << "\t\tvOut.z = vr * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = M_2PI / m_P;
		T qa = M_2PI / m_Q;
		T r = -(cos(pa) - 1) / (cos(pa) + cos(qa));

		if (r > 0)
			r = 1 / sqrt(1 + r);
		else
			r = 1;

		m_Pa = pa;
		m_Cx = r;
		m_C2 = SQR(m_Cx);
		m_C2x = 2 * m_Cx;
		m_S2x = 1 + SQR(m_Cx);
		m_S2y = 1 - SQR(m_Cx);
		m_S2z = 1 - SQR(m_Cx);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D2_p", 3, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D2_q", 7, INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa,  prefix + "hypertile3D2_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cx,  prefix + "hypertile3D2_cx"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2,  prefix + "hypertile3D2_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "hypertile3D2_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "hypertile3D2_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "hypertile3D2_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D2_s2z"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_Cx;
	T m_C2;
	T m_C2x;
	T m_S2x;
	T m_S2y;
	T m_S2z;
};

/// <summary>
/// IDisc.
/// </summary>
template <typename T>
class EMBER_API IDiscVariation : public ParametricVariation<T>
{
public:
	IDiscVariation(T weight = 1.0) : ParametricVariation<T>("idisc", VAR_IDISC, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(IDiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = T(M_PI) / (helper.m_PrecalcSqrtSumSquares + 1);
		T s = sin(a);
		T c = cos(a);
		T r = helper.m_PrecalcAtanyx * m_V;

		helper.Out.x = r * c;
		helper.Out.y = r * s;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		ss << "\t{\n"
		   << "\t\treal_t a = M_PI / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t s = sin(a);\n"
		   << "\t\treal_t c = cos(a);\n"
		   << "\t\treal_t r = precalcAtanyx * " << v << ";\n"
		   << "\n"
		   << "\t\tvOut.x = r * c;\n"
		   << "\t\tvOut.y = r * s;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight * T(M_1_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "idisc_v"));//Precalcs only, no params.
	}

private:
	T m_V;//Precalcs only, no params.
};

/// <summary>
/// Julian2.
/// </summary>
template <typename T>
class EMBER_API Julian2Variation : public ParametricVariation<T>
{
public:
	Julian2Variation(T weight = 1.0) : ParametricVariation<T>("julian2", VAR_JULIAN2, weight)
	{
		Init();
	}

	PARVARCOPY(Julian2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = m_A * helper.In.x + m_B * helper.In.y + m_E;
		T y = m_C * helper.In.x + m_D * helper.In.y + m_F;
		T angle = (atan2(y, x) + M_2PI * rand.Rand(int(m_AbsN))) / m_Power;
		T sina = sin(angle);
		T cosa = cos(angle);
		T r = m_Weight * pow(SQR(x) + SQR(y), m_Cn);

		helper.Out.x = r * cosa;
		helper.Out.y = r * sina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string a     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string e     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = " << a << " * vIn.x + " << b << " * vIn.y + " << e << ";\n"
		   << "\t\treal_t y = " << c << " * vIn.x + " << d << " * vIn.y + " << f << ";\n"
		   << "\t\treal_t angle = (atan2(y, x) + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << power << ";\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(SQR(x) + SQR(y), " << cn << ");\n"
		   << "\n"
		   << "\t\tvOut.x = r * cosa;\n"
		   << "\t\tvOut.y = r * sina;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power == 0)
			m_Power = 2;

		m_AbsN = T(int(abs(m_Power)));
		m_Cn = m_Dist / m_Power / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A,     prefix + "julian2_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B,     prefix + "julian2_b"));
		m_Params.push_back(ParamWithName<T>(&m_C,     prefix + "julian2_c"));
		m_Params.push_back(ParamWithName<T>(&m_D,     prefix + "julian2_d", 1));
		m_Params.push_back(ParamWithName<T>(&m_E,     prefix + "julian2_e"));
		m_Params.push_back(ParamWithName<T>(&m_F,     prefix + "julian2_f"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "julian2_power", 2, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Dist,  prefix + "julian2_dist", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "julian2_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn,   prefix + "julian2_cn"));
	}

private:
	T m_A;
	T m_B;
	T m_C;
	T m_D;
	T m_E;
	T m_F;
	T m_Power;
	T m_Dist;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// JuliaQ.
/// </summary>
template <typename T>
class EMBER_API JuliaQVariation : public ParametricVariation<T>
{
public:
	JuliaQVariation(T weight = 1.0) : ParametricVariation<T>("juliaq", VAR_JULIAQ, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaQVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx * m_InvPower + rand.Rand() * m_InvPower2pi;
		T sina = sin(a);
		T cosa = cos(a);
		T r = m_Weight * pow(helper.m_PrecalcSumSquares, m_HalfInvPower);

		helper.Out.x = r * cosa;
		helper.Out.y = r * sina;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string power        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfInvPower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower2Pi  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx * " << invPower << " + MwcNext(mwc) * " << invPower2Pi << ";\n"
		   << "\t\treal_t sina = sin(a);\n"
		   << "\t\treal_t cosa = cos(a);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << halfInvPower << ");\n"
		   << "\n"
		   << "\t\tvOut.x = r * cosa;\n"
		   << "\t\tvOut.y = r * sina;\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_HalfInvPower = T(0.5) * m_Divisor / m_Power;
		m_InvPower = m_Divisor / m_Power;
		m_InvPower2pi = M_2PI / m_Power;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,   prefix + "juliaq_power", 3, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "juliaq_divisor", 2, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfInvPower, prefix + "juliaq_half_inv_power"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower,     prefix + "juliaq_inv_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower2pi,  prefix + "juliaq_inv_power_2pi"));
	}

private:
	T m_Power;
	T m_Divisor;
	T m_HalfInvPower;//Precalc.
	T m_InvPower;
	T m_InvPower2pi;
};

/// <summary>
/// Murl.
/// </summary>
template <typename T>
class EMBER_API MurlVariation : public ParametricVariation<T>
{
public:
	MurlVariation(T weight = 1.0) : ParametricVariation<T>("murl", VAR_MURL, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(MurlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = helper.m_PrecalcAtanyx * m_Power;
		T sina = sin(angle);
		T cosa = cos(angle);
		T r = m_Cp * pow(helper.m_PrecalcSumSquares, m_P2);
		T re = r * cosa + 1;
		T im = r * sina;
		T r1 = m_Vp / (SQR(re) + SQR(im));

		helper.Out.x = r1 * (helper.In.x * re + helper.In.y * im);
		helper.Out.y = r1 * (helper.In.y * re - helper.In.x * im);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string c     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p2    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t angle = precalcAtanyx * " << power << ";\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t r = " << cp << " * pow(precalcSumSquares, " << p2 << ");\n"
		   << "\t\treal_t re = r * cosa + 1;\n"
		   << "\t\treal_t im = r * sina;\n"
		   << "\t\treal_t r1 = " << vp << " / (SQR(re) + SQR(im));\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * (vIn.x * re + vIn.y * im);\n"
		   << "\t\tvOut.y = r1 * (vIn.y * re - vIn.x * im);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power != 1)
			m_Cp = m_C / (m_Power - 1);
		else
			m_Cp = m_C;

		m_P2 = m_Power / 2;
		m_Vp = m_Weight * (m_Cp + 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C,     prefix + "murl_c"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "murl_power", 2, INTEGER, 2, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Cp, prefix + "murl_cp"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_P2, prefix + "murl_p2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vp, prefix + "murl_vp"));
	}

private:
	T m_C;
	T m_Power;
	T m_Cp;//Precalc.
	T m_P2;
	T m_Vp;
};

/// <summary>
/// Murl2.
/// </summary>
template <typename T>
class EMBER_API Murl2Variation : public ParametricVariation<T>
{
public:
	Murl2Variation(T weight = 1.0) : ParametricVariation<T>("murl2", VAR_MURL2, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Murl2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = helper.m_PrecalcAtanyx * m_Power;
		T sina = sin(angle);
		T cosa = cos(angle);
		T r = m_C * pow(helper.m_PrecalcSumSquares, m_P2);
		T re = r * cosa + 1;
		T im = r * sina;

		r = pow(SQR(re) + SQR(im), m_InvP);
		angle = atan2(im, re) * m_InvP2;
		sina = sin(angle);
		cosa = cos(angle);
		re = r * cosa;
		im = r * sina;

		T r1 = m_Vp / SQR(r);

		helper.Out.x = r1 * (helper.In.x * re + helper.In.y * im);
		helper.Out.y = r1 * (helper.In.y * re - helper.In.x * im);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string c     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p2    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invp  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invp2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t angle = precalcAtanyx * " << power << ";\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t r = " << c << " * pow(precalcSumSquares, " << p2 << ");\n"
		   << "\t\treal_t re = r * cosa + 1;\n"
		   << "\t\treal_t im = r * sina;\n"
		   << "\n"
		   << "\t\tr = pow(SQR(re) + SQR(im), " << invp << ");\n"
		   << "\t\tangle = atan2(im, re) * " << invp2 << ";\n"
		   << "\t\tsina = sin(angle);\n"
		   << "\t\tcosa = cos(angle);\n"
		   << "\t\tre = r * cosa;\n"
		   << "\t\tim = r * sina;\n"
		   << "\n"
		   << "\t\treal_t r1 = " << vp << " / SQR(r);\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * (vIn.x * re + vIn.y * im);\n"
		   << "\t\tvOut.y = r1 * (vIn.y * re - vIn.x * im);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_P2 = m_Power / 2;
		m_InvP = 1 / m_Power;
		m_InvP2 = 2 / m_Power;

		if (m_C == -1)
			m_Vp = 0;
		else
			m_Vp = m_Weight * pow(m_C + 1, 2 / m_Power);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C,     prefix + "murl2_c", 0, REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "murl2_power", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_P2,    prefix + "murl2_p2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvP,  prefix + "murl2_invp"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvP2, prefix + "murl2_invp2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vp,    prefix + "murl2_vp"));
	}

private:
	T m_C;
	T m_Power;
	T m_P2;//Precalc.
	T m_InvP;
	T m_InvP2;
	T m_Vp;
};

/// <summary>
/// NPolar.
/// </summary>
template <typename T>
class EMBER_API NPolarVariation : public ParametricVariation<T>
{
public:
	NPolarVariation(T weight = 1.0) : ParametricVariation<T>("npolar", VAR_NPOLAR, weight, true, false, false, true, false)
	{
		Init();
	}

	PARVARCOPY(NPolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = (m_IsOdd != 0) ? helper.In.x : m_Vvar * helper.m_PrecalcAtanxy;
		T y = (m_IsOdd != 0) ? helper.In.y : m_Vvar2 * log(helper.m_PrecalcSumSquares);
		T angle = (atan2(y, x) + M_2PI * rand.Rand(int(m_AbsN))) / m_Nnz;
		T r = m_Weight * pow(SQR(x) + SQR(y), m_Cn) * ((m_IsOdd == 0) ? 1 : m_Parity);
		T sina = sin(angle) * r;
		T cosa = cos(angle) * r;

		x = (m_IsOdd != 0) ? cosa : (m_Vvar2 * log(SQR(cosa) + SQR(sina)));
		y = (m_IsOdd != 0) ? sina : (m_Vvar * atan2(cosa, sina));
		helper.Out.x = x;
		helper.Out.y = y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string parity = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string nnz    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string isOdd  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = (" << isOdd << " != 0) ? vIn.x : " << vvar << " * precalcAtanxy;\n"
		   << "\t\treal_t y = (" << isOdd << " != 0) ? vIn.y : " << vvar2 << " * log(precalcSumSquares);\n"
		   << "\t\treal_t angle = (atan2(y, x) + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << nnz << ";\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(SQR(x) + SQR(y), " << cn << ") * ((" << isOdd << " == 0) ? 1 : " << parity << ");\n"
		   << "\t\treal_t sina = sin(angle) * r;\n"
		   << "\t\treal_t cosa = cos(angle) * r;\n"
		   << "\n"
		   << "\t\tx = (" << isOdd << " != 0) ? cosa : (" << vvar2 << " * log(SQR(cosa) + SQR(sina)));\n"
		   << "\t\ty = (" << isOdd << " != 0) ? sina : (" << vvar << " * atan2(cosa, sina));\n"
		   << "\t\tvOut.x = x;\n"
		   << "\t\tvOut.y = y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Nnz = (m_N == 0) ? 1 : m_N;
		m_Vvar = m_Weight / T(M_PI);
		m_Vvar2 = m_Vvar * T(0.5);
		m_AbsN = abs(m_Nnz);
		m_Cn = 1 / m_Nnz / 2;
		m_IsOdd = T(abs(int(m_Parity)) % 2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Parity, prefix + "npolar_parity", 0, INTEGER));
		m_Params.push_back(ParamWithName<T>(&m_N,      prefix + "npolar_n", 1, INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_Nnz,   prefix + "npolar_nnz"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar,  prefix + "npolar_vvar"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "npolar_vvar_2"));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN,  prefix + "npolar_absn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cn,    prefix + "npolar_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_IsOdd, prefix + "npolar_isodd"));
	}

private:
	T m_Parity;
	T m_N;
	T m_Nnz;//Precalc.
	T m_Vvar;
	T m_Vvar2;
	T m_AbsN;
	T m_Cn;
	T m_IsOdd;
};

/// <summary>
/// Ortho.
/// </summary>
template <typename T>
class EMBER_API OrthoVariation : public ParametricVariation<T>
{
public:
	OrthoVariation(T weight = 1.0) : ParametricVariation<T>("ortho", VAR_ORTHO, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(OrthoVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, a;
		T xo;
		T ro;
		T c,s;
		T x, y, tc, ts;
		T theta;

		r = helper.m_PrecalcSumSquares;

		if (r < 1)
		{
			if (helper.In.x >= 0)
			{
				xo = (r + 1) / (2 * helper.In.x);
				ro = sqrt(SQR(helper.In.x - xo) + SQR(helper.In.y));
				theta = atan2(T(1), ro);
				a = fmod(m_In * theta + atan2(helper.In.y, xo - helper.In.x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);

				helper.Out.x = m_Weight * (xo - c * ro);
				helper.Out.y = m_Weight * s * ro;
			}
			else
			{
				xo = - (r + 1) / (2 * helper.In.x);
				ro = sqrt(SQR(-helper.In.x - xo) + SQR(helper.In.y));
				theta = atan2(T(1), ro);
				a = fmod(m_In * theta + atan2(helper.In.y, xo + helper.In.x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);

				helper.Out.x = -(m_Weight * (xo - c * ro));
				helper.Out.y = m_Weight * s * ro;
			}
		}
		else
		{
			r = 1 / sqrt(r);
			ts = sin(helper.m_PrecalcAtanyx);
			tc = cos(helper.m_PrecalcAtanyx);
			x = r * tc;
			y = r * ts;

			if (x >= 0)
			{
				xo = (SQR(x) + SQR(y) + 1) / (2 * x);
				ro = sqrt(SQR(x - xo) + SQR(y));
				theta = atan2(T(1), ro);
				a = fmod(m_Out * theta + atan2(y, xo - x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);

				x = (xo - c * ro);
				y =  s * ro;
				theta = atan2(y, x);
				sincos(theta, &ts, &tc);
				r = 1 / sqrt(SQR(x) + SQR(y));

				helper.Out.x = m_Weight * r * tc;
				helper.Out.y = m_Weight * r * ts;
			}
			else
			{
				xo = - (SQR(x) + SQR(y) + 1) / (2 * x);
				ro = sqrt(SQR(-x - xo) + SQR(y));
				theta = atan2(T(1), ro);
				a = fmod(m_Out * theta + atan2(y, xo + x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);

				x = (xo - c * ro);
				y =  s * ro;
				theta = atan2(y, x);
				sincos(theta, &ts, &tc);
				r = 1 / sqrt(SQR(x) + SQR(y));

				helper.Out.x = -(m_Weight * r * tc);
				helper.Out.y = m_Weight * r * ts;
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
		string in       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string out      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r, a;\n"
		   << "\t\treal_t xo;\n"
		   << "\t\treal_t ro;\n"
		   << "\t\treal_t c,s;\n"
		   << "\t\treal_t x, y, tc, ts;\n"
		   << "\t\treal_t theta;\n"
		   << "\n"
		   << "\t\tr = precalcSumSquares;\n"
		   << "\n"
		   << "\t\tif (r < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = (r + 1) / (2 * vIn.x);\n"
		   << "\t\t		ro = sqrt(SQR(vIn.x - xo) + SQR(vIn.y));\n"
		   << "\t\t		theta = atan2(1, ro);\n"
		   << "\t\t		a = fmod(" << in << " * theta + atan2(vIn.y, xo - vIn.x) + theta, 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * (xo - c * ro);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * s * ro;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = - (r + 1) / (2 * vIn.x);\n"
		   << "\t\t		ro = sqrt(SQR(-vIn.x - xo) + SQR(vIn.y));\n"
		   << "\t\t		theta = atan2(1 , ro);\n"
		   << "\t\t		a = fmod(" << in << " * theta + atan2(vIn.y, xo + vIn.x) + theta, 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * (xo - c * ro));\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * s * ro;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	r = 1 / sqrt(r);\n"
		   << "\t\t	ts = sin(precalcAtanyx);\n"
		   << "\t\t	tc = cos(precalcAtanyx);\n"
		   << "\t\t	x = r * tc;\n"
		   << "\t\t	y = r * ts;\n"
		   << "\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = (SQR(x) + SQR(y) + 1) / (2 * x);\n"
		   << "\t\t		ro = sqrt(SQR(x - xo) + SQR(y));\n"
		   << "\t\t		theta = atan2(1 , ro);\n"
		   << "\t\t		a = fmod(" << out << " * theta + atan2(y, xo - x) + theta, 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		x = (xo - c * ro);\n"
		   << "\t\t		y =  s * ro;\n"
		   << "\t\t		theta = atan2(y, x);\n"
		   << "\t\t		ts = sin(theta);\n"
		   << "\t\t		tc = cos(theta);\n"
		   << "\t\t		r = 1 / sqrt(SQR(x) + SQR(y));\n"
		   << "\n"
		   << "\t\t		vOut.x = xform->m_VariationWeights[" << varIndex << "] * r * tc;\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * r * ts;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = - (SQR(x) + SQR(y) + 1) / (2 * x);\n"
		   << "\t\t		ro = sqrt(SQR(-x - xo) + SQR(y));\n"
		   << "\t\t		theta = atan2(1 , ro);\n"
		   << "\t\t		a = fmod(" << out << " * theta + atan2(y, xo + x) + theta, 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		x = (xo - c * ro);\n"
		   << "\t\t		y =  s * ro;\n"
		   << "\t\t		theta = atan2(y, x);\n"
		   << "\t\t		ts = sin(theta);\n"
		   << "\t\t		tc = cos(theta);\n"
		   << "\t\t		r = 1 / sqrt(SQR(x) + SQR(y));\n"
		   << "\n"
		   << "\t\t		vOut.x = -(xform->m_VariationWeights[" << varIndex << "] * r * tc);\n"
		   << "\t\t		vOut.y = xform->m_VariationWeights[" << varIndex << "] * r * ts;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_In,  prefix + "ortho_in",  0, REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Out, prefix + "ortho_out", 0, REAL_CYCLIC, T(-M_PI), T(M_PI)));
	}

private:
	T m_In;
	T m_Out;
};

/// <summary>
/// Poincare.
/// </summary>
template <typename T>
class EMBER_API PoincareVariation : public ParametricVariation<T>
{
public:
	PoincareVariation(T weight = 1.0) : ParametricVariation<T>("poincare", VAR_POINCARE, weight)
	{
		Init();
	}

	PARVARCOPY(PoincareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = m_C1x + (SQR(m_C1r) * (helper.In.x - m_C1x)) / (SQR(helper.In.x - m_C1x) + SQR(helper.In.y - m_C1y));
		T y = m_C1y + (SQR(m_C1r) * (helper.In.y - m_C1y)) / (SQR(helper.In.x - m_C1x) + SQR(helper.In.y - m_C1y));

		helper.Out.x = m_C2x + (SQR(m_C2r) * (x - m_C2x)) / (SQR(x - m_C2x) + SQR(y - m_C2y));
		helper.Out.y = m_C2y + (SQR(m_C2r) * (y - m_C2y)) / (SQR(x - m_C2x) + SQR(y - m_C2y));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string c1r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = " << c1x << " + (SQR(" << c1r << ") * (vIn.x - " << c1x << ")) / (SQR(vIn.x - " << c1x << ") + SQR(vIn.y - " << c1y << "));\n"
		   << "\t\treal_t y = " << c1y << " + (SQR(" << c1r << ") * (vIn.y - " << c1y << ")) / (SQR(vIn.x - " << c1x << ") + SQR(vIn.y - " << c1y << "));\n"
		   << "\n"
		   << "\t\tvOut.x = " << c2x << " + (SQR(" << c2r << ") * (x - " << c2x << ")) / (SQR(x - " << c2x << ") + SQR(y - " << c2y << "));\n"
		   << "\t\tvOut.y = " << c2y << " + (SQR(" << c2r << ") * (y - " << c2y << ")) / (SQR(x - " << c2x << ") + SQR(y - " << c2y << "));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C1d = sqrt(1 + SQR(m_C1r));
		m_C2d = sqrt(1 + SQR(m_C2r));

		m_C1x = m_C1d * cos(fmod(m_C1a, T(M_PI)));
		m_C1y = m_C1d * sin(fmod(m_C1a, T(M_PI)));
		m_C2x = m_C2d * cos(fmod(m_C2a, T(M_PI)));
		m_C2y = m_C2d * sin(fmod(m_C2a, T(M_PI)));
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C1r, prefix + "poincare_c1r", 1));
		m_Params.push_back(ParamWithName<T>(&m_C1a, prefix + "poincare_c1a", -1, REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_C2r, prefix + "poincare_c2r", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2a, prefix + "poincare_c2a", 1, REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(true, &m_C1x, prefix + "poincare_c1x"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C1y, prefix + "poincare_c1y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "poincare_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "poincare_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C1d, prefix + "poincare_c1d"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2d, prefix + "poincare_c2d"));
	}

private:
	T m_C1r;
	T m_C1a;
	T m_C2r;
	T m_C2a;
	T m_C1x;//Precalc.
	T m_C1y;
	T m_C2x;
	T m_C2y;
	T m_C1d;
	T m_C2d;
};

/// <summary>
/// Poincare3D.
/// </summary>
template <typename T>
class EMBER_API Poincare3DVariation : public ParametricVariation<T>
{
public:
	Poincare3DVariation(T weight = 1.0) : ParametricVariation<T>("poincare3D", VAR_POINCARE3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Poincare3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = m_C2x * helper.In.x;
		T y2cy = m_C2y * helper.In.y;
		T z2cz = m_C2z * helper.In.z;
		T val = Zeps(m_C2 * r2 - x2cx - y2cy - z2cz + 1);
		T d = m_Weight / val;

		helper.Out.x = d * (helper.In.x * m_S2x + m_Cx * (y2cy + z2cz - r2 - 1));
		helper.Out.y = d * (helper.In.y * m_S2y + m_Cy * (x2cx + z2cz - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z + m_Cz * (y2cy + x2cx - r2 - 1));
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string r   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string a   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string b   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string cy  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string cz  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t y2cy = " << c2y << " * vIn.y;\n"
		   << "\t\treal_t z2cz = " << c2z << " * vIn.z;\n"
		   << "\t\treal_t val = Zeps(" << c2 << " * r2 - x2cx - y2cy - z2cz + (real_t)(1.0));\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] / val;\n"
		   << "\n"
		   << "\t\tvOut.x = d * (vIn.x * " << s2x << " + " << cx << " * (y2cy + z2cz - r2 - (real_t)(1.0)));\n"
		   << "\t\tvOut.y = d * (vIn.y * " << s2y << " + " << cy << " * (x2cx + z2cz - r2 - (real_t)(1.0)));\n"
		   << "\t\tvOut.z = d * (vIn.z * " << s2z << " + " << cz << " * (y2cy + x2cx - r2 - (real_t)(1.0)));\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Cx = -m_R * cos(m_A * T(M_PI_2)) * cos(m_B * T(M_PI_2));
		m_Cy =  m_R * sin(m_A * T(M_PI_2)) * cos(m_B * T(M_PI_2));
		m_Cz = -m_R * sin(m_B * T(M_PI_2));

		m_C2 = SQR(m_Cx) + SQR(m_Cy) + SQR(m_Cz);

		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_C2z = 2 * m_Cz;

		m_S2x = SQR(m_Cx) - SQR(m_Cy) - SQR(m_Cz) + 1;
		m_S2y = SQR(m_Cy) - SQR(m_Cx) - SQR(m_Cz) + 1;
		m_S2z = SQR(m_Cz) - SQR(m_Cy) - SQR(m_Cx) + 1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R, prefix + "poincare3D_r"));
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "poincare3D_a"));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "poincare3D_b"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx,  prefix + "poincare3D_cx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cy,  prefix + "poincare3D_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cz,  prefix + "poincare3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2,  prefix + "poincare3D_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "poincare3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "poincare3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "poincare3D_c2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "poincare3D_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "poincare3D_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "poincare3D_s2z"));
	}

private:
	T m_R;
	T m_A;
	T m_B;
	T m_Cx;//Precalc.
	T m_Cy;
	T m_Cz;
	T m_C2;
	T m_C2x;
	T m_C2y;
	T m_C2z;
	T m_S2x;
	T m_S2y;
	T m_S2z;
};

/// <summary>
/// Polynomial.
/// </summary>
template <typename T>
class EMBER_API PolynomialVariation : public ParametricVariation<T>
{
public:
	PolynomialVariation(T weight = 1.0) : ParametricVariation<T>("polynomial", VAR_POLYNOMIAL, weight)
	{
		Init();
	}

	PARVARCOPY(PolynomialVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp = pow(fabs(m_Weight) * fabs(helper.In.x), m_Powx);//Original did not fabs.
		T yp = pow(fabs(m_Weight) * fabs(helper.In.y), m_Powy);

		helper.Out.x = xp * Sign(helper.In.x) + m_Lcx * helper.In.x + m_Scx;
		helper.Out.y = yp * Sign(helper.In.y) + m_Lcy * helper.In.y + m_Scy;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string powx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string lcx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string lcy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t xp = pow(fabs(xform->m_VariationWeights[" << varIndex << "]) * fabs(vIn.x), " << powx << ");\n"
		   << "\t\treal_t yp = pow(fabs(xform->m_VariationWeights[" << varIndex << "]) * fabs(vIn.y), " << powy << ");\n"
		   << "\t\treal_t zp = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\n"
		   << "\t\tvOut.x = xp * Sign(vIn.x) + " << lcx << " * vIn.x + " << scx << ";\n"
		   << "\t\tvOut.y = yp * Sign(vIn.y) + " << lcy << " * vIn.y + " << scy << ";\n"
		   << "\t\tvOut.z = zp;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Powx, prefix + "polynomial_powx", 1));
		m_Params.push_back(ParamWithName<T>(&m_Powy, prefix + "polynomial_powy", 1));
		m_Params.push_back(ParamWithName<T>(&m_Lcx,  prefix + "polynomial_lcx"));
		m_Params.push_back(ParamWithName<T>(&m_Lcy,  prefix + "polynomial_lcy"));
		m_Params.push_back(ParamWithName<T>(&m_Scx,  prefix + "polynomial_scx"));
		m_Params.push_back(ParamWithName<T>(&m_Scy,  prefix + "polynomial_scy"));
	}

private:
	T m_Powx;
	T m_Powy;
	T m_Lcx;
	T m_Lcy;
	T m_Scx;
	T m_Scy;
};

/// <summary>
/// PSphere.
/// </summary>
template <typename T>
class EMBER_API PSphereVariation : public ParametricVariation<T>
{
public:
	PSphereVariation(T weight = 1.0) : ParametricVariation<T>("psphere", VAR_PSPHERE, weight)
	{
		Init();
	}

	PARVARCOPY(PSphereVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c0 = helper.In.x * m_Vpi;
		T c1 = helper.In.y * m_Vpi;
		T sinc0, cosc0, sinc1, cosc1;

		sincos(c0, &sinc0, &cosc0);
		sincos(c1, &sinc1, &cosc1);

		helper.Out.x = cosc0 * -sinc1;
		helper.Out.y = sinc0 * cosc1;
		helper.Out.z = cosc1 * m_ZScale;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string zscale = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string vpi    = "parVars[" + ToUpper(m_Params[i++].Name())  + index;

		ss << "\t{\n"
		   << "\t\treal_t c0 = vIn.x * " << vpi << ";\n"
		   << "\t\treal_t c1 = vIn.y * " << vpi << ";\n"
		   << "\n"
		   << "\t\treal_t sinc0 = sin(c0);\n"
		   << "\t\treal_t cosc0 = cos(c0);\n"
		   << "\t\treal_t sinc1 = sin(c1);\n"
		   << "\t\treal_t cosc1 = cos(c1);\n"
		   << "\n"
		   << "\t\tvOut.x = cosc0 * -sinc1;\n"
		   << "\t\tvOut.y = sinc0 * cosc1;\n"
		   << "\t\tvOut.z = cosc1 * " << zscale << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vpi = m_Weight * T(M_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ZScale, prefix + "psphere_zscale"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vpi,  prefix + "psphere_vpi"));//Precalc.
	}

private:
	T m_ZScale;
	T m_Vpi;//Precalc.
};

/// <summary>
/// Rational3.
/// </summary>
template <typename T>
class EMBER_API Rational3Variation : public ParametricVariation<T>
{
public:
	Rational3Variation(T weight = 1.0) : ParametricVariation<T>("rational3", VAR_RATIONAL3, weight)
	{
		Init();
	}

	PARVARCOPY(Rational3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xsqr = helper.In.x * helper.In.x;
		T ysqr = helper.In.y * helper.In.y;
		T xcb  = helper.In.x * helper.In.x * helper.In.x;
		T ycb  = helper.In.y * helper.In.y * helper.In.y;

		T tr = m_T3 * (xcb - 3 * helper.In.x * ysqr) + m_T2 * (xsqr - ysqr) + m_T1 * helper.In.x + m_Tc;
		T ti = m_T3 * (3 * xsqr * helper.In.y - ycb) + m_T2 * 2 * helper.In.x * helper.In.y + m_T1 * helper.In.y;

		T br = m_B3 * (xcb - 3 * helper.In.x * ysqr) + m_B2 * (xsqr - ysqr) + m_B1 * helper.In.x + m_Bc;
		T bi = m_B3 * (3 * xsqr * helper.In.y - ycb) + m_B2 * 2 * helper.In.x * helper.In.y + m_B1 * helper.In.y;

		T r3den = 1 / (br * br + bi * bi);

		helper.Out.x = m_Weight * (tr * br + ti * bi) * r3den;
		helper.Out.y = m_Weight * (ti * br - tr * bi) * r3den;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string t3 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string t2 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string t1 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string tc = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string b3 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string b2 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string b1 = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string bc = "parVars[" + ToUpper(m_Params[i++].Name())  + index;

		ss << "\t{\n"
		   << "\t\treal_t xsqr = vIn.x * vIn.x;\n"
		   << "\t\treal_t ysqr = vIn.y * vIn.y;\n"
		   << "\t\treal_t xcb  = vIn.x * vIn.x * vIn.x;\n"
		   << "\t\treal_t ycb  = vIn.y * vIn.y * vIn.y;\n"
		   << "\n"
		   << "\t\treal_t tr = " << t3 << " * (xcb - 3 * vIn.x * ysqr) + " << t2 << " * (xsqr - ysqr) + " << t1 << " * vIn.x + " << tc << ";\n"
		   << "\t\treal_t ti = " << t3 << " * (3 * xsqr * vIn.y - ycb) + " << t2 << " * 2 * vIn.x * vIn.y + " << t1 << " * vIn.y;\n"
		   << "\n"
		   << "\t\treal_t br = " << b3 << " * (xcb - 3 * vIn.x * ysqr) + " << b2 << " * (xsqr - ysqr) + " << b1 << " * vIn.x + " << bc << ";\n"
		   << "\t\treal_t bi = " << b3 << " * (3 * xsqr * vIn.y - ycb) + " << b2 << " * 2 * vIn.x * vIn.y + " << b1 << " * vIn.y;\n"
		   << "\n"
		   << "\t\treal_t r3den = 1 / (br * br + bi * bi);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * (tr * br + ti * bi) * r3den;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * (ti * br - tr * bi) * r3den;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_T3, prefix + "rational3_t3", 1));
		m_Params.push_back(ParamWithName<T>(&m_T2, prefix + "rational3_t2"));
		m_Params.push_back(ParamWithName<T>(&m_T1, prefix + "rational3_t1"));
		m_Params.push_back(ParamWithName<T>(&m_Tc, prefix + "rational3_tc", 1));
		m_Params.push_back(ParamWithName<T>(&m_B3, prefix + "rational3_b3"));
		m_Params.push_back(ParamWithName<T>(&m_B2, prefix + "rational3_b2", 1));
		m_Params.push_back(ParamWithName<T>(&m_B1, prefix + "rational3_b1"));
		m_Params.push_back(ParamWithName<T>(&m_Bc, prefix + "rational3_bc", 1));
	}

private:
	T m_T3;
	T m_T2;
	T m_T1;
	T m_Tc;
	T m_B3;
	T m_B2;
	T m_B1;
	T m_Bc;
};

/// <summary>
/// Ripple.
/// </summary>
template <typename T>
class EMBER_API RippleVariation : public ParametricVariation<T>
{
public:
	RippleVariation(T weight = 1.0) : ParametricVariation<T>("ripple", VAR_RIPPLE, weight)
	{
		Init();
	}

	PARVARCOPY(RippleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Align input x, y to given center and multiply with scale.
		T x = (helper.In.x * m_S) - m_CenterX;
		T y = (helper.In.y * m_S) + m_CenterY;

		//Calculate distance from center but constrain it to EPS.
		T d = std::max(EPS, sqrt(SQR(x) * SQR(y)));

		//Normalize x and y.
		T nx = x / d;
		T ny = y / d;

		//Calculate cosine wave with given frequency, velocity
		//and phase based on the distance to center.
		T wave = cos(m_F * d - m_Vxp);

		//Calculate the wave offsets
		T d1 = wave * m_Pxa + d;
		T d2 = wave * m_Pixa + d;

		//We got two offsets, so we also got two new positions (u,v).
		T u1 = m_CenterX  + nx * d1;
		T v1 = -m_CenterY + ny * d1;
		T u2 = m_CenterX  + nx * d2;
		T v2 = -m_CenterY + ny * d2;

		//Interpolate the two positions by the given phase and
		//invert the multiplication with scale from before.
		helper.Out.x = m_Weight * Lerp<T>(u1, u2, m_P) * m_Is;//Original did a direct assignment to outPoint, which is incompatible with Ember's design.
		helper.Out.y = m_Weight * Lerp<T>(v1, v2, m_P) * m_Is;
		helper.Out.z = (m_VarType == VARTYPE_REG) ? 0 : helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string frequency = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string velocity  = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string amplitude = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string centerx   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string centery   = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string phase     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string scale     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string f         = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string a         = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string p         = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string s         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string is        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vxp       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pxa       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pixa      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = (vIn.x * " << s << ") - " << centerx << ";\n"
		   << "\t\treal_t y = (vIn.y * " << s << ") + " << centery << ";\n"
		   << "\n"
		   << "\t\treal_t d = max(EPS, sqrt(SQR(x) * SQR(y)));\n"
		   << "\n"
		   << "\t\treal_t nx = x / d;\n"
		   << "\t\treal_t ny = y / d;\n"
		   << "\n"
		   << "\t\treal_t wave = cos(" << f << " * d - " << vxp << ");\n"
		   << "\n"
		   << "\t\treal_t d1 = wave * " << pxa << " + d;\n"
		   << "\t\treal_t d2 = wave * " << pixa << " + d;\n"
		   << "\n"
		   << "\t\treal_t u1 = " << centerx << "  + nx * d1;\n"
		   << "\t\treal_t v1 = -" << centery << " + ny * d1;\n"
		   << "\t\treal_t u2 = " << centerx << "  + nx * d2;\n"
		   << "\t\treal_t v2 = -" << centery << " + ny * d2;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * Lerp(u1, u2, " << p << ") * " << is << ";\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * Lerp(v1, v2, " << p << ") * " << is << ";\n"
		   << "\t\tvOut.z = " << ((m_VarType == VARTYPE_REG) ? "0" : "vIn.z") << ";\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_F = m_Frequency * 5;
		m_A = m_Amplitude * T(0.01);
		m_P = m_Phase * M_2PI - T(M_PI);
		m_S = Zeps(m_Scale);//Scale must not be zero.
		m_Is = 1 / m_S;//Need the inverse scale.

		//Pre-multiply velocity + phase, phase + amplitude and (PI - phase) + amplitude.
		m_Vxp = m_Velocity * m_P;
		m_Pxa = m_P * m_A;
		m_Pixa = (T(M_PI) - m_P) * m_A;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Frequency, prefix + "ripple_frequency", 2));
		m_Params.push_back(ParamWithName<T>(&m_Velocity,  prefix + "ripple_velocity", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amplitude, prefix + "ripple_amplitude", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_CenterX,   prefix + "ripple_centerx"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY,   prefix + "ripple_centery"));
		m_Params.push_back(ParamWithName<T>(&m_Phase,     prefix + "ripple_phase"));
		m_Params.push_back(ParamWithName<T>(&m_Scale,     prefix + "ripple_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_F,    prefix + "ripple_f"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_A,    prefix + "ripple_a"));
		m_Params.push_back(ParamWithName<T>(true, &m_P,    prefix + "ripple_p"));
		m_Params.push_back(ParamWithName<T>(true, &m_S,    prefix + "ripple_s"));
		m_Params.push_back(ParamWithName<T>(true, &m_Is,   prefix + "ripple_is"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vxp,  prefix + "ripple_vxp"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pxa , prefix + "ripple_pxa"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pixa, prefix + "ripple_pixa"));
	}

private:
	T m_Frequency;
	T m_Velocity;
	T m_Amplitude;
	T m_CenterX;
	T m_CenterY;
	T m_Phase;
	T m_Scale;
	T m_F;//Precalc.
	T m_A;
	T m_P;
	T m_S;
	T m_Is;
	T m_Vxp;
	T m_Pxa;
	T m_Pixa;
};

/// <summary>
/// Sigmoid.
/// </summary>
template <typename T>
class EMBER_API SigmoidVariation : public ParametricVariation<T>
{
public:
	SigmoidVariation(T weight = 1.0) : ParametricVariation<T>("sigmoid", VAR_SIGMOID, weight)
	{
		Init();
	}

	PARVARCOPY(SigmoidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c0 = m_Ax / (1 + exp(m_Sx * helper.In.x));
		T c1 = m_Ay / (1 + exp(m_Sy * helper.In.y));
		T x = (2 * (c0 - T(0.5)));
		T y = (2 * (c1 - T(0.5)));

		helper.Out.x = m_Vv * x;
		helper.Out.y = m_Vv * y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string shiftX = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string shiftY = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string sx     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string sy     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string ax     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string ay     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;
		string vv     = "parVars[" + ToUpper(m_Params[i++].Name())  + index;

		ss << "\t{\n"
		   << "\t\treal_t c0 = " << ax << " / (1 + exp(" << sx << " * vIn.x));\n"
		   << "\t\treal_t c1 = " << ay << " / (1 + exp(" << sy << " * vIn.y));\n"
		   << "\t\treal_t x = (2 * (c0 - (real_t)(0.5)));\n"
		   << "\t\treal_t y = (2 * (c1 - (real_t)(0.5)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << vv << " * x;\n"
		   << "\t\tvOut.y = " << vv << " * y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Sx = m_ShiftX;
		m_Sy = m_ShiftY;
		m_Ax = 1;
		m_Ay = 1;

		if (m_Sx < 1 && m_Sx > -1)
		{
			if (m_Sx == 0)
			{
				m_Sx = EPS;
				m_Ax = 1;
			}
			else
			{
				m_Ax = T(m_Sx < 0 ? -1 : 1);
				m_Sx = 1 / m_Sx;
			}
		}

		if (m_Sy < 1 && m_Sy > -1)
		{
			if (m_Sy == 0)
			{
				m_Sy = EPS;
				m_Ay = 1;
			}
			else
			{
				m_Ay = T(m_Sy < 0 ? -1 : 1);
				m_Sy = 1 / m_Sy;
			}
		}

		m_Sx *= -5;
		m_Sy *= -5;

		m_Vv = fabs(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ShiftX, prefix + "sigmoid_shiftx", 1));
		m_Params.push_back(ParamWithName<T>(&m_ShiftY, prefix + "sigmoid_shifty", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Sx, prefix + "sigmoid_sx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Sy, prefix + "sigmoid_sy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "sigmoid_ax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "sigmoid_ay"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vv, prefix + "sigmoid_vv"));
	}

private:
	T m_ShiftX;
	T m_ShiftY;
	T m_Sx;//Precalc.
	T m_Sy;
	T m_Ax;
	T m_Ay;
	T m_Vv;
};

/// <summary>
/// SinusGrid.
/// </summary>
template <typename T>
class EMBER_API SinusGridVariation : public ParametricVariation<T>
{
public:
	SinusGridVariation(T weight = 1.0) : ParametricVariation<T>("sinusgrid", VAR_SINUS_GRID, weight)
	{
		Init();
	}

	PARVARCOPY(SinusGridVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x;
		T y = helper.In.y;
		T sx = -1 * cos(x * m_Fx);
		T sy = -1 * cos(y * m_Fy);
		T tx = Lerp(helper.In.x, sx, m_Ax);
		T ty = Lerp(helper.In.y, sy, m_Ay);

		helper.Out.x = m_Weight * tx;
		helper.Out.y = m_Weight * ty;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string ampX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ampY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fx    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fy    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x;\n"
		   << "\t\treal_t y = vIn.y;\n"
		   << "\t\treal_t sx = -1 * cos(x * " << fx << ");\n"
		   << "\t\treal_t sy = -1 * cos(y * " << fy << ");\n"
		   << "\t\treal_t tx = Lerp(vIn.x, sx, " << ax << ");\n"
		   << "\t\treal_t ty = Lerp(vIn.y, sy, " << ay << ");\n"
		   << "\t\treal_t tz = vIn.z;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * tx;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * ty;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * tz;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ax = m_AmpX;
		m_Ay = m_AmpY;
		m_Fx = Zeps(m_FreqX * M_2PI);
		m_Fy = Zeps(m_FreqY * M_2PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_AmpX,  prefix + "sinusgrid_ampx", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_AmpY,  prefix + "sinusgrid_ampy", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_FreqX, prefix + "sinusgrid_freqx", 1));
		m_Params.push_back(ParamWithName<T>(&m_FreqY, prefix + "sinusgrid_freqy", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Fx, prefix + "sinusgrid_fx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Fy, prefix + "sinusgrid_fy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "sinusgrid_ax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "sinusgrid_ay"));
	}

private:
	T m_AmpX;
	T m_AmpY;
	T m_FreqX;
	T m_FreqY;
	T m_Fx;//Precalc.
	T m_Fy;
	T m_Ax;
	T m_Ay;
};

/// <summary>
/// Stwin.
/// </summary>
template <typename T>
class EMBER_API StwinVariation : public ParametricVariation<T>
{
public:
	StwinVariation(T weight = 1.0) : ParametricVariation<T>("stwin", VAR_STWIN, weight)
	{
		Init();
	}

	PARVARCOPY(StwinVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T multiplier = T(0.05);
		T x = helper.In.x * m_Weight * multiplier;
		T y = helper.In.y * m_Weight * multiplier;
		T x2 = SQR(x);
		T y2 = SQR(y);
		T xPlusy = x + y;
		T x2Minusy2 = x2 - y2;
		T x2Plusy2 = x2 + y2;
		T result = x2Minusy2 * sin(M_2PI * m_Distort * xPlusy);
		T divident = 1;

		if (x2Plusy2 != 0)
			divident = x2Plusy2;

		result /= divident;

		helper.Out.x = m_Weight * helper.In.x + result;
		helper.Out.y = m_Weight * helper.In.y + result;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string distort = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x * xform->m_VariationWeights[" << varIndex << "] * (real_t)(0.05);\n"
		   << "\t\treal_t y = vIn.y * xform->m_VariationWeights[" << varIndex << "] * (real_t)(0.05);\n"
		   << "\t\treal_t x2 = SQR(x);\n"
		   << "\t\treal_t y2 = SQR(y);\n"
		   << "\t\treal_t xPlusy = x + y;\n"
		   << "\t\treal_t x2Minusy2 = x2 - y2;\n"
		   << "\t\treal_t x2Plusy2 = x2 + y2;\n"
		   << "\t\treal_t result = x2Minusy2 * sin(M_2PI * " << distort << " * xPlusy);\n"
		   << "\t\treal_t divident = 1;\n"
		   << "\n"
		   << "\t\tif (x2Plusy2 != 0)\n"
		   << "\t\t	divident = x2Plusy2;\n"
		   << "\n"
		   << "\t\tresult /= divident;\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * vIn.x + result;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vIn.y + result;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Distort, prefix + "stwin_distort", 1));//Original had a misspelling of swtin, which is incompatible with Ember's design.
	}

private:
	T m_Distort;
};

/// <summary>
/// TwoFace.
/// </summary>
template <typename T>
class EMBER_API TwoFaceVariation : public Variation<T>
{
public:
	TwoFaceVariation(T weight = 1.0) : Variation<T>("twoface", VAR_TWO_FACE, weight, true) { }

	VARCOPY(TwoFaceVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight;

		if (helper.In.x > 0)
			r /= helper.m_PrecalcSumSquares;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "];\n"
		   << "\n"
		   << "\t\tif (vIn.x > 0)\n"
		   << "\t\t	r /= precalcSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Unpolar.
/// </summary>
template <typename T>
class EMBER_API UnpolarVariation : public ParametricVariation<T>
{
public:
	UnpolarVariation(T weight = 1.0) : ParametricVariation<T>("unpolar", VAR_UNPOLAR, weight)
	{
		Init();
	}

	PARVARCOPY(UnpolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = exp(helper.In.y);
		T s = sin(helper.In.x);
		T c = cos(helper.In.x);

		helper.Out.x = m_Vvar2 * r * s;
		helper.Out.y = m_Vvar2 * r * c;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string vvar2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		ss << "\t{\n"
		   << "\t\treal_t r = exp(vIn.y);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\n"
		   << "\t\tvOut.x = " << vvar2 << " * r * s;\n"
		   << "\t\tvOut.y = " << vvar2 << " * r * c;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar2 = (m_Weight / T(M_PI)) * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "unpolar_vvar_2"));//Precalcs only, no params.
	}

private:
	T m_Vvar2;//Precalcs only, no params.
};

/// <summary>
/// WavesN.
/// </summary>
template <typename T>
class EMBER_API WavesNVariation : public ParametricVariation<T>
{
public:
	WavesNVariation(T weight = 1.0) : ParametricVariation<T>("wavesn", VAR_WAVESN, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WavesNVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(int(m_AbsN))) / m_Power;
		T r = m_Weight * pow(helper.m_PrecalcSumSquares, m_Cn);
		T sina = sin(angle);
		T cosa = cos(angle);
		T xn = r * cosa;
		T yn = r * sina;
		T siny = sin(m_FreqX * yn);
		T sinx = sin(m_FreqY * xn);
		T dx = xn + T(0.5) * (m_ScaleX * siny + fabs(xn) * m_IncX * siny);
		T dy = yn + T(0.5) * (m_ScaleY * sinx + fabs(yn) * m_IncY * sinx);

		helper.Out.x = m_Weight * dx;
		helper.Out.y = m_Weight * dy;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string freqX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string incX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string incY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t angle = (precalcAtanyx + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << power << ";\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t xn = r * cosa;\n"
		   << "\t\treal_t yn = r * sina;\n"
		   << "\t\treal_t siny = sin(" << freqX << " * yn);\n"
		   << "\t\treal_t sinx = sin(" << freqY << " * xn);\n"
		   << "\t\treal_t dx = xn + (real_t)(0.5) * (" << scaleX << " * siny + fabs(xn) * " << incX << " * siny);\n"
		   << "\t\treal_t dy = yn + (real_t)(0.5) * (" << scaleY << " * sinx + fabs(yn) * " << incY << " * sinx);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * dx;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * dy;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power == 0)
			m_Power = 2;

		m_AbsN = T(int(fabs(m_Power)));

		m_Cn = 1 / m_Power / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_FreqX,  prefix + "wavesn_freqx", 2));
		m_Params.push_back(ParamWithName<T>(&m_FreqY,  prefix + "wavesn_freqy", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleX, prefix + "wavesn_scalex", 1));
		m_Params.push_back(ParamWithName<T>(&m_ScaleY, prefix + "wavesn_scaley", 1));
		m_Params.push_back(ParamWithName<T>(&m_IncX,   prefix + "wavesn_incx"));
		m_Params.push_back(ParamWithName<T>(&m_IncY,   prefix + "wavesn_incy"));
		m_Params.push_back(ParamWithName<T>(&m_Power,  prefix + "wavesn_power", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "wavesn_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn,   prefix + "wavesn_cn"));
	}

private:
	T m_FreqX;
	T m_FreqY;
	T m_ScaleX;
	T m_ScaleY;
	T m_IncX;
	T m_IncY;
	T m_Power;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// XHeart.
/// </summary>
template <typename T>
class EMBER_API XHeartVariation : public ParametricVariation<T>
{
public:
	XHeartVariation(T weight = 1.0) : ParametricVariation<T>("xheart", VAR_XHEART, weight, true)
	{
		Init();
	}

	PARVARCOPY(XHeartVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2_4 = helper.m_PrecalcSumSquares + 4;

		if (r2_4 == 0)
			r2_4 = 1;

		T bx = 4 / r2_4;
		T by = m_Rat / r2_4;
		T x = m_Cosa * (bx * helper.In.x) - m_Sina * (by * helper.In.y);
		T y = m_Sina * (bx * helper.In.x) + m_Cosa * (by  *helper.In.y);

		if (x > 0)
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = -m_Weight * y;
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ratio = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rat   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t r2_4 = precalcSumSquares + 4;\n"
		   << "\n"
		   << "\t\tif (r2_4 == 0)\n"
		   << "\t\t	r2_4 = 1;\n"
		   << "\n"
		   << "\t\treal_t bx = 4 / r2_4;\n"
		   << "\t\treal_t by = " << rat << " / r2_4;\n"
		   << "\t\treal_t x = " << cosa << " * (bx * vIn.x) - " << sina << " * (by * vIn.y);\n"
		   << "\t\treal_t y = " << sina << " * (bx * vIn.x) + " << cosa << " * (by * vIn.y);\n"
		   << "\n"
		   << "\t\tif (x > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\t	vOut.y = xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = xform->m_VariationWeights[" << varIndex << "] * x;\n"
		   << "\t\t	vOut.y = -xform->m_VariationWeights[" << varIndex << "] * y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		T ang = T(M_PI_4) + (T(0.5) * T(M_PI_4) * m_Angle);

		sincos(ang, &m_Sina, &m_Cosa);
		m_Rat = 6 + 2 * m_Ratio;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "xheart_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Ratio, prefix + "xheart_ratio"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "xheart_cosa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "xheart_sina"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rat,  prefix + "xheart_rat"));
	}

private:
	T m_Angle;
	T m_Ratio;
	T m_Cosa;//Precalc.
	T m_Sina;
	T m_Rat;
};

/// <summary>
/// Barycentroid.
/// </summary>
template <typename T>
class EMBER_API BarycentroidVariation : public ParametricVariation<T>
{
public:
	BarycentroidVariation(T weight = 1.0) : ParametricVariation<T>("barycentroid", VAR_BARYCENTROID, weight)
	{
		Init();
	}

	PARVARCOPY(BarycentroidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Compute dot products.
		T dot00 = SQR(m_A) + SQR(m_B);//v0 * v0.
		T dot01 = m_A * m_C + m_B * m_D;//v0 * v1.
		T dot02 = m_A * helper.In.x + m_B * helper.In.y;//v0 * v2.
		T dot11 = SQR(m_C) + SQR(m_D);//v1 * v1.
		T dot12 = m_C * helper.In.x + m_D * helper.In.y;//v1 * v2.

		//Compute inverse denomiator.
		T invDenom = 1 / (dot00 * dot11 - dot01 * dot01);

		//Now we can pull [u,v] as the barycentric coordinates of the point
		//P in the triangle [A, B, C].
		T u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		T v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		// now combine with input
		T um = sqrt(SQR(u) + SQR(helper.In.x)) * Sign<T>(u);
		T vm = sqrt(SQR(v) + SQR(helper.In.y)) * Sign<T>(v);

		helper.Out.x = m_Weight * um;
		helper.Out.y = m_Weight * vm;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t dot00 = SQR(" << a << ") + SQR(" << b << ");\n"
		   << "\t\treal_t dot01 = " << a << " * " << c << " + " << b << " * " << d << ";\n"
		   << "\t\treal_t dot02 = " << a << " * vIn.x + " << b << " * vIn.y;\n"
		   << "\t\treal_t dot11 = SQR(" << c << ") + SQR(" << d << ");\n"
		   << "\t\treal_t dot12 = " << c << " * vIn.x + " << d << " * vIn.y;\n"
		   << "\t\treal_t invDenom = (real_t)(1.0) / (dot00 * dot11 - dot01 * dot01);\n"
		   << "\t\treal_t u = (dot11 * dot02 - dot01 * dot12) * invDenom;\n"
		   << "\t\treal_t v = (dot00 * dot12 - dot01 * dot02) * invDenom;\n"
		   << "\t\treal_t um = sqrt(SQR(u) + SQR(vIn.x)) * Sign(u);\n"
		   << "\t\treal_t vm = sqrt(SQR(v) + SQR(vIn.y)) * Sign(v);\n"
		   << "\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * um;\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * vm;\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "barycentroid_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "barycentroid_b"));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "barycentroid_c"));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "barycentroid_d", 1));
	}

private:
	T m_A;
	T m_B;
	T m_C;
	T m_D;
};

/// <summary>
/// BiSplit.
/// </summary>
template <typename T>
class EMBER_API BiSplitVariation : public ParametricVariation<T>
{
public:
	BiSplitVariation(T weight = 1.0) : ParametricVariation<T>("bisplit", VAR_BISPLIT, weight)
	{
		Init();
	}

	PARVARCOPY(BiSplitVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight01 / SafeTan<T>(helper.In.x) * cos(helper.In.y);
		helper.Out.y = m_Weight01 / sin(helper.In.x) * (-helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight01 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight01 << " / tan(vIn.x) * cos(vIn.y);\n"
		   << "\t\tvOut.y = " << weight01 << " / sin(vIn.x) * (-vIn.y);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Weight01 = m_Weight * T(0.1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Weight01, prefix + "bisplit_weight01"));//Precalc only.
	}

private:
	T m_Weight01;
};

/// <summary>
/// Crescents.
/// </summary>
template <typename T>
class EMBER_API CrescentsVariation : public Variation<T>
{
public:
	CrescentsVariation(T weight = 1.0) : Variation<T>("crescents", VAR_CRESCENTS, weight) { }

	VARCOPY(CrescentsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * sin(helper.In.x) * (cosh(helper.In.y) + 1) * Sqr<T>(sin(helper.In.x));
		helper.Out.y = m_Weight * cos(helper.In.x) * (cosh(helper.In.y) + 1) * Sqr<T>(sin(helper.In.x));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\tvOut.x = xform->m_VariationWeights[" << varIndex << "] * sin(vIn.x) * (cosh(vIn.y) + (real_t)(1.0)) * Sqr(sin(vIn.x));\n"
		   << "\t\tvOut.y = xform->m_VariationWeights[" << varIndex << "] * cos(vIn.x) * (cosh(vIn.y) + (real_t)(1.0)) * Sqr(sin(vIn.x));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Mask.
/// </summary>
template <typename T>
class EMBER_API MaskVariation : public Variation<T>
{
public:
	MaskVariation(T weight = 1.0) : Variation<T>("mask", VAR_MASK, weight, true) { }

	VARCOPY(MaskVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = m_Weight / helper.m_PrecalcSumSquares;

		helper.Out.x = d * sin(helper.In.x) * (cosh(helper.In.y) + 1) * Sqr(sin(helper.In.x));
		helper.Out.y = d * cos(helper.In.x) * (cosh(helper.In.y) + 1) * Sqr(sin(helper.In.x));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss;
		intmax_t varIndex = IndexInXform();

		ss << "\t{\n"
		   << "\t\treal_t d = xform->m_VariationWeights[" << varIndex << "] / precalcSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = d * sin(vIn.x) * (cosh(vIn.y) + (real_t)(1.0)) * Sqr(sin(vIn.x));\n"
		   << "\t\tvOut.y = d * cos(vIn.x) * (cosh(vIn.y) + (real_t)(1.0)) * Sqr(sin(vIn.x));\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}
};

/// <summary>
/// Cpow2.
/// </summary>
template <typename T>
class EMBER_API Cpow2Variation : public ParametricVariation<T>
{
public:
	Cpow2Variation(T weight = 1.0) : ParametricVariation<T>("cpow2", VAR_CPOW2, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Cpow2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		int n = rand.Rand(uint(m_Spread));

		if (a < 0)
			n++;

		a += M_2PI * n;

		if (cos(a * m_InvSpread) < rand.Rand() * 2 / 0xFFFFFFFF - 1)//Rand max.
			a -= m_FullSpread;

		T lnr2 = log(helper.m_PrecalcSumSquares);
		T r = m_Weight * exp(m_HalfC * lnr2 - m_D * a);
		T temp = m_C * a + m_HalfD * lnr2 + m_Ang * rand.Rand();

		helper.Out.x = r * cos(temp);
		helper.Out.y = r * sin(temp);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() override
	{
		ostringstream ss, ss2;
		intmax_t i = 0, varIndex = IndexInXform();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string r          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spread     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfC      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfD      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ang        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invSpread  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fullSpread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\tint n = MwcNextRange(mwc, (uint)" << spread << ");\n"
		   << "\n"
		   << "\t\tif (a < 0)\n"
		   << "\t\t	n++;\n"
		   << "\n"
		   << "\t\ta += M_2PI * n;\n"
		   << "\n"
		   << "\t\tif (cos(a * " << invSpread << ") < MwcNext(mwc) * 2 / 0xFFFFFFFF - 1)\n"
		   << "\t\t	a -= " << fullSpread << ";\n"
		   << "\n"
		   << "\t\treal_t lnr2 = log(precalcSumSquares);\n"
		   << "\t\treal_t r = xform->m_VariationWeights[" << varIndex << "] * exp(" << halfC << " * lnr2 - " << d << " * a);\n"
		   << "\t\treal_t temp = " << c << " * a + " << halfD << " * lnr2 + " << ang << " * MwcNext(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(temp);\n"
		   << "\t\tvOut.y = r * sin(temp);\n"
		   << "\t\tvOut.z = xform->m_VariationWeights[" << varIndex << "] * vIn.z;\n"
		   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ang = M_2PI / m_Divisor;
		m_C = m_R * cos(T(M_PI) / 2 * m_A) / m_Divisor;
		m_D = m_R * sin(T(M_PI) / 2 * m_A) / m_Divisor;
		m_HalfC = m_C / 2;
		m_HalfD = m_D / 2;
		m_InvSpread = T(0.5) / m_Spread;
		m_FullSpread = M_2PI * m_Spread;
	}

protected:
	void Init()
	{
		string prefix = Prefix();

		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R,       prefix + "cpow2_r", 1));
		m_Params.push_back(ParamWithName<T>(&m_A,       prefix + "cpow2_a"));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "cpow2_divisor", 1, INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Spread,  prefix + "cpow2_spread",  1, INTEGER, 1, T(0x7FFFFFFF)));
		m_Params.push_back(ParamWithName<T>(true, &m_C,          prefix + "cpow2_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfC,      prefix + "cpow2_halfc"));
		m_Params.push_back(ParamWithName<T>(true, &m_D,          prefix + "cpow2_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfD,      prefix + "cpow2_halfd"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ang,        prefix + "cpow2_ang"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvSpread,  prefix + "cpow2_inv_spread"));
		m_Params.push_back(ParamWithName<T>(true, &m_FullSpread, prefix + "cpow2_full_spread"));
	}

private:
	T m_R;
	T m_A;
	T m_Divisor;
	T m_Spread;
	T m_C;//Precalc.
	T m_HalfC;
	T m_D;
	T m_HalfD;
	T m_Ang;
	T m_InvSpread;
	T m_FullSpread;
};

MAKEPREPOSTVAR(Hemisphere, hemisphere, HEMISPHERE)
MAKEPREPOSTPARVAR(Epispiral, epispiral, EPISPIRAL)
MAKEPREPOSTPARVAR(Bwraps, bwraps, BWRAPS)
MAKEPREPOSTVARASSIGN(BlurCircle, blur_circle, BLUR_CIRCLE, ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(BlurZoom, blur_zoom, BLUR_ZOOM)
MAKEPREPOSTPARVAR(BlurPixelize, blur_pixelize, BLUR_PIXELIZE)
MAKEPREPOSTPARVAR(Crop, crop, CROP)
MAKEPREPOSTPARVAR(BCircle, bcircle, BCIRCLE)
MAKEPREPOSTPARVAR(BlurLinear, blur_linear, BLUR_LINEAR)
MAKEPREPOSTPARVARASSIGN(BlurSquare, blur_square, BLUR_SQUARE, ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Flatten, flatten, FLATTEN)
MAKEPREPOSTVARASSIGN(Zblur, zblur, ZBLUR, ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(Blur3D, blur3D, BLUR3D, ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(ZScale, zscale, ZSCALE, ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(ZTranslate, ztranslate, ZTRANSLATE, ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(ZCone, zcone, ZCONE)
MAKEPREPOSTVAR(Spherical3D, Spherical3D, SPHERICAL3D)
MAKEPREPOSTPARVAR(Curl3D, curl3D, CURL3D)
MAKEPREPOSTPARVAR(Disc3D, disc3d, DISC3D)
MAKEPREPOSTPARVAR(Boarders2, boarders2, BOARDERS2)
MAKEPREPOSTPARVAR(Cardioid, cardioid, CARDIOID)
MAKEPREPOSTPARVAR(Checks, checks, CHECKS)
MAKEPREPOSTPARVAR(Circlize, circlize, CIRCLIZE)
MAKEPREPOSTPARVAR(Circlize2, circlize2, CIRCLIZE2)
MAKEPREPOSTPARVAR(CosWrap, coswrap, COS_WRAP)
MAKEPREPOSTVAR(DeltaA, deltaa, DELTA_A)
MAKEPREPOSTPARVAR(Expo, expo, EXPO)
MAKEPREPOSTPARVAR(Extrude, extrude, EXTRUDE)
MAKEPREPOSTVAR(FDisc, fdisc, FDISC)
MAKEPREPOSTPARVAR(Fibonacci, fibonacci, FIBONACCI)
MAKEPREPOSTPARVAR(Fibonacci2, fibonacci2, FIBONACCI2)
MAKEPREPOSTPARVAR(Glynnia, glynnia, GLYNNIA)
MAKEPREPOSTVAR(GridOut, gridout, GRIDOUT)
MAKEPREPOSTPARVAR(Hole, hole, HOLE)
MAKEPREPOSTPARVAR(Hypertile, hypertile, HYPERTILE)
MAKEPREPOSTPARVAR(Hypertile1, hypertile1, HYPERTILE1)
MAKEPREPOSTPARVAR(Hypertile2, hypertile2, HYPERTILE2)
MAKEPREPOSTPARVAR(Hypertile3D, hypertile3D, HYPERTILE3D)
MAKEPREPOSTPARVAR(Hypertile3D1, hypertile3D1, HYPERTILE3D1)
MAKEPREPOSTPARVAR(Hypertile3D2, hypertile3D2, HYPERTILE3D2)
MAKEPREPOSTPARVAR(IDisc, idisc, IDISC)
MAKEPREPOSTPARVAR(Julian2, julian2, JULIAN2)
MAKEPREPOSTPARVAR(JuliaQ, juliaq, JULIAQ)
MAKEPREPOSTPARVAR(Murl, murl, MURL)
MAKEPREPOSTPARVAR(Murl2, murl2, MURL2)
MAKEPREPOSTPARVAR(NPolar, npolar, NPOLAR)
MAKEPREPOSTPARVAR(Ortho, ortho, ORTHO)
MAKEPREPOSTPARVAR(Poincare, poincare, POINCARE)
MAKEPREPOSTPARVAR(Poincare3D, poincare3D, POINCARE3D)
MAKEPREPOSTPARVAR(Polynomial, polynomial, POLYNOMIAL)
MAKEPREPOSTPARVAR(PSphere, psphere, PSPHERE)
MAKEPREPOSTPARVAR(Rational3, rational3, RATIONAL3)
MAKEPREPOSTPARVAR(Ripple, ripple, RIPPLE)
MAKEPREPOSTPARVAR(Sigmoid, sigmoid, SIGMOID)
MAKEPREPOSTPARVAR(SinusGrid, sinusgrid, SINUS_GRID)
MAKEPREPOSTPARVAR(Stwin, stwin, STWIN)
MAKEPREPOSTVAR(TwoFace, twoface, TWO_FACE)
MAKEPREPOSTPARVAR(Unpolar, unpolar, UNPOLAR)
MAKEPREPOSTPARVAR(WavesN, wavesn, WAVESN)
MAKEPREPOSTPARVAR(XHeart, xheart, XHEART)
MAKEPREPOSTPARVAR(Barycentroid, barycentroid, BARYCENTROID)
MAKEPREPOSTPARVAR(BiSplit, bisplit, BISPLIT)
MAKEPREPOSTVAR(Crescents, crescents, CRESCENTS)
MAKEPREPOSTVAR(Mask, mask, MASK)
MAKEPREPOSTPARVAR(Cpow2, cpow2, CPOW2)
}
