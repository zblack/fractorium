#pragma once

#include "VariationList.h"
#include "Interpolate.h"

/// <summary>
/// Xform class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Xform and Variation need each other, but each can't include the other.
/// So Xform includes this file, and Ember is declared as a forward declaration here.
/// </summary>
template <typename T> class Ember;

/// <summary>
/// If both polymorphism and templating are needed, uncomment this, fill it out and derive from it.
/// </summary>
//class EMBER_API XformBase
//{
//};

/// <summary>
/// An xform is a pre affine transform, a list of variations, and an optional final affine transform.
/// This is what gets applied to a point for each iteration.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Xform
{
public:
	/// <summary>
	/// Default constructor which calls Init() to set default values.
	/// Pre and post affine are defaulted to the identity matrix.
	/// </summary>
	Xform()
	{
		Init();
	}

	/// <summary>
	/// Constructor that takes default arguments. Mostly used for testing.
	/// Post affine is defaulted to the identity matrix.
	/// </summary>
	/// <param name="density">The probability that this xform is chosen</param>
	/// <param name="colorX">The color index</param>
	/// <param name="colorSpeed">The color speed</param>
	/// <param name="opacity">The opacity</param>
	/// <param name="a">The a value of the pre affine transform</param>
	/// <param name="d">The d value of the pre affine transform</param>
	/// <param name="b">The b value of the pre affine transform</param>
	/// <param name="e">The e value of the pre affine transform</param>
	/// <param name="c">The c value of the pre affine transform</param>
	/// <param name="f">The f value of the pre affine transform</param>
	/// <param name="pa">The a value of the post affine transform. Default: 1.</param>
	/// <param name="pd">The d value of the post affine transform. Default: 0.</param>
	/// <param name="pb">The b value of the post affine transform. Default: 0.</param>
	/// <param name="pe">The e value of the post affine transform. Default: 1.</param>
	/// <param name="pc">The c value of the post affine transform. Default: 0.</param>
	/// <param name="pf">The f value of the post affine transform. Default: 0.</param>
	Xform(T weight, T colorX, T colorSpeed, T opacity,
		  T a, T d, T b, T e, T c, T f,
		  T pa = 1,
		  T pd = 0,
		  T pb = 0,
		  T pe = 1,
		  T pc = 0,
		  T pf = 0)

	{
		Init();

		m_Weight = weight;
		m_ColorX = colorX;
		m_ColorSpeed = colorSpeed;
		m_Opacity = opacity;

		m_Affine.A(a);
		m_Affine.B(b);
		m_Affine.C(c);
		m_Affine.D(d);
		m_Affine.E(e);
		m_Affine.F(f);

		m_Post.A(pa);
		m_Post.B(pb);
		m_Post.C(pc);
		m_Post.D(pd);
		m_Post.E(pe);
		m_Post.F(pf);
		m_HasPost = !m_Post.IsID();
		m_HasPreOrRegularVars = PreVariationCount() > 0 || VariationCount() > 0;

		CacheColorVals();//Init already called this, but must call again since color was assigned above.
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="xform">The Xform object to copy</param>
	Xform(const Xform<T>& xform)
		: m_ParentEmber(nullptr)//Hack.
	{
		Xform<T>::operator=<T>(xform);
	}

	/// <summary>
	/// Copy constructor to copy an Xform object of type U.
	/// </summary>
	/// <param name="xform">The Xform object to copy</param>
	template <typename U>
	Xform(const Xform<U>& xform)
		: m_ParentEmber(nullptr)//Hack.
	{
		Xform<T>::operator=<U>(xform);
	}

	/// <summary>
	/// Deletes each element of the variation vector and clears it.
	/// </summary>
	~Xform()
	{
		ClearAndDeleteVariations();
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="xform">The Xform object to copy</param>
	Xform<T>& operator = (const Xform<T>& xform)
	{
		if (this != &xform)
			Xform<T>::operator=<T>(xform);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Xform object of type U.
	/// This will delete all of the variations in the vector
	/// and repopulate it with copes of the variation in xform's vector.
	/// All other values are assigned directly.
	/// </summary>
	/// <param name="xform">The Xform object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Xform<T>& operator = (const Xform<U>& xform)
	{
		m_Affine = xform.m_Affine;
		m_Post = xform.m_Post;
		m_Weight = T(xform.m_Weight);
		m_ColorX = T(xform.m_ColorX);
		m_ColorY = T(xform.m_ColorY);
		m_DirectColor = T(xform.m_DirectColor);
		m_ColorSpeed = T(xform.m_ColorSpeed);
		m_Animate = T(xform.m_Animate);
		m_Opacity = T(xform.m_Opacity);
		CacheColorVals();
		m_HasPost = xform.HasPost();
		m_HasPreOrRegularVars = xform.PreVariationCount() > 0 || xform.VariationCount() > 0;
		m_Wind[0] = T(xform.m_Wind[0]);
		m_Wind[1] = T(xform.m_Wind[1]);
		m_MotionFreq = xform.m_MotionFreq;
		m_MotionFunc = xform.m_MotionFunc;

		ClearAndDeleteVariations();

		//Must manually add them via the AddVariation() function so that
		//the variation's m_IndexInXform member gets properly set to this.
		for (unsigned int i = 0; i < xform.TotalVariationCount(); i++)
		{
			Variation<T>* var = nullptr;

			if (Variation<U>* varOrig = xform.GetVariation(i))
			{
				varOrig->Copy(var);//Will convert from type U to type T.
				AddVariation(var);//Will internally call SetPrecalcFlags().
			}
		}

		if (TotalVariationCount() == 0)
			SetPrecalcFlags();

		//If this xform was already part of a different ember, then do not assign, else do.
		if (!m_ParentEmber && (typeid(T) == typeid(U)))
			m_ParentEmber = (Ember<T>*)xform.ParentEmber();

		CopyVec<T, U>(m_Xaos, xform.XaosVec());
		CopyVec(m_Motion, xform.m_Motion);
		m_Name = xform.m_Name;

		return *this;
	}

	/// <summary>
	/// Init default values.
	/// </summary>
	void Init()
	{
		static unsigned int count = 0;

		m_Weight = 0;
		m_ColorSpeed = T(0.5);
		m_Animate = 1;
		m_ColorX = T(count & 1);
		m_ColorY = 0;
		m_DirectColor = 1;
		m_Opacity = 1;

		m_Affine.A(1);
		m_Affine.B(0);
		m_Affine.C(0);
		m_Affine.D(0);
		m_Affine.E(1);
		m_Affine.F(0);

		m_Post.A(1);
		m_Post.B(0);
		m_Post.C(0);
		m_Post.D(0);
		m_Post.E(1);
		m_Post.F(0);

		m_Wind[0] = 0;
		m_Wind[1] = 0;
		m_MotionFreq = 0;
		m_MotionFunc = MOTION_SIN;
		m_Motion.clear();

		m_NeedPrecalcSumSquares = false;
		m_NeedPrecalcSqrtSumSquares = false;
		m_NeedPrecalcAngles = false;
		m_NeedPrecalcAtanXY = false;
		m_NeedPrecalcAtanYX = false;
		m_HasPost = false;
		m_HasPreOrRegularVars = false;
		m_ParentEmber = nullptr;
		m_PreVariations.reserve(MAX_VARS_PER_XFORM);
		m_Variations.reserve(MAX_VARS_PER_XFORM);
		m_PostVariations.reserve(MAX_VARS_PER_XFORM);
		CacheColorVals();
		count++;
	}

	/// <summary>
	/// Add a pointer to a variation which will be deleted on destruction so the caller should not delete.
	/// This checks if the total number of variations is less than or equal to MAX_VARS_PER_XFORM.
	/// It also checks if the variation is already present, in which case it doesn't add.
	/// If add, set all precalcs.
	/// </summary>
	/// <param name="variation">Pointer to a varation to add</param>
	/// <returns>True if the successful, else false.</returns>
	bool AddVariation(Variation<T>* variation)
	{
		if (variation && (GetVariationById(variation->VariationId()) == nullptr))
		{
			string name = variation->Name();
			bool pre = name.find("pre_") == 0;
			bool post = name.find("post_") == 0;
			vector<Variation<T>*>* vec;

			if (pre)
				vec = &m_PreVariations;
			else if (post)
				vec = &m_PostVariations;
			else
				vec = &m_Variations;

			if (vec->size() < MAX_VARS_PER_XFORM)
			{
				vec->push_back(variation);

				//Flatten must always be last.
				for (size_t i = 0; i < vec->size(); i++)
				{
					if ((i != vec->size() - 1) && ((*vec)[i]->Name().find("flatten") != string::npos))
					{
						std::swap((*vec)[i], (*vec)[vec->size() - 1]);
						break;
					}
				}

				SetPrecalcFlags();
				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// Get a pointer to the variation at the specified index.
	/// </summary>
	/// <param name="index">The index in the list to retrieve</param>
	/// <returns>A pointer to the variation at the index if in range, else nullptr.</returns>
	Variation<T>* GetVariation(size_t index) const
	{
		size_t count = 0;
		Variation<T>* var = nullptr;

		const_cast<Xform<T>*>(this)->AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++, count++)
			{
				if (count == index)
				{
					var = variations[i];
					keepGoing = false;
					break;
				}
			}
		});

		return var;
	}

	/// <summary>
	/// Get a pointer to the variation with the specified ID.
	/// </summary>
	/// <param name="id">The ID to search for</param>
	/// <returns>A pointer to the variation if found, else nullptr.</returns>
	Variation<T>* GetVariationById(eVariationId id) const
	{
		Variation<T>* var = nullptr;

		const_cast<Xform<T>*>(this)->AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++)
			{
				if (variations[i] != nullptr && variations[i]->VariationId() == id)
				{
					var = variations[i];
					keepGoing = false;
					break;
				}
			}
		});

		return var;
	}

	/// <summary>
	/// Get a pointer to the variation with the specified name.
	/// </summary>
	/// <param name="name">The name to search for</param>
	/// <returns>A pointer to the variation if found, else nullptr.</returns>
	Variation<T>* GetVariationByName(string name) const
	{
		Variation<T>* var = nullptr;

		const_cast<Xform<T>*>(this)->AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++)
			{
				if (variations[i] != nullptr && variations[i]->Name() == name)
				{
					var = variations[i];
					keepGoing = false;
					break;
				}
			}
		});

		return var;
	}

	/// <summary>
	/// Get the index in the list of the variation pointer.
	/// Note this is searching for the exact pointer address and not the name or ID of the variation.
	/// </summary>
	/// <param name="var">A pointer to the variation to search for</param>
	/// <returns>The index of the variation if found, else -1</returns>
	int GetVariationIndex(Variation<T>* var) const
	{
		int count = 0, index = -1;

		const_cast<Xform<T>*>(this)->AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++, count++)
			{
				if (variations[i] == var)
				{
					index = count;
					keepGoing = false;
					break;
				}
			}
		});

		return index;
	}

	/// <summary>
	/// Delete the variation with the matching ID.
	/// Update precalcs if deletion successful.
	/// </summary>
	/// <param name="id">The ID to search for</param>
	/// <returns>True if deletion successful, else false.</returns>
	bool DeleteVariationById(eVariationId id)
	{
		bool found = false;

		AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++)
			{
				if (variations[i] != nullptr && variations[i]->VariationId() == id)
				{
					delete variations[i];
					variations.erase(variations.begin() + i);
					found = true;
				}
			}
		});

		if (found)
			SetPrecalcFlags();

		return found;
	}

	/// <summary>
	/// Delete the motion elements.
	/// </summary>
	void DeleteMotionElements()
	{
		m_Motion.clear();
	}

	/// <summary>
	/// Delete all variations, clear the list and update precalc flags.
	/// </summary>
	void ClearAndDeleteVariations()
	{
		AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing) { ClearVec<Variation<T>>(variations); });
		SetPrecalcFlags();
	}

	/// <summary>
	/// Reset this xform to be totally empty by clearing all variations, resetting both affines to the
	/// identity matrix, clearing xaos, color, visibility, wind, animate and setting name
	/// to the empty string.
	/// Note that this also sets the parent ember to nullptr, so if this xform is reused after calling Clear(),
	/// the caller must reset the parent ember to whatever ember they add it to again.
	/// </summary>
	void Clear()
	{
		ClearAndDeleteVariations();
		DeleteMotionElements();
		m_Affine.MakeID();
		m_Post.MakeID();
		m_Xaos.clear();
		m_ParentEmber = nullptr;
		m_ColorSpeedCache = 0;
		m_OneMinusColorCache = 0;
		m_VizAdjusted = 0;
		m_Animate = 0;
		m_Wind[0] = 0;
		m_Wind[1] = 0;
		m_Name = "";
	}

	/// <summary>
	/// Compute color cache values: color speed, one minus color speed and adjusted visibility.
	/// </summary>
	void CacheColorVals()
	{
		//Figure out which is right. //TODO.
		//m_ColorSpeedCache = m_ColorX * (1 - m_ColorSpeed) / 2;//Apo style.
		//m_OneMinusColorCache = (1 + m_ColorSpeed) / 2;

		m_ColorSpeedCache = m_ColorSpeed * m_ColorX;//Flam3 style.
		m_OneMinusColorCache = T(1.0) - m_ColorSpeed;
		m_VizAdjusted = AdjustOpacityPercentage(m_Opacity);
	}

	/// <summary>
	/// Return the xaos value at the specified index.
	/// If the index is out of range, return 1.
	/// This has the convenient effect that xaos is not present
	/// by default and only has a value if explicitly added.
	/// </summary>
	/// <param name="i">The xaos index to retrieve</param>
	/// <returns>The value at the index if in range, else 1.</returns>
	T Xaos(size_t i) const
	{
		return i < m_Xaos.size() ? m_Xaos[i] : 1;
	}

	/// <summary>
	/// Set the xaos value for a given xform index.
	/// If the index is out of range, a 1 value will be added
	/// to the xaos vector repeatedly until it's one less than the
	/// requested index in length, then finally add the specified value.
	/// </summary>
	/// <param name="i">The index to set</param>
	/// <param name="val">The xaos value to set it to</param>
	void SetXaos(unsigned int i, T val)
	{
		if (i < m_Xaos.size())
		{
			m_Xaos[i] = val;
		}
		else
		{
			while (m_Xaos.size() <= i)
				m_Xaos.push_back(1);

			m_Xaos[i] = val;
		}
	}

	/// <summary>
	/// Determine if any xaos value in the vector up to the xform count
	/// of the parent ember is anything other than 1.
	/// </summary>
	/// <returns>True if found, else false.</returns>
	bool XaosPresent() const
	{
		if (m_ParentEmber)
			for (unsigned int i = 0; i < m_Xaos.size(); i++)
				if (i < m_ParentEmber->XformCount())
					if (!IsClose<T>(m_Xaos[i], 1))
						return true;//If at least one entry is not equal to 1, then xaos is present.

		return false;
	}

	/// <summary>
	/// Truncate the xaos vector to match the xform count of the parent ember.
	/// </summary>
	void TruncateXaos()
	{
		if (m_ParentEmber)
			while (m_Xaos.size() > m_ParentEmber->XformCount())
				m_Xaos.pop_back();
	}

	/// <summary>
	/// Remove all xaos from this xform.
	/// </summary>
	void ClearXaos()
	{
		m_Xaos.clear();
	}

	/// <summary>
	/// Normalize the variation weights.
	/// </summary>
	void NormalizeVariationWeights()
	{
		AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			T norm = 0;

			ForEach(variations, [&](Variation<T>* var) { norm += var->m_Weight; });
			ForEach(variations, [&](Variation<T>* var) { var->m_Weight /= norm; });
		});
	}

	/// <summary>
	/// Applies this xform to the point passed in and saves the result in the out point.
	/// It's important to understand what happens here since it's the inner core of the algorithm.
	/// See the internal comments for step by step details.
	/// </summary>
	/// <param name="inPoint">The initial point from the previous iteration</param>
	/// <param name="outPoint">The output point</param>
	/// <param name="rand">The random context to use</param>
	/// <returns>True if a bad value was calculated, else false.</returns>
	bool Apply(Point<T>* inPoint, Point<T>* outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t i;

		//This must be local, rather than a member, because this function can be called
		//from multiple threads. If it were a member, they'd be clobbering each others' values.
		IteratorHelper<T> iterHelper;

		//Calculate the color coordinate/index in the palette to look up later when accumulating the output point
		//to the histogram. Calculate this value by interpolating between the index value of the
		//last iteration with the one specified in this xform. Note that some cached values are used
		//to reduce the amount of processing.
		outPoint->m_VizAdjusted = m_VizAdjusted;
		iterHelper.m_Color.x = outPoint->m_ColorX = m_ColorSpeedCache + (m_OneMinusColorCache * inPoint->m_ColorX);

		//This modification returns the affine transformed points if no variations are present.
		//Note this differs from flam3, which would just return zero in that scenario.
		if (m_HasPreOrRegularVars)
		{
			//Compute the pre affine portion of the transform.
			//These x, y values are what get passed to the variations below.
			//Note that they are not changed after this, except in the case of pre_ variations.
			iterHelper.m_TransX = (m_Affine.A() * inPoint->m_X) + (m_Affine.B() * inPoint->m_Y) + m_Affine.C();
			iterHelper.m_TransY = (m_Affine.D() * inPoint->m_X) + (m_Affine.E() * inPoint->m_Y) + m_Affine.F();
			iterHelper.m_TransZ = inPoint->m_Z;

			//Apply pre_ variations, these don't affect outPoint, only iterHelper.m_TransX, Y, Z.
			for (i = 0; i < PreVariationCount(); i++)
			{
				iterHelper.In.x = iterHelper.m_TransX;//Read must be done before every pre variation because transX/Y are changing.
				iterHelper.In.y = iterHelper.m_TransY;
				iterHelper.In.z = iterHelper.m_TransZ;
				m_PreVariations[i]->PrecalcHelper(iterHelper, inPoint);//Apply per-variation precalc, the second parameter is unused for pre variations.
				m_PreVariations[i]->Func(iterHelper, *outPoint, rand);
				WritePre(iterHelper, m_PreVariations[i]->AssignType());
			}

			if (VariationCount() > 0)
			{
				//The original calculates sumsq and sumsqrt every time, regardless if they're used or not.
				//With Precalc(), only calculate those values if they're needed.
				Precalc(iterHelper);//Only need per-xform precalc with regular variations.
				iterHelper.In.x = iterHelper.m_TransX;//Only need to read once with regular variations, because transX/Y are fixed.
				iterHelper.In.y = iterHelper.m_TransY;
				iterHelper.In.z = iterHelper.m_TransZ;

				//Since these get summed, initialize them to zero.
				outPoint->m_X = outPoint->m_Y = outPoint->m_Z = 0;

				//Apply variations to the transformed points, accumulating each time, and store the final value in outPoint.
				//Using a virtual function is about 3% faster than using a large case statement like the original did.
				//Although research says that using virtual functions is slow, experience says otherwise. They execute
				//with the exact same speed as both regular and static member functions.
				for (i = 0; i < VariationCount(); i++)
				{
					m_Variations[i]->Func(iterHelper, *outPoint, rand);
					outPoint->m_X += iterHelper.Out.x;
					outPoint->m_Y += iterHelper.Out.y;
					outPoint->m_Z += iterHelper.Out.z;
				}
			}
			else//Only pre variations are present, no regular ones, so assign the affine transformed points directly to the output points.
			{
				outPoint->m_X = iterHelper.m_TransX;
				outPoint->m_Y = iterHelper.m_TransY;
				outPoint->m_Z = iterHelper.m_TransZ;
			}
		}
		else
		{
			//There are no variations, so the affine transformed points can be assigned directly to the output points.
			T inX = inPoint->m_X;

			outPoint->m_X = (m_Affine.A() * inX) + (m_Affine.B() * inPoint->m_Y) + m_Affine.C();
			outPoint->m_Y = (m_Affine.D() * inX) + (m_Affine.E() * inPoint->m_Y) + m_Affine.F();
			outPoint->m_Z = inPoint->m_Z;
		}

		//Apply post variations, these will modify outPoint.
		for (i = 0; i < PostVariationCount(); i++)
		{
			iterHelper.In.x = outPoint->m_X;//Read must be done before every post variation because the out point is changing.
			iterHelper.In.y = outPoint->m_Y;
			iterHelper.In.z = outPoint->m_Z;
			m_PostVariations[i]->PrecalcHelper(iterHelper, outPoint);//Apply per-variation precalc.
			m_PostVariations[i]->Func(iterHelper, *outPoint, rand);
			WritePost(iterHelper, *outPoint, m_PostVariations[i]->AssignType());
		}

		//Optionally apply the post affine transform if it's present.
		if (m_HasPost)
		{
			T postX = outPoint->m_X;

			outPoint->m_X = (m_Post.A() * postX) + (m_Post.B() * outPoint->m_Y) + m_Post.C();
			outPoint->m_Y = (m_Post.D() * postX) + (m_Post.E() * outPoint->m_Y) + m_Post.F();
		}

		outPoint->m_ColorX = outPoint->m_ColorX + m_DirectColor * (iterHelper.m_Color.x - outPoint->m_ColorX);

		//Has the trajectory of x or y gone either to infinity, or too close to zero?
		return BadVal(outPoint->m_X) || BadVal(outPoint->m_Y)/* || BadVal(outPoint->m_Z)*/;
	}

