#pragma once

#include "Ember.h"

/// <summary>
/// Iterator and derived classes.
/// </summary>

//#define CHOOSE_XFORM_GRAIN 256
#define CHOOSE_XFORM_GRAIN 10000//The size of xform random selection buffer. Multiply by the (number of non-final xforms present + 1) if xaos is used.

namespace EmberNs
{
#define ITERATORUSINGS \
	using Iterator<T>::NextXformFromIndex; \
	using Iterator<T>::DoFinalXform; \
	using Iterator<T>::DoBadVals;

/// <summary>
/// Iterator base class.
/// Iterating is one loop level outside of the inner xform application loop so it's still very important
/// to take every optimization possible here.
/// The original had many temporary assignments in order to feed the output of the current iteration
/// into the input of the next iteration. All unneccessary temporary assignments are eliminated by simply using i and i + 1
/// as the input and output indices on the samples array passed to Xform.Apply().
/// Note that the samples array is assigned to while fusing. Although this technically doesn't make sense
/// since values computed during fusing get thrown out, it doesn't matter because it will get overwritten
/// in the actual loop below it since the index counter is reset to zero when fusing is complete.
/// Flam3 needlessly computed the final xform on each fuse iteration only to throw it away. It's omitted here as an optimization.
/// Rather than place many conditionals inside the iteration loop, they are broken into separate classes depending
/// on what's contained in the ember's xforms.
/// The biggest difference is whether xaos is present or not it requires extra work when picking
/// the next random xform to use. Further, each of those is broken into two loops, one for embers with a final xform
/// and one without.
/// Last, the fuse loop and real loop are separated and duplicated to omit the conditional check for fuse inside the real loop.
/// Although this makes this file about four times as verbose as it would normally be, it does lead to performance improvements.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Iterator
{
public:
	/// <summary>
	/// Empty constructor.
	/// </summary>
	Iterator()
	{
	}

	/// <summary>
	/// Empty virtual destructor so proper derived class destructors get called.
	/// </summary>
	virtual ~Iterator()
	{
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	const unsigned char* XformDistributions() const { return m_XformDistributions.empty() ? nullptr : &m_XformDistributions[0]; }
	const size_t   XformDistributionsSize() const { return m_XformDistributions.size(); }

	/// <summary>
	/// Virtual empty iteration function that will be overidden in derived iterator classes.
	/// </summary>
	/// <param name="ember">The ember whose xforms will be applied</param>
	/// <param name="count">The number of iterations to do</param>
	/// <param name="skip">The number of times to fuse</param>
	/// <param name="samples">The buffer to store the output points</param>
	/// <param name="rand">The random context to use</param>
	/// <returns>The number of bad values</returns>
	virtual size_t Iterate(Ember<T>& ember, size_t count, size_t skip, Point<T>* samples, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) { return 0; }

	/// <summary>
	/// Initialize the xform selection vector by normalizing the weights of all xforms and
	/// setting the corresponding percentage of elements in the vector to each xform's index in its
	/// parent ember.
	/// Note that this method of looking up and index in a vector is how flam3 did it and is about 10%
	/// faster than using a while loop to check a random number against a normalized weight.
	/// Also, the ember used to initialize this must be the same ember, unchanged, used to iterate.
	/// If one is passed to this function, its parameters are changed and then it's passed to Iterate(),
	/// the behavior is undefined.
	/// </summary>
	/// <param name="ember">The ember whose xforms will be used to populate the distribution vector</param>
	/// <returns>True if success, else false.</returns>
	bool InitDistributions(Ember<T>& ember)
	{
		size_t i;
		size_t distribCount = ember.XaosPresent() ? ember.XformCount() + 1 : 1;
		const Xform<T>* xforms = ember.Xforms();

		if (m_XformDistributions.size() < CHOOSE_XFORM_GRAIN * distribCount)
			m_XformDistributions.resize(CHOOSE_XFORM_GRAIN * distribCount);

		if (m_XformDistributions.size() < CHOOSE_XFORM_GRAIN * distribCount)
			return false;

		for (size_t distrib = 0; distrib < distribCount; distrib++)
		{
			T totalDensity = 0;

			//First find the total densities of all xforms.
			for (i = 0; i < ember.XformCount(); i++)
			{
				T d = xforms[i].m_Weight;

				if (distrib > 0)
					d *= xforms[distrib - 1].Xaos(i);

				totalDensity += d;
			}

			//Original returned false if all were 0, but it's allowed here
			//which will just end up setting all elements to 0 which means
			//only the first xform will get used.

			//Calculate how much of a fraction of a the total density each element represents.
			size_t j = 0;
			T tempDensity = 0, currentDensityLimit = 0, densityPerElement = totalDensity / CHOOSE_XFORM_GRAIN;

			//Assign xform indices in order to each element of m_XformDistributions.
			//The number of elements assigned a given index is proportional to that xform's
			//density relative to the sum of all densities.
			for (i = 0; i < ember.XformCount(); i++)
			{
				T temp = xforms[i].m_Weight;

				if (distrib > 0)
					temp *= xforms[distrib - 1].Xaos(i);

				currentDensityLimit += temp;

				//Populate points corresponding to this xform's weight/density.
				//Also check that j is within the bounds of the distribution array just to be safe in the case of a rounding error.
				while (tempDensity < currentDensityLimit && j < CHOOSE_XFORM_GRAIN)
				{
					//printf("offset = %d, xform = %d, running sum = %f\n", j, i, tempDensity);
					m_XformDistributions[(distrib * CHOOSE_XFORM_GRAIN) + j] = (unsigned char)i;
					tempDensity += densityPerElement;
					j++;
				}
			}

			//Flam3 did this, which gives the same result.
			//T t = xforms[0].m_Weight;
			//
			//if (distrib > 0)
			//	t *= xforms[distrib - 1].Xaos(0);
			//
			//T r = 0;
			//
			//for (i = 0; i < CHOOSE_XFORM_GRAIN; i++)
			//{
			//	while (r >= t)
			//	{
			//		j++;
			//
			//		if (distrib > 0)
			//			t += xforms[j].m_Weight * xforms[distrib - 1].Xaos(j);
			//		else
			//			t += xforms[j].m_Weight;
			//	}
			//
			//	m_XformDistributions[(distrib * CHOOSE_XFORM_GRAIN) + i] = j;
			//	r += densityPerElement;
			//}
		}

		return true;
	}

protected:
	/// <summary>
	/// When iterating, if the computed location of the point is either very close to zero, or very close to infinity,
	/// it's considered a bad value. In that case, a new random input point is fed into a new randomly chosen xform. This
	/// process is repeated up to 5 times until a good value is computed. If after 5 tries, a good value is not found, then
	/// the coordinates of the output point are just set to a random number between -1 and 1.
	/// </summary>
	/// <param name="xforms">The xforms array</param>
	/// <param name="badVals">The counter for the total number of bad values this sub batch</param>
	/// <param name="point">The point which initially had the bad values and which will store the newly computed values</param>
	/// <param name="rand">The random context this iterator is using</param>
	/// <returns>True if a good value was computed within 5 tries, else false</returns>
	inline bool DoBadVals(Xform<T>* xforms, size_t& badVals, Point<T>* point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t xformIndex, consec = 0;
		Point<T> firstBadPoint;

		while (consec < 5)
		{
			consec++;
			badVals++;
			firstBadPoint.m_X = rand.Frand11<T>();//Re-randomize points, but keep the computed color and viz.
			firstBadPoint.m_Y = rand.Frand11<T>();
			firstBadPoint.m_Z = 0;
			firstBadPoint.m_ColorX = point->m_ColorX;
			firstBadPoint.m_VizAdjusted = point->m_VizAdjusted;

			xformIndex = NextXformFromIndex(rand.Rand());

			if (!xforms[xformIndex].Apply(&firstBadPoint, point, rand))
				return true;
		}

		//After 5 tries, nothing worked, so just assign random values between -1 and 1.
		if (consec == 5)
		{
			point->m_X = rand.Frand11<T>();
			point->m_Y = rand.Frand11<T>();
			point->m_Z = 0;
		}

		return false;
	}

	/// <summary>
	/// Apply the final xform.
	/// Note that as stated in the paper, the output of the final xform is not fed back into the next iteration.
	/// Rather, only the value computed from the randomly chosen xform is. However, the output of the final xform
	/// is still saved in the output samples buffer and accumulated to the histogram later.
	/// </summary>
	/// <param name="ember">The ember being iterated</param>
	/// <param name="tempPoint">The input point</param>
	/// <param name="sample">The output point</param>
	/// <param name="rand">The random context to use.</param>
	inline void DoFinalXform(Ember<T>& ember, Point<T>& tempPoint, Point<T>* sample, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		if (IsClose<T>(ember.FinalXform()->m_Opacity, 1) || rand.Frand01<T>() < ember.FinalXform()->m_Opacity)
		{
			T tempVizAdjusted = tempPoint.m_VizAdjusted;

			ember.NonConstFinalXform()->Apply(&tempPoint, sample, rand);
			sample->m_VizAdjusted = tempVizAdjusted;
		}
		else
		{
			*sample = tempPoint;
		}
	}

	/// <summary>
	/// Retrieve an element in the distributions vector between 0 and CHOOSE_XFORM_GRAIN which will
	/// contain the index of the next xform to use. When xaos is prsent, the offset is the index in
	/// the ember of the previous xform used when.
	/// </summary>
	/// <param name="index">The index to retrieve</param>
	/// <param name="distribOffset">When xaos is prsent, the index of the previous xform used. Default: 0 (xaos not present).</param>
	/// <returns></returns>
	size_t NextXformFromIndex(size_t index, size_t distribOffset = 0)
	{
		return (size_t)m_XformDistributions[(index % CHOOSE_XFORM_GRAIN) + (CHOOSE_XFORM_GRAIN * distribOffset)];
	}

	vector<unsigned char> m_XformDistributions;
};

/// <summary>
/// Derived iterator class for embers whose xforms do not use xaos.
/// </summary>
template <typename T>
class EMBER_API StandardIterator : public Iterator<T>
{
ITERATORUSINGS
public:
	/// <summary>
	/// Empty constructor.
	/// </summary>
	StandardIterator()
	{
	}

	/// <summary>
	/// Overridden virtual function which iterates an ember a given number of times and does not use xaos.
	/// </summary>
	/// <param name="ember">The ember whose xforms will be applied</param>
	/// <param name="count">The number of iterations to do</param>
	/// <param name="skip">The number of times to fuse</param>
	/// <param name="samples">The buffer to store the output points</param>
	/// <param name="rand">The random context to use</param>
	/// <returns>The number of bad values</returns>
	virtual size_t Iterate(Ember<T>& ember, size_t count, size_t skip, Point<T>* samples, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t i, badVals = 0;
		Point<T> tempPoint, p1;
		Xform<T>* xforms = ember.NonConstXforms();

		if (ember.ProjBits())
		{
			if (ember.UseFinalXform())
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))
						DoBadVals(xforms, badVals, &p1, rand);
				}

				DoFinalXform(ember, p1, samples, rand);//Apply to last fuse point and store as the first element in samples.
				ember.Proj(samples[0], rand);

				for (i = 1; i < count; i++)//Real loop.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))
						DoBadVals(xforms, badVals, &p1, rand);

					DoFinalXform(ember, p1, samples + i, rand);
					ember.Proj(samples[i], rand);
				}
			}
			else
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))
						DoBadVals(xforms, badVals, &p1, rand);
				}

				samples[0] = p1;
				ember.Proj(samples[0], rand);

				for (i = 1; i < count; i++)//Real loop.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &samples[i], rand))
						DoBadVals(xforms, badVals, samples + i, rand);

					p1 = samples[i];
					ember.Proj(samples[i], rand);
				}
			}
		}
		else
		{
			if (ember.UseFinalXform())
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))
						DoBadVals(xforms, badVals, &p1, rand);
				}

				DoFinalXform(ember, p1, samples, rand);//Apply to last fuse point and store as the first element in samples.

				for (i = 1; i < count; i++)//Real loop.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))//Feed the resulting value of applying the randomly selected xform back into the next iter, and not the result of applying the final xform.
						DoBadVals(xforms, badVals, &p1, rand);

					DoFinalXform(ember, p1, samples + i, rand);
				}
			}
			else
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(&p1, &p1, rand))
						DoBadVals(xforms, badVals, &p1, rand);
				}

				samples[0] = p1;

				for (i = 0; i < count - 1; i++)//Real loop.
					if (xforms[NextXformFromIndex(rand.Rand())].Apply(samples + i, samples + i + 1, rand))
						DoBadVals(xforms, badVals, samples + i + 1, rand);
			}
		}

		return badVals;
	}
};

