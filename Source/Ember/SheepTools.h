#pragma once

#include "EmberDefines.h"
#include "Isaac.h"
#include "VariationList.h"
#include "Renderer.h"

/// <summary>
/// SheepTools class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Mutation mode enum.
/// </summary>
enum eMutateMode
{
	MUTATE_NOT_SPECIFIED   = -1,
	MUTATE_ALL_VARIATIONS  = 0,
	MUTATE_ONE_XFORM_COEFS = 1,
	MUTATE_ADD_SYMMETRY    = 2,
	MUTATE_POST_XFORMS     = 3,
	MUTATE_COLOR_PALETTE   = 4,
	MUTATE_DELETE_XFORM    = 5,
	MUTATE_ALL_COEFS       = 6
};

/// <summary>
/// Cross mode enum.
/// </summary>
enum eCrossMode
{
	CROSS_NOT_SPECIFIED = -1,
	CROSS_UNION         = 0,
	CROSS_INTERPOLATE   = 1,
	CROSS_ALTERNATE     = 2
};

/// <summary>
/// SheepTools contains miscellaneous functions for mutating, rotating
/// crossing and randomizing embers. It is named so because these functions
/// are used in the electric sheep genome mutation process.
/// Most functions in this class perform a particular action and return
/// a string describing what it did so it can be recorded in an Xml edit doc
/// to be saved with the ember when converting to Xml.
/// Since it's members can occupy significant memory space and also have
/// hefty initialization sequences, it's important to declare one instance
/// and reuse it for the duration of the program instead of creating and deleting
/// them as local variables.
/// Template argument expected to be float or double.
/// </summary>
template <typename T, typename bucketT>
class EMBER_API SheepTools
{
public:
	/// <summary>
	/// Constructor which takes a palette path and pre-constructed renderer.
	/// This class will take over ownership of the passed in renderer so the
	/// caller should not delete it.
	/// </summary>
	/// <param name="palettePath">The full path and filename of the palette file</param>
	/// <param name="renderer">A pre-constructed renderer to use. The caller should not delete this.</param>
	SheepTools(const string& palettePath, Renderer<T, bucketT>* renderer)
	{
		Timing t;

		m_Smooth = true;
		m_SheepGen = -1;
		m_SheepId = -1;
		m_Stagger = 0;
		m_OffsetX = 0;
		m_OffsetY = 0;

		m_PaletteList.Init(palettePath);
		m_StandardIterator = unique_ptr<StandardIterator<T>>(new StandardIterator<T>());
		m_XaosIterator = unique_ptr<XaosIterator<T>>(new XaosIterator<T>());
		m_Renderer = unique_ptr<Renderer<T, bucketT>>(renderer);
		m_Rand = QTIsaac<ISAAC_SIZE, ISAAC_INT>(ISAAC_INT(t.Tic()), ISAAC_INT(t.Tic() * 2), ISAAC_INT(t.Tic() * 3));
	}

	/// <summary>
	/// Create the linear default ember with a random palette.
	/// </summary>
	/// <returns>The newly constructed linear default ember</returns>
	Ember<T> CreateLinearDefault()
	{
		Ember<T> ember;

		Xform<T> xform1(T(0.25), T(1),    T(0.5), T(1), T(0.5), T(0), T(0), T(0.5), T(0.5),  T(0.25));
		Xform<T> xform2(T(0.25), T(0.66), T(0.5), T(1), T(0.5), T(0), T(0), T(0.5), T(-0.5), T(0.25));
		Xform<T> xform3(T(0.25), T(0.33), T(0.5), T(1), T(0.5), T(0), T(0), T(0.5), T(0.0),  T(-0.5));

		xform1.AddVariation(new LinearVariation<T>());
		xform2.AddVariation(new LinearVariation<T>());
		xform3.AddVariation(new LinearVariation<T>());

		ember.AddXform(xform1);
		ember.AddXform(xform2);
		ember.AddXform(xform3);

		if (m_PaletteList.Init())
			ember.m_Palette = *m_PaletteList.GetPalette(-1);

		return ember;
	}

	/// <summary>
	/// Ensure all xforms, including final, have no more than the specified number of variations.
	/// Remove variations in order of smallest weight to largest weight.
	/// Also remove all xforms whose density is less than 0.001.
	/// </summary>
	/// <param name="ember">The ember whose xforms will be truncated</param>
	/// <param name="maxVars">The maximum number of variations each xform can have</param>
	/// <returns>A string describing what was done</returns>
	string TruncateVariations(Ember<T>& ember, size_t maxVars)
	{
		intmax_t smallest;
		size_t i, j, numVars;
		T sv = 0;
		ostringstream os;

		//First clear out any xforms that are not the final, and have a density of less than 0.001.
		for (i = 0; i < ember.XformCount(); i++)
		{
			Xform<T>* xform = ember.GetXform(i);

			if (xform->m_Weight < T(0.001))
			{
				os << "trunc_density " << i;
				ember.DeleteXform(i);
				i = 0;//Size will have changed, so start over.
			}
		}

		//Now consider all xforms, including final.
		for (i = 0; i < ember.TotalXformCount(); i++)
		{
			Xform<T>* xform = ember.GetTotalXform(i);

			do
			{
				Variation<T>* var = nullptr;
				Variation<T>* smallestVar = nullptr;

				numVars = 0;
				smallest = -1;

				for (j = 0; j < xform->TotalVariationCount(); j++)
				{
					var = xform->GetVariation(j);

					if (var && var->m_Weight != 0.0)
					{
						T v = var->m_Weight;
						numVars++;

						if (smallest == -1 || fabs(v) < sv)
						{
							smallest = j;
							smallestVar = var;
							sv = fabs(v);
						}
					}
				}

				if (numVars > maxVars)
				{
					os << " trunc " << i << " " << smallest;

					if (smallestVar)
						xform->DeleteVariationById(smallestVar->VariationId());
				}
			} while (numVars > maxVars);
		}

		return os.str();
	}

