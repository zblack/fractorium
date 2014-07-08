#pragma once

#include "EmberDefines.h"

/// <summary>
/// TemporalFilter base, derived and factory classes.
/// </summary>

namespace EmberNs
{
/// <summary>
/// The types of temporal filters available.
/// </summary>
enum eTemporalFilterType
{
	BOX_TEMPORAL_FILTER = 0,
	GAUSSIAN_TEMPORAL_FILTER = 1,
	EXP_TEMPORAL_FILTER = 2
};

/// <summary>
/// Temporal filter is for doing motion blur while rendering a series of frames for animation.
/// The filter created is used as a vector of scalar values to multiply the time value by in between embers.
/// There are three possible types: Gaussian, Box and Exp.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API TemporalFilter
{
public:
	/// <summary>
	/// Constructor to set up basic filtering parameters, allocate buffers and calculate deltas.
	/// Derived class constructors will complete the final part of filter setup.
	/// </summary>
	/// <param name="filterType">Type of the filter.</param>
	/// <param name="passes">The number of passes used in the ember being rendered</param>
	/// <param name="temporalSamples">The number of temporal samples in the ember being rendered</param>
	/// <param name="filterWidth">The width of the filter.</param>
	TemporalFilter(eTemporalFilterType filterType, unsigned int passes, unsigned int temporalSamples, T filterWidth)
	{
		unsigned int i, steps = passes * temporalSamples;

		m_Deltas.resize(steps);
		m_Filter.resize(steps);
		m_FilterType = filterType;

		if (steps == 1)
		{
			m_SumFilt = 1;
			m_Deltas[0] = 0;
			m_Filter[0] = 1;
		}
		else
		{
			//Define the temporal deltas.
			for (i = 0; i < steps; i++)
				m_Deltas[i] = (T(i) / T(steps - 1) - T(0.5)) * filterWidth;
		}
	}
	
	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="filter">The TemporalFilter object to copy</param>
	TemporalFilter(const TemporalFilter<T>& filter)
	{
		*this = filter;
	}

	/// <summary>
	/// Virtual destructor so derived class destructors get called.
	/// </summary>
	virtual ~TemporalFilter()
	{
	}

	/// <summary>
	/// Assignment operator.
	/// </summary>
	/// <param name="filter">The TemporalFilter object to copy.</param>
	/// <returns>Reference to updated self</returns>
	TemporalFilter<T>& operator = (const TemporalFilter<T>& filter)
	{
		if (this != &filter)
		{
			m_SumFilt = filter.m_SumFilt;
			m_Deltas = filter.m_Deltas;
			m_Filter = filter.m_Filter;
			m_FilterType = filter.m_FilterType;
		}

		return *this;
	}

	/// <summary>
	/// Return a string representation of this filter.
	/// </summary>
	/// <returns>The string representation of this filter</returns>
	string ToString() const
	{
		unsigned int i;
		stringstream ss;

		ss  << "Temporal Filter:" << endl
			<< "	       Size: " << Size() << endl
			<< "           Type: " << TemporalFilterCreator<T>::ToString(m_FilterType) << endl
			<< "       Sum Filt: " << SumFilt() << endl;

		ss << "Deltas: " << endl;

		for (i = 0; i < m_Deltas.size(); i++)
		{
			ss << "Deltas[" << i << "]: " << m_Deltas[i] << endl;
		}

		ss << "Filter: " << endl;

		for (i = 0; i < m_Filter.size(); i++)
		{
			ss << "Filter[" << i << "]: " << m_Filter[i] << endl;
		}

		return ss.str();
	}
	
	/// <summary>
	/// Accessors.
	/// </summary>
	size_t Size() const { return m_Filter.size(); }
	T SumFilt() const { return m_SumFilt; }
	T* Deltas() { return &m_Deltas[0]; }
	T* Filter() { return &m_Filter[0]; }
	eTemporalFilterType FilterType() const { return m_FilterType; }

protected:
	/// <summary>
	/// Normalize the filter and the sum filt.
	/// </summary>
	/// <param name="maxFilt">The maximum filter value contained in the filter vector after it was created</param>
	void FinishFilter(T maxFilt)
	{
		m_SumFilt = 0;

		for (unsigned int i = 0; i < Size(); i++)
		{
			m_Filter[i] /= maxFilt;
			m_SumFilt += m_Filter[i];
		}
		 
		m_SumFilt /= Size();
	}

	T m_SumFilt;//The sum of all filter values.
	vector<T> m_Deltas;//Delta vector.
	vector<T> m_Filter;//Filter vector.
	eTemporalFilterType m_FilterType;//The type of filter this is.
};

/// <summary>
/// Derivation which implements the Exp filter.
/// </summary>
template <typename T>
class EMBER_API ExpTemporalFilter : public TemporalFilter<T>
{
public:
	/// <summary>
	/// Constructor to create an Exp filter.
	/// </summary>
	/// <param name="passes">The number of passes used in the ember being rendered</param>
	/// <param name="temporalSamples">The number of temporal samples in the ember being rendered</param>
	/// <param name="filterWidth">The width of the filter.</param>
	/// <param name="filterExp">The filter exp.</param>
	ExpTemporalFilter(unsigned int passes, unsigned int temporalSamples, T filterWidth, T filterExp)
		: TemporalFilter<T>(BOX_TEMPORAL_FILTER, passes, temporalSamples, filterWidth)
	{
		if (Size() > 1)
		{
			T slpx, maxFilt = 0;

			for (unsigned int i = 0; i < Size(); i++)
			{
				if (filterExp >= 0)
					slpx = (T(i) + 1) / Size();
				else
					slpx = T(Size() - i) / Size();

				//Scale the color based on these values.
				m_Filter[i] = pow(slpx, fabs(filterExp));
		 
				//Keep the max.
				if (m_Filter[i] > maxFilt)
					maxFilt = m_Filter[i];
			}

			FinishFilter(maxFilt);
		}
	}
};

/// <summary>
/// Derivation which implements the Gaussian filter.
/// </summary>
template <typename T>
class EMBER_API GaussianTemporalFilter : public TemporalFilter<T>
{
public:
	/// <summary>
	/// Constructor to create a Gaussian filter.
	/// </summary>
	/// <param name="passes">The number of passes used in the ember being rendered</param>
	/// <param name="temporalSamples">The number of temporal samples in the ember being rendered</param>
	/// <param name="filterWidth">The width of the filter.</param>
	GaussianTemporalFilter(unsigned int passes, unsigned int temporalSamples, T filterWidth)
		: TemporalFilter<T>(GAUSSIAN_TEMPORAL_FILTER, passes, temporalSamples, filterWidth)
	{
		if (Size() > 1)
		{
			T maxFilt = 0, halfSteps = T(Size()) / T(2);
			GaussianFilter<T> gaussian(1, 1);//Just pass dummy values, they are unused in this case.

			for (unsigned int i = 0; i < Size(); i++)
			{
				m_Filter[i] = gaussian.Filter(gaussian.Support() * fabs(i - halfSteps) / halfSteps);
				
				//Keep the max.
				if (m_Filter[i] > maxFilt)
					maxFilt = m_Filter[i];
			}

			FinishFilter(maxFilt);
		}
	}
};

/// <summary>
/// Derivation which implements the Box filter.
/// </summary>
template <typename T>
class EMBER_API BoxTemporalFilter : public TemporalFilter<T>
{
public:
	/// <summary>
	/// Constructor to create a Box filter.
	/// </summary>
	/// <param name="passes">The number of passes used in the ember being rendered</param>
	/// <param name="temporalSamples">The number of temporal samples in the ember being rendered</param>
	/// <param name="filterWidth">The width of the filter.</param>
	BoxTemporalFilter(unsigned int passes, unsigned int temporalSamples, T filterWidth)
		: TemporalFilter<T>(BOX_TEMPORAL_FILTER, passes, temporalSamples, filterWidth)
	{
		if (Size() > 1)
		{
			for (unsigned int i = 0; i < Size(); i++)
				m_Filter[i] = 1;
		 
			FinishFilter(1);
		}
	}
};

/// <summary>
/// Convenience class to assist in converting between filter names and the filter objects themselves.
/// </summary>
template <typename T>
class EMBER_API TemporalFilterCreator
{
public:
	/// <summary>
	/// Creates the specified filter type based on the filterType enum parameter.
	/// </summary>
	/// <param name="filterType">Type of the filter</param>
	/// <param name="passes">The number of passes used in the ember being rendered</param>
	/// <param name="temporalSamples">The number of temporal samples in the ember being rendered</param>
	/// <param name="filterWidth">The width of the filter</param>
	/// <param name="filterExp">The filter exp, only used with Exp filter, otherwise ignored.</param>
	/// <returns>A pointer to the newly created filter object</returns>
	static TemporalFilter<T>* Create(eTemporalFilterType filterType, unsigned int passes, unsigned int temporalSamples, T filterWidth, T filterExp = 1)
	{
		TemporalFilter<T>* filter = NULL;

		if (filterType == BOX_TEMPORAL_FILTER)
			filter = new BoxTemporalFilter<T>(passes, temporalSamples, filterWidth);
		else if (filterType == GAUSSIAN_TEMPORAL_FILTER)
			filter = new GaussianTemporalFilter<T>(passes, temporalSamples, filterWidth);
		else if (filterType == EXP_TEMPORAL_FILTER)
			filter = new ExpTemporalFilter<T>(passes, temporalSamples, filterWidth, filterExp);
		else
			filter = new BoxTemporalFilter<T>(passes, temporalSamples, filterWidth);//Default to box if bad enum passed in.

		return filter;
	}