/// <summary>
/// Derived iterator class for embers whose xforms use xaos.
/// </summary>
template <typename T>
class EMBER_API XaosIterator : public Iterator<T>
{
ITERATORUSINGS
public:
	/// <summary>
	/// Empty constructor.
	/// </summary>
	XaosIterator()
	{
	}

	/// <summary>
	/// Handler for bad values similar to the one in the base class, except it takes the last xform used
	/// as a parameter and saves the xform used back out because this iterator is meant to be used with xaos.
	/// </summary>
	/// <param name="xforms">The xforms array</param>
	/// <param name="xformIndex">Index of the last used xform before calling this function</param>
	/// <param name="lastXformUsed">The saved index of the last xform used within this function</param>
	/// <param name="badVals">The counter for the total number of bad values this sub batch</param>
	/// <param name="point">The point which initially had the bad values and which will store the newly computed values</param>
	/// <param name="rand">The random context this iterator is using</param>
	/// <returns>True if a good value was computed within 5 tries, else false</returns>
	inline bool DoBadVals(Xform<T>* xforms, size_t& xformIndex, size_t lastXformUsed, size_t& badVals, Point<T>* point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t consec = 0;
		Point<T> firstBadPoint;

		while (consec < 5)
		{
			consec++;
			badVals++;
			firstBadPoint.m_X = rand.Frand11<T>();//Re-randomize points, but keep the computed color and viz.
			firstBadPoint.m_Y = rand.Frand11<T>();
			firstBadPoint.m_Z = 0;
			firstBadPoint.m_ColorX = point->m_ColorX;
			firstBadPoint.m_VizAdjusted = point->m_VizAdjusted;

			xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

			if (!xforms[xformIndex].Apply(&firstBadPoint, point, rand))
				return true;
		}

		//After 5 tries, nothing worked, so just assign random.
		if (consec == 5)
		{
			point->m_X = rand.Frand11<T>();
			point->m_Y = rand.Frand11<T>();
			point->m_Z = 0;
		}

		return false;
	}