//Why are we not using template with member var addr as arg here?//TODO
#define APPMOT(x)  do { x += mot[i].x * Interpolater<T>::MotionFuncs(func, freq * blend); } while (0)

	/// <summary>
	/// Apply the motion functions from the passed in xform to this xform.
	/// </summary>
	/// <param name="xform">The xform containing the motion functions</param>
	/// <param name="blend">The time blending value 0-1</param>
	void ApplyMotion(Xform<T>& xform, T blend)
	{
		unsigned int i, j, k;
		Xform<T>* mot = xform.m_Motion.data();

		//Loop over the motion elements and add their contribution to the original vals.
		for (i = 0; i < xform.m_Motion.size(); i++)
		{
			//Original only pulls these from the first motion xform which is a bug. Want to pull it from each one.
			Xform<T>* currentMot = &xform.m_Motion[i];
			int freq = currentMot->m_MotionFreq;
			eMotion func = currentMot->m_MotionFunc;

			//Clamp these to the appropriate range after all are applied.
			APPMOT(m_Weight);
			APPMOT(m_ColorX);
			//APPMOT(m_ColorY);
			APPMOT(m_DirectColor);
			APPMOT(m_Opacity);
			APPMOT(m_ColorSpeed);
			APPMOT(m_Animate);

			for (j = 0; j < currentMot->TotalVariationCount(); j++)//For each variation in the motion xform.
			{
				Variation<T>* motVar = currentMot->GetVariation(j);//Get the variation, which may or may not be present in this xform.
				ParametricVariation<T>* motParVar = dynamic_cast<ParametricVariation<T>*>(motVar);

				Variation<T>* var = GetVariationById(motVar->VariationId());//See if the variation in the motion xform was present in the xform.

				if (!var)//It wasn't present, so add it and set the weight.
				{
					Variation<T>* newVar = motVar->Copy();
					newVar->m_Weight = motVar->m_Weight * Interpolater<T>::MotionFuncs(func, freq * blend);
					AddVariation(newVar);
					var = newVar;//Use this below for params.
				}
				else//It was present, so apply the motion func to the weight.
				{
					var->m_Weight += motVar->m_Weight * Interpolater<T>::MotionFuncs(func, freq * blend);
				}

				//At this point, we've added if needed, or just applied the motion func to the weight.
				//Now apply the motion func to the params if needed.
				if (motParVar != nullptr)
				{
					ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);
					ParamWithName<T>* params = parVar->Params();
					ParamWithName<T>* motParams = motParVar->Params();

					for (k = 0; k < motParVar->ParamCount(); k++)
					{
						if (!motParams[k].IsPrecalc())
							*(params[k].Param()) += motParams[k].ParamVal() * Interpolater<T>::MotionFuncs(func, freq * blend);
					}
				}
			}

			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 3; k++)
				{
					APPMOT(m_Affine.m_Mat[j][k]);
					APPMOT(m_Post.m_Mat[j][k]);
				}
			}
		}

		//Make sure certain params are within reasonable bounds.
		ClampRef<T>(m_ColorX, 0, 1);
		//ClampRef<T>(m_ColorY, 0, 1);
		ClampRef<T>(m_DirectColor, 0, 1);
		ClampRef<T>(m_Opacity, 0, 1);//Original didn't clamp these, but do it here for correctness.
		ClampRef<T>(m_ColorSpeed, 0, 1);
		ClampGte0Ref<T>(m_Weight);
	}

	/// <summary>
	/// Accessors.
	/// The precalc flags are duplicated in each variation. Each value here
	/// is true if any of the variations need it precalculated.
	/// </summary>
	inline bool NeedPrecalcSumSquares()     const { return m_NeedPrecalcSumSquares;     }
	inline bool NeedPrecalcSqrtSumSquares() const { return m_NeedPrecalcSqrtSumSquares; }
	inline bool NeedPrecalcAngles()         const { return m_NeedPrecalcAngles;         }
	inline bool NeedPrecalcAtanXY()         const { return m_NeedPrecalcAtanXY;         }
	inline bool NeedPrecalcAtanYX()         const { return m_NeedPrecalcAtanYX;         }
	inline bool NeedAnyPrecalc()            const { return NeedPrecalcSumSquares() || NeedPrecalcSqrtSumSquares() || NeedPrecalcAngles() || NeedPrecalcAtanXY() || NeedPrecalcAtanYX(); }
	bool HasPost() const { return m_HasPost; }
	unsigned int PreVariationCount()   const { return (unsigned int)m_PreVariations.size();  }
	unsigned int VariationCount()      const { return (unsigned int)m_Variations.size();     }
	unsigned int PostVariationCount()  const { return (unsigned int)m_PostVariations.size(); }
	unsigned int TotalVariationCount() const { return PreVariationCount() + VariationCount() + PostVariationCount(); }
	bool Empty() const { return TotalVariationCount() == 0 && m_Affine.IsID(); }//Use this instead of padding like the original did.
	T VizAdjusted() const { return m_VizAdjusted; }
	T ColorSpeedCache() const { return m_ColorSpeedCache; }
	T OneMinusColorCache() const { return m_OneMinusColorCache; }
	const vector<T>& XaosVec() const { return m_Xaos; }
	Ember<T>* ParentEmber() const { return m_ParentEmber; }
	void ParentEmber(Ember<T>* ember) { m_ParentEmber = ember; }
	int IndexInParentEmber() { return m_ParentEmber ? m_ParentEmber->GetTotalXformIndex(this) : -1; }

	/// <summary>
	/// Set the precalc flags based on whether any variation in the vector needs them.
	/// Also call Precalc() virtual function on each variation, which will setup any needed
	/// precalcs in parametric variations.
	/// Set the parent xform of each variation to this.
	/// </summary>
	void SetPrecalcFlags()
	{
		m_NeedPrecalcSumSquares = false;
		m_NeedPrecalcSqrtSumSquares = false;
		m_NeedPrecalcAngles = false;
		m_NeedPrecalcAtanXY = false;
		m_NeedPrecalcAtanYX = false;
		m_HasPost = !m_Post.IsID();
		m_HasPreOrRegularVars = PreVariationCount() > 0 || VariationCount() > 0;

		//Only set precalcs for regular variations, they work differently for pre and post.
		for (size_t i = 0; i < m_Variations.size(); i++)
		{
			if (m_Variations[i]->NeedPrecalcSumSquares())
				m_NeedPrecalcSumSquares = true;

			if (m_Variations[i]->NeedPrecalcSqrtSumSquares())
				m_NeedPrecalcSqrtSumSquares = true;

			if (m_Variations[i]->NeedPrecalcAngles())
				m_NeedPrecalcAngles = true;

			if (m_Variations[i]->NeedPrecalcAtanXY())
				m_NeedPrecalcAtanXY = true;

			if (m_Variations[i]->NeedPrecalcAtanYX())
				m_NeedPrecalcAtanYX = true;
		}

		AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (size_t i = 0; i < variations.size(); i++)
			{
				/*if (variations[i]->NeedPrecalcSumSquares())
					m_NeedPrecalcSumSquares = true;

				if (variations[i]->NeedPrecalcSqrtSumSquares())
					m_NeedPrecalcSqrtSumSquares = true;

				if (variations[i]->NeedPrecalcAngles())
					m_NeedPrecalcAngles = true;

				if (variations[i]->NeedPrecalcAtanXY())
					m_NeedPrecalcAtanXY = true;

				if (variations[i]->NeedPrecalcAtanYX())
					m_NeedPrecalcAtanYX = true;*/

				variations[i]->ParentXform(this);
				variations[i]->Precalc();
			}
		});
	}

	/// <summary>
	/// Based on the precalc flags determined in SetPrecalcFlags(), do the appropriate precalcs.
	/// </summary>
	/// <param name="helper">The iterator helper to store the precalculated values in</param>
	void Precalc(IteratorHelper<T>& helper)
	{
		if (m_NeedPrecalcSumSquares)
		{
			helper.m_PrecalcSumSquares = SQR(helper.m_TransX) + SQR(helper.m_TransY);

			if (m_NeedPrecalcSqrtSumSquares)
			{
				helper.m_PrecalcSqrtSumSquares = sqrt(helper.m_PrecalcSumSquares);

				if (m_NeedPrecalcAngles)
				{
					helper.m_PrecalcSina = helper.m_TransX / Zeps(helper.m_PrecalcSqrtSumSquares);
					helper.m_PrecalcCosa = helper.m_TransY / Zeps(helper.m_PrecalcSqrtSumSquares);
				}
			}
		}

		if (m_NeedPrecalcAtanXY)
			helper.m_PrecalcAtanxy = atan2(helper.m_TransX, helper.m_TransY);

		if (m_NeedPrecalcAtanYX)
			helper.m_PrecalcAtanyx = atan2(helper.m_TransY, helper.m_TransX);
	}

	/// <summary>
	/// Flatten this xform by adding a flatten variation if none is present, and if any of the
	/// variations or parameters in the vector are present.
	/// </summary>
	/// <param name="names">Vector of variation and parameter names</param>
	/// <returns>True if flatten was added, false if it already was present or if none of the specified variations or parameters were present.</returns>
	bool Flatten(vector<string>& names)
	{
		bool shouldFlatten = true;

		if (GetVariationById(VAR_FLATTEN) == nullptr)
		{
			AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
			{
				for (size_t i = 0; i < variations.size(); i++)
				{
					Variation<T>* var = variations[i];

					if (var->m_Weight != 0)//This should never happen, but just to be safe.
					{
						if (FindIf(names, [&] (const string& s) -> bool { return !_stricmp(s.c_str(), var->Name().c_str()); }))
						{
							shouldFlatten = false;
							keepGoing = false;
							break;
						}
					}

					//Now traverse the parameters for this variation.
					if (ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var))
					{
						ForEach(names, [&] (const string& s)
						{
							if (parVar->GetParamVal(s.c_str()) != 0)
							{
								shouldFlatten = false;
								keepGoing = false;
							}
						});
					}
				}
			});

			if (shouldFlatten)
			{
				Variation<T>* var = new FlattenVariation<T>();

				if (AddVariation(var))
				{
					return true;
				}
				else
				{
					delete var;
					return false;
				}
			}
		}

		return false;
	}

	/// <summary>
	/// Generate the OpenCL string for reading input values to
	/// be passed to a variation.
	/// </summary>
	/// <param name="varType">Type of the variation these values will be passed to.</param>
	/// <returns>The OpenCL string</returns>
	string ReadOpenCLString(eVariationType varType)
	{
		string s;

		switch (varType)
		{
		case VARTYPE_REG:
		case VARTYPE_PRE:
			s =
				"\tvIn.x = transX;\n"
				"\tvIn.y = transY;\n"
				"\tvIn.z = transZ;\n";
			break;
		case VARTYPE_POST:
		default:
			s =
				"\tvIn.x = outPoint->m_X;\n"
				"\tvIn.y = outPoint->m_Y;\n"
				"\tvIn.z = outPoint->m_Z;\n";
			break;
		}

		return s;
	}

	/// <summary>
	/// Assing output values from the result of a pre variation.
	/// </summary>
	/// <param name="helper">The helper to store the output values in</param>
	/// <param name="assignType">The type of assignment this variation uses, assign or sum.</param>
	inline void WritePre(IteratorHelper<T>& helper, eVariationAssignType assignType)
	{
		switch (assignType)
		{
			case ASSIGNTYPE_SET:
			{
				helper.m_TransX = helper.Out.x;
				helper.m_TransY = helper.Out.y;
				helper.m_TransZ = helper.Out.z;
				break;
			}
			case ASSIGNTYPE_SUM:
			default:
			{
				helper.m_TransX += helper.Out.x;
				helper.m_TransY += helper.Out.y;
				helper.m_TransZ += helper.Out.z;
				break;
			}
		}
	}

	/// <summary>
	/// Assing output values from the result of a post variation.
	/// </summary>
	/// <param name="helper">The helper to store the output values in</param>
	/// <param name="assignType">The type of assignment this variation uses, assign or sum.</param>
	inline void WritePost(IteratorHelper<T>& helper, Point<T>& outPoint, eVariationAssignType assignType)
	{
		switch (assignType)
		{
			case ASSIGNTYPE_SET:
			{
				outPoint.m_X = helper.Out.x;
				outPoint.m_Y = helper.Out.y;
				outPoint.m_Z = helper.Out.z;
				break;
			}
			case ASSIGNTYPE_SUM:
			default:
			{
				outPoint.m_X += helper.Out.x;
				outPoint.m_Y += helper.Out.y;
				outPoint.m_Z += helper.Out.z;
				break;
			}
		}
	}

	/// <summary>
	/// Generate the OpenCL string for writing output values from a call to a variation.
	/// </summary>
	/// <param name="varType">The type of variation these values were calculated from, pre, reg or post.</param>
	/// <param name="assignType">The type of assignment used by the variation these values were calculated from, assign or sum.</param>
	/// <returns>The OpenCL string</returns>
	string WriteOpenCLString(eVariationType varType, eVariationAssignType assignType)
	{
		string s;

		switch (varType)
		{
			case VARTYPE_REG:
			{
				s =
				"\toutPoint->m_X += vOut.x;\n"
				"\toutPoint->m_Y += vOut.y;\n"
				"\toutPoint->m_Z += vOut.z;\n";
				break;
			}
			case VARTYPE_PRE:
			{
				switch (assignType)
				{
					case ASSIGNTYPE_SET:
					{
						s =
						"\ttransX = vOut.x;\n"
						"\ttransY = vOut.y;\n"
						"\ttransZ = vOut.z;\n";
						break;
					}
					case ASSIGNTYPE_SUM:
					default:
					{
						s =
						"\ttransX += vOut.x;\n"
						"\ttransY += vOut.y;\n"
						"\ttransZ += vOut.z;\n";
						break;
					}
				}

				break;
			}
			case VARTYPE_POST:
			default:
			{
				switch (assignType)
				{
					case ASSIGNTYPE_SET:
					{
						s =
						"\toutPoint->m_X = vOut.x;\n"
						"\toutPoint->m_Y = vOut.y;\n"
						"\toutPoint->m_Z = vOut.z;\n";
						break;
					}
					case ASSIGNTYPE_SUM:
					default:
					{
						s =
						"\toutPoint->m_X += vOut.x;\n"
						"\toutPoint->m_Y += vOut.y;\n"
						"\toutPoint->m_Z += vOut.z;\n";
						break;
					}
				}

				break;
			}
		}

		return s;
	}

	/// <summary>
	/// Return a string representation of this xform.
	/// It will include all pre affine values, and optionally post affine values if present.
	/// Various variables, all variations as strings and xaos values if present.
	/// </summary>
	/// <returns>The string representation of this xform</returns>
	string ToString() const
	{
		ostringstream ss;

		ss << "A: " << m_Affine.A() << " "
		   << "B: " << m_Affine.B() << " "
		   << "C: " << m_Affine.C() << " "
		   << "D: " << m_Affine.D() << " "
		   << "E: " << m_Affine.E() << " "
		   << "F: " << m_Affine.F() << " " << endl;

		if (m_HasPost)
		{
			ss << "Post A: " << m_Post.A() << " "
			   << "Post B: " << m_Post.B() << " "
			   << "Post C: " << m_Post.C() << " "
			   << "Post D: " << m_Post.D() << " "
			   << "Post E: " << m_Post.E() << " "
			   << "Post F: " << m_Post.F() << " " << endl;
		}

		ss << "Weight: " << m_Weight << endl;
		ss << "ColorX: " << m_ColorX << endl;
		ss << "ColorY: " << m_ColorY << endl;
		ss << "Direct Color: " << m_DirectColor << endl;
		ss << "Color Speed: " << m_ColorSpeed << endl;
		ss << "Animate: " << m_Animate << endl;
		ss << "Opacity: " << m_Opacity << endl;
		ss << "Viz Adjusted: " << m_VizAdjusted << endl;
		ss << "Wind: " << m_Wind[0] << ", " << m_Wind[1] << endl;
		ss << "Motion Frequency: " << m_MotionFreq << endl;
		ss << "Motion Func: " << m_MotionFunc << endl;

		const_cast<Xform<T>*>(this)->AllVarsFunc([&] (vector<Variation<T>*>& variations, bool& keepGoing)
		{
			for (unsigned int i = 0; i < variations.size(); i++)
				ss << variations[i]->ToString() << endl;

			ss << endl;
		});

		if (XaosPresent())
		{
			for (unsigned int i = 0; i < m_Xaos.size(); i++)
				ss << m_Xaos[i] << " ";

			ss << endl;
		}

		return ss.str();
	}

	/// <summary>
	/// Members are listed in the exact order they are used in Apply() to make them
	/// as cache efficient as possible. Not all are public, so there is repeated public/private
	/// access specifiers.
	/// </summary>

