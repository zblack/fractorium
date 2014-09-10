#pragma once

#include "Ember.h"

/// <summary>
/// Interpolater class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// g++ needs a forward declaration here.
/// </summary>
template <typename T> class Ember;

/// <summary>
/// Contains many static functions for handling interpolation and other miscellaneous operations on
/// embers and vectors of embers. This class is similar to, and used in conjunction with SheepTools.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Interpolater
{
public:
	/// <summary>
	/// Aligns the specified array of embers and stores in the output array.
	/// This is used to prepare embers before interpolating them.
	/// Alignment means that every ember in a list will have the same number of xforms.
	/// Each xform at a given position will have mostly the same variations as the xform
	/// in the same position in the rest of the embers. However some
	/// intelligence is applied to add or remove variations that wouldn't look good with
	/// the others present.
	/// After this function completes, sourceEmbers will remain unchanged and destEmbers
	/// will contain the aligned list of embers from sourceEmbers.
	/// </summary>
	/// <param name="sourceEmbers">The array of embers to align</param>
	/// <param name="destEmbers">The array which will contain the aligned embers </param>
	/// <param name="count">The number of elements in sourceEmbers</param>
	static void Align(Ember<T>* sourceEmbers, Ember<T>* destEmbers, unsigned int count)
	{
		bool aligned = true;
		bool currentFinal, final = sourceEmbers[0].UseFinalXform();
		unsigned int i, xf, currentCount, maxCount = sourceEmbers[0].XformCount();
		Xform<T>* destXform;
		Xform<T>* destOtherXform;

		//Determine the max number of xforms present in sourceEmbers.
		//Also check if final xforms are used in any of them.
		for (i = 1; i < count; i++)
		{
			currentCount = sourceEmbers[i].XformCount();

			if (currentCount != maxCount)//Any difference, less or more, means unaligned.
			{
				aligned = false;

				if (currentCount > maxCount)
					maxCount = currentCount;
			}

			currentFinal = sourceEmbers[i].UseFinalXform();

			if (final != currentFinal)//Check if any used final.
			{
				aligned = false;
				final |= currentFinal;
			}
		}

		//Copy them using the max xform count, and do final if any had final.
		for (i = 0; i < count; i++)
			destEmbers[i] = sourceEmbers[i].Copy(maxCount, final);

		if (final)
			maxCount++;

		//Check to see if there's a parametric variation present in one xform
		//but not in an aligned xform.  If this is the case, use the parameters
		//from the xform with the variation as the defaults for the blank one.
		//All embers will have the same number of xforms at this point.
		for (i = 0; i < count; i++)
		{
			unsigned int ii;

			for (xf = 0; xf < maxCount; xf++)//This will include both normal xforms and the final.
			{
				destXform = destEmbers[i].GetTotalXform(xf, final);

				//Ensure every parametric variation contained in every xform at either position i - 1 or i + 1 is also contained in the dest xform.
				if (i > 0)
					destOtherXform = destEmbers[i - 1].GetTotalXform(xf);
				else if (i < count - 1)
					destOtherXform = destEmbers[i + 1].GetTotalXform(xf);
				else
					destOtherXform = nullptr;//Should never happen

				if (destOtherXform)
					MergeXformVariations1Way(destOtherXform, destXform, true, true);

				//This is a new xform.  Let's see if it's possible to choose a better 'identity' xform.
				//Check the neighbors to see if any of these variations are used:
				//rings2, fan2, blob, perspective, julian, juliascope, ngon, curl, super_shape, split
				//If so, can use a better starting point for these.
				//If the current xform index is greater than what the original xform count was for this ember, then it's a padding xform.
				if (xf >= sourceEmbers[i].TotalXformCount() && !aligned)
				{
					unsigned int found = 0;

					//Remove linear.
					destXform->DeleteVariationById(VAR_LINEAR);

					//Only do the next substitution for log interpolation.
					if ((i == 0 && destEmbers[i].m_AffineInterp     == INTERP_LOG) ||
						(i > 0  && destEmbers[i - 1].m_AffineInterp == INTERP_LOG))
					{
						for (ii = -1; ii <= 1; ii += 2)
						{
							//Skip if out of bounds.
							if (i + ii < 0 || i + ii >= count)
								continue;

							//Skip if this is also padding.
							if (xf >= sourceEmbers[i + ii].TotalXformCount())
								continue;

							destOtherXform = destEmbers[i + ii].GetTotalXform(xf);

							//Spherical / Ngon (trumps all others due to holes)
							//Interpolate these against a 180 degree rotated identity
							//with weight -1.
							//Added JULIAN/JULIASCOPE to get rid of black wedges.
							if (destOtherXform->GetVariationById(VAR_SPHERICAL) ||
								destOtherXform->GetVariationById(VAR_NGON) ||
								destOtherXform->GetVariationById(VAR_JULIAN) ||
								destOtherXform->GetVariationById(VAR_JULIASCOPE) ||
								destOtherXform->GetVariationById(VAR_POLAR) ||
								destOtherXform->GetVariationById(VAR_WEDGE_SPH) ||
								destOtherXform->GetVariationById(VAR_WEDGE_JULIA))
							{
								destXform->AddVariation(new LinearVariation<T>(-1));

								//Set the coefs appropriately.
								destXform->m_Affine.A(-1);
								destXform->m_Affine.D(0);
								destXform->m_Affine.B(0);
								destXform->m_Affine.E(-1);
								destXform->m_Affine.C(0);
								destXform->m_Affine.F(0);
								found = -1;
							}
						}
					}

					if (found == 0)
					{
						for (ii = -1; ii <= 1; ii += 2)
						{
							//Skip if out of bounds.
							if (i + ii < 0 || i + ii >= count)
								continue;

							//Skip if this is also padding.
							if (xf >= sourceEmbers[i + ii].TotalXformCount())
								continue;

							destOtherXform = destEmbers[i + ii].GetTotalXform(xf);

							if (destOtherXform->GetVariationById(VAR_RECTANGLES))
							{
								RectanglesVariation<T>* var = new RectanglesVariation<T>();

								var->SetParamVal("rectangles_x", 0);
								var->SetParamVal("rectangles_y", 0);
								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_RINGS2))
							{
								Rings2Variation<T>* var = new Rings2Variation<T>();

								var->SetParamVal("rings2_val", 0);
								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_FAN2))
							{
								Fan2Variation<T>* var = new Fan2Variation<T>();

								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_BLOB))
							{
								BlobVariation<T>* var = new BlobVariation<T>();

								var->SetParamVal("blob_low", 1);
								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_PERSPECTIVE))
							{
								PerspectiveVariation<T>* var = new PerspectiveVariation<T>();

								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_CURL))
							{
								CurlVariation<T>* var = new CurlVariation<T>();

								var->SetParamVal("curl_c1", 0);
								destXform->AddVariation(var);
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_SUPER_SHAPE))
							{
								SuperShapeVariation<T>* var = new SuperShapeVariation<T>();

								var->SetParamVal("super_shape_n1", 2);
								var->SetParamVal("super_shape_n2", 2);
								var->SetParamVal("super_shape_n3", 2);
								destXform->AddVariation(var);
								found++;
							}
						}
					}

					//If none matched those, try the affine ones, fan and rings.
					if (found == 0)
					{
						for (ii = -1; ii <= 1; ii += 2)
						{
							//Skip if out of bounds.
							if (i + ii < 0 || i + ii >= count)
								continue;

							//Skip if this is also padding.
							if (xf >= sourceEmbers[i + ii].TotalXformCount())
								continue;

							destOtherXform = destEmbers[i + ii].GetTotalXform(xf);

							if (destOtherXform->GetVariationById(VAR_FAN))
							{
								destXform->AddVariation(new FanVariation<T>());
								found++;
							}

							if (destOtherXform->GetVariationById(VAR_RINGS))
							{
								destXform->AddVariation(new RingsVariation<T>());
								found++;
							}
						}

						if (found > 0)
						{
							//Set the coefs appropriately.
							destXform->m_Affine.A(0);
							destXform->m_Affine.B(1);//This will be swapping x and y, seems strange, but it's what the original did.
							destXform->m_Affine.C(0);
							destXform->m_Affine.D(1);
							destXform->m_Affine.E(0);
							destXform->m_Affine.F(0);
						}
					}

					//If there still are no matches, switch back to linear.
					if (found == 0)
					{
						destXform->AddVariation(new LinearVariation<T>());
					}
					else if (found > 0)
					{
						//Otherwise, normalize the weights.
						destXform->NormalizeVariationWeights();
					}
				}
			}//Xforms.
		}//Embers.
	}

	/// <summary>
	/// Thin wrapper around AnyXaosPresent().
	/// </summary>
	/// <param name="embers">The vector of embers to inspect for xaos</param>
	/// <returns>True if at least one ember contained xaos, else false.</returns>
	static bool AnyXaosPresent(vector<Ember<T>>& embers)
	{
		return AnyXaosPresent(embers.data(), embers.size());
	}

	/// <summary>
	/// Determine whether at least one ember in the array contained xaos.
	/// </summary>
	/// <param name="embers">The array of embers to inspect</param>
	/// <param name="size">The size of the embers array</param>
	/// <returns>True if at least one ember contained xaos, else false.</returns>
	static bool AnyXaosPresent(Ember<T>* embers, size_t size)
	{
		for (unsigned int i = 0; i < size; i++)
			if (embers[i].XaosPresent())
				return true;

		return false;
	}

	/// <summary>
	/// Thin wrapper around MaxXformCount().
	/// </summary>
	/// <param name="embers">The vector of embers to inspect for the greatest xform count</param>
	/// <returns>The greatest non-final xform count in any of the embers</returns>
	static unsigned int MaxXformCount(vector<Ember<T>>& embers)
	{
		return MaxXformCount(embers.data(), embers.size());
	}

	/// <summary>
	/// Find the maximum number of non-final xforms present in the array of embers.
	/// </summary>
	/// <param name="embers">The array of embers to inspect</param>
	/// <param name="size">The size of the embers array</param>
	/// <returns>The greatest non-final xform count in any of the embers</returns>
	static unsigned int MaxXformCount(Ember<T>* embers, size_t size)
	{
		unsigned int i, maxCount = 0;

		for (i = 0; i < size; i++)
			if (embers[i].XformCount() > maxCount)
				maxCount = embers[i].XformCount();

		return maxCount;
	}

	/// <summary>
	/// Thin wrapper around AnyFinalPresent().
	/// </summary>
	/// <param name="embers">The vector of embers to inspect the presence of a final xform</param>
	/// <returns>True if any contained a non-empty final xform, else false.</returns>
	static bool AnyFinalPresent(vector<Ember<T>>& embers)
	{
		return AnyFinalPresent(embers.data(), embers.size());
	}

	/// <summary>
	/// Determine whether at least one ember in the array contained a non-empty final xform.
	/// </summary>
	/// <param name="embers">The array of embers to inspect the presence of a final xform</param>
	/// <param name="size">The size of the embers array</param>
	/// <returns>True if any contained a final xform, else false.</returns>
	static bool AnyFinalPresent(Ember<T>* embers, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			if (embers[i].UseFinalXform())
				return true;

		return false;
	}

	/// <summary>
	/// Thin wrapper around Interpolate().
	/// </summary>
	/// <param name="embers">The vector of embers to interpolate</param>
	/// <param name="time">The time position in the vector specifying the point of interpolation</param>
	/// <param name="stagger">Stagger if > 0</param>
	/// <param name="result">The interpolated result</param>
	static void Interpolate(vector<Ember<T>>& embers, T time, T stagger, Ember<T>& result)
	{
		Interpolate(embers.data(), embers.size(), time, stagger, result);
	}

	/// <summary>
	/// Interpolates the array of embers at a specified time and stores the result.
	/// </summary>
	/// <param name="embers">The embers array</param>
	/// <param name="size">The size of the embers array</param>
	/// <param name="time">The time position in the vector specifying the point of interpolation</param>
	/// <param name="stagger">Stagger if > 0</param>
	/// <param name="result">The interpolated result</param>
	static void Interpolate(Ember<T>* embers, size_t size, T time, T stagger, Ember<T>& result)
	{
		if (size == 1)
		{
			result = embers[0];//Deep copy.
			return;
		}

		size_t i1, i2;
		vector<T> c(2);
		Ember<T> localEmbers[4];
		bool smoothFlag = false;
		c.resize(2);

		if (embers[0].m_Time >= time)
		{
			i1 = 0;
			i2 = 1;
		}
		else if (embers[size - 1].m_Time <= time)
		{
			i1 = size - 2;
			i2 = size - 1;
		}
		else
		{
			i1 = 0;

			while (embers[i1].m_Time < time)
				i1++;

			i1--;
			i2 = i1 + 1;
		}

		c[0] = (embers[i2].m_Time - time) / (embers[i2].m_Time - embers[i1].m_Time);
		c[1] = 1 - c[0];

		//To interpolate the xforms, make copies of the source embers
		//and ensure that they both have the same number of xforms before progressing.
		if (embers[i1].m_Interp == EMBER_INTERP_LINEAR)
		{
			Align(&embers[i1], &localEmbers[0], 2);
			smoothFlag = false;
		}
		else
		{
			if (i1 == 0)
			{
				//fprintf(stderr, "error: cannot use smooth interpolation on first segment.\n");
				//fprintf(stderr, "reverting to linear interpolation.\n");
				Align(&embers[i1], &localEmbers[0], 2);
				smoothFlag = false;
			}

			if (i2 == size - 1)
			{
				//fprintf(stderr, "error: cannot use smooth interpolation on last segment.\n");
				//fprintf(stderr, "reverting to linear interpolation.\n");
				Align(&embers[i1], &localEmbers[0], 2);
				smoothFlag = false;
			}

			Align(&embers[i1 - 1], &localEmbers[0], 4);//Should really be doing some sort of checking here to ensure the ember vectors have 4 elements.
			smoothFlag = true;
		}

		result.m_Time = time;
		result.m_Interp = EMBER_INTERP_LINEAR;
		result.m_AffineInterp = embers[0].m_AffineInterp;
		result.m_PaletteInterp = INTERP_HSV;

		if (!smoothFlag)
			result.Interpolate(&localEmbers[0], 2, c, stagger);
		else
			result.InterpolateCatmullRom(&localEmbers[0], 4, c[1]);
	}

	/// <summary>
	/// Merge the variations in a vector of xforms into a single xform so that
	/// it contains one variation for each variation type that was present in the
	/// vector of xforms.
	/// </summary>
	/// <param name="xforms">The xforms to merge</param>
	/// <param name="clearWeights">Clear weights if true, else copy weights</param>
	/// <returns>The xform whose variations are a result of the merge</returns>
	static Xform<T> MergeXforms(vector<Xform<T>*>& xforms, bool clearWeights = false)
	{
		Xform<T> xform;

		for (unsigned int xf = 0; xf < xforms.size(); xf++)
			MergeXformVariations1Way(xforms[xf], &xform, false, clearWeights);

		return xform;
	}

	/// <summary>
	/// Merges the xform variations from one xform to another, but not back.
	/// </summary>
	/// <param name="source">The source xform to merge from</param>
	/// <param name="dest">The destination xform to merge to</param>
	/// <param name="parVarsOnly">If true, only merge parametric variations, else merge all</param>
	/// <param name="clearWeights">If true, set variation weights in dest to 0, else copy weights</param>
	static void MergeXformVariations1Way(Xform<T>* source, Xform<T>* dest, bool parVarsOnly, bool clearWeights)
	{
		for (unsigned int i = 0; i < source->TotalVariationCount(); i++)//Iterate through the first xform's variations.
		{
			Variation<T>* var = source->GetVariation(i);//Grab the variation at index in in the first xform.
			Variation<T>* var2 = dest->GetVariationById(var->VariationId());//See if the same variation exists in the second xform.
			ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);//Parametric cast of the first var for later.

			if (!var2)//Only take action if the second xform did not contain this variation.
			{
				if (parVarsOnly)//Only add if parametric.
				{
					if (parVar)
					{
						Variation<T>* parVarCopy = parVar->Copy();

						if (clearWeights)
							parVarCopy->m_Weight = 0;

						dest->AddVariation(parVarCopy);
					}
				}
				else//Add regardless of type.
				{
					Variation<T>* varCopy = var->Copy();

					if (clearWeights)
						varCopy->m_Weight = 0;

					dest->AddVariation(varCopy);
				}
			}
		}
	}

	/// <summary>
	/// Merges the xform variations from one xform to another, and back.
	/// After this function completes, both xforms will have the same variations.
	/// </summary>
	/// <param name="source">The source xform to merge from, and to</param>
	/// <param name="dest">The destination xform to merge to, and from</param>
	/// <param name="parVarsOnly">If true, only merge parametric variations, else merge all</param>
	/// <param name="clearWeights">If true, set variation weights in dest to 0, else copy weights</param>
	static void MergeXformVariations2Way(Xform<T>* source, Xform<T>* dest, bool parVarsOnly, bool clearWeights)
	{
		MergeXformVariations1Way(source, dest, parVarsOnly, clearWeights);
		MergeXformVariations1Way(dest, source, parVarsOnly, clearWeights);
	}

	/// <summary>
	/// Interpolate a vector of parametric variations by a vector of coefficients and store the ouput in a new parametric variation.
	/// Elements in first which are not the same variation type as second will be ignored.
	/// </summary>
	/// <param name="first">The vector of parametric variations to interpolate</param>
	/// <param name="second">The parametric variation to store the output. This must be initialized first to the desired type.</param>
	/// <param name="c">The vector of coefficients used to interpolate</param>
	static void InterpParametricVar(vector<ParametricVariation<T>*>& first, ParametricVariation<T>* second, vector<T>& c)
	{
		//First, make sure the variation vector is the same size as the coefficient vector.
		if (second != nullptr && first.size() == c.size())
		{
			second->Clear();
			ParamWithName<T>* secondParams = second->Params();

			//Iterate through each of the source variations.
			for (unsigned int i = 0; i < first.size(); i++)
			{
				ParametricVariation<T>* firstVar = first[i];

				//Make sure the source variation at this index is the same type as the variation being written to.
				if (firstVar->VariationId() == second->VariationId())
				{
					unsigned int size = firstVar->ParamCount();
					ParamWithName<T>* firstParams = firstVar->Params();

					//Multiply each parameter of the variation at this index by the coefficient at this index, and add
					//the result to the corresponding parameter in second.
					for (unsigned int j = 0; j < size; j++)
					{
						if (!firstParams[j].IsPrecalc())
							*(secondParams[j].Param()) += c[i] * firstParams[j].ParamVal();
					}
				}
			}

			second->Precalc();
		}
	}

	/// <summary>
	/// Thin wrapper around ConvertLinearToPolar().
	/// </summary>
	/// <param name="embers">The vector of embers whose affine transforms will be copied and converted</param>
	/// <param name="xfi">The xform index in each ember to convert</param>
	/// <param name="cflag">If 0 convert pre affine, else post affine.</param>
	/// <param name="cxAng">The vec2 vector to store the polar angular values</param>
	/// <param name="cxMag">The vec2 vector to store the polar magnitude values</param>
	/// <param name="cxTrn">The vec2 vector to store the polar translation values</param>
	static void ConvertLinearToPolar(vector<Ember<T>>& embers, int xfi, int cflag, vector<v2T>& cxAng, vector<v2T>& cxMag, vector<v2T>& cxTrn)
	{
		ConvertLinearToPolar(embers.data(), embers.size(), xfi, cflag, cxAng, cxMag, cxTrn);
	}

	/// <summary>
	/// Convert pre or post affine coordinates of the xform at a specific index in each ember from linear to polar and store as separate
	/// vec2 components in the vector parameters cxAng, cxMag and cxTrn.
	/// </summary>
	/// <param name="embers">The array of embers whose affine transforms will be copied and converted</param>
	/// <param name="size">The size of the embers array</param>
	/// <param name="xfi">The xform index in each ember to convert</param>
	/// <param name="cflag">If 0 convert pre affine, else post affine.</param>
	/// <param name="cxAng">The vec2 vector to store the polar angular values</param>
	/// <param name="cxMag">The vec2 vector to store the polar magnitude values</param>
	/// <param name="cxTrn">The vec2 vector to store the polar translation values</param>
	static void ConvertLinearToPolar(Ember<T>* embers, size_t size, int xfi, int cflag, vector<v2T>& cxAng, vector<v2T>& cxMag, vector<v2T>& cxTrn)
	{
		if (size == cxAng.size() &&
			size == cxMag.size() &&
			size == cxTrn.size())
		{
			T c1[2], d, t, refang;
			glm::length_t col, k;
			int zlm[2];
			const char* loc = __FUNCTION__;

			for (k = 0; k < size; k++)
			{
				//Establish the angles and magnitudes for each component.
				//Keep translation linear.
				zlm[0] = zlm[1] = 0;

				if (Xform<T>* xform = embers[k].GetTotalXform(xfi))
				{
					for (col = 0; col < 2; col++)
					{
						if (cflag == 0)
						{
							c1[0] = xform->m_Affine.m_Mat[0][col];//a or b.
							c1[1] = xform->m_Affine.m_Mat[1][col];//d or e.
							t = xform->m_Affine.m_Mat[col][2];//c or f.
						}
						else
						{
							c1[0] = xform->m_Post.m_Mat[0][col];
							c1[1] = xform->m_Post.m_Mat[1][col];
							t = xform->m_Post.m_Mat[col][2];
						}

						cxAng[k][col] = atan2(c1[1], c1[0]);
						cxMag[k][col] = sqrt(c1[0] * c1[0] + c1[1] * c1[1]);

						if (cxMag[k][col] == 0)
							zlm[col] = 1;

						cxTrn[k][col] = t;
					}

					if (zlm[0] == 1 && zlm[1] == 0)
						cxAng[k][0] = cxAng[k][1];
					else if (zlm[0] == 0 && zlm[1] == 1)
						cxAng[k][1] = cxAng[k][0];
				}
				else
				{
					cout << loc << ": xform " << xfi << " is missing when it was expected, something is severely wrong." << endl;
				}
			}

			//Make sure the rotation is the shorter direction around the circle
			//by adjusting each angle in succession, and rotate clockwise if 180 degrees.
			for (col = 0; col < 2; col++)
			{
				for (k = 1; k < size; k++)
				{
					if (Xform<T>* xform = embers[k].GetTotalXform(xfi))
					{
						//Adjust angles differently if an asymmetric case.
						if (xform->m_Wind[col] > 0 && cflag == 0)
						{
							//Adjust the angles to make sure that it's within wind : wind + 2pi.
							refang = xform->m_Wind[col] - M_2PI;

							//Make sure both angles are within [refang refang + 2 * pi].
							while(cxAng[k - 1][col] < refang)
								cxAng[k - 1][col] += M_2PI;

							while(cxAng[k - 1][col] > refang + M_2PI)
								cxAng[k - 1][col] -= M_2PI;

							while(cxAng[k][col] < refang)
								cxAng[k][col] += M_2PI;

							while(cxAng[k][col] > refang + M_2PI)
								cxAng[k][col] -= M_2PI;
						}
						else
						{
							//Normal way of adjusting angles.
							d = cxAng[k][col] - cxAng[k - 1][col];

							//Adjust to avoid the -pi/pi discontinuity.
							if (d > M_PI + EPS)
								cxAng[k][col] -= M_2PI;
							else if (d < -(M_PI - EPS))//Forces clockwise rotation at 180.
								cxAng[k][col] += M_2PI;
						}
					}
					else
					{
						cout << loc << ": xform " << xfi << " is missing when it was expected, something is severely wrong." << endl;
					}
				}
			}
		}
	}

	/// <summary>
	/// Never really understood what this did, but it has to do with winding.
	/// </summary>
	/// <param name="embers">The array of embers</param>
	/// <param name="count">The size of the embers array</param>
	static void AsymmetricRefAngles(Ember<T>* embers, unsigned int count)
	{
		unsigned int k, xfi, col;
		T cxang[4][2], c1[2], d;

		for (xfi = 0; xfi < embers[0].XformCount(); xfi++)//Final xforms don't rotate regardless of their symmetry.
		{
			for (k = 0; k < count; k++)
			{
				//Establish the angle for each component.
				//Should potentially functionalize.
				for (col = 0; col < 2; col++)
				{
					c1[0] = embers[k].GetXform(xfi)->m_Affine.m_Mat[0][col];//A,D then B,E.
					c1[1] = embers[k].GetXform(xfi)->m_Affine.m_Mat[1][col];

					cxang[k][col] = atan2(c1[1], c1[0]);
				}
			}

			for (k = 1; k < count; k++)
			{
				for (col = 0; col < 2; col++)
				{
					int sym0, sym1;
					int padSymFlag;

					d = cxang[k][col] - cxang[k-1][col];

					//Adjust to avoid the -pi/pi discontinuity.
					if (d > T(M_PI + EPS))
						cxang[k][col] -= 2 * T(M_PI);
					else if (d < -T(M_PI - EPS) )
						cxang[k][col] += 2 * T(M_PI);

					//If this is an asymmetric case, store the NON-symmetric angle
					//Check them pairwise and store the reference angle in the second
					//to avoid overwriting if asymmetric on both sides.
					padSymFlag = 0;

					sym0 = (embers[k - 1].GetXform(xfi)->m_Animate == 0 || (embers[k - 1].GetXform(xfi)->Empty() && padSymFlag));
					sym1 = (embers[k    ].GetXform(xfi)->m_Animate == 0 || (embers[k    ].GetXform(xfi)->Empty() && padSymFlag));

					if (sym1 && !sym0)
						embers[k].GetXform(xfi)->m_Wind[col] = cxang[k-1][col] + 2 * T(M_PI);
					else if (sym0 && !sym1)
						embers[k].GetXform(xfi)->m_Wind[col] = cxang[k][col] + 2 * T(M_PI);
				}
			}
		}
	}

	/// <summary>
	/// Never really understood what this did.
	/// </summary>
	/// <param name="coefs">The coefficients vector</param>
	/// <param name="cxAng">The vec2 vector to store the polar angular values</param>
	/// <param name="cxMag">The vec2 vector to store the polar magnitude values</param>
	/// <param name="cxTrn">The vec2 vector to store the polar translation values</param>
	/// <param name="store">The Affine2D to store the inerpolated values in</param>
	static void InterpAndConvertBack(vector<T>& coefs, vector<v2T>& cxAng, vector<v2T>& cxMag, vector<v2T>& cxTrn, Affine2D<T>& store)
	{
		size_t size = coefs.size();
		glm::length_t i, col, accmode[2] = { 0, 0 };
		T expmag, accang[2] = { 0, 0 }, accmag[2] = { 0, 0 };

		//Accumulation mode defaults to logarithmic, but in special
		//cases switch to linear accumulation.
		for (col = 0; col < 2; col++)
		{
			for (i = 0; i < size; i++)
			{
				if (log(cxMag[i][col]) < -10)
					accmode[col] = 1;//Mode set to linear interp.
			}
		}

		for (i = 0; i < size; i++)
		{
			for (col = 0; col < 2; col++)
			{
				accang[col] += coefs[i] * cxAng[i][col];

				if (accmode[col] == 0)
					accmag[col] += coefs[i] * log(cxMag[i][col]);
				else
					accmag[col] += coefs[i] * (cxMag[i][col]);

				//Translation is ready to go.
				store.m_Mat[col][2] += coefs[i] * cxTrn[i][col];
			}
		}

		//Convert the angle back to rectangular.
		for (col = 0; col < 2; col++)
		{
			if (accmode[col] == 0)
				expmag = exp(accmag[col]);
			else
				expmag = accmag[col];

			store.m_Mat[0][col] = expmag * cos(accang[col]);
			store.m_Mat[1][col] = expmag * sin(accang[col]);
		}
	}

	/// <summary>
	/// Smooths the time values for animations.
	/// </summary>
	/// <param name="t">The time value to smooth</param>
	/// <returns>the smoothed time value</returns>
	static inline T Smoother(T t)
	{
		return 3 * t * t - 2 * t * t * t;
	}

	/// <summary>
	/// Gets the stagger coef based on the position of the current xform among the others.
	/// Never really understood what this did.
	/// </summary>
	/// <param name="t">The time value</param>
	/// <param name="staggerPercent">The stagger percentage</param>
	/// <param name="numXforms">The number xforms in the ember</param>
	/// <param name="thisXform">The index of this xform within the ember</param>
	/// <returns>The stagger coefficient</returns>
	static inline T GetStaggerCoef(T t, T staggerPercent, int numXforms, int thisXform)
	{
		//maxStag is the spacing between xform start times if staggerPercent = 1.0.
		T maxStag = T(numXforms - 1) / numXforms;

		//Scale the spacing by staggerPercent.
		T stagScaled = staggerPercent * maxStag;

		//t ranges from 1 to 0 (the contribution of cp[0] to the blend).
		//The first line below makes the first xform interpolate first.
		//The second line makes the last xform interpolate first.
		T st = stagScaled * (numXforms - 1 - thisXform) / (numXforms - 1);
		T et = st + (1 - stagScaled);

		if (t <= st)
			return 0;
		else if (t >= et)
			return 1;
		else
			return Smoother((t - st) / (1 - stagScaled));
	}

	/// <summary>
	/// Apply the specified motion function to a value.
	/// </summary>
	/// <param name="funcNum">The function type to apply, sin, triangle or hill.</param>
	/// <param name="timeVal">The time value to apply the motion function to</param>
	/// <returns>The new time value computed by applying the specified motion function to the time value</returns>
	static T MotionFuncs(int funcNum, T timeVal)
	{
		//Motion funcs should be cyclic, and equal to 0 at integral time values
		//abs peak values should be not be greater than 1.
		if (funcNum == MOTION_SIN)
		{
			return sin(T(2.0) * T(M_PI) * timeVal);
		}
		else if (funcNum == MOTION_TRIANGLE)
		{
			T fr = fmod(timeVal, T(1.0));

			if (fr < 0)
				fr += 1;

			if (fr <= T(0.25))
				fr *= 4;
			else if (fr <= T(0.75))
				fr = -4 * fr + 2;
			else
				fr = 4 * fr - 4;

			return fr;
		}
		else//MOTION_HILL
		{
			return ((1 - cos(T(2.0) * T(M_PI) * timeVal)) * T(0.5));
		}
	}

	/*
	//Will need to alter this to handle 2D palettes.
	static bool InterpMissingColors(vector<glm::detail::tvec4<T>>& palette)
	{
		//Check for a non-full palette.
		int minIndex, maxIndex;
		int colorli, colorri;
		int wrapMin, wrapMax;
		int intl, intr;
		int str, enr;
		int i, j, k;
		double prcr;

		if (palette.size() != 256)
			return false;

		for (i = 0; i < 256; i++)
		{
			if (palette[i].m_Index >= 0)
			{
				minIndex = i;
				break;
			}
		}

		if (i == 256)
		{
			//No colors. Set all indices properly.
			for (i = 0; i < 256; i++)
				palette[i].m_Index = (T)i;

			return false;
		}

		wrapMin = minIndex + 256;

		for (i = 255; i >= 0; i--)//Moving backwards, ouch!
		{
			if (palette[i].m_Index >= 0)
			{
				maxIndex = i;
				break;
			}
		}

		wrapMax = maxIndex - 256;

		//Loop over the indices looking for negs,
		i = 0;

		while (i < 256)
		{
			if (palette[i].m_Index < 0)
			{
				//Start of a range of negs.
				str = i;
				intl = i - 1;
				colorli = intl;

				while (palette[i].m_Index < 0 && i < 256)
				{
					enr = i;
					intr = i + 1;
					colorri = intr;
					i++;
				}

				if (intl == -1)
				{
					intl = wrapMax;
					colorli = maxIndex;
				}

				if (intr == 256)
				{
					intr = wrapMin;
					colorri = minIndex;
				}

				for (j = str; j <= enr; j++)
				{
					prcr = (j - intl) / T(intr - intl);

					for (k = 0; k <= 3; k++)
						palette[j].Channels[k] = T(palette[colorli].Channels[k] * (1 - prcr) + palette[colorri].Channels[k] * prcr);

					palette[j].m_Index = T(j);
				}

				i = colorri + 1;
			}
			else
				i++;
		}

		return true;
	}
	*/

	/// <summary>
	/// Compare xforms for sorting based first on color speed and second on determinants if
	/// color speeds are equal.
	/// </summary>
	/// <param name="a">The first xform to compare</param>
	/// <param name="b">The second xform to compare</param>
	/// <returns>true if a > b, else false.</returns>
	static inline bool CompareXforms(const Xform<T>& a, const Xform<T>& b)
	{
		if (a.m_ColorSpeed > b.m_ColorSpeed) return true;
		if (a.m_ColorSpeed < b.m_ColorSpeed) return false;

		//Original did this every time, even though it's only needed if the color speeds are equal.
		m2T aMat2 = a.m_Affine.ToMat2ColMajor();
		m2T bMat2 = b.m_Affine.ToMat2ColMajor();

		T ad = glm::determinant(aMat2);
		T bd = glm::determinant(bMat2);

		if (a.m_ColorSpeed > 0)
		{
			if (ad < 0) return false;
			if (bd < 0) return true;

			ad = atan2(a.m_Affine.A(), a.m_Affine.D());
			bd = atan2(b.m_Affine.A(), b.m_Affine.D());
		}

		return ad > bd;
	}
};
}