	/// <summary>
	/// Overridden virtual function which iterates an ember a given number of times and uses xaos.
	/// </summary>
	/// <param name="ember">The ember whose xforms will be applied</param>
	/// <param name="count">The number of iterations to do</param>
	/// <param name="skip">The number of times to fuse</param>
	/// <param name="samples">The buffer to store the output points</param>
	/// <param name="rand">The random context to use</param>
	/// <returns>The number of bad values</returns>
	virtual size_t Iterate(Ember<T>& ember, size_t count, size_t skip, Point<T>* samples, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		size_t i, xformIndex;
		size_t lastXformUsed = 0;
		size_t badVals = 0;
		Point<T> tempPoint, p1;
		Xform<T>* xforms = ember.NonConstXforms();

		if (ember.ProjBits())
		{
			if (ember.UseFinalXform())
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}

				DoFinalXform(ember, p1, samples, rand);//Apply to last fuse point and store as the first element in samples.
				ember.Proj(samples[0], rand);

				for (i = 1; i < count; i++)//Real loop.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))//Feed the resulting value of applying the randomly selected xform back into the next iter, and not the result of applying the final xform.
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					DoFinalXform(ember, p1, samples + i, rand);
					ember.Proj(samples[i], rand);
					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}
			}
			else
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}

				samples[0] = p1;
				ember.Proj(samples[0], rand);

				for (i = 1; i < count; i++)//Real loop.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					samples[i] = p1;
					ember.Proj(samples[i], rand);
					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}
			}
		}
		else
		{
			if (ember.UseFinalXform())
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}

				DoFinalXform(ember, p1, samples, rand);//Apply to last fuse point and store as the first element in samples.

				for (i = 1; i < count; i++)//Real loop.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))//Feed the resulting value of applying the randomly selected xform back into the next iter, and not the result of applying the final xform.
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					DoFinalXform(ember, p1, samples + i, rand);
					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}
			}
			else
			{
				p1 = samples[0];

				for (i = 0; i < skip; i++)//Fuse.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(&p1, &p1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, &p1, rand);

					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}

				samples[0] = p1;

				for (i = 0; i < count - 1; i++)//Real loop.
				{
					xformIndex = NextXformFromIndex(rand.Rand(), lastXformUsed);

					if (xforms[xformIndex].Apply(samples + i, samples + i + 1, rand))
						DoBadVals(xforms, xformIndex, lastXformUsed, badVals, samples + i + 1, rand);

					lastXformUsed = xformIndex + 1;//Store the last used transform.
				}
			}
		}

		return badVals;
	}
};
}