private:
	bool m_HasPreOrRegularVars;//Whethere there are any pre or regular variations present.
	T m_VizAdjusted;//Adjusted visibility for better transitions.

public:
	//Color coordinates for this function. This is the index into the palette used to look up a color and add to the histogram for each iter.
	//The original only allows for an x coord. Will eventually allow for a y coord like Fractron for 2D palettes.
	T m_ColorX, m_ColorY;

private:
	T m_ColorSpeedCache;//Cache of m_ColorSpeed * m_ColorX. Need to recalc cache values whenever anything relating to color is set. Made private because one affects the other.
	T m_OneMinusColorCache;//Cache of 1 - m_ColorSpeedCache.

public:
	//Coefficients for the affine portion of the transform.
	//Discussed on page 3 of the paper:
	//Fi(x, y) = (aix + biy + ci, dix + eiy + fi)
	Affine2D<T> m_Affine;

private:
	vector<Variation<T>*> m_PreVariations;//The list of pre variations to call when applying this xform.
	vector<Variation<T>*> m_Variations;//The list of variations to call when applying this xform.
	bool m_HasPost;//Whether a post affine transform is present.

public:
	//Coefficients for the affine portion of the post transform.
	//Discussed on page 5 of the paper:
	//Pi(x, y) = (αix + βiy + γi, δix + ǫiy + ζi).
	Affine2D<T> m_Post;