	/// <summary>
	/// Mutate the ember using the specified mode.
	/// </summary>
	/// <param name="ember">The ember to mutate</param>
	/// <param name="mode">The mutation mode</param>
	/// <param name="useVars">The variations to use if the mutation mode is random</param>
	/// <param name="sym">The type of symmetry to add if random specified. If 0, it will be added randomly.</param>
	/// <param name="speed">The speed to multiply the pre affine transforms by if the mutate mode is MUTATE_ALL_COEFS, else ignored.</param>
	/// <returns>A string describing what was done</returns>
	string Mutate(Ember<T>& ember, eMutateMode mode, vector<eVariationId>& useVars, int sym, T speed)
	{
		bool done = false;
		size_t modXform;
		char ministr[32];
		T randSelect;
		ostringstream os;
		Ember<T> mutation;

		mutation.Clear();

		//If mutate_mode = -1, choose a random mutation mode.
		if (mode == MUTATE_NOT_SPECIFIED)
		{
			randSelect = m_Rand.Frand01<T>();

			if (randSelect < T(0.1))
				mode = MUTATE_ALL_VARIATIONS;
			else if (randSelect < T(0.3))
				mode = MUTATE_ONE_XFORM_COEFS;
			else if (randSelect < T(0.5))
				mode = MUTATE_ADD_SYMMETRY;
			else if (randSelect < T(0.6))
				mode = MUTATE_POST_XFORMS;
			else if (randSelect < T(0.7))
				mode = MUTATE_COLOR_PALETTE;
			else if (randSelect < T(0.8))
				mode = MUTATE_DELETE_XFORM;
			else
				mode = MUTATE_ALL_COEFS;
		}

		if (mode == MUTATE_ALL_VARIATIONS)
		{
			os << "mutate all variations";

			do
			{
				//Create a random flame, and use the variations to replace those in the original.
				Random(mutation, useVars, sym, ember.TotalXformCount());

				for (size_t i = 0; i < ember.TotalXformCount(); i++)
				{
					Xform<T>* xform1 = ember.GetTotalXform(i);
					Xform<T>* xform2 = mutation.GetTotalXform(i);

					if (xform1 && xform2)
					{
						for (size_t j = 0; j < xform1->TotalVariationCount(); j++)
						{
							Variation<T>* var1 = xform1->GetVariation(j);
							Variation<T>* var2 = xform2->GetVariationById(var1->VariationId());

							if ((var1 == nullptr) ^ (var2 == nullptr))//If any of them are different, clear the first and copy all of the second and exit the while loop.
							{
								xform1->ClearAndDeleteVariations();

								for (size_t k = 0; k < xform2->TotalVariationCount(); k++)
									xform1->AddVariation(xform2->GetVariation(k)->Copy());

								done = true;
							}
						}
					}
				}
			} while (!done);
		}
		else if (mode == MUTATE_ONE_XFORM_COEFS)
		{
			//Generate a 2-xform random.
			Random(mutation, useVars, sym, 2);

			//Which xform to mutate?
			modXform = m_Rand.Rand() % ember.TotalXformCount();
			Xform<T>* xform1 = ember.GetTotalXform(modXform);
			Xform<T>* xform2 = mutation.GetTotalXform(0);
			os << "mutate xform " << modXform << " coefs";

			//If less than 3 xforms, then change only the translation part.
			if (ember.TotalXformCount() < 2)
			{
				xform1->m_Affine.C(xform2->m_Affine.C());
				xform1->m_Affine.F(xform2->m_Affine.F());
			}
			else
			{
				for (glm::length_t i = 0; i < 2; i++)
					for (glm::length_t j = 0; j < 3; j++)
						xform1->m_Affine.m_Mat[i][j] = xform2->m_Affine.m_Mat[i][j];
			}
		}
		else if (mode == MUTATE_ADD_SYMMETRY)
		{
			os << "mutate symmetry";
			ember.AddSymmetry(0, m_Rand);
		}
		else if (mode == MUTATE_POST_XFORMS)
		{
			bool same = (m_Rand.Rand() & 3) > 0;//25% chance of using the same post for all of them.
			uint b = 1 + m_Rand.Rand() % 6;

			sprintf_s(ministr, 32, "(%d%s)", b, same ? " same" : "");
			os << "mutate post xforms " << ministr;

			for (size_t i = 0; i < ember.TotalXformCount(); i++)
			{
				int copy = (i > 0) && same;
				Xform<T>* xform = ember.GetTotalXform(i);

				if (copy)//Copy the post from the first xform to the rest of them.
				{
					xform->m_Post = ember.GetTotalXform(0)->m_Post;
				}
				else
				{
					//50% chance.
					if (b & 1)
					{
						T f = T(M_PI) * m_Rand.Frand11<T>();
						T ra, rb, rd, re;

						ra = (xform->m_Affine.A() * cos(f) + xform->m_Affine.B() * -sin(f));
						rd = (xform->m_Affine.A() * sin(f) + xform->m_Affine.D() *  cos(f));
						rb = (xform->m_Affine.B() * cos(f) + xform->m_Affine.E() * -sin(f));
						re = (xform->m_Affine.B() * sin(f) + xform->m_Affine.E() *  cos(f));

						xform->m_Affine.A(ra);
						xform->m_Affine.B(rb);
						xform->m_Affine.D(rd);
						xform->m_Affine.E(re);

						f *= -1;

						ra = (xform->m_Post.A() * cos(f) + xform->m_Post.B() * -sin(f));
						rd = (xform->m_Post.A() * sin(f) + xform->m_Post.D() *  cos(f));
						rb = (xform->m_Post.B() * cos(f) + xform->m_Post.E() * -sin(f));
						re = (xform->m_Post.B() * sin(f) + xform->m_Post.E() *  cos(f));

						xform->m_Post.A(ra);
						xform->m_Post.B(rb);
						xform->m_Post.D(rd);
						xform->m_Post.E(re);
					}

					//33% chance.
					if (b & 2)
					{
						T f = T(0.2) + m_Rand.Frand01<T>();
						T g = T(0.2) + m_Rand.Frand01<T>();

						if (m_Rand.RandBit())
							f = 1 / f;

						if (m_Rand.RandBit())
							g = f;
						else
							if (m_Rand.RandBit())
								g = 1 / g;

						xform->m_Affine.A(xform->m_Affine.A() / f);
						xform->m_Affine.D(xform->m_Affine.D() / f);
						xform->m_Affine.B(xform->m_Affine.B() / g);
						xform->m_Affine.E(xform->m_Affine.E() / g);
						xform->m_Post.A(xform->m_Post.A() * f);
						xform->m_Post.B(xform->m_Post.B() * f);
						xform->m_Post.D(xform->m_Post.D() * g);
						xform->m_Post.E(xform->m_Post.E() * g);
					}

					if (b & 4)//16% chance.
					{
						T f = m_Rand.Frand11<T>();
						T g = m_Rand.Frand11<T>();

						xform->m_Affine.C(xform->m_Affine.C() - f);
						xform->m_Affine.F(xform->m_Affine.F() - g);
						xform->m_Post.C(xform->m_Post.C() + f);
						xform->m_Post.F(xform->m_Post.F() + g);
					}
				}
			}
		}
		else if (mode == MUTATE_COLOR_PALETTE)
		{
			T s = m_Rand.Frand01<T>();

			if (s < T(0.4))//Randomize xform color coords.
			{
				ImproveColors(ember, 100, false, 10);
				os << "mutate color coords";
			}
			else if (s < T(0.8))//Randomize xform color coords and palette.
			{
				ImproveColors(ember, 25, true, 10);
				os << "mutate color all";
			}
			else//Randomize palette only.
			{
				Palette<T> palette;

				if (m_PaletteList.Init())
					palette = *m_PaletteList.GetPalette(-1);

				palette.MakeHueAdjustedPalette(ember.m_Palette, ember.m_Hue);

				//If the palette retrieval fails, skip the mutation.
				if (ember.m_Palette.m_Index >= 0)
				{
					os << "mutate color palette";
				}
				else
				{
					palette.Clear(false);
					ember.m_Palette = palette;
					cout << "Failure getting random palette, palette set to white\n";
				}
			}
		}
		else if (mode == MUTATE_DELETE_XFORM)
		{
			size_t nx = m_Rand.Rand() % ember.TotalXformCount();
			os << "mutate delete xform " << nx;

			if (ember.TotalXformCount() > 1)
				ember.DeleteTotalXform(nx);
		}
		else if (mode == MUTATE_ALL_COEFS)
		{
			os << "mutate all coefs";
			Random(mutation, useVars, sym, ember.TotalXformCount());

			//Change all the coefs by a fraction of the random.
			for (size_t x = 0; x < ember.TotalXformCount(); x++)
			{
				Xform<T>* xform1 = ember.GetTotalXform(x);
				Xform<T>* xform2 = mutation.GetTotalXform(x);

				for (glm::length_t i = 0; i < 2; i++)
					for (glm::length_t j = 0; j < 3; j++)
						xform1->m_Affine.m_Mat[i][j] += speed * xform2->m_Affine.m_Mat[i][j];

				//Eventually, mutate the parametric variation parameters here.
			}
		}

		return os.str();
	}

