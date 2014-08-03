#pragma once

#include "SpatialFilter.h"

/// <summary>
/// DensityFilter class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// A base class with virtual functions to allow both templating and polymorphism to work together.
/// Derived classes will implement all of these functions.
/// </summary>
class EMBER_API DensityFilterBase
{
public:
	DensityFilterBase() { }
	virtual ~DensityFilterBase() { }

	virtual int FilterWidth() { return 0; }
};

/// <summary>
/// The density estimation filter is used after iterating, but before final accumulation.
/// It's a variable width Gaussian filter, whose width is inversely proportional
/// to the number of hits a given histogram cell has received.
/// That means the fewer hits in a cell, the more blur is applied. The greater the hits,
/// the less blur.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API DensityFilter : public DensityFilterBase
{
public:
	/// <summary>
	/// Constructor that assigns various fields but does not create the actual filter vector.
	/// This is done because filter creation could fail, so the user must manually call it
	/// after construction.
	/// </summary>
	/// <param name="minRad">The minimum filter radius</param>
	/// <param name="maxRad">The maximum filter radius</param>
	/// <param name="curve">The curve of the filter</param>
	/// <param name="supersample">The supersample of the ember this filter will be used with</param>
	DensityFilter(T minRad, T maxRad, T curve, unsigned int supersample)
	{
		m_MinRad = minRad;
		m_MaxRad = maxRad;
		m_Curve = curve;
		m_Supersample = supersample;
		m_MaxFilterIndex = 0;

		//Make sure the values make sense.
		if (m_Curve <= 0.0)
			m_Curve = T(0.5);

		if (m_MaxRad < m_MinRad)
			m_MaxRad = m_MinRad + 1;
	}

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="filter">The DensityFilter object to copy</param>
	DensityFilter(const DensityFilter<T>& filter)
	{
		*this = filter;
	}

	/// <summary>
	/// Assignment operator.
	/// </summary>
	/// <param name="filter">The DensityFilter object to copy.</param>
	/// <returns>Reference to updated self</returns>
	DensityFilter<T>& operator = (const DensityFilter<T>& filter)
	{
		if (this != &filter)
		{
			m_MinRad            = filter.m_MinRad;           
			m_MaxRad			= filter.m_MaxRad;
			m_Curve			    = filter.m_Curve;
			m_Supersample		= filter.m_Supersample;
			m_KernelSize		= filter.m_KernelSize;
			m_MaxFilterIndex	= filter.m_MaxFilterIndex;
			m_MaxFilteredCounts = filter.m_MaxFilteredCounts;
			m_FilterWidth		= filter.m_FilterWidth;
			m_Coefs			    = filter.m_Coefs;
			m_Widths			= filter.m_Widths;
		}

		return *this;
	}

	/// <summary>
	/// Create the filter vector of up to 10M entries.
	/// If more than that are requested, it isn't created and 
	/// false is returned.
	/// </summary>
	/// <returns>True if success, else false.</returns>
	bool Create()
	{
		int i, j, w;
		int intFilterCount, maxIndex;
		int rowSize;
		int filterLoop;
		int keepThresh = 100;
		unsigned int filterCoefIndex = 0;
		T decFilterCount;
		T finalMinRad = m_MinRad * m_Supersample + 1;//Should scale the filter width by the oversample.
		T finalMaxRad = m_MaxRad * m_Supersample + 1;//The '+1' comes from the assumed distance to the first pixel.
		GaussianFilter<T> gaussianFilter(m_MaxRad, m_Supersample);

		m_KernelSize = 0;
		m_MaxFilterIndex = 0;

		//Calculate how many filter kernels are needed based on the decay function
		//
		//    num filters = (de_max_width / de_min_width)^(1 / estimator_curve)
		//
		decFilterCount = pow(finalMaxRad / finalMinRad, T(1.0) / m_Curve);
		
		if (decFilterCount > 1e7)//Too many filters.
			return false;

		intFilterCount = (int)ceil(decFilterCount);

		//Condense the smaller kernels to save space.
		if (intFilterCount > keepThresh)
		{ 
			maxIndex = (int)ceil(DE_THRESH + pow(T(intFilterCount - DE_THRESH), m_Curve)) + 1;
			m_MaxFilteredCounts = (int)pow(T(maxIndex - DE_THRESH), T(1.0) / m_Curve) + DE_THRESH;
		}
		else
		{
			maxIndex = intFilterCount;
			m_MaxFilteredCounts = maxIndex;
		}

		//Allocate the memory for these filters and the hit/width lookup array.
		rowSize = (int)(2 * ceil(finalMaxRad) - 1);
		m_FilterWidth = (rowSize - 1) / 2;
		m_KernelSize = (m_FilterWidth + 1) * (2 + m_FilterWidth) / 2;

		m_Coefs.resize(maxIndex * m_KernelSize);
		m_Widths.resize(maxIndex);

		//Generate the filter coefficients.
		for (filterLoop = 0; filterLoop < maxIndex; filterLoop++)
		{
			int dej, dek;
			int coefIndex;
			T filterSum = 0.0;
			T filterVal;
			T filterHeight;
			T loopAdjust;

			//Calculate the filter width for this number of hits in a bin.
			if (filterLoop < keepThresh)
			{
				filterHeight = (finalMaxRad / pow(T(filterLoop + 1), m_Curve));
			}
			else
			{
				loopAdjust = pow(T(filterLoop - keepThresh), (T(1.0) / m_Curve)) + keepThresh;
				filterHeight = (finalMaxRad / pow(loopAdjust + 1, m_Curve));
			}

			//Once we've reached the min radius, don't populate any more.
			if (filterHeight <= finalMinRad)
			{
				filterHeight = finalMinRad;
				m_MaxFilterIndex = filterLoop;
			}

			m_Widths[filterLoop] = filterHeight;

			//Calculate norm of kernel separately (easier).
			for (dej = -m_FilterWidth; dej <= m_FilterWidth; dej++)
			{
				for (dek = -m_FilterWidth; dek <= m_FilterWidth; dek++)
				{
					filterVal = sqrt((T)(dej * dej + dek * dek)) / filterHeight;

					//Only populate the coefs within this radius.
					if (filterVal <= 1.0)
						filterSum += gaussianFilter.Filter(gaussianFilter.Support() * filterVal);
				}
			}

			coefIndex = filterLoop * m_KernelSize;

			//Calculate the unique entries of the kernel.
			for (dej = 0; dej <= m_FilterWidth; dej++)
			{
				for (dek = 0; dek <= dej; dek++)
				{
					filterVal = sqrt(T(dej * dej + dek * dek)) / filterHeight;

					//Only populate the coefs within this radius.
					if (filterVal > 1.0)
						m_Coefs[coefIndex] = 0.0;
					else
						m_Coefs[coefIndex] = gaussianFilter.Filter(gaussianFilter.Support() * filterVal) / filterSum;
				  
					coefIndex++;
				}
			}

			if (m_MaxFilterIndex > 0)
				break;
		}

		if (m_MaxFilterIndex == 0)
			m_MaxFilterIndex = maxIndex - 1;

		w = m_FilterWidth + 1;
		m_CoefIndices.resize(w * w);

		//This will populate one quadrant of filter indices.
		//Really only need 1/8th, but that would require a sparse matrix.
		for (j = 0; j <= m_FilterWidth; j++)
		{
			for (i = 0; i <= j; i++, filterCoefIndex++)
			{
				if (j == 0 && i == 0)
				{
					m_CoefIndices[(j * w) + i] = filterCoefIndex;
				}
				else if (i == 0)
				{
					m_CoefIndices[(0 * w) + j] = filterCoefIndex;
					m_CoefIndices[(j * w) + 0] = filterCoefIndex;
				}
				else if (j == i)
				{
					m_CoefIndices[(j * w) + i] = filterCoefIndex;
				}
				else
				{
					m_CoefIndices[(i * w) + j] = filterCoefIndex;
					m_CoefIndices[(j * w) + i] = filterCoefIndex;
				}
			}
		}

		return true;
	}

	/// <summary>
	/// Return whether the requested dimensions are valid.
	/// Meaning, is the requested filter size less than or equal to 10M?
	/// </summary>
	/// <returns>True if requested filter size is less than or equal to 10M, else false.</returns>
	inline bool Valid() const
	{
		T finalMaxRad = m_MaxRad * m_Supersample + 1;
		T finalMinRad = m_MinRad * m_Supersample + 1;

		return pow(finalMaxRad / finalMinRad, T(1.0) / m_Curve) <= 1e7;
	}

	/// <summary>
	/// Return a string representation of this density estimation filter.
	/// </summary>
	/// <returns>The string representation of this density estimation filter</returns>
	string ToString() const
	{
		unsigned int i, j, coefIndex = 0, w = m_FilterWidth + 1;
		stringstream ss;

		ss
			<< "Density Filter:" << endl
			<< "	     Min radius: " << MinRad() << endl
			<< "	     Max radius: " << MaxRad() << endl
			<< "		      Curve: " << Curve() << endl
			<< "        Kernel size: " << KernelSize() << endl
			<< "   Max filter index: " << MaxFilterIndex() << endl
			<< "Max Filtered counts: " << MaxFilteredCounts() << endl
			<< "       Filter width: " << FilterWidth() << endl;

		ss << "Coefficients: " << endl;

		for (i = 0; i < m_Widths.size(); i++)
		{
			for (coefIndex = 0; coefIndex < m_KernelSize; coefIndex++)
				ss << "Kernel[" << i << "].Coefs[" << coefIndex << "]: " << m_Coefs[(i * m_KernelSize) + coefIndex] << endl;
		}

		ss << endl << "Widths: " << endl;

		for (i = 0; i < m_Widths.size(); i++)
		{
			ss << "Widths[" << i << "]: " << m_Widths[i] << endl;
		}

		for (i = 0; i < w; i++)
		{
			for (j = 0; j < w; j++)
			{
				cout << std::setw(2) << std::setfill('0') << m_CoefIndices[i * w + j] << "\t";
			}

			cout << endl;
		}

		return ss.str();
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	inline T MinRad() const { return m_MinRad; }
	inline T MaxRad() const { return m_MaxRad; }
	inline T Curve() const { return m_Curve; }
	inline unsigned int Supersample() const { return m_Supersample; }
	inline unsigned int KernelSize() const { return m_KernelSize; }
	inline unsigned int MaxFilterIndex() const { return m_MaxFilterIndex; }
	inline unsigned int MaxFilteredCounts() const { return m_MaxFilteredCounts; }
	virtual int FilterWidth() const { return m_FilterWidth; }
	inline unsigned int BufferSize() const { return (unsigned int)m_Widths.size(); }
	inline unsigned int CoefsSizeBytes() const { return BufferSize() * m_KernelSize * sizeof(T); }
	inline unsigned int WidthsSizeBytes() const { return BufferSize() * sizeof(T); }
	inline unsigned int CoefsIndicesSizeBytes() const { return unsigned int(m_CoefIndices.size() * sizeof(unsigned int)); }
	inline const T* Coefs() const { return m_Coefs.data(); }
	inline const T* Widths() const { return m_Widths.data(); }
	inline const unsigned int* CoefIndices() const { return m_CoefIndices.data(); }

private:
	T m_MinRad;
	T m_MaxRad;//The original specified filter radius.
	T m_Curve;
	unsigned int m_Supersample;
	unsigned int m_KernelSize;
	unsigned int m_MaxFilterIndex;
	unsigned int m_MaxFilteredCounts;
	int m_FilterWidth;//The new radius after scaling for super sample and rounding. This is what's actually used.
	vector<T> m_Coefs;
	vector<T> m_Widths;
	vector<unsigned int> m_CoefIndices;
};
}