private:
	vector<Variation<T>*> m_PostVariations;//The list of post variations to call when applying this xform.

public:
	T m_DirectColor;//Used with direct color variations.

	//Probability that this function is chosen. Can be greater than 1.
	//Discussed on page 4 of the paper:
	//Probability wi.
	T m_Weight;

	//Scaling factor on color added to current iteration, also known as color weight. Normally defaults to 0.5.
	//Discussed on page 9 of the paper with a hard coded default value of 0.5:
	//C = (C + Ci) * m_ColorSpeed.
	T m_ColorSpeed;
	T m_Opacity;//How much of this xform is seen. Range: 0.0 (invisible) - 1.0 (totally visible).
	T m_Animate;//Whether or not this xform rotates during animation. 0 means stationary, > 0 means rotate. Use T instead of bool so it can be interpolated.
	T m_Wind[2];
	eMotion m_MotionFunc;
	int m_MotionFreq;
	vector<Xform<T>> m_Motion;
	string m_Name;

private:
	/// <summary>
	/// Perform an operation on all variation vectors.
	/// The operation is supplied in the func parameter.
	/// To stop performing the operation on vectors after the current one,
	/// set the keepGoing parameter to false;
	/// </summary>
	/// <param name="func">The function to call for each variation vector.</param>
	void AllVarsFunc(std::function<void (vector<Variation<T>*>&, bool&)> func)
	{
		bool keepGoing = true;

		func(m_PreVariations, keepGoing);

		if (keepGoing)
			func(m_Variations, keepGoing);

		if (keepGoing)
			func(m_PostVariations, keepGoing);
	}

	/// <summary>
	/// Adjust opacity.
	/// </summary>
	/// <param name="in">The opacity to adjust, range 0-1.</param>
	/// <returns>The adjusted opacity</returns>
	static T AdjustOpacityPercentage(T in)
	{
		if (in == 0)
			return 0;
		else
			return pow(T(10.0), -log(T(1.0) / T(in)) / log(T(2)));
	}

	vector<T> m_Xaos;//Xaos vector which affects the probability that this xform is chosen. Usually empty.
	Ember<T>* m_ParentEmber;//The parent ember that contains this xform.
	bool m_NeedPrecalcSumSquares;//Whether any variation uses the precalc sum squares value in its calculations.
	bool m_NeedPrecalcSqrtSumSquares;//Whether any variation uses the sqrt precalc sum squares value in its calculations.
	bool m_NeedPrecalcAngles;//Whether any variation uses the precalc sin and cos values in its calculations.
	bool m_NeedPrecalcAtanXY;//Whether any variation uses the precalc atan XY value in its calculations.
	bool m_NeedPrecalcAtanYX;//Whether any variation uses the precalc atan YX value in its calculations.
};
}