	/// <summary>
	/// Crosse the two embers and place the result in emberOut.
	/// </summary>
	/// <param name="ember0">The first ember to cross</param>
	/// <param name="ember1">The second ember to cross</param>
	/// <param name="emberOut">The result ember</param>
	/// <param name="crossMode">The cross mode</param>
	/// <returns>A string describing what was done</returns>
	string Cross(Ember<T>& ember0, Ember<T>& ember1, Ember<T>& emberOut, int crossMode)
	{
		uint rb;
		size_t i;
		T t;
		ostringstream os;
		char ministr[32];

		if (crossMode == CROSS_NOT_SPECIFIED)
		{
			T s = m_Rand.Frand01<T>();

			if (s < 0.1)
				crossMode = CROSS_UNION;
			else if (s < 0.2)
				crossMode = CROSS_INTERPOLATE;
			else
				crossMode = CROSS_ALTERNATE;
		}

		if (crossMode == CROSS_UNION)
		{
			//Make a copy of the first ember.
			emberOut = ember0;

			//Copy all xforms in the second ember except the final. Default behavior keeps the final from parent0.
			for (i = 0; i < ember1.XformCount(); i++)
				emberOut.AddXform(*ember1.GetXform(i));

			os << "cross union";
		}
		else if (crossMode == CROSS_INTERPOLATE)
		{
			//Linearly interpolate somewhere between the two.
			Ember<T> parents[2];
			//t = 0.5;//If you ever need to test.
			t = m_Rand.Frand01<T>();

			parents[0] = ember0;
			parents[1] = ember1;
			parents[0].m_Time = T(0);
			parents[1].m_Time = T(1);
			Interpolater<T>::Interpolate(parents, 2, t, 0, emberOut);

			for (i = 0; i < emberOut.TotalXformCount(); i++)
				emberOut.GetTotalXform(i)->DeleteMotionElements();

			sprintf_s(ministr, 32, "%7.5g", t);
			os << "cross interpolate " << ministr;
		}
		else//Alternate mode.
		{
			int got0, got1, usedParent;
			string trystr;

			//Each xform comes from a random parent, possible for an entire parent to be excluded.
			do
			{
				got0 = got1 = 0;
				rb = m_Rand.RandBit();
				os << rb << ":";

				//Copy the parent, sorting the final xform to the end if it's present.
				emberOut = rb ? ember1 : ember0;
				usedParent = rb;

				//Only replace non-final xforms.
				for (i = 0; i < emberOut.XformCount(); i++)
				{
					rb = m_Rand.RandBit();

					//Replace xform if bit is 1.
					if (rb == 1)
					{
						if (usedParent == 0)
						{
							if (i < ember1.XformCount() && ember1.GetXform(i)->m_Weight > 0)
							{
								Xform<T>* xform = emberOut.GetXform(i);
								*xform = *ember1.GetXform(i);
								os << " 1";
								got1 = 1;
							}
							else
							{
								os << " 0";
								got0 = 1;
							}
						}
						else
						{
							if (i < ember0.XformCount() && ember0.GetXform(i)->m_Weight > 0)
							{
								Xform<T>* xform = emberOut.GetXform(i);
								*xform = *ember0.GetXform(i);
								os << " 0";
								got0 = 1;
							}
							else
							{
								os << " 1";
								got1 = 1;
							}
						}
					}
					else
					{
						os << " " << usedParent;

						if (usedParent)
							got1 = 1;
						else
							got0 = 1;
					}
				}

				if (usedParent == 0 && ember0.UseFinalXform())
					got0 = 1;
				else if (usedParent == 1 && ember1.UseFinalXform())
					got1 = 1;

			} while ((i > 1) && !(got0 && got1));

			os << "cross alternate ";
			os << trystr;
		}

		//Reset color coords.
		for (i = 0; i < emberOut.TotalXformCount(); i++)
		{
			emberOut.GetTotalXform(i)->m_ColorX = T(i & 1);//Original pingponged between 0 and 1, which gives bad coloring but is useful for testing.
			//emberOut.GetTotalXform(i)->m_ColorX = m_Rand.Frand01<T>();//Do rand which gives better coloring but produces different results every time it's run.
			//emberOut.GetTotalXform(i)->m_ColorY = ?????;//Will need to update this if 2D coordinates are ever supported.
		}

		//Potentially genetically cross the two palettes together.
		if (m_Rand.Frand01<T>() < T(0.4))
		{
			//Select the starting parent.
			size_t ci;
			uint startParent = m_Rand.RandBit();

			os << " cmap_cross " << startParent << ":";

			//Loop over the entries, switching to the other parent 1% of the time.
			for (ci = 0; ci < 256; ci++)//Will need to update this if 2D coordinates are ever supported.
			{
				if (m_Rand.Frand01<T>() < T(.01))
				{
					startParent = 1 - startParent;
					os << " " << ci;
				}

				emberOut.m_Palette.m_Entries[ci] = startParent ? ember1.m_Palette.m_Entries[ci] : ember0.m_Palette.m_Entries[ci];
			}
		}

		return os.str();
	}