	/// <summary>
	/// Return a string vector of the available filter types.
	/// </summary>
	/// <returns>A vector of strings populated with the available filter types</returns>
	static vector<string> FilterTypes()
	{
		vector<string> v;

		v.reserve(3);
		v.push_back("Box");
		v.push_back("Gaussian");
		v.push_back("Exp");

		return v;
	}

	/// <summary>
	/// Convert between the filter name string and its type enum.
	/// </summary>
	/// <param name="filterType">The string name of the filter</param>
	/// <returns>The filter type enum</returns>
	static eTemporalFilterType FromString(string filterType)
	{
		if (!_stricmp(filterType.c_str(), "box"))
			return BOX_TEMPORAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "gaussian"))
			return GAUSSIAN_TEMPORAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "exp"))
			return EXP_TEMPORAL_FILTER;
		else
			return BOX_TEMPORAL_FILTER;
	}

	/// <summary>
	/// Convert between the filter type enum and its name string.
	/// </summary>
	/// <param name="eTemporalFilterType">The filter type enum</param>
	/// <returns>The string name of the filter</returns>
	static string ToString(eTemporalFilterType filterType)
	{
		string filter;

		if (filterType == BOX_TEMPORAL_FILTER)
			filter = "Box";
		else if (filterType == GAUSSIAN_TEMPORAL_FILTER)
			filter = "Gaussian";
		else if (filterType == EXP_TEMPORAL_FILTER)
			filter = "Exp";
		else
			filter = "Box";

		return filter;
	}
};
}