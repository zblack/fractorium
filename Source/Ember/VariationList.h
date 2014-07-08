#pragma once

#include "Variations01.h"
#include "Variations02.h"
#include "Variations03.h"
#include "Variations04.h"
#include "Variations05.h"
#include "VariationsDC.h"

/// <summary>
/// VariationList class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Since the list of variations is numerous, it's convenient to be able to make copies
/// of specific ones. This class holds a list of pointers to variation objects for every
/// variation available. Similar to the PaletteList class, a caller can look up a variation
/// by name or ID and retrieve a copy of it.
/// All variations are deleted upon destruction.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API VariationList
{
#define ADDPREPOSTREGVAR(varName) \
	m_Variations.push_back(new varName##Variation<T>()); \
	m_Variations.push_back(new Pre##varName##Variation<T>()); \
	m_Variations.push_back(new Post##varName##Variation<T>());

public:
	/// <summary>
	/// Constructor which initializes all of the variation objects and stores them in the list.
	/// </summary>
	VariationList()
	{
		m_Variations.reserve(900);//Change this as the list grows.
		ADDPREPOSTREGVAR(Linear)
		ADDPREPOSTREGVAR(Sinusoidal)
		ADDPREPOSTREGVAR(Spherical)
		ADDPREPOSTREGVAR(Swirl)
		ADDPREPOSTREGVAR(Horseshoe)
		ADDPREPOSTREGVAR(Polar)
		ADDPREPOSTREGVAR(Handkerchief)
		ADDPREPOSTREGVAR(Heart)
		ADDPREPOSTREGVAR(Disc)
		ADDPREPOSTREGVAR(Spiral)
		ADDPREPOSTREGVAR(Hyperbolic)
		ADDPREPOSTREGVAR(Diamond)
		ADDPREPOSTREGVAR(Ex)
		ADDPREPOSTREGVAR(Julia)
		ADDPREPOSTREGVAR(Bent)
		ADDPREPOSTREGVAR(Waves)
		ADDPREPOSTREGVAR(Fisheye)
		ADDPREPOSTREGVAR(Popcorn)
		ADDPREPOSTREGVAR(Exponential)
		ADDPREPOSTREGVAR(Power)
		ADDPREPOSTREGVAR(Cosine)
		ADDPREPOSTREGVAR(Rings)
		ADDPREPOSTREGVAR(Fan)
		ADDPREPOSTREGVAR(Blob)
		ADDPREPOSTREGVAR(Pdj)
		ADDPREPOSTREGVAR(Fan2)
		ADDPREPOSTREGVAR(Rings2)
		ADDPREPOSTREGVAR(Eyefish)
		ADDPREPOSTREGVAR(Bubble)
		ADDPREPOSTREGVAR(Cylinder)
		ADDPREPOSTREGVAR(Perspective)
		ADDPREPOSTREGVAR(Noise)
		ADDPREPOSTREGVAR(JuliaNGeneric)
		ADDPREPOSTREGVAR(JuliaScope)
		ADDPREPOSTREGVAR(Blur)
		ADDPREPOSTREGVAR(GaussianBlur)
		ADDPREPOSTREGVAR(RadialBlur)
		ADDPREPOSTREGVAR(Pie)
		ADDPREPOSTREGVAR(Ngon)
		ADDPREPOSTREGVAR(Curl)
		ADDPREPOSTREGVAR(Rectangles)
		ADDPREPOSTREGVAR(Arch)
		ADDPREPOSTREGVAR(Tangent)
		ADDPREPOSTREGVAR(Square)
		ADDPREPOSTREGVAR(Rays)
		ADDPREPOSTREGVAR(Blade)
		ADDPREPOSTREGVAR(Secant2)
		ADDPREPOSTREGVAR(TwinTrian)
		ADDPREPOSTREGVAR(Cross)
		ADDPREPOSTREGVAR(Disc2)
		ADDPREPOSTREGVAR(SuperShape)
		ADDPREPOSTREGVAR(Flower)
		ADDPREPOSTREGVAR(Conic)
		ADDPREPOSTREGVAR(Parabola)
		ADDPREPOSTREGVAR(Bent2)
		ADDPREPOSTREGVAR(Bipolar)
		ADDPREPOSTREGVAR(Boarders)
		ADDPREPOSTREGVAR(Butterfly)
		ADDPREPOSTREGVAR(Cell)
		ADDPREPOSTREGVAR(Cpow)
		ADDPREPOSTREGVAR(Curve)
		ADDPREPOSTREGVAR(Edisc)
		ADDPREPOSTREGVAR(Elliptic)
		ADDPREPOSTREGVAR(Escher)
		ADDPREPOSTREGVAR(Foci)
		ADDPREPOSTREGVAR(LazySusan)
		ADDPREPOSTREGVAR(Loonie)
		ADDPREPOSTREGVAR(Modulus)
		ADDPREPOSTREGVAR(Oscilloscope)
		ADDPREPOSTREGVAR(Polar2)
		ADDPREPOSTREGVAR(Popcorn2)
		ADDPREPOSTREGVAR(Scry)
		ADDPREPOSTREGVAR(Separation)
		ADDPREPOSTREGVAR(Split)
		ADDPREPOSTREGVAR(Splits)
		ADDPREPOSTREGVAR(Stripes)
		ADDPREPOSTREGVAR(Wedge)
		ADDPREPOSTREGVAR(WedgeJulia)
		ADDPREPOSTREGVAR(WedgeSph)
		ADDPREPOSTREGVAR(Whorl)
		ADDPREPOSTREGVAR(Waves2)
		ADDPREPOSTREGVAR(Exp)
		ADDPREPOSTREGVAR(Log)
		ADDPREPOSTREGVAR(Sin)
		ADDPREPOSTREGVAR(Cos)
		ADDPREPOSTREGVAR(Tan)
		ADDPREPOSTREGVAR(Sec)
		ADDPREPOSTREGVAR(Csc)
		ADDPREPOSTREGVAR(Cot)
		ADDPREPOSTREGVAR(Sinh)
		ADDPREPOSTREGVAR(Cosh)
		ADDPREPOSTREGVAR(Tanh)
		ADDPREPOSTREGVAR(Sech)
		ADDPREPOSTREGVAR(Csch)
		ADDPREPOSTREGVAR(Coth)
		ADDPREPOSTREGVAR(Auger)
		ADDPREPOSTREGVAR(Flux)
		ADDPREPOSTREGVAR(Hemisphere)
		ADDPREPOSTREGVAR(Epispiral)
		ADDPREPOSTREGVAR(Bwraps)
		ADDPREPOSTREGVAR(BlurCircle)
		ADDPREPOSTREGVAR(BlurZoom)
		ADDPREPOSTREGVAR(BlurPixelize)
		ADDPREPOSTREGVAR(Crop)
		ADDPREPOSTREGVAR(BCircle)
		ADDPREPOSTREGVAR(BlurLinear)
		ADDPREPOSTREGVAR(BlurSquare)
		ADDPREPOSTREGVAR(Boarders2)
		ADDPREPOSTREGVAR(Cardioid)
		ADDPREPOSTREGVAR(Checks)
		ADDPREPOSTREGVAR(Circlize)
		ADDPREPOSTREGVAR(Circlize2)
		ADDPREPOSTREGVAR(CosWrap)
		ADDPREPOSTREGVAR(DeltaA)
		ADDPREPOSTREGVAR(Expo)
		ADDPREPOSTREGVAR(Extrude)
		ADDPREPOSTREGVAR(FDisc)
		ADDPREPOSTREGVAR(Fibonacci)
		ADDPREPOSTREGVAR(Fibonacci2)
		ADDPREPOSTREGVAR(Glynnia)
		ADDPREPOSTREGVAR(GridOut)
		ADDPREPOSTREGVAR(Hole)
		ADDPREPOSTREGVAR(Hypertile)
		ADDPREPOSTREGVAR(Hypertile1)
		ADDPREPOSTREGVAR(Hypertile2)
		ADDPREPOSTREGVAR(Hypertile3D)
		ADDPREPOSTREGVAR(Hypertile3D1)
		ADDPREPOSTREGVAR(Hypertile3D2)
		ADDPREPOSTREGVAR(IDisc)
		ADDPREPOSTREGVAR(Julian2)
		ADDPREPOSTREGVAR(JuliaQ)
		ADDPREPOSTREGVAR(Murl)
		ADDPREPOSTREGVAR(Murl2)
		ADDPREPOSTREGVAR(NPolar)
		ADDPREPOSTREGVAR(Ortho)
		ADDPREPOSTREGVAR(Poincare)
		ADDPREPOSTREGVAR(Poincare3D)
		ADDPREPOSTREGVAR(Polynomial)
		ADDPREPOSTREGVAR(PSphere)
		ADDPREPOSTREGVAR(Rational3)
		ADDPREPOSTREGVAR(Ripple)
		ADDPREPOSTREGVAR(Sigmoid)
		ADDPREPOSTREGVAR(SinusGrid)
		ADDPREPOSTREGVAR(Stwin)
		ADDPREPOSTREGVAR(TwoFace)
		ADDPREPOSTREGVAR(Unpolar)
		ADDPREPOSTREGVAR(WavesN)
		ADDPREPOSTREGVAR(XHeart)
		ADDPREPOSTREGVAR(Barycentroid)
		ADDPREPOSTREGVAR(BiSplit)
		ADDPREPOSTREGVAR(Crescents)
		ADDPREPOSTREGVAR(Mask)
		ADDPREPOSTREGVAR(Cpow2)
		ADDPREPOSTREGVAR(Curl3D)
		ADDPREPOSTREGVAR(Disc3D)
		ADDPREPOSTREGVAR(Funnel)
		ADDPREPOSTREGVAR(Linear3D)
		ADDPREPOSTREGVAR(PowBlock)
		ADDPREPOSTREGVAR(Squirrel)
		ADDPREPOSTREGVAR(Ennepers)
		ADDPREPOSTREGVAR(SphericalN)
		ADDPREPOSTREGVAR(Kaleidoscope)
		ADDPREPOSTREGVAR(GlynnSim1)
		ADDPREPOSTREGVAR(GlynnSim2)
		ADDPREPOSTREGVAR(GlynnSim3)
		ADDPREPOSTREGVAR(Starblur)
		ADDPREPOSTREGVAR(Sineblur)
		ADDPREPOSTREGVAR(Circleblur)
		ADDPREPOSTREGVAR(CropN)
		ADDPREPOSTREGVAR(ShredRad)
		ADDPREPOSTREGVAR(Blob2)
		ADDPREPOSTREGVAR(Julia3D)
		ADDPREPOSTREGVAR(Julia3Dz)
		ADDPREPOSTREGVAR(LinearT)
		ADDPREPOSTREGVAR(LinearT3D)
		ADDPREPOSTREGVAR(Ovoid)
		ADDPREPOSTREGVAR(Ovoid3D)
		ADDPREPOSTREGVAR(Spirograph)
		ADDPREPOSTREGVAR(Petal)
		ADDPREPOSTREGVAR(RoundSpher)
		ADDPREPOSTREGVAR(RoundSpher3D)
		ADDPREPOSTREGVAR(SpiralWing)
		ADDPREPOSTREGVAR(Squarize)
		ADDPREPOSTREGVAR(Sschecks)
		ADDPREPOSTREGVAR(PhoenixJulia)
		ADDPREPOSTREGVAR(Mobius)
		ADDPREPOSTREGVAR(MobiusN)
		ADDPREPOSTREGVAR(MobiusStrip)
		ADDPREPOSTREGVAR(Lissajous)
		ADDPREPOSTREGVAR(Svf)
		ADDPREPOSTREGVAR(Target)
		ADDPREPOSTREGVAR(Taurus)
		ADDPREPOSTREGVAR(Collideoscope)
		ADDPREPOSTREGVAR(BMod)
		ADDPREPOSTREGVAR(BSwirl)
		ADDPREPOSTREGVAR(BTransform)
		ADDPREPOSTREGVAR(BCollide)
		ADDPREPOSTREGVAR(Eclipse)
		ADDPREPOSTREGVAR(FlipCircle)
		ADDPREPOSTREGVAR(FlipY)
		ADDPREPOSTREGVAR(ECollide)
		ADDPREPOSTREGVAR(EJulia)
		ADDPREPOSTREGVAR(EMod)
		ADDPREPOSTREGVAR(EMotion)
		ADDPREPOSTREGVAR(EPush)
		ADDPREPOSTREGVAR(ERotate)
		ADDPREPOSTREGVAR(EScale)
		ADDPREPOSTREGVAR(ESwirl)
		ADDPREPOSTREGVAR(LazyTravis)
		ADDPREPOSTREGVAR(Squish)
		ADDPREPOSTREGVAR(Circus)
		ADDPREPOSTREGVAR(Tancos)
		ADDPREPOSTREGVAR(Rippled)
		ADDPREPOSTREGVAR(RotateX)
		ADDPREPOSTREGVAR(RotateY)
		ADDPREPOSTREGVAR(RotateZ)
		ADDPREPOSTREGVAR(Flatten)
		ADDPREPOSTREGVAR(Zblur)
		ADDPREPOSTREGVAR(Blur3D)
		ADDPREPOSTREGVAR(ZScale)
		ADDPREPOSTREGVAR(ZTranslate)
		ADDPREPOSTREGVAR(ZCone)
		ADDPREPOSTREGVAR(MirrorX)
		ADDPREPOSTREGVAR(MirrorY)
		ADDPREPOSTREGVAR(MirrorZ)
		ADDPREPOSTREGVAR(Depth)
		ADDPREPOSTREGVAR(Spherical3D)
		ADDPREPOSTREGVAR(RBlur)
		ADDPREPOSTREGVAR(JuliaNab)
		ADDPREPOSTREGVAR(Sintrange)
		ADDPREPOSTREGVAR(Voron)
		ADDPREPOSTREGVAR(Waffle)
		ADDPREPOSTREGVAR(Square3D)
		ADDPREPOSTREGVAR(SuperShape3D)
		ADDPREPOSTREGVAR(Sphyp3D)
		ADDPREPOSTREGVAR(Circlecrop)
		ADDPREPOSTREGVAR(Julian3Dx)
		ADDPREPOSTREGVAR(Fourth)
		ADDPREPOSTREGVAR(Mobiq)
		ADDPREPOSTREGVAR(Spherivoid)
		ADDPREPOSTREGVAR(Farblur)
		ADDPREPOSTREGVAR(CurlSP)
		ADDPREPOSTREGVAR(Heat)
		ADDPREPOSTREGVAR(Interference2)
		ADDPREPOSTREGVAR(Sinq)
		ADDPREPOSTREGVAR(Sinhq)
		ADDPREPOSTREGVAR(Secq)
		ADDPREPOSTREGVAR(Sechq)
		ADDPREPOSTREGVAR(Tanq)
		ADDPREPOSTREGVAR(Tanhq)
		ADDPREPOSTREGVAR(Cosq)
		ADDPREPOSTREGVAR(Coshq)
		ADDPREPOSTREGVAR(Cotq)
		ADDPREPOSTREGVAR(Cothq)
		ADDPREPOSTREGVAR(Cscq)
		ADDPREPOSTREGVAR(Cschq)
		ADDPREPOSTREGVAR(Estiq)
		ADDPREPOSTREGVAR(Loq)
		ADDPREPOSTREGVAR(Curvature)
		ADDPREPOSTREGVAR(Qode)
		ADDPREPOSTREGVAR(BlurHeart)
		ADDPREPOSTREGVAR(Truchet)
		ADDPREPOSTREGVAR(Gdoffs)
		ADDPREPOSTREGVAR(Octagon)
		ADDPREPOSTREGVAR(Trade)
		ADDPREPOSTREGVAR(Juliac)
		ADDPREPOSTREGVAR(Blade3D)
		ADDPREPOSTREGVAR(Blob3D)
		ADDPREPOSTREGVAR(Blocky)
		ADDPREPOSTREGVAR(Bubble2)
		ADDPREPOSTREGVAR(CircleLinear)
		ADDPREPOSTREGVAR(CircleRand)
		ADDPREPOSTREGVAR(CircleTrans1)
		ADDPREPOSTREGVAR(Cubic3D)
		ADDPREPOSTREGVAR(CubicLattice3D)
		ADDPREPOSTREGVAR(Foci3D)
		ADDPREPOSTREGVAR(Ho)
		ADDPREPOSTREGVAR(Julia3Dq)
		ADDPREPOSTREGVAR(Line)
		ADDPREPOSTREGVAR(Loonie3D)
		ADDPREPOSTREGVAR(Mcarpet)
		ADDPREPOSTREGVAR(Waves23D)
		ADDPREPOSTREGVAR(Pie3D)
		ADDPREPOSTREGVAR(Popcorn23D)
		ADDPREPOSTREGVAR(Sinusoidal3D)
		ADDPREPOSTREGVAR(Scry3D)
		ADDPREPOSTREGVAR(Shredlin)
		ADDPREPOSTREGVAR(SplitBrdr)
		ADDPREPOSTREGVAR(Wdisc)
		ADDPREPOSTREGVAR(Falloff)
		ADDPREPOSTREGVAR(Falloff2)
		ADDPREPOSTREGVAR(Falloff3)
		ADDPREPOSTREGVAR(Xtrb)
		//ADDPREPOSTREGVAR(LinearXZ)
		//ADDPREPOSTREGVAR(LinearYZ)

		//DC are special.
		m_Variations.push_back(new DCBubbleVariation<T>());
		ADDPREPOSTREGVAR(DCCarpet)
		ADDPREPOSTREGVAR(DCCube)
		m_Variations.push_back(new DCCylinderVariation<T>());
		ADDPREPOSTREGVAR(DCGridOut)
		m_Variations.push_back(new DCLinearVariation<T>());
		ADDPREPOSTREGVAR(DCTriangle)
		ADDPREPOSTREGVAR(DCZTransl)

		std::for_each(m_Variations.begin(), m_Variations.end(), [&](Variation<T>* var) { var->Precalc(); });
		std::sort(m_Variations.begin(), m_Variations.end(), [&](const Variation<T>* var1, const Variation<T>* var2) { return var1->VariationId() < var2->VariationId(); });

		m_RegVariations.reserve(m_Variations.size()  / 3);
		m_PreVariations.reserve(m_Variations.size()  / 3);
		m_PostVariations.reserve(m_Variations.size() / 3);

		std::for_each(m_Variations.begin(), m_Variations.end(), [&](Variation<T>* var) { if (var->VarType() == VARTYPE_REG)  m_RegVariations.push_back(var);  });
		std::for_each(m_Variations.begin(), m_Variations.end(), [&](Variation<T>* var) { if (var->VarType() == VARTYPE_PRE)  m_PreVariations.push_back(var);  });
		std::for_each(m_Variations.begin(), m_Variations.end(), [&](Variation<T>* var) { if (var->VarType() == VARTYPE_POST) m_PostVariations.push_back(var); });

		//Keep a list of which variations derive from ParametricVariation.
		//Note that these are not new copies, rather just pointers to the original instances in m_Variations.
		for (unsigned int i = 0; i < m_Variations.size(); i++)
		{
			if (ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(m_Variations[i]))
				m_ParametricVariations.push_back(parVar);
		}
	}

	/// <summary>
	/// Delete each element of the list.
	/// </summary>
	~VariationList()
	{
		ClearVec(m_Variations);//No need to delete parametric because they point to the entries in original vector.
	}

	/// <summary>
	/// Get a pointer to the variation at the specified index.
	/// </summary>
	/// <param name="index">The index in the list to retrieve</param>
	/// <returns>A pointer to the variation at the index if in range, else NULL.</returns>
	Variation<T>* GetVariation(size_t index) { return index < m_Variations.size() ? m_Variations[index] : NULL; }

	/// <summary>
	/// Get a pointer to the variation of a specified type at the specified index.
	/// </summary>
	/// <param name="index">The index in the list to retrieve</param>
	/// <param name="varType">The type of variation to retrieve</param>
	/// <returns>A pointer to the variation of the specified type at the index if in range, else NULL.</returns>
	Variation<T>* GetVariation(size_t index, eVariationType varType)
	{
		if (varType == VARTYPE_REG)
			return index < m_RegVariations.size() ? m_RegVariations[index] : NULL;
		else if (varType == VARTYPE_PRE)
			return index < m_PreVariations.size() ? m_PreVariations[index] : NULL;
		else if (varType == VARTYPE_POST)
			return index < m_PostVariations.size() ? m_PostVariations[index] : NULL;
		else
			return NULL;
	}

	/// <summary>
	/// Gets a pointer to a copy of the variation at the specified index.
	/// Optionally specify a weight to assign the new copy.
	/// </summary>
	/// <param name="index">The index in the list to make a copy of</param>
	/// <param name="weight">The weight to assign the new copy. Default: 1</param>
	/// <returns>A pointer to the variation at the index if in range, else NULL.</returns>
	Variation<T>* GetVariationCopy(size_t index, T weight = 1) { return MakeCopyWithWeight(GetVariation(index), weight); }
	Variation<T>* GetVariationCopy(size_t index, eVariationType varType, T weight = 1) { return MakeCopyWithWeight(GetVariation(index, varType), weight); }

	/// <summary>
	/// Get a pointer to the variation with the specified ID.
	/// </summary>
	/// <param name="id">The ID to search for</param>
	/// <returns>A pointer to the variation if found, else NULL.</returns>
	Variation<T>* GetVariation(eVariationId id)
	{
		for (unsigned int i = 0; i < m_Variations.size() && m_Variations[i] != NULL; i++)
			if (id == m_Variations[i]->VariationId())
				return m_Variations[i];

		return NULL;
	}

	/// <summary>
	/// Gets a pointer to a copy of the variation with the specified ID.
	/// Optionally specify a weight to assign the new copy.
	/// </summary>
	/// <param name="id">The id of the variation in the list to make a copy of</param>
	/// <param name="weight">The weight to assign the new copy. Default: 1</param>
	/// <returns>A pointer to the variation with a matching ID, else NULL.</returns>
	Variation<T>* GetVariationCopy(eVariationId id, T weight = 1) { return MakeCopyWithWeight(GetVariation(id), weight); }

	/// <summary>
	/// Get a pointer to the variation with the specified name.
	/// </summary>
	/// <param name="name">The name to search for</param>
	/// <returns>A pointer to the variation if found, else NULL.</returns>
	Variation<T>* GetVariation(string name)
	{
		for (unsigned int i = 0; i < m_Variations.size() && m_Variations[i] != NULL; i++)
			if (name == m_Variations[i]->Name())
				return m_Variations[i];
		
		return NULL;
	}

	/// <summary>
	/// Gets a pointer to a copy of the variation with the specified name.
	/// Optionally specify a weight to assign the new copy.
	/// </summary>
	/// <param name="name">The name of the variation in the list to make a copy of</param>
	/// <param name="weight">The weight to assign the new copy. Default: 1</param>
	/// <returns>A pointer to the variation with a matching name, else NULL.</returns>
	Variation<T>* GetVariationCopy(string name, T weight = 1) { return MakeCopyWithWeight(GetVariation(name), weight); }

	/// <summary>
	/// Get a parametric variation at the specified index.
	/// Note this is the index in the parametric variations list, not in the master list.
	/// </summary>
	/// <param name="index">The index in the parametric variations list to retrieve</param>
	/// <returns>The parametric variation at the index specified if in range, else NULL.</returns>
	ParametricVariation<T>* GetParametricVariation(size_t index) { return index < m_ParametricVariations.size() ? m_ParametricVariations[index] : NULL; }
	
	/// <summary>
	/// Get a parametric variation with the specified name.
	/// </summary>
	/// <param name="name">The name of the variation in the parametric variations list to retrieve</param>
	/// <returns>The parametric variation with a matching name, else NULL.</returns>
	ParametricVariation<T>* GetParametricVariation(string name)
	{
		for (unsigned int i = 0; i < m_ParametricVariations.size() && m_ParametricVariations[i] != NULL; i++)
			if (name == m_ParametricVariations[i]->Name())
				return m_ParametricVariations[i];
		
		return NULL;
	}

	/// <summary>
	/// Get the index of the variation with the specified name.
	/// </summary>
	/// <param name="name">The name of the variation whose index is returned</param>
	/// <returns>The index of the variation with the matching name, else -1</returns>
	int GetVariationIndex(string name)
	{
		for (unsigned int i = 0; i < m_Variations.size() && m_Variations[i] != NULL; i++)
			if (name == m_Variations[i]->Name())
				return i;
		
		return -1;
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	size_t Size() { return m_Variations.size(); }
	size_t RegSize() { return m_RegVariations.size(); }
	size_t PreSize() { return m_PreVariations.size(); }
	size_t PostSize() { return m_PostVariations.size(); }
	size_t ParametricSize() { return m_ParametricVariations.size(); }

private:
	/// <summary>
	/// Make a dyncamically allocated copy of a variation and assign it a specified weight.
	/// Return a pointer to the new copy.
	/// </summary>
	/// <param name="var">The variation to copy</param>
	/// <param name="weight">The weight to assign it</param>
	/// <returns>A pointer to the new variation copy if success, else NULL.</returns>
	Variation<T>* MakeCopyWithWeight(Variation<T>* var, T weight)
	{
		if (var)
		{
			Variation<T>* var2 = var->Copy();

			var2->m_Weight = weight;
			return var2;
		}

		return NULL;
	}

	/// <summary>
	/// Assignment operator which does nothing since these are non-copyable.
	/// Do not provide a copy constructor and ensure the assignment operator does nothing.
	/// </summary>
	/// <param name="varList">The VariationList object which won't be copied</param>
	/// <returns>Reference to unchanged self</returns>
	VariationList<T>& operator = (const VariationList<T>& varList)
	{
		return *this;
	}

	vector<Variation<T>*> m_Variations;//A list of pointers to dynamically allocated variation objects.
	vector<Variation<T>*> m_RegVariations;
	vector<Variation<T>*> m_PreVariations;
	vector<Variation<T>*> m_PostVariations;
	vector<ParametricVariation<T>*> m_ParametricVariations;//A list of pointers to elements in m_Variations which are derived from ParametricVariation.
};
}