	/// <summary>
	/// Thin wrapper around Random() that passes an empty vector for useVars, a random value for symmetry and 0 for max xforms.
	/// </summary>
	/// <param name="ember">The newly created random ember</param>
	void Random(Ember<T>& ember)
	{
		vector<eVariationId> useVars;

		Random(ember, useVars, (int)m_Rand.Frand<T>(-2, 2), 0);
	}

	/// <summary>
	/// Create a random ember.
	/// </summary>
	/// <param name="ember">The newly created random ember</param>
	/// <param name="useVars">A list of variations to use. If empty, any variation can be used.</param>
	/// <param name="sym">The symmetry type to use from -2 to 2</param>
	/// <param name="specXforms">The number of xforms to use. If 0, a quasi random count is used.</param>
	void Random(Ember<T>& ember, vector<eVariationId>& useVars, int sym, size_t specXforms)
	{
		bool postid, addfinal = false;
		int var, samed, multid, samepost;
		glm::length_t i, j, k, n;
		size_t varCount = m_VariationList.Size();
		Palette<T> palette;

		static size_t xformDistrib[] =
		{
			2, 2, 2, 2,
			3, 3, 3, 3,
			4, 4, 4,
			5, 5,
			6
		};

		ember.Clear();
		ember.m_Hue = (m_Rand.Rand() & 7) ? 0 : m_Rand.Frand01<T>();

		if (m_PaletteList.Init())
			palette = *m_PaletteList.GetPalette(-1);

		palette.MakeHueAdjustedPalette(ember.m_Palette, ember.m_Hue);
		ember.m_Time = 0;
		ember.m_Interp = EMBER_INTERP_LINEAR;
		ember.m_PaletteInterp = INTERP_HSV;

		//Choose the number of xforms.
		if (specXforms > 0)
		{
			ember.AddXforms(specXforms);
		}
		else
		{
			ember.AddXforms(xformDistrib[m_Rand.Rand() % Vlen(xformDistrib)]);
			addfinal = m_Rand.Frand01<T>() < T(0.15);//Add a final xform 15% of the time.

			if (addfinal)
			{
				Xform<T> xform;

				xform.m_Affine.A(T(1.1));//Just put something in there so it doesn't show up as being an empty final xform.
				ember.SetFinalXform(xform);
			}
		}

		//If first input variation is -1 random choose one to use or decide to use multiple.
		if (useVars.empty() || useVars[0] == -1)
			var = m_Rand.RandBit() ? m_Rand.Rand() % varCount : -1;
		else
			var = -2;

		samed = m_Rand.RandBit();
		multid = m_Rand.RandBit();
		postid = m_Rand.Frand01<T>() < T(0.6);
		samepost = m_Rand.RandBit();

		//Loop over xforms.
		for (i = 0; i < ember.TotalXformCount(); i++)
		{
			Xform<T>* xform = ember.GetTotalXform(i);

			xform->m_Weight = T(1) / ember.TotalXformCount();
			xform->m_ColorX = m_Rand.Frand01<T>();//Original pingponged between 0 and 1, which gives bad coloring. Ember does random instead.
			xform->m_ColorY = m_Rand.Frand01<T>();//Will need to update this if 2D coordinates are ever supported.
			xform->m_ColorSpeed = T(0.5);
			xform->m_Animate = 1;
			xform->m_Affine.A(m_Rand.Frand11<T>());
			xform->m_Affine.B(m_Rand.Frand11<T>());
			xform->m_Affine.C(m_Rand.Frand11<T>());
			xform->m_Affine.D(m_Rand.Frand11<T>());
			xform->m_Affine.E(m_Rand.Frand11<T>());
			xform->m_Affine.F(m_Rand.Frand11<T>());
			xform->m_Post.MakeID();

			if (!ember.IsFinalXform(xform))
			{
				if (!postid)
				{
					for (j = 0; j < 2; j++)
					{
						for (k = 0; k < 3; k++)
						{
							if (samepost || (i == 0))
								xform->m_Post.m_Mat[j][k] = m_Rand.Frand11<T>();
							else
								xform->m_Post.m_Mat[j][k] = ember.GetTotalXform(0)->m_Post.m_Mat[j][k];
						}
					}
				}

				if (var > -1)
				{
					xform->AddVariation(m_VariationList.GetVariation(var)->Copy());//Use only one variation specified for all xforms.
				}
				else if (multid && var == -1)
				{
					xform->AddVariation(m_VariationList.GetVariation(m_Rand.Rand() % varCount)->Copy());//Choose a random var for this xform.
				}
				else
				{
					if (samed && i > 0)
					{
						//Copy the same variations from the previous xform.
						Xform<T>* prevXform = ember.GetXform(i - 1);

						for (j = 0; j < prevXform->TotalVariationCount(); j++)
							xform->AddVariation(prevXform->GetVariation(j)->Copy());
					}
					else
					{
						//Choose a random number of vars to use, at least 2
						//but less than varCount. Probability leans
						//towards fewer variations.
						n = 2;
						while (m_Rand.RandBit() && (n < varCount))
							n++;

						//Randomly choose n variations, and change their weights.
						//A var can be selected more than once, further reducing
						//the probability that multiple vars are used.
						for (j = 0; j < n; j++)
						{
							if (var != -2)
							{
								//Pick a random variation and use a random weight from 0-1.
								Variation<T>* v = m_VariationList.GetVariationCopy((size_t)(m_Rand.Rand() % varCount), m_Rand.Frand<T>(T(0.001), 1));

								if (v && !xform->AddVariation(v))
									delete v;//It already existed and therefore was not added.
							}
							else
							{
								//Pick a random variation from the suppled IDs and use a random weight from 0-1.
								Variation<T>* v = m_VariationList.GetVariationCopy(useVars[m_Rand.Rand() % useVars.size()], m_Rand.Frand<T>(T(0.001), 1));

								if (v && !xform->AddVariation(v))
									delete v;
							}
						}

						xform->NormalizeVariationWeights();//Normalize weights to 1.0 total.
					}
				}
			}
			else
			{
				//Handle final xform randomness.
				n = 1;

				if (m_Rand.RandBit())
					n++;

				//Randomly choose n variations, and change their weights.
				//A var can be selected more than once, further reducing
				//the probability that multiple vars are used.
				for (j = 0; j < n; j++)
				{
					if (var != -2)
					{
						//Pick a random variation and use a random weight from 0-1.
						xform->AddVariation(m_VariationList.GetVariationCopy((size_t)(m_Rand.Rand() % varCount), m_Rand.Frand<T>(T(0.001), 1)));
					}
					else
					{
						//Pick a random variation from the suppled IDs and use a random weight from 0-1.
						xform->AddVariation(m_VariationList.GetVariationCopy(useVars[m_Rand.Rand() % useVars.size()], m_Rand.Frand<T>(T(0.001), 1)));
					}
				}

				xform->NormalizeVariationWeights();//Normalize weights to 1.0 total.
			}

			//Randomize parametric variations.
			for (j = 0; j < xform->TotalVariationCount(); j++)
				xform->GetVariation(j)->Random(m_Rand);
		}

		//Randomly add symmetry (but not if we've already added a final xform).
		if (sym || (!(m_Rand.Rand() % 4) && !addfinal))
			ember.AddSymmetry(sym, m_Rand);
		else
			ember.m_Symmetry = 0;
	}

