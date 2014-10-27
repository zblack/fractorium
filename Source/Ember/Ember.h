#pragma once

#include "Xform.h"
#include "PaletteList.h"
#include "SpatialFilter.h"
#include "TemporalFilter.h"

/// <summary>
/// Ember class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Bit position specifying the presence of each type of 3D parameter.
/// One, none, some or all of these can be present.
/// </summary>
enum eProjBits
{
	PROJBITS_ZPOS = 1,
	PROJBITS_PERSP = 2,
	PROJBITS_PITCH = 4,
	PROJBITS_YAW = 8,
	PROJBITS_BLUR = 16,
};

/// <summary>
/// Ember is the main class for holding all of the information required to render a fractal flame.
/// This includes a vector of xforms, a final xform, size and color information as well as an Xml edit
/// document that users can use to keep track of changes.
/// Operations will often desire operating on just the regular xforms, or the regular xforms plus the final one.
/// The word "total" is used to signify when the final xform is included in the operation.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Ember
{
public:
	/// <summary>
	/// Default constructor which calls Init() to set default values.
	/// </summary>
	Ember()
	{
		Init();
	}

	Ember(const Ember<T>& ember)
		: m_Edits(nullptr)
	{
		Ember<T>::operator=<T>(ember);
	}

	/// <summary>
	/// Copy constructor to copy an Ember object of type U, where T is usually the same type as U.
	/// </summary>
	/// <param name="ember">The Ember object to copy</param>
	template <typename U>
	Ember(const Ember<U>& ember)
		: m_Edits(nullptr)
	{
		Ember<T>::operator=<U>(ember);
	}

	/// <summary>
	/// Destructor which clears the Xml edits.
	/// </summary>
	~Ember()
	{
		ClearEdit();
	}

	//For some reason the compiler requires Xform to define two assignment operators,
	//however it gets confused when Ember has two.
	Ember<T>& operator = (const Ember<T>& ember)
	{
		if (this != &ember)
			Ember<T>::operator=<T>(ember);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign an Ember object of type U, where T is usually the same type as U.
	/// </summary>
	/// <param name="ember">The Ember object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Ember<T>& operator = (const Ember<U>& ember)
	{
		m_FinalRasW			  = ember.m_FinalRasW;
		m_FinalRasH			  = ember.m_FinalRasH;
		m_OrigFinalRasW		  = ember.m_OrigFinalRasW;
		m_OrigFinalRasH		  = ember.m_OrigFinalRasH;
		m_OrigPixPerUnit	  = ember.m_OrigPixPerUnit;
		m_Supersample		  = ember.m_Supersample;
		m_Passes			  = ember.m_Passes;
		m_TemporalSamples	  = ember.m_TemporalSamples;
		m_Symmetry			  = ember.m_Symmetry;

		m_Quality			  = T(ember.m_Quality);
		m_PixelsPerUnit		  = T(ember.m_PixelsPerUnit);
		m_Zoom				  = T(ember.m_Zoom);
		m_CamZPos			  = T(ember.m_CamZPos);
		m_CamPerspective	  = T(ember.m_CamPerspective);
		m_CamYaw			  = T(ember.m_CamYaw);
		m_CamPitch			  = T(ember.m_CamPitch);
		m_CamDepthBlur		  = T(ember.m_CamDepthBlur);
		m_CamMat			  = ember.m_CamMat;
		m_CenterX			  = T(ember.m_CenterX);
		m_CenterY			  = T(ember.m_CenterY);
		m_RotCenterY		  = T(ember.m_RotCenterY);
		m_Rotate			  = T(ember.m_Rotate);
		m_Hue				  = T(ember.m_Hue);
		m_Brightness		  = T(ember.m_Brightness);
		m_Gamma				  = T(ember.m_Gamma);
		m_Vibrancy			  = T(ember.m_Vibrancy);
		m_GammaThresh		  = T(ember.m_GammaThresh);
		m_HighlightPower	  = T(ember.m_HighlightPower);
		m_Time				  = T(ember.m_Time);
		m_Background		  = ember.m_Background;
		m_Interp			  = ember.m_Interp;
		m_AffineInterp		  = ember.m_AffineInterp;

		m_MinRadDE			  = T(ember.m_MinRadDE);
		m_MaxRadDE			  = T(ember.m_MaxRadDE);
		m_CurveDE			  = T(ember.m_CurveDE);

		m_SpatialFilterType	  = ember.m_SpatialFilterType;
		m_SpatialFilterRadius = T(ember.m_SpatialFilterRadius);

		m_TemporalFilterType  = ember.m_TemporalFilterType;
		m_TemporalFilterExp	  = T(ember.m_TemporalFilterExp);
		m_TemporalFilterWidth = T(ember.m_TemporalFilterWidth);

		m_PaletteMode		  = ember.m_PaletteMode;
		m_PaletteInterp		  = ember.m_PaletteInterp;

		m_Name				  = ember.m_Name;
		m_ParentFilename	  = ember.m_ParentFilename;

		m_Index		  = ember.m_Index;
		m_ScaleType	  = ember.ScaleType();
		m_Palette	  = ember.m_Palette;

		m_Xforms.clear();

		for (size_t i = 0; i < ember.XformCount(); i++)
		{
			if (Xform<U>* p = ember.GetXform(i))
			{
				Xform<T> xform = *p;//Will call assignment operator to convert between types T and U.

				AddXform(xform);
			}
		}

		Xform<T> finalXform = *ember.FinalXform();//Will call assignment operator to convert between types T and U.

		SetFinalXform(finalXform);

		//Interpolated-against final xforms need animate & color speed set to 0.
		if (!ember.UseFinalXform())
		{
			m_FinalXform.m_Motion.clear();
			m_FinalXform.m_Animate = 0;
			m_FinalXform.m_ColorSpeed = 0;
		}

		SetProjFunc();
		ClearEdit();

		if (ember.m_Edits != nullptr)
			m_Edits = xmlCopyDoc(ember.m_Edits, 1);

		return *this;
	}

	/// <summary>
	/// Set common default values.
	/// </summary>
	void Init()
	{
		m_FinalRasW = 1920;
		m_FinalRasH = 1080;
		m_OrigFinalRasW = 1920;
		m_OrigFinalRasH = 1080;
		m_OrigPixPerUnit = 240;
		m_Supersample = 1;
		m_Passes = 1;
		m_TemporalSamples = 1000;
		m_Symmetry = 0;
		m_Quality = 100;
		m_PixelsPerUnit = 240;
		m_Zoom = 0;
		m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
		m_CamZPos = 0;
		m_CamPerspective = 0;
		m_CamYaw = 0;
		m_CamPitch = 0;
		m_CamDepthBlur = 0;
		m_BlurCoef = 0;
		m_CamMat = m3T(0);
		m_CenterX = 0;
		m_CenterY = 0;
		m_RotCenterY = 0;
		m_Rotate = 0;
		m_Hue = 0;
		m_Brightness = 4;
		m_Gamma = 4;
		m_Vibrancy = 1;
		m_GammaThresh = T(0.01);
		m_HighlightPower = -1;
		m_Time = 0;
		m_Background.Reset();
		m_Interp = EMBER_INTERP_LINEAR;
		m_AffineInterp = INTERP_LOG;

		//DE filter.
		m_MinRadDE = 0;
		m_MaxRadDE = 9;
		m_CurveDE = T(0.4);

		//Spatial filter.
		m_SpatialFilterType = GAUSSIAN_SPATIAL_FILTER;
		m_SpatialFilterRadius = T(0.5);

		//Temporal filter.
		m_TemporalFilterType = BOX_TEMPORAL_FILTER;
		m_TemporalFilterExp = 0;
		m_TemporalFilterWidth = 1;

		//Palette.
		m_PaletteMode = PALETTE_STEP;
		m_PaletteInterp = INTERP_HSV;

		m_Name = "No name";
		m_ParentFilename = "No parent";

		//Internal values.
		m_Index = 0;
		m_ScaleType = eScaleType::SCALE_NONE;
		m_Xforms.reserve(12);

		m_Edits = nullptr;
	}

	/// <summary>
	/// Add a copy of a new xform to the xforms vector.
	/// </summary>
	/// <param name="xform">The xform to copy and add</param>
	void AddXform(const Xform<T>& xform)
	{
		m_Xforms.push_back(xform);
		m_Xforms[m_Xforms.size() - 1].CacheColorVals();
		m_Xforms[m_Xforms.size() - 1].ParentEmber(this);
	}

	/// <summary>
	/// Add the specified number of empty xforms.
	/// </summary>
	/// <param name="count">The number of xforms to add</param>
	void AddXforms(size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			Xform<T> xform;

			AddXform(xform);
		}
	}

	/// <summary>
	/// Add empty padding xforms until the total xform count is xformPad.
	/// </summary>
	/// <param name="xformPad">The total number of xforms to finish with</param>
	void PadXforms(size_t xformPad)
	{
		if (xformPad > XformCount())
			AddXforms(xformPad - XformCount());
	}

	/// <summary>
	/// Copy this ember with optional padding xforms added.
	/// </summary>
	/// <param name="xformPad">The total number of xforms if additional padding xforms are desired. Default: 0.</param>
	/// <param name="doFinal">Whether to copy the final xform. Default: false.</param>
	/// <returns>The newly constructed ember</returns>
	Ember<T> Copy(size_t xformPad = 0, bool doFinal = false)
	{
		Ember<T> ember(*this);

		ember.PadXforms(xformPad);

		if (doFinal)
		{
			if (UseFinalXform())//Caller wanted one and this ember has one.
			{
				ember.m_FinalXform = m_FinalXform;
			}
			else//Caller wanted one and this ember doesn't have one.
			{
				//Interpolated-against final xforms need animate & color speed set to 0 and motion elements cleared.
				ember.m_FinalXform.m_Animate = 0;
				ember.m_FinalXform.m_ColorSpeed = 0;
				ember.m_FinalXform.m_Motion.clear();
				ember.m_FinalXform.ClearAndDeleteVariations();
				ember.m_FinalXform.AddVariation(new LinearVariation<T>(0));//Do this so it doesn't appear empty.
			}
		}

		return ember;
	}

	/// <summary>
	/// Delete an xform at the specified index.
	/// Shuffle xaos if present to reflect the deleted xform.
	/// </summary>
	/// <param name="i">The index to delete</param>
	/// <returns>True if success, else false.</returns>
	bool DeleteXform(size_t i)
	{
		Xform<T>* xform;

		if (i < XformCount())
		{
			m_Xforms.erase(m_Xforms.begin() + i);

			//Now shuffle xaos values from i on back by 1 for every xform.
			for (size_t x1 = 0; x1 < XformCount(); x1++)
			{
				if ((xform = GetXform(x1)))
				{
					for (size_t x2 = i + 1; x2 <= XformCount(); x2++)//Iterate from the position after the deletion index up to the old count.
						xform->SetXaos(x2 - 1, xform->Xaos(x2));

					xform->TruncateXaos();//Make sure no old values are hanging around in case more xforms are added to this ember later.
				}
			}

			return true;
		}

		return false;
	}

	/// <summary>
	/// Delete the xform at the specified index, including the final one.
	/// </summary>
	/// <param name="i">The index to delete</param>
	/// <returns>True if success, else false.</returns>
	bool DeleteTotalXform(size_t i)
	{
		if (DeleteXform(i))
			{ }
		else if (i == XformCount() && UseFinalXform())
			m_FinalXform.Clear();
		else
			return false;

		return true;
	}

	/// <summary>
	/// Get a pointer to the xform at the specified index, excluding the final one.
	/// </summary>
	/// <param name="i">The index to get</param>
	/// <returns>A pointer to the xform at the index if successful, else nullptr.</returns>
	Xform<T>* GetXform(size_t i) const
	{
		if (i < XformCount())
			return (Xform<T>*)&m_Xforms[i];
		else
			return nullptr;
	}

	/// <summary>
	/// Get a pointer to the xform at the specified index, including the final one.
	/// </summary>
	/// <param name="i">The index to get</param>
	/// <param name="forceFinal">If true, return the final xform when its index is requested even if one is not present</param>
	/// <returns>A pointer to the xform at the index if successful, else nullptr.</returns>
	Xform<T>* GetTotalXform(size_t i, bool forceFinal = false) const
	{
		if (i < XformCount())
			return (Xform<T>*)&m_Xforms[i];
		else if (i == XformCount() || forceFinal)
			return (Xform<T>*)&m_FinalXform;
		else
			return nullptr;
	}

	/// <summary>
	/// Search the xforms, excluding final, to find which one's address matches the address of the specified xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to find</param>
	/// <returns>The index of the matched xform if found, else -1.</returns>
	intmax_t GetXformIndex(Xform<T>* xform) const
	{
		intmax_t index = -1;

		for (size_t i = 0; i < m_Xforms.size(); i++)
			if (GetXform(i) == xform)
				return (intmax_t)i;

		return index;
	}

	/// <summary>
	/// Search the xforms, including final, to find which one's address matches the address of the specified xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to find</param>
	/// <returns>The index of the matched xform if found, else -1.</returns>
	intmax_t GetTotalXformIndex(Xform<T>* xform) const
	{
		size_t totalXformCount = TotalXformCount();

		for (size_t i = 0; i < totalXformCount; i++)
			if (GetTotalXform(i) == xform)
				return (intmax_t)i;

		return -1;
	}

	/// <summary>
	/// Assign the final xform.
	/// </summary>
	/// <param name="xform">The xform to copy and assign to the final xform</param>
	void SetFinalXform(const Xform<T>& xform)
	{
		m_FinalXform = xform;
		m_FinalXform.CacheColorVals();
		m_FinalXform.ParentEmber(this);
	}

	/// <summary>
	/// Delete the final xform.
	/// </summary>
	void DeleteFinalXform()
	{
		m_FinalXform.ClearAndDeleteVariations();
	}

	/// <summary>
	/// Determine whether the specified xform has the same address as the final xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to test</param>
	/// <returns>True if matched, else false.</returns>
	bool IsFinalXform(Xform<T>* xform)
	{
		return &m_FinalXform == xform;
	}

	/// <summary>
	/// Delete all motion elements from all xforms including final.
	/// </summary>
	void DeleteMotionElements()
	{
		for (size_t i = 0; i < TotalXformCount(); i++)
			GetTotalXform(i)->DeleteMotionElements();
	}

	/// <summary>
	/// Call CacheColorVals() and SetPrecalcFlags() on all xforms including final.
	/// </summary>
	void CacheXforms()
	{
		for (size_t i = 0; i < TotalXformCount(); i++)
		{
			Xform<T>* xform = GetTotalXform(i);

			xform->CacheColorVals();
			xform->SetPrecalcFlags();
		}
	}

	/// <summary>
	/// Set the projection function pointer based on the
	/// values of the 3D fields.
	/// </summary>
	void SetProjFunc()
	{
		size_t projBits = ProjBits();

		if (!projBits)//No 3D at all, then do nothing.
		{
			m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
		}
		else
		{
			m_CamMat[0][0] =  cos(-m_CamYaw);
			m_CamMat[1][0] = -sin(-m_CamYaw);
			m_CamMat[2][0] = 0;
			m_CamMat[0][1] =  cos(m_CamPitch) * sin(-m_CamYaw);
			m_CamMat[1][1] =  cos(m_CamPitch) * cos(-m_CamYaw);
			m_CamMat[2][1] = -sin(m_CamPitch);
			m_CamMat[0][2] =  sin(m_CamPitch) * sin(-m_CamYaw);
			m_CamMat[1][2] =  sin(m_CamPitch) * cos(-m_CamYaw);
			m_CamMat[2][2] =  cos(m_CamPitch);

			if (projBits & PROJBITS_BLUR)
			{
				if (projBits & PROJBITS_YAW)
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchYawDepthBlur;
				else
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchDepthBlur;
			}
			else if ((projBits & PROJBITS_PITCH) || (projBits & PROJBITS_YAW))
			{
				if (projBits & PROJBITS_YAW)
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchYaw;
				else
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitch;
			}
			else
			{
				m_ProjFunc = &EmberNs::Ember<T>::ProjectZPerspective;
			}
		}

		m_BlurCoef = T(0.1) * m_CamDepthBlur;
	}

	/// <summary>
	/// Determine whether xaos is used in any xform, excluding final.
	/// </summary>
	/// <returns>True if any xaos found, else false.</returns>
	bool XaosPresent() const
	{
		bool b = false;

		ForEach(m_Xforms, [&](const Xform<T>& xform) { b |= xform.XaosPresent(); });//If at least one entry is not equal to 1, then xaos is present.

		return b;
	}

	/// <summary>
	/// Remove all xaos from this ember.
	/// </summary>
	void ClearXaos()
	{
		ForEach(m_Xforms, [&](Xform<T>& xform) { xform.ClearXaos(); });
	}

	/// <summary>
	/// Change the size of the final output image and adjust the pixels per unit
	/// so that the orientation of the image remains the same in the new size.
	/// </summary>
	/// <param name="width">New width</param>
	/// <param name="height">New height</param>
	/// <param name="onlyScaleIfNewIsSmaller">True to only scale if the new dimensions are smaller than the original, else always scale.</param>
	void SetSizeAndAdjustScale(size_t width, size_t height, bool onlyScaleIfNewIsSmaller, eScaleType scaleType)
	{
		if ((onlyScaleIfNewIsSmaller && (width < m_OrigFinalRasW || height < m_OrigFinalRasH)) || !onlyScaleIfNewIsSmaller)
		{
			if (scaleType == SCALE_WIDTH)
				m_PixelsPerUnit = m_OrigPixPerUnit * (T(width) / T(m_OrigFinalRasW));
			else if (scaleType == SCALE_HEIGHT)
				m_PixelsPerUnit = m_OrigPixPerUnit * (T(height) / T(m_OrigFinalRasH));
		}

		m_ScaleType = scaleType;
		m_FinalRasW = width;
		m_FinalRasH = height;
	}

	/// <summary>
	/// Set the original final output image dimensions to be equal to the current ones.
	/// </summary>
	void SyncSize()
	{
		m_OrigFinalRasW = m_FinalRasW;
		m_OrigFinalRasH = m_FinalRasH;
		m_OrigPixPerUnit = m_PixelsPerUnit;
	}

	/// <summary>
	/// Set the current final output image dimensions to be equal to the original ones.
	/// </summary>
	void RestoreSize()
	{
		m_FinalRasW = m_OrigFinalRasW;
		m_FinalRasH = m_OrigFinalRasH;
		m_PixelsPerUnit = m_OrigPixPerUnit;
	}

	/// <summary>
	/// Set all xform weights to 1 / xform count.
	/// </summary>
	void EqualizeWeights()
	{
		T weight = T(1) / m_Xforms.size();

		ForEach(m_Xforms, [&](Xform<T>& xform) { xform.m_Weight = weight; });
	}

	/// <summary>
	/// Calculates the normalized weights of the xforms and places them in the passed in vector.
	/// <param name="normalizedWeights">A vector to hold the normalized weights</param>
	/// </summary>
	void CalcNormalizedWeights(vector<T>& normalizedWeights)
	{
		T norm = 0;
		size_t i = 0;

		if (normalizedWeights.size() != m_Xforms.size())
			normalizedWeights.resize(m_Xforms.size());

		ForEach(m_Xforms, [&](Xform<T>& xform) { norm += xform.m_Weight; });
		ForEach(normalizedWeights, [&](T& weight) { weight = m_Xforms[i].m_Weight / norm; i++; });
	}

	/// <summary>
	/// Get a vector of pointers to all variations present in all Xforms, with no duplicates.
	/// Meaning, that if a variation is present in more than one Xform, only the first one encountered
	/// will be added.
	/// <param name="variations">A vector to hold pointers to the variations present. This will be cleared first.</param>
	/// </summary>
	void GetPresentVariations(vector<Variation<T>*>& variations, bool baseOnly = true) const
	{
		size_t i = 0, xformIndex = 0, totalVarCount = m_FinalXform.TotalVariationCount();

		variations.clear();
		ForEach(m_Xforms, [&](const Xform<T>& xform) { totalVarCount += xform.TotalVariationCount(); });
		variations.reserve(totalVarCount);

		while (Xform<T>* xform = GetTotalXform(xformIndex++))
		{
			i = 0;
			totalVarCount = xform->TotalVariationCount();

			while (Variation<T>* tempVar = xform->GetVariation(i++))
			{
				if (baseOnly)
				{
					string tempVarBaseName = tempVar->BaseName();

					if (!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVar->VariationId() == var->VariationId(); }) &&
						!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVarBaseName == var->BaseName(); }))
						variations.push_back(tempVar);
				}
				else
				{
					if (!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVar->VariationId() == var->VariationId(); }))
						variations.push_back(tempVar);
				}
			}
		}
	}

	/// <summary>
	/// Flatten all xforms by adding a flatten variation if none is present, and if any of the
	/// variations or parameters in the vector are present.
	/// </summary>
	/// <param name="names">Vector of variation and parameter names</param>
	/// <returns>True if flatten was added to any of the xforms, false if it already was present or if none of the specified variations or parameters were present.</returns>
	bool Flatten(vector<string>& names)
	{
		bool flattened = false;

		ForEach(m_Xforms, [&](Xform<T>& xform) { flattened |= xform.Flatten(names); });

		return flattened;
	}

	/// <summary>
	/// Flatten all xforms by adding a flatten variation in each if not already present.
	/// </summary>
	/// <returns>True if flatten was removed, false if it wasn't present.</returns>
	bool Unflatten()
	{
		bool unflattened = false;

		ForEach(m_Xforms, [&](Xform<T>& xform)
		{
			unflattened |= xform.DeleteVariationById(VAR_PRE_FLATTEN);
			unflattened |= xform.DeleteVariationById(VAR_FLATTEN);
			unflattened |= xform.DeleteVariationById(VAR_POST_FLATTEN);
		});

		return unflattened;
	}

	/// <summary>
	/// Thin wrapper around Interpolate() which takes a vector of embers rather than a pointer and size.
	/// All embers are expected to be aligned, including the final xform. If not the behavior is undefined.
	/// </summary>
	/// <param name="embers">Vector of embers</param>
	/// <param name="coefs">Coefficients vector which must be the same length as the vector of embers</param>
	/// <param name="stagger">Stagger if greater than 0</param>
	void Interpolate(vector<Ember<T>>& embers, vector<T>& coefs, T stagger)
	{
		Interpolate(embers.data(), embers.size(), coefs, stagger);
	}

	/// <summary>
	/// Interpolate the specified embers using the coefficients supplied.
	/// All embers are expected to be aligned, including the final xform. If not the behavior is undefined.
	/// </summary>
	/// <param name="embers">Pointer to buffer of embers</param>
	/// <param name="size">Number of elements in the buffer of embers</param>
	/// <param name="coefs">Coefficients vector which must be the same length as the vector of embers</param>
	/// <param name="stagger">Stagger if greater than 0</param>
	void Interpolate(Ember<T>* embers, size_t size, vector<T>& coefs, T stagger)
	{
		if (size != coefs.size() || size < 2)
			return;

		bool allID, final;
		size_t l, maxXformCount, totalXformCount;
		T bgAlphaSave = m_Background.a;
		T coefSave[2] {0, 0};
		vector<Xform<T>*> xformVec;

		//Palette and others
		if (embers[0].m_PaletteInterp == INTERP_HSV)
		{
			for (glm::length_t i = 0; i < 256; i++)
			{
				T t[3], s[4] = { 0, 0, 0, 0 };

				for (glm::length_t k = 0; k < size; k++)
				{
					Palette<T>::RgbToHsv(glm::value_ptr(embers[k].m_Palette[i]), t);

					for (size_t j = 0; j < 3; j++)
						s[j] += coefs[k] * t[j];

					s[3] += coefs[k] * embers[k].m_Palette[i][3];
				}

				Palette<T>::HsvToRgb(s, glm::value_ptr(m_Palette[i]));
				m_Palette[i][3] = s[3];

				for (glm::length_t j = 0; j < 4; j++)
					Clamp<T>(m_Palette[i][j], 0, 1);
			}
		}
		else if (embers[0].m_PaletteInterp == INTERP_SWEEP)
		{
			//Sweep - not the best option for float indices.
			for (glm::length_t i = 0; i < 256; i++)
			{
				size_t j = (i < (256 * coefs[0])) ? 0 : 1;
				m_Palette[i] = embers[j].m_Palette[i];
			}
		}

		m_Palette.m_Index = -1;//Random.
		m_Symmetry = 0;
		m_SpatialFilterType = embers[0].m_SpatialFilterType;
		m_TemporalFilterType = embers[0].m_TemporalFilterType;
		m_PaletteMode = embers[0].m_PaletteMode;
		m_AffineInterp = embers[0].m_AffineInterp;

		//Interpolate ember parameters.
		InterpT<&Ember<T>::m_Brightness>(embers, coefs, size);
		InterpT<&Ember<T>::m_HighlightPower>(embers, coefs, size);
		InterpT<&Ember<T>::m_Gamma>(embers, coefs, size);
		InterpT<&Ember<T>::m_Vibrancy>(embers, coefs, size);
		InterpT<&Ember<T>::m_Hue>(embers, coefs, size);
		InterpI<&Ember<T>::m_FinalRasW>(embers, coefs, size);
		InterpI<&Ember<T>::m_FinalRasH>(embers, coefs, size);
		InterpI<&Ember<T>::m_Supersample>(embers, coefs, size);
		InterpT<&Ember<T>::m_CenterX>(embers, coefs, size);
		InterpT<&Ember<T>::m_CenterY>(embers, coefs, size);
		InterpT<&Ember<T>::m_RotCenterY>(embers, coefs, size);
		InterpX<Color<T>, &Ember<T>::m_Background>(embers, coefs, size); m_Background.a = bgAlphaSave;//Don't interp alpha.
		InterpT<&Ember<T>::m_PixelsPerUnit>(embers, coefs, size);
		InterpT<&Ember<T>::m_SpatialFilterRadius>(embers, coefs, size);
		InterpT<&Ember<T>::m_TemporalFilterExp>(embers, coefs, size);
		InterpT<&Ember<T>::m_TemporalFilterWidth>(embers, coefs, size);
		InterpT<&Ember<T>::m_Quality>(embers, coefs, size);
		InterpT<&Ember<T>::m_Zoom>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamZPos>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamPerspective>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamYaw>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamPitch>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamDepthBlur>(embers, coefs, size);
		InterpX<m3T, &Ember<T>::m_CamMat>(embers, coefs, size);
		InterpT<&Ember<T>::m_Rotate>(embers, coefs, size);
		InterpI<&Ember<T>::m_Passes>(embers, coefs, size);
		InterpI<&Ember<T>::m_TemporalSamples>(embers, coefs, size);
		InterpT<&Ember<T>::m_MaxRadDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_MinRadDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_CurveDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_GammaThresh>(embers, coefs, size);

		//An extra step needed here due to the OOD that was not needed in the original.
		//A small price to pay for the conveniences it affords us elsewhere.
		//First clear the xforms, and find out the max number of xforms in all of the embers in the list.
		m_Xforms.clear();
		maxXformCount = Interpolater<T>::MaxXformCount(embers, size);//Max number of standard transforms in embers, excluding final.
		//m_Xforms.reserve(maxXformCount);
		final = Interpolater<T>::AnyFinalPresent(embers, size);//Did any embers have a final xform?
		totalXformCount = maxXformCount + (final ? 1 : 0);
		xformVec.reserve(size);

		//Populate the xform list member such that each element is a merge of all of the xforms at that position in all of the embers.
		for (size_t i = 0; i < totalXformCount; i++)//For each xform to populate.
		{
			for (size_t j = 0; j < size; j++)//For each ember in the list.
			{
				if (i < embers[j].TotalXformCount())//Xform in this position in this ember.
				{
					xformVec.push_back(embers[j].GetTotalXform(i));//Temporary list to pass to MergeXforms().
				}
			}

			if (i < maxXformCount)//Working with standard xforms?
				AddXform(Interpolater<T>::MergeXforms(xformVec, true));//Merge, set weights to zero, and add to the xform list.
			else if (final)//Or is it the final xform (i will be == maxXformCount)?
				m_FinalXform = Interpolater<T>::MergeXforms(xformVec, true);

			xformVec.clear();
		}

		//Now have a merged list, so interpolate the weight values.
		//This includes all xforms plus final.
		for (size_t i = 0; i < totalXformCount; i++)
		{
			Xform<T>* thisXform = GetTotalXform(i);

			if (size == 2 && stagger > 0 && thisXform != &m_FinalXform)
			{
				coefSave[0] = coefs[0];
				coefSave[1] = coefs[1];
				coefs[0] = Interpolater<T>::GetStaggerCoef(coefSave[0], stagger, XformCount(), i);//Standard xform count without final.
				coefs[1] = 1 - coefs[0];
			}

			for (size_t j = 0; j < thisXform->TotalVariationCount(); j++)//For each variation in this xform.
			{
				Variation<T>* var = thisXform->GetVariation(j);
				ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);//Will use below if it's parametric.

				var->m_Weight = 0;

				if (parVar != nullptr)
					parVar->Clear();

				for (size_t k = 0; k < size; k++)//For each ember in the list.
				{
					Xform<T>* tempXform = embers[k].GetTotalXform(i);//Xform in this position in this ember, including final.

					if (tempXform)
					{
						Variation<T>* tempVar = tempXform->GetVariationById(var->VariationId());//See if the variation at this xform index exists in that ember at this xform index.

						if (tempVar != nullptr)
						{
							//Interp weight.
							var->m_Weight += tempVar->m_Weight * coefs[k];

							//If it was a parametric variation, interp params.
							if (parVar != nullptr)
							{
								ParametricVariation<T>* tempParVar = dynamic_cast<ParametricVariation<T>*>(tempVar);

								if (tempParVar != nullptr && (parVar->ParamCount() == tempParVar->ParamCount()))//This check will should always be true, but just check to be absolutely sure to avoid clobbering memory.
								{
									ParamWithName<T>* params = parVar->Params();
									ParamWithName<T>* tempParams = tempParVar->Params();

									for (l = 0; l < parVar->ParamCount(); l++)
									{
										if (!tempParams[l].IsPrecalc())
											*(params[l].Param()) += tempParams[l].ParamVal() * coefs[k];
									}
								}
							}
						}
					}
				}
			}

			InterpXform<&Xform<T>::m_Weight>	(thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_ColorX>	(thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_ColorSpeed>(thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_Opacity>	(thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_Animate>	(thisXform, i, embers, coefs, size);

			ClampGte0Ref(thisXform->m_Weight);
			ClampRef<T>(thisXform->m_ColorX, 0, 1);
			ClampRef<T>(thisXform->m_ColorSpeed, -1, 1);

			//Interp affine and post.
			if (m_AffineInterp == INTERP_LOG)
			{
				vector<v2T> cxMag(size);
				vector<v2T> cxAng(size);
				vector<v2T> cxTrn(size);

				thisXform->m_Affine.m_Mat = m23T(0);

				//Affine part.
				Interpolater<T>::ConvertLinearToPolar(embers, size, i, 0, cxAng, cxMag, cxTrn);
				Interpolater<T>::InterpAndConvertBack(coefs, cxAng, cxMag, cxTrn, thisXform->m_Affine);

				//Post part.
				allID = true;

				for (size_t k = 0; k < size; k++)//For each ember in the list.
				{
					if (i < embers[k].TotalXformCount())//Xform in this position in this ember.
					{
						Xform<T>* tempXform = embers[k].GetTotalXform(i);
						allID &= tempXform->m_Post.IsID();
					}
				}

				thisXform->m_Post.m_Mat = m23T(0);

				if (allID)
				{
					thisXform->m_Post.A(1);
					thisXform->m_Post.E(1);
				}
				else
				{
					Interpolater<T>::ConvertLinearToPolar(embers, size, i, 1, cxAng, cxMag, cxTrn);
					Interpolater<T>::InterpAndConvertBack(coefs, cxAng, cxMag, cxTrn, thisXform->m_Post);
				}
			}
			else if (m_AffineInterp == INTERP_LINEAR)
			{
				//Interpolate pre and post affine using coefs.
				allID = true;
				thisXform->m_Affine.m_Mat = m23T(0);
				thisXform->m_Post.m_Mat	  = m23T(0);

				for (size_t k = 0; k < size; k++)
				{
					Xform<T>* tempXform = embers[k].GetTotalXform(i);//Xform in this position in this ember.

					if (tempXform)
					{
						allID &= tempXform->m_Post.IsID();
						thisXform->m_Affine.m_Mat += (coefs[k] * tempXform->m_Affine.m_Mat);
						thisXform->m_Post.m_Mat	  += (coefs[k] * tempXform->m_Post.m_Mat);
					}
				}

				if (allID)
					thisXform->m_Post.m_Mat = glm::mat2x3(1);
			}

			//Final stagger cleanup goes here.
			if (size == 2 && stagger > 0 && thisXform != &m_FinalXform)
			{
				coefs[0] = coefSave[0];
				coefs[1] = coefSave[1];
			}
		}

		//Normally these functions are called automatically when
		//adding variations to a xform, or when setting a parametric
		//variation value via name lookup. However, since the values
		//were directly written to, must manually call them here.
		CacheXforms();

		//Need to merge chaos. Original does chaos all the time, but really only need to do it if at least one xform in at least one ember uses it, else skip.
		//Omit final xform from chaos processing.
		if (Interpolater<T>::AnyXaosPresent(embers, size))
		{
			for (size_t i = 0; i < XformCount(); i++)
			{
				m_Xforms[i].SetXaos(i, 0);//First make each xform xaos array be maxXformCount elements long and set them to zero.

				//Now fill them with interpolated values.
				for (size_t j = 0; j < size; j++)//For each ember in the list.
				{
					Xform<T>* tempXform = embers[j].GetXform(i);

					for (size_t k = 0; k < XformCount(); k++)//For each xaos entry in this xform's xaos array, sum it with the same entry in all of the embers multiplied by the coef for that ember.
					{
						m_Xforms[i].SetXaos(k, m_Xforms[i].Xaos(k) + tempXform->Xaos(k) * coefs[j]);
					}
				}

				//Make sure no xaos entries for this xform were less than zero.
				for (size_t k = 0; k < XformCount(); k++)
					if (m_Xforms[i].Xaos(k) < 0)
						m_Xforms[i].SetXaos(k, 0);
			}
		}
	}

	/// <summary>
	/// Thin wrapper around InterpolateCatmullRom().
	/// The ember vector is expected to be a length of 4 with all xforms aligned, including final.
	/// </summary>
	/// <param name="embers">Vector of embers</param>
	/// <param name="t">Used in calculating Catmull-Rom coefficients</param>
	void InterpolateCatmullRom(vector<Ember<T>>& embers, T t)
	{
		InterpolateCatmullRom(embers.data(), embers.size(), t);
	}

	/// <summary>
	/// Use Catmull-Rom coefficients and pass to Interpolate().
	/// The ember array is expected to be a length of 4 with all xforms aligned, including final.
	/// </summary>
	/// <param name="embers">Pointer to buffer of embers</param>
	/// <param name="size">Number of elements in the buffer of embers</param>
	/// <param name="t">Used in calculating Catmull-Rom coefficients</param>
	void InterpolateCatmullRom(Ember<T>* embers, size_t size, T t)
	{
		T t2 = t * t;
		T t3 = t2 * t;
		vector<T> cmc(4);

		cmc[0] = (2 * t2 - t - t3) / 2;
		cmc[1] = (3 * t3 - 5 * t2 + 2) / 2;
		cmc[2] = (4 * t2 - 3 * t3 + t) / 2;
		cmc[3] = (t3 - t2) / 2;

		Interpolate(embers, size, cmc, 0);
	}

	/// <summary>
	/// Rotate all pre-affine transforms in non-final xforms whose animate value is non-zero by
	/// the specified angle in a counter-clockwise direction.
	/// Omit padding xforms.
	/// Do not rotate post affine transforms.
	/// </summary>
	/// <param name="angle">The angle to rotate by</param>
	void RotateAffines(T angle)
	{
		for (size_t i = 0; i < XformCount(); i++)//Only look at normal xforms, exclude final.
		{
			//Don't rotate xforms with animate set to 0.
			if (m_Xforms[i].m_Animate == 0)
				continue;

			//Assume that if there are no variations, then it's a padding xform.
			if (m_Xforms[i].Empty() && m_AffineInterp != INTERP_LOG)
				continue;

			m_Xforms[i].m_Affine.Rotate(angle);
			//Don't rotate post.
		}
	}

	/// <summary>
	/// Adds symmetry to this ember by adding additional xforms.
	/// sym = 2 or more means rotational.
	/// sym = 1 means identity, ie no symmetry.
	/// sym = 0 means pick a random symmetry (maybe none).
	/// sym = -1 means bilateral (reflection).
	/// sym = -2 or less means rotational and reflective.
	/// </summary>
	/// <param name="sym">The type of symmetry to add</param>
	/// <param name="rand">The random context to use for generating random symmetry</param>
	void AddSymmetry(int sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t i, k, result = 0;
		T a;

		if (sym == 0)
		{
			static int symDistrib[] = {
				-4, -3,
				-2, -2, -2,
				-1, -1, -1,
				 2,  2,  2,
				 3,  3,
				 4,  4,
			};

			if (rand.Rand() & 1)
				sym = symDistrib[rand.Rand() % Vlen(symDistrib)];
			else if (rand.Rand() & 31)
				sym = (rand.Rand() % 13) - 6;
			else
				sym = (rand.Rand() % 51) - 25;
		}

		if (sym == 1 || sym == 0)
			return;

		m_Symmetry = sym;

		if (sym < 0)
		{
			i = XformCount();//Don't apply sym to final.
			Xform<T> xform;
			AddXform(xform);

			m_Xforms[i].m_Weight = 1;
			m_Xforms[i].m_ColorSpeed = 0;
			m_Xforms[i].m_Animate = 0.0;
			m_Xforms[i].m_ColorX = 1;
			m_Xforms[i].m_ColorY = 1;//Added in case 2D palette support is ever added.
			m_Xforms[i].m_Affine.A(-1);
			m_Xforms[i].m_Affine.B(0);
			m_Xforms[i].m_Affine.C(0);
			m_Xforms[i].m_Affine.D(0);
			m_Xforms[i].m_Affine.E(1);
			m_Xforms[i].m_Affine.F(0);
			m_Xforms[i].AddVariation(new LinearVariation<T>());

			result++;
			sym = -sym;
		}

		a = T(2 * M_PI / sym);

		for (k = 1; k < sym; k++)
		{
			i = XformCount();
			Xform<T> xform;
			AddXform(xform);

			m_Xforms[i].m_Weight = 1.0;
			m_Xforms[i].m_ColorSpeed = 0.0;
			m_Xforms[i].m_Animate = 0.0;
			m_Xforms[i].m_ColorX = m_Xforms[i].m_ColorY = (sym < 3) ? 0 : (T(k - 1) / T(sym - 2));//Added Y.
			m_Xforms[i].m_Affine.A(Round6(cos(k * a)));
			m_Xforms[i].m_Affine.D(Round6(sin(k * a)));
			m_Xforms[i].m_Affine.B(Round6(-m_Xforms[i].m_Affine.D()));
			m_Xforms[i].m_Affine.E(m_Xforms[i].m_Affine.A());
			m_Xforms[i].m_Affine.C(0);
			m_Xforms[i].m_Affine.F(0);
			m_Xforms[i].AddVariation(new LinearVariation<T>());

			result++;
		}

		//Sort the newly added xforms, do not touch the original ones.
		std::sort(m_Xforms.end() - result, m_Xforms.end(), &Interpolater<T>::CompareXforms);
	}

	/// <summary>
	/// Return a uint with bits set to indicate which kind of projection should be done.
	/// </summary>
	/// <param name="onlyScaleIfNewIsSmaller">A uint with bits set for each kind of projection that is needed</param>
	size_t ProjBits()
	{
		size_t val = 0;

		if (m_CamZPos != 0) val |= PROJBITS_ZPOS;
		if (m_CamPerspective != 0) val |= PROJBITS_PERSP;
		if (m_CamPitch != 0) val |= PROJBITS_PITCH;
		if (m_CamYaw != 0) val |= PROJBITS_YAW;
		if (m_CamDepthBlur != 0) val |= PROJBITS_BLUR;

		return val;
	}

	/// <summary>
	/// Call the appropriate projection function via function pointer.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">The Isaac object to pass to the projection functions</param>
	inline void Proj(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		(this->*m_ProjFunc)(point, rand);
	}

	/// <summary>
	/// Placeholder to do nothing.
	/// </summary>
	/// <param name="point">Unused</param>
	/// <param name="rand">Unused</param>
	void ProjectNone(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
	}

	/// <summary>
	/// Project when only z is set, and not pitch, yaw, projection or depth blur.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Unused</param>
	void ProjectZPerspective(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T zr = Zeps(1 - m_CamPerspective * (point.m_Z - m_CamZPos));

		point.m_X /= zr;
		point.m_Y /= zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when pitch, and optionally z and perspective are set, but not depth blur or yaw.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Unused</param>
	void ProjectPitch(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T z  = point.m_Z - m_CamZPos;
		T y  = m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		T zr = Zeps(1 - m_CamPerspective * (m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z));

		point.m_X /= zr;
		point.m_Y  = y / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when depth blur, and optionally pitch, perspective and z are set, but not yaw.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Used for blurring</param>
	void ProjectPitchDepthBlur(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T y, z, zr;
		T dsin, dcos;
		T t = rand.Frand01<T>() * M_2PI;

		z = point.m_Z - m_CamZPos;
		y = m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		z = m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z;
		zr = Zeps(1 - m_CamPerspective * z);

		sincos(t, &dsin, &dcos);

		T dr = rand.Frand01<T>() * m_BlurCoef * z;

		point.m_X = (point.m_X + dr * dcos) / zr;
		point.m_Y = (y + dr * dsin) / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when depth blur, yaw and optionally pitch are set, but not perspective and z.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Used for blurring</param>
	void ProjectPitchYawDepthBlur(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T dsin, dcos;
		T t = rand.Frand01<T>() * M_2PI;
		T z = point.m_Z - m_CamZPos;
		T x = m_CamMat[0][0] * point.m_X + m_CamMat[1][0] * point.m_Y;
		T y = m_CamMat[0][1] * point.m_X + m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;

		z = m_CamMat[0][2] * point.m_X + m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z;

		T zr = Zeps(1 - m_CamPerspective * z);
		T dr = rand.Frand01<T>() * m_BlurCoef * z;

		sincos(t, &dsin, &dcos);

		point.m_X = (x + dr * dcos) / zr;
		point.m_Y = (y + dr * dsin) / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when yaw and optionally pitch, z, and perspective are set, but not depth blur.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Unused</param>
	void ProjectPitchYaw(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		T z = point.m_Z - m_CamZPos;
		T x = m_CamMat[0][0] * point.m_X + m_CamMat[1][0] * point.m_Y;
		T y = m_CamMat[0][1] * point.m_X + m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		T zr = Zeps(1 - m_CamPerspective * (m_CamMat[0][2] * point.m_X + m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z));

		point.m_X = x / zr;
		point.m_Y = y / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Clear this ember and set to either reasonable or unreasonable default values.
	/// </summary>
	/// <param name="useDefaults">Use reasonable default if true, else use out of bounds values.</param>
	void Clear(bool useDefaults = true)
	{
		m_Palette.m_Index = -1;
		m_CenterX = 0;
		m_CenterY = 0;
		m_RotCenterY = 0;
		m_Gamma = 4;
		m_Vibrancy = 1;
		m_Brightness = 4;
		m_Symmetry = 0;
		m_Hue = 0;
		m_Rotate = 0;
		m_PixelsPerUnit = 50;
		m_Interp = EMBER_INTERP_LINEAR;
		m_PaletteInterp = INTERP_HSV;
		m_Index = 0;
		m_ParentFilename = "";
		m_ScaleType = eScaleType::SCALE_NONE;

		if (useDefaults)
		{
			//If defaults are on, set to reasonable values.
			m_HighlightPower = -1;
			m_Background.Reset();
			m_FinalRasW = 100;
			m_FinalRasH = 100;
			m_Supersample = 1;
			m_SpatialFilterRadius = T(0.5);
			m_Zoom = 0;
			m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
			m_CamZPos = 0;
			m_CamPerspective = 0;
			m_CamYaw = 0;
			m_CamPitch = 0;
			m_CamDepthBlur = 0;
			m_BlurCoef = 0;
			m_CamMat = m3T(0);
			m_Quality = 1;
			m_MaxRadDE = T(9.0);
			m_MinRadDE = 0;
			m_CurveDE = T(0.4);
			m_GammaThresh = T(0.01);
			m_Passes = 1;
			m_TemporalSamples = 1000;
			m_SpatialFilterType = GAUSSIAN_SPATIAL_FILTER;
			m_AffineInterp = INTERP_LOG;
			m_TemporalFilterType = BOX_TEMPORAL_FILTER;
			m_TemporalFilterWidth = 1;
			m_TemporalFilterExp = 0;
			m_PaletteMode = PALETTE_STEP;
		}
		else
		{
			//Defaults are off, so set to UN-reasonable values.
			m_HighlightPower = -1;
			m_Background = Color<T>(-1, -1, -1, 1);
			m_FinalRasW = 0;
			m_FinalRasH = 0;
			m_Supersample = 0;
			m_SpatialFilterRadius = -1;
			m_Zoom = 999999;
			m_ProjFunc = nullptr;
			m_CamZPos = 999999;
			m_CamPerspective = 999999;
			m_CamYaw = 999999;
			m_CamPitch = 999999;
			m_CamDepthBlur = 999999;
			m_BlurCoef = 999999;
			m_CamMat = m3T(999999);
			m_Quality = -1;
			m_MaxRadDE = -1;
			m_MinRadDE = -1;
			m_CurveDE = -1;
			m_GammaThresh = -1;
			m_Passes = 0;
			m_TemporalSamples = 0;
			m_SpatialFilterType = GAUSSIAN_SPATIAL_FILTER;
			m_AffineInterp = INTERP_LOG;
			m_TemporalFilterType = BOX_TEMPORAL_FILTER;
			m_TemporalFilterWidth = -1;
			m_TemporalFilterExp = -999;
			m_PaletteMode = PALETTE_STEP;
		}

		m_Xforms.clear();
		m_FinalXform.Clear();
		ClearEdit();
	}

	/// <summary>
	/// Thin wrapper to clear edit doc if not nullptr and set to nullptr.
	/// </summary>
	void ClearEdit()
	{
		if (m_Edits != nullptr)
			xmlFreeDoc(m_Edits);

		m_Edits = nullptr;
	}

	/// <summary>
	/// Return a string representation of this ember.
	/// </summary>
	/// <returns>The string representation of this ember</returns>
	string ToString() const
	{
		size_t i;
		ostringstream ss;

		ss << "Final Raster Width: " << m_FinalRasW << endl
		   << "Final Raster Height: " << m_FinalRasH << endl
		   << "Original Raster Width: " << m_OrigFinalRasW << endl
		   << "Original Raster Height: " << m_OrigFinalRasH << endl
		   << "Supersample: " << m_Supersample << endl
		   << "Passes: " << m_Passes << endl
		   << "Temporal Samples: " << m_TemporalSamples << endl
		   << "Symmetry: " << m_Symmetry << endl

		   << "Quality: " << m_Quality << endl
		   << "Pixels Per Unit: " << m_PixelsPerUnit << endl
		   << "Original Pixels Per Unit: " << m_OrigPixPerUnit << endl
		   << "Zoom: " << m_Zoom << endl
		   << "ZPos: " << m_CamZPos << endl
		   << "Perspective: " << m_CamPerspective << endl
		   << "Yaw: " << m_CamYaw << endl
		   << "Pitch: " << m_CamPitch << endl
		   << "Depth Blur: " << m_CamDepthBlur << endl
		   << "CenterX: " << m_CenterX << endl
		   << "CenterY: " << m_CenterY << endl
		   << "RotCenterY: " << m_RotCenterY << endl
		   << "Rotate: " << m_Rotate << endl
		   << "Hue: " << m_Hue << endl
		   << "Brightness: " << m_Brightness << endl
		   << "Gamma: " << m_Gamma << endl
		   << "Vibrancy: " << m_Vibrancy << endl
		   << "Gamma Threshold: " << m_GammaThresh << endl
		   << "Highlight Power: " << m_HighlightPower << endl
		   << "Time: " << m_Time << endl
		   << "Background: " << m_Background.r << ", " << m_Background.g << ", " << m_Background.b << ", " << m_Background.a << endl

		   << "Interp: " << m_Interp << endl
		   << "Affine Interp Type: " << m_AffineInterp << endl

		   << "Minimum DE Radius: " << m_MinRadDE << endl
		   << "Maximum DE Radius: " << m_MaxRadDE << endl
		   << "DE Curve: " << m_CurveDE << endl

		   << "Spatial Filter Type: " << m_SpatialFilterType << endl
		   << "Spatial Filter Radius: " << m_SpatialFilterRadius << endl

		   << "Temporal Filter Type: " << m_TemporalFilterType << endl
		   << "Temporal Filter Exp: " << m_TemporalFilterExp << endl
		   << "Temporal Filter Width: " << m_TemporalFilterWidth << endl

		   << "Palette Mode: " << m_PaletteMode << endl
		   << "Palette Interp: " << m_PaletteInterp << endl
		   << "Palette Index: " << m_Palette.m_Index << endl
		   //Add palette info here if needed.

		   << "Name: " << m_Name << endl
		   << "Index: " << m_Index << endl
		   << "Scale Type: " << m_ScaleType << endl
		   << "Parent Filename: " << m_ParentFilename << endl
		   << endl;

		for (i = 0; i < XformCount(); i++)
		{
			ss << "Xform " << i << ":" << endl << m_Xforms[i].ToString() << endl;
		}

		if (UseFinalXform())
			ss << "Final Xform: " << m_FinalXform.ToString() << endl;

		return ss.str();
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	inline const Xform<T>* Xforms() const { return &m_Xforms[0]; }
	inline Xform<T>* NonConstXforms() { return &m_Xforms[0]; }
	inline size_t XformCount() const { return m_Xforms.size(); }
	inline const Xform<T>* FinalXform() const { return &m_FinalXform; }
	inline Xform<T>* NonConstFinalXform() { return &m_FinalXform; }
	inline bool UseFinalXform() const { return !m_FinalXform.Empty(); }
	inline size_t TotalXformCount() const { return XformCount() + (UseFinalXform() ? 1 : 0); }
	inline int PaletteIndex() const { return m_Palette.m_Index; }
	inline T BlurCoef() { return m_BlurCoef; }
	inline eScaleType ScaleType() const { return m_ScaleType; }

	//The width and height in pixels of the final output image. The size of the histogram and DE filtering buffers will differ from this.
	//Xml fields: "size".
	size_t m_FinalRasW;
	size_t m_FinalRasH;
	size_t m_OrigFinalRasW;//Keep track of the originals read from the Xml, because...
	size_t m_OrigFinalRasH;//the dimension may change in an editor and the originals are needed for the aspect ratio.
	T m_OrigPixPerUnit;

	//The multiplier in size of the histogram and DE filtering buffers. Must be at least one, preferrably never larger than 4, only useful at 2.
	//Xml field: "supersample" or "overample (deprecated)".
	size_t m_Supersample;

	//Times to run the algorithm while clearing the histogram, but not the filter. Almost always set to 1 and may even be deprecated.
	//Xml field: "passes".
	size_t m_Passes;

	//When animating, split each pass into this many pieces, each doing a fraction of the total iterations. Each temporal sample
	//will render an interpolated instance of the ember that is a fraction of the current ember and the next one.
	//When rendering a single image, this field is always set to 1.
	//Xml field: "temporal_samples".
	size_t m_TemporalSamples;

	//Whether or not any symmetry was added. This field is in a bit of a state of conflict right now as flam3 has a severe bug.
	//Xml field: "symmetry".
	int m_Symmetry;

	//The number of iterations per pixel of the final output image. Note this is not affected by the increase in pixels in the
	//histogram and DE filtering buffer due to supersampling. It can be affected by a non-zero zoom value though.
	//10 is a good value for interactive/real-time rendering, 100-200 is good for previewing, 1000 is good for final output. Above that is mostly a waste of energy.
	//Xml field: "quality".
	T m_Quality;

	//The number of pixels in the final output image that corresponds to the distance from 0-1 in the cartesian plane used for iterating.
	//A larger value produces a more zoomed in imgage. A value of 240 is commonly used, but in practice it varies widely depending on the image.
	//Note that increasing this value does not adjust the quality by a proportional amount, so an increased value may produce a degraded image.
	//Xml field: "scale".
	T m_PixelsPerUnit;

	//A value greater than 0 will zoom in the field of view, however it will also increase the quality by a proportional amount. This is used to
	//overcome the shortcoming of scale by also adjusting the quality.
	//Xml field: "zoom".
	T m_Zoom;

	//3D fields.
private:
	typedef void (Ember<T>::*ProjFuncPtr)(Point<T>&, QTIsaac<ISAAC_SIZE, ISAAC_INT>&);
	ProjFuncPtr m_ProjFunc;

public:
	//Xml field: "cam_zpos".
	T m_CamZPos;

	//Xml field: "cam_persp".
	T m_CamPerspective;

	//Xml field: "cam_yaw".
	T m_CamYaw;

	//Xml field: "cam_pitch".
	T m_CamPitch;

	//Xml field: "cam_dof".
	T m_CamDepthBlur;

private:
	T m_BlurCoef;

public:
	m3T m_CamMat;

	//The camera offset from the center of the cartesian plane. Since this is the camera offset, the final output image will be moved in the opposite
	//direction of the values specified. There is also a second copy of the Y coordinate needed because m_CenterY will be modified during strips rendering.
	//Xml field: "center".
	T m_CenterX;
	T m_CenterY;
	T m_RotCenterY;

	//Rotate the camera by this many degrees. Since this is a camera rotation, the final output image will be rotated counter-clockwise.
	//Xml field: "rotate".
	T m_Rotate;

	//When specifying the palette as an index in the palette file, rather than inserted in the Xml, it can optionally have its hue
	//rotated by this amount.
	//Xml field: "hue".
	T m_Hue;

	//Determine how bright to make the image during final accumulation.
	//Xml field: "brightness".
	T m_Brightness;

	//Gamma level used in gamma correction during final accumulation.
	//Xml field: "gamma".
	T m_Gamma;

	//Used in color correction during final accumulation.
	//Xml field: "vibrancy".
	T m_Vibrancy;

	//Gamma threshold used in gamma correction during final accumulation.
	//Xml field: "gamma_threshold".
	T m_GammaThresh;

	//Value to control saturation of some pixels in gamma correction during final accumulation.
	//Xml field: "highlight_power".
	T m_HighlightPower;

	//When animating a file full of many embers, this value is used to specify the time in the animation
	//that this ember should be rendered. They must all be sequential and increase by a default value of 1.
	//Xml field: "time".
	T m_Time;

	//The background color of the image used in final accumulation, ranged 0-1.
	//Xml field: "background".
	Color<T> m_Background;

	//Animation/interpolation.

	//The type of interpolation to use when interpolating between embers for animation.
	//Xml field: "interpolation".
	eInterp m_Interp;

	//The type of interpolation to use on affine transforms when interpolating embers for animation.
	//Xml field: "interpolation_type" or "interpolation_space (deprecated)".
	eAffineInterp m_AffineInterp;

	//The type of interpolation to use for the palette when interpolating embers for animation.
	//Xml field: "palette_interpolation".
	ePaletteInterp m_PaletteInterp;

	//Temporal Filter.

	//Only used if temporal filter type is exp, else unused.
	//Xml field: "temporal_filter_exp".
	T m_TemporalFilterExp;

	//The width of the temporal filter.
	//Xml field: "temporal_filter_width".
	T m_TemporalFilterWidth;

	//The type of the temporal filter: Gaussian, Box or Exp.
	//Xml field: "temporal_filter_type".
	eTemporalFilterType m_TemporalFilterType;

	//Density Estimation Filter.

	//The minimum radius of the DE filter.
	//Xml field: "estimator_minimum".
	T m_MinRadDE;

	//The maximum radius of the DE filter.
	//Xml field: "estimator_radius".
	T m_MaxRadDE;

	//The shape of the curve that governs how quickly or slowly the filter drops off as it moves away from the center point.
	//Xml field: "estimator_curve".
	T m_CurveDE;

	//Spatial Filter.

	//The radius of the spatial filter used in final accumulation.
	//Xml field: "filter".
	T m_SpatialFilterRadius;

	//The type of spatial filter used in final accumulation:
	//Gaussian, Hermite, Box, Triangle, Bell, Bspline, Lanczos3
	//Lanczos2, Mitchell, Blackman, Catrom, Hamming, Hanning, Quadratic.
	//Xml field: "filter_shape".
	eSpatialFilterType m_SpatialFilterType;

	//Palette.

	//The method used for retrieving a color from the palette when accumulating to the histogram: step, linear.
	//Xml field: "palette_mode".
	ePaletteMode m_PaletteMode;

	//The color palette to use. Can be specified inline as Xml color fields, or as a hex buffer. Can also be specified
	//as an index into the palette file with an optional hue rotation applied. Inserting as a hex buffer is the preferred method.
	//Xml field: "color" or "colors" or "palette" .
	Palette<T> m_Palette;

	//Strings.

	//The name of this ember.
	//Xml field: "name".
	string m_Name;

	//The name of the file that this ember was contained in.
	//Xml field: "".
	string m_ParentFilename;

	//An Xml edit document describing information about the author as well as an edit history of the ember.
	//Xml field: "edit".
	xmlDocPtr m_Edits;

	//The 0-based position of this ember in the file it was contained in.
	size_t m_Index;

private:
	/// <summary>
	/// The type of scaling used when resizing.
	/// </summary>
	eScaleType m_ScaleType;

	/// <summary>
	/// Interpolation function that takes the address of a member variable of type T as a template parameter.
	/// This is an alternative to using macros.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <T Ember<T>::*m>
	void InterpT(Ember<T>* embers, vector<T>& coefs, size_t size)
	{
		this->*m = 0;

		for (size_t k = 0; k < size; k++)
			this->*m += coefs[k] * embers[k].*m;
	}

	/// <summary>
	/// Interpolation function that takes the address of a member variable of any type as a template parameter.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <typename M, M Ember<T>::*m>
	void InterpX(Ember<T>* embers, vector<T>& coefs, size_t size)
	{
		this->*m = M();

		for (size_t k = 0; k < size; k++)
			this->*m += coefs[k] * embers[k].*m;
	}

	/// <summary>
	/// Interpolation function that takes the address of a member variable of type integer as a template parameter.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <size_t Ember<T>::*m>
	void InterpI(Ember<T>* embers, vector<T>& coefs, size_t size)
	{
		T t = 0;

		for (size_t k = 0; k < size; k++)
			t += coefs[k] * embers[k].*m;

		this->*m = (size_t)Rint(t);
	}

	/// <summary>
	/// Interpolation function that takes the address of an xform member variable of type T as a template parameter.
	/// This is an alternative to using macros.
	/// </summary>
	/// <param name="xform">A pointer to a list of xforms to interpolate</param>
	/// <param name="i">The xform index to interpolate</param>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <T Xform<T>::*m>
	void InterpXform(Xform<T>* xform, size_t i, Ember<T>* embers, vector<T>& coefs, size_t size)
	{
		xform->*m = T(0);

		for (size_t k = 0; k < size; k++)
			xform->*m += coefs[k] * embers[k].GetTotalXform(i)->*m;
	}

	//The vector containing all of the xforms.
	//Xml field: "xform".
	vector<Xform<T>> m_Xforms;

	//Optional final xform. Default is empty.
	//Discussed in section 3.2 of the paper, page 6.
	//Xml field: "finalxform".
	Xform<T> m_FinalXform;
};

/// <summary>
/// Comparer for sorting embers based on time.
/// </summary>
/// <param name="av">Pointer to the first ember to compare</param>
/// <param name="bv">Pointer to the second ember to compare</param>
/// <returns>True if av's time is less than bv's time, else false.</returns>
template <typename T>
static inline bool CompareEmbers(const Ember<T>& a, const Ember<T>& b)
{
	return a.m_Time < b.m_Time;
}
}