	/// <summary>
	/// Attempt to make colors better by doing some test renders.
	/// </summary>
	/// <param name="ember">The ember to render</param>
	/// <param name="tries">The number of test renders to try before giving up</param>
	/// <param name="changePalette">Change palette if true, else keep trying with the same palette.</param>
	/// <param name="colorResolution">The resolution of the test histogram. This value ^ 3 will be used for the total size. Common value is 10.</param>
	void ImproveColors(Ember<T>& ember, int tries, bool changePalette, int colorResolution)
	{
		int i;
		T best, b;
		Ember<T> bestEmber = ember;

		best = TryColors(ember, colorResolution);

		if (best < 0)
		{
			cout << "Error in TryColors(), skipping ImproveColors()" << endl;
			return;
		}

		for (i = 0; i < tries; i++)
		{
			ChangeColors(ember, changePalette);
			b = TryColors(ember, colorResolution);

			if (b < 0)
			{
				cout << "Error in TryColors, aborting tries." << endl;
				break;
			}

			if (b > best)
			{
				best = b;
				bestEmber = ember;
			}
		}

		ember = bestEmber;
	}

	/// <summary>
	/// Run a test render to improve the colors.
	/// </summary>
	/// <param name="ember">The ember to render</param>
	/// <param name="colorResolution">The resolution of the test histogram. This value ^ 3 will be used for the total size. Common value is 10.</param>
	/// <returns>The number of histogram cells that weren't black</returns>
	T TryColors(Ember<T>& ember, int colorResolution)
	{
		byte* p;
		size_t i, hits = 0, res = colorResolution;
		size_t pixTotal, res3 = res * res * res;
		T scalar;
		Ember<T> adjustedEmber = ember;

		adjustedEmber.m_Quality = 1;
		adjustedEmber.m_Supersample = 1;
		adjustedEmber.m_MaxRadDE = 0;

		//Scale the image so that the total number of pixels is ~10,000.
		pixTotal = ember.m_FinalRasW * ember.m_FinalRasH;
		scalar = sqrt(T(10000) / pixTotal);
		adjustedEmber.m_FinalRasW = (size_t)(ember.m_FinalRasW  * scalar);
		adjustedEmber.m_FinalRasH = (size_t)(ember.m_FinalRasH  * scalar);
		adjustedEmber.m_PixelsPerUnit *= scalar;
		adjustedEmber.m_TemporalSamples = 1;

		m_Renderer->SetEmber(adjustedEmber);
		m_Renderer->BytesPerChannel(1);
		m_Renderer->EarlyClip(true);
		m_Renderer->PixelAspectRatio(1);
		m_Renderer->ThreadCount(Timing::ProcessorCount());
		m_Renderer->Callback(nullptr);

		if (m_Renderer->Run(m_FinalImage) != RENDER_OK)
		{
			cout << "Error rendering test image for TryColors().  Aborting." << endl;
			return -1;
		}

		m_Hist.resize(res3);
		memset(m_Hist.data(), 0, res3);
		
		p = m_FinalImage.data();

		for (i = 0; i < m_Renderer->FinalDimensions(); i++)
		{
			m_Hist[(p[0] * res / 256) +
				   (p[1] * res / 256) * res +
				   (p[2] * res / 256) * res * res]++;
			p += m_Renderer->NumChannels();
		}

		for (i = 0; i < res3; i++)
		{
			if (m_Hist[i])
				hits++;
		}

		return T(hits / res3);
	}

	/// <summary>
	/// Change around color coordinates. Optionall change out the entire palette.
	/// </summary>
	/// <param name="ember">The ember whose xform's color coordinates will be changed</param>
	/// <param name="changePalette">Change palette if true, else don't</param>
	void ChangeColors(Ember<T>& ember, bool changePalette)
	{
		size_t i;
		Xform<T>* xform0;
		Xform<T>* xform1;

		if (changePalette)
		{
			Palette<T>* palette = nullptr;

			ember.m_Hue = 0.0;

			if (m_PaletteList.Init())
				palette = m_PaletteList.GetPalette(-1);

			if (palette)
			{
				palette->MakeHueAdjustedPalette(ember.m_Palette, ember.m_Hue);
			}
			else
			{
				ember.m_Palette.Clear(false);
				cout << "Error retrieving random palette, setting to all white" << endl;
			}
		}

		for (i = 0; i < ember.TotalXformCount(); i++)
		{
			ember.GetTotalXform(i)->m_ColorX = m_Rand.Frand01<T>();
			ember.GetTotalXform(i)->m_ColorY = m_Rand.Frand01<T>();
		}

		xform0 = RandomXform(ember, -1);
		xform1 = RandomXform(ember, ember.GetXformIndex(xform0));

		if (xform0 && (m_Rand.Rand() & 1))
		{
			xform0->m_ColorX = 0;
			xform0->m_ColorY = 0;
		}

		if (xform1 && (m_Rand.Rand() & 1))
		{
			xform1->m_ColorX = 1;
			xform1->m_ColorY = 1;
		}
	}

	/// <summary>
	/// Try to get a random xform from the ember, including final, whose density is non-zero.
	/// Give up after 100 tries.
	/// </summary>
	/// <param name="ember">The ember to get a random xform from</param>
	/// <param name="excluded">Optionally exclude an xform. Pass -1 to include all for consideration.</param>
	/// <returns>The random xform if successful, else nullptr.</returns>
	Xform<T>* RandomXform(Ember<T>& ember, intmax_t excluded)
	{
		size_t ntries = 0;

		while (ntries++ < 100)
		{
			size_t i = m_Rand.Rand() % ember.TotalXformCount();

			if (i != excluded)
			{
				Xform<T>* xform = ember.GetTotalXform(i);

				if (xform->m_Weight > 0)
					return xform;
			}
		}

		return nullptr;
	}

	/// <summary>
	/// Rotate affine transforms and optionally apply motion elements,
	/// and store the result in rotated.
	/// </summary>
	/// <param name="ember">The ember to rotate</param>
	/// <param name="rotated">The rotated xform</param>
	/// <param name="blend">The time percentage value which dictates how much of a percentage of 360 degrees it should be rotated and the time position for the motion elements</param>
	void Loop(Ember<T>& ember, Ember<T>& rotated, T blend)
	{
		rotated = ember;

		//Insert motion magic here :
		//If there are motion elements, modify the contents of
		//the result xforms before rotate is called.
		for (size_t i = 0; i < ember.TotalXformCount(); i++)
		{
			Xform<T>* xform1 = ember.GetTotalXform(i);
			Xform<T>* xform2 = rotated.GetTotalXform(i);

			if (!xform1->m_Motion.empty())
				xform2->ApplyMotion(*xform1, blend);

			xform2->DeleteMotionElements();
		}

		rotated.RotateAffines(-blend * 360);//Rotate the affines.
	}

	/// <summary>
	/// Interpolate two embers and place the output in result.
	/// The embers parameter is expected to be a pointer to an array of at least 2 elements.
	/// </summary>
	/// <param name="embers">The embers to interpolate</param>
	/// <param name="result">The result of the interpolation</param>
	/// <param name="blend">The interpolation time</param>
	/// <param name="seqFlag">True if embers points to the first or last ember in the entire sequence, else false.</param>
	void Edge(Ember<T>* embers, Ember<T>& result, T blend, bool seqFlag)
	{
		size_t i, si;
		Ember<T> spun[2], prealign[2];

		//Insert motion magic here :
		//If there are motion elements, modify the contents of
		//the result xforms before rotate is called.
		for (si = 0; si < 2; si++)
		{
			prealign[si] = embers[si];

			for (i = 0; i < embers[si].TotalXformCount(); i++)
			{
				Xform<T>* xform = embers[si].GetTotalXform(i);

				if (!xform->m_Motion.empty())
					xform->ApplyMotion(*(prealign[si].GetTotalXform(i)), blend);//Apply motion parameters to result.xform[i] using blend parameter.
			}
		}

		//Use the un-padded original for blend=0 when creating a sequence.
		//This keeps the original interpolation type intact.
		if (seqFlag && blend == 0)
		{
			result = prealign[0];
		}
		else
		{
			//Align what's going to be interpolated.
			Interpolater<T>::Align(prealign, spun, 2);

			spun[0].m_Time = 0;
			spun[1].m_Time = 1;

			//Call this first to establish the asymmetric reference angles.
			Interpolater<T>::AsymmetricRefAngles(spun, 2);

			//Rotate the aligned xforms.
			spun[0].RotateAffines(-blend * 360);
			spun[1].RotateAffines(-blend * 360);

			Interpolater<T>::Interpolate(spun, 2, m_Smooth ? Interpolater<T>::Smoother(blend) : blend, m_Stagger, result);
		}

		//Make sure there are no motion elements in the result.
		result.DeleteMotionElements();
	}

	/// <summary>
	/// Spin the specified ember, optionally apply a template ember, and place the output in result.
	/// Create auto-generated name
	/// Append edits using the Nick, Url and Id members.
	/// Apply subpixel jitter to center using offset members.
	/// </summary>
	/// <param name="parent">The ember to spin</param>
	/// <param name="templ">The template to apply if not nullptr, else ignore.</param>
	/// <param name="result">The result of the spin</param>
	/// <param name="frame">The frame in the sequence to be stored in the m_Time member of result</param>
	/// <param name="blend">The interpolation time</param>
	void Spin(Ember<T>& parent, Ember<T>* templ, Ember<T>& result, int frame, T blend)
	{
		char temp[50];

		//Spin the parent blend degrees.
		Loop(parent, result, blend);

		//Apply the template if necessary.
		if (templ)
			ApplyTemplate(result, *templ);

		//Set ember parameters accordingly.
		result.m_Time = T(frame);
		result.m_Interp = EMBER_INTERP_LINEAR;
		result.m_PaletteInterp = INTERP_HSV;

		//Create the edit doc xml.
		sprintf_s(temp, 50, "rotate %g", blend * 360.0);
		result.ClearEdit();
		result.m_Edits = m_EmberToXml.CreateNewEditdoc(&parent, nullptr, temp, m_Nick, m_Url, m_Id, m_Comment, m_SheepGen, m_SheepId);

		//Subpixel jitter.
		Offset(result, m_OffsetX, m_OffsetY);

		//Make the name of the flame the time.
		sprintf_s(temp, 50, "%f", result.m_Time);
		result.m_Name = string(temp);
	}

	/// <summary>
	/// Call Edge() on parents, optionally apply a template ember, and place the output in result.
	/// Create auto-generated name
	/// Append edits using the Nick, Url and Id members.
	/// Apply subpixel jitter to center using offset members.
	/// </summary>
	/// <param name="parents">The embers to interpolate</param>
	/// <param name="templ">The template to apply if not nullptr, else ignore.</param>
	/// <param name="result">The result of the spin</param>
	/// <param name="frame">The frame in the sequence to be stored in the m_Time member of result</param>
	/// <param name="seqFlag">True if embers points to the first or last ember in the entire sequence, else false.</param>
	/// <param name="blend">The interpolation time</param>
	void SpinInter(Ember<T>* parents, Ember<T>* templ, Ember<T>& result, int frame, bool seqFlag, T blend)
	{
		char temp[50];

		//Interpolate between spun parents.
		Edge(parents, result, blend, seqFlag);

		//Original did an interpolated palette hack here for random palettes, but it was never used anywhere so ember omits it.//ORIG

		//Apply the template if necessary.
		if (templ)
			ApplyTemplate(result, *templ);

		//Set ember parameters accordingly.
		result.m_Time = T(frame);

		//Create the edit doc xml.
		sprintf_s(temp, 50, "interpolate %g", blend * 360.0);
		result.ClearEdit();
		result.m_Edits = m_EmberToXml.CreateNewEditdoc(&parents[0], &parents[1], temp, m_Nick, m_Url, m_Id, m_Comment, m_SheepGen, m_SheepId);

		//Subpixel jitter.
		Offset(result, m_OffsetX, m_OffsetY);

		//Make the name of the flame the time.
		sprintf_s(temp, 50, "%f", result.m_Time);
		result.m_Name = string(temp);
	}

	/// <summary>
	/// Apply a template to an ember.
	/// </summary>
	/// <param name="ember">The ember to apply the template to</param>
	/// <param name="templ">The template to apply</param>
	void ApplyTemplate(Ember<T>& ember, Ember<T>& templ)
	{
		//Check for invalid values - only replace those with valid ones.
		for (glm::length_t i = 0; i < 3; i++)
			if (templ.m_Background[i] >= 0)
				ember.m_Background[i] = templ.m_Background[i];

		if (templ.m_Zoom < 999999998)
			ember.m_Zoom = templ.m_Zoom;

		if (templ.m_Supersample > 0)
			ember.m_Supersample = templ.m_Supersample;

		if (templ.m_SpatialFilterRadius >= 0)
			ember.m_SpatialFilterRadius = templ.m_SpatialFilterRadius;

		if (templ.m_Quality > 0)
			ember.m_Quality = templ.m_Quality;

		if (templ.m_TemporalSamples > 0)
			ember.m_TemporalSamples = templ.m_TemporalSamples;

		if (templ.m_FinalRasW > 0)
		{
			//Preserving scale should be an option.
			ember.m_PixelsPerUnit = ember.m_PixelsPerUnit * templ.m_FinalRasW / ember.m_FinalRasW;
			ember.m_FinalRasW = templ.m_FinalRasW;
		}

		if (templ.m_FinalRasH > 0)
			ember.m_FinalRasH = templ.m_FinalRasH;

		if (templ.m_MaxRadDE >= 0)
			ember.m_MaxRadDE = templ.m_MaxRadDE;

		if (templ.m_MinRadDE >= 0)
			ember.m_MinRadDE = templ.m_MinRadDE;

		if (templ.m_CurveDE >= 0)
			ember.m_CurveDE = templ.m_CurveDE;

		if (templ.m_GammaThresh >= 0)
			ember.m_GammaThresh = templ.m_GammaThresh;

		if (templ.m_SpatialFilterType > 0)
			ember.m_SpatialFilterType = templ.m_SpatialFilterType;

		if (templ.m_Interp >= 0)
			ember.m_Interp = templ.m_Interp;

		if (templ.m_AffineInterp >= 0)
			ember.m_AffineInterp = templ.m_AffineInterp;

		if (templ.m_TemporalFilterType >= 0)
			ember.m_TemporalFilterType = templ.m_TemporalFilterType;

		if (templ.m_TemporalFilterWidth > 0)
			ember.m_TemporalFilterWidth = templ.m_TemporalFilterWidth;

		if (templ.m_TemporalFilterExp > -900)
			ember.m_TemporalFilterExp = templ.m_TemporalFilterExp;

		if (templ.m_HighlightPower >= 0)
			ember.m_HighlightPower = templ.m_HighlightPower;

		if (templ.m_PaletteMode >= 0)
			ember.m_PaletteMode = templ.m_PaletteMode;
	}

	/// <summary>
	/// Move the center of the ember by the specified amount.
	/// </summary>
	/// <param name="ember">The ember to move</param>
	/// <param name="offsetX">The x offset.</param>
	/// <param name="offsetY">The y offset.</param>
	void Offset(Ember<T>& ember, T offsetX, T offsetY)
	{
		if (!IsNearZero<T>(offsetX))
			ember.m_CenterX += offsetX / (ember.m_PixelsPerUnit * ember.m_Supersample);

		if (!IsNearZero<T>(offsetY))
			ember.m_CenterY += offsetY / (ember.m_PixelsPerUnit * ember.m_Supersample);
	}

	/// <summary>
	/// Translate the first center point by the second, rotate it, translate back.
	/// </summary>
	/// <param name="newCenterX">The new center x</param>
	/// <param name="newCenterY">The new center y</param>
	/// <param name="oldCenterX">The old center x</param>
	/// <param name="oldCenterY">The old center y</param>
	/// <param name="by">The angle to rotate by</param>
	void RotateOldCenterBy(T& newCenterX, T& newCenterY, T oldCenterX, T oldCenterY, T by)
	{
		T r[2];
		T th = by * 2 * T(M_PI) / 360;
		T c = cos(th);
		T s = -sin(th);

		newCenterX -= oldCenterX;
		newCenterY -= oldCenterY;
		r[0] = c * newCenterX - s * newCenterY;
		r[1] = s * newCenterX + c * newCenterY;
		newCenterX = r[0] + oldCenterX;
		newCenterY = r[1] + oldCenterY;
	}

	/// <summary>
	/// Find a 2D bounding box that does not enclose eps of the fractal density in each compass direction.
	/// This will run the inner loops of iteration without all of the surrounding interpolation and filtering.
	/// </summary>
	/// <param name="ember">The ember to iterate</param>
	/// <param name="eps">The eps</param>
	/// <param name="samples">The number samples to iterate</param>
	/// <param name="bmin">The bmin[0] and bmin[1] will be the minimum x and y values.</param>
	/// <param name="bmax">The bmax[0] and bmax[1] will be the maximum x and y values.</param>
	/// <returns>The number of iterations ran</returns>
	size_t EstimateBoundingBox(Ember<T>& ember, T eps, size_t samples, T* bmin, T* bmax)
	{
		bool newAlloc = false;
		size_t i, lowTarget, highTarget;
		T min[2], max[2];
		IterParams<T> params;

		m_Renderer->SetEmber(ember);
		m_Renderer->CreateSpatialFilter(newAlloc);
		m_Renderer->CreateDEFilter(newAlloc);
		m_Renderer->ComputeBounds();
		m_Renderer->ComputeCamera();

		if (ember.XaosPresent())
			m_Iterator = m_XaosIterator.get();
		else
			m_Iterator = m_StandardIterator.get();

		m_Iterator->InitDistributions(ember);
		m_Samples.resize(samples);
		params.m_Count = samples;
		params.m_Skip = 20;
		//params.m_OneColDiv2 = m_Renderer->CoordMap()->OneCol() / 2;
		//params.m_OneRowDiv2 = m_Renderer->CoordMap()->OneRow() / 2;

		size_t bv = m_Iterator->Iterate(ember, params, m_Samples.data(), m_Rand);//Use a special fuse of 20, all other calls to this will use 15, or 100.

		if (bv / T(samples) > eps)
			eps = 3 * bv / T(samples);

		if (eps > T(0.3))
			eps = T(0.3);

		lowTarget = (size_t)(samples * eps);
		highTarget = samples - lowTarget;

		min[0] = min[1] =  1e10;
		max[0] = max[1] = -1e10;

		for (i = 0; i < samples; i++)
		{
			if (m_Samples[i].m_X < min[0]) min[0] = m_Samples[i].m_X;
			if (m_Samples[i].m_Y < min[1]) min[1] = m_Samples[i].m_Y;
			if (m_Samples[i].m_X > max[0]) max[0] = m_Samples[i].m_X;
			if (m_Samples[i].m_Y > max[1]) max[1] = m_Samples[i].m_Y;
		}

		if (lowTarget == 0)
		{
			bmin[0] = min[0];
			bmin[1] = min[1];
			bmax[0] = max[0];
			bmax[1] = max[1];

			return bv;
		}

		std::sort(m_Samples.begin(), m_Samples.end(), &SortPointByX<T>);
		bmin[0] = m_Samples[lowTarget].m_X;
		bmax[0] = m_Samples[highTarget].m_X;

		std::sort(m_Samples.begin(), m_Samples.end(), &SortPointByY<T>);
		bmin[1] = m_Samples[lowTarget + 1].m_Y;
		bmax[1] = m_Samples[highTarget + 1].m_Y;

		return bv;
	}

	/// <summary>
	/// When doing spin or edge, an edit doc is made to record what was done.
	/// Doing so takes many extra parameters such as name and url. Passing these every
	/// time is cumbersome and are unlikely to change for the duration of a program run, so
	/// they are made to be member variables. After setting these, their values will be used
	/// in all edits within this class.
	/// </summary>
	/// <param name="smooth">Use smoothing if true, else false</param>
	/// <param name="stagger">Use stagger if > 0, else false</param>
	/// <param name="offsetX">X amount of subpixel jitter to apply in Spin() and Edge()</param>
	/// <param name="offsetY">Y amount of subpixel jitter to apply in Spin() and Edge()</param>
	/// <param name="nick">The nickname of the author</param>
	/// <param name="url">The Url of the author</param>
	/// <param name="id">The id of the author</param>
	/// <param name="comment">The comment to include</param>
	/// <param name="sheepGen">The sheep generation used if > 0. Default: 0.</param>
	/// <param name="sheepId">The sheep id used if > 0. Default: 0.</param>
	void SetSpinParams(bool smooth, T stagger, T offsetX, T offsetY, string nick, string url, string id, string comment, int sheepGen, int sheepId)
	{
		m_Smooth = smooth;
		m_SheepGen = sheepGen;
		m_SheepId = sheepId;
		m_Stagger = stagger;
		m_OffsetX = offsetX;
		m_OffsetY = offsetY;
		m_Nick = nick;
		m_Url = url;
		m_Id = id;
		m_Comment = comment;
	}

private:
	bool m_Smooth;
	int m_SheepGen;
	int m_SheepId;
	T m_Stagger;
	T m_OffsetX;
	T m_OffsetY;
	string m_Nick;
	string m_Url;
	string m_Id;
	string m_Comment;

	vector<Point<T>> m_Samples;
	vector<byte> m_FinalImage;
	vector<uint> m_Hist;
	EmberToXml<T> m_EmberToXml;
	Iterator<T>* m_Iterator;
	unique_ptr<StandardIterator<T>> m_StandardIterator;
	unique_ptr<XaosIterator<T>> m_XaosIterator;
	unique_ptr<Renderer<T, bucketT>> m_Renderer;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> m_Rand;
	PaletteList<T> m_PaletteList;
	VariationList<T> m_VariationList;
};
}
