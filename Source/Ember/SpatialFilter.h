#pragma once

#include "EmberDefines.h"

/// <summary>
/// SpatialFilter base, derived and factory classes.
/// </summary>

namespace EmberNs
{
/// <summary>
/// The types of spatial filters available.
/// </summary>
enum eSpatialFilterType
{
	GAUSSIAN_SPATIAL_FILTER = 0,
	HERMITE_SPATIAL_FILTER = 1,
	BOX_SPATIAL_FILTER = 2,
	TRIANGLE_SPATIAL_FILTER = 3,
	BELL_SPATIAL_FILTER = 4,
	BSPLINE_SPATIAL_FILTER = 5,
	LANCZOS3_SPATIAL_FILTER = 6,
	LANCZOS2_SPATIAL_FILTER = 7,
	MITCHELL_SPATIAL_FILTER = 8,
	BLACKMAN_SPATIAL_FILTER = 9,
	CATROM_SPATIAL_FILTER = 10,
	HAMMING_SPATIAL_FILTER = 11,
	HANNING_SPATIAL_FILTER = 12,
	QUADRATIC_SPATIAL_FILTER = 13
};

/// <summary>
/// Spatial filtering is done in the final accumulation stage to add some additional
/// bluring to smooth out noisy areas.
/// The bulk of the work is done in this base class Create() function.
/// Because it calls the virtual Filter() function, it cannot be automatically called in the constructor.
/// So the caller must manually call it after constructing the filter object.
/// Each derived class will implement an override of Filter() which
/// contains the specific filter calculation for the algorithm whose name the class matches.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API SpatialFilter
{
public:
	/// <summary>
	/// Assign basic parameters for creating a spatial filter. The caller must still call Create().
	/// </summary>
	/// <param name="filterType">Type of filter to create</param>
	/// <param name="support">Miscellaneous value</param>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	SpatialFilter(eSpatialFilterType filterType, T support, T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
	{
		m_FilterType = filterType;
		m_Support = support;
		m_FilterRadius = filterRadius;
		m_Supersample = superSample;
		m_PixelAspectRatio = pixelAspectRatio;
		//Sadly, cannot call create here because it calls the Filter() virtual function and unlike C#, the vtables
		//are not yet set up in C++ constructors. The code that instantiates this object must explicitly call Create().
	}

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="filter">The SpatialFilter object to copy</param>
	SpatialFilter(const SpatialFilter<T>& filter)
	{
		*this = filter;
	}

	/// <summary>
	/// Virtual destructor so derived class destructors get called.
	/// </summary>
	virtual ~SpatialFilter()
	{
	}

	/// <summary>
	/// Assignment operator.
	/// </summary>
	/// <param name="filter">The SpatialFilter object to copy.</param>
	/// <returns>Reference to updated self</returns>
	SpatialFilter<T>& operator = (const SpatialFilter<T>& filter)
	{
		if (this != &filter)
		{
			m_FinalFilterWidth = filter.m_FinalFilterWidth;
			m_Supersample = filter.m_Supersample;
			m_Support = filter.m_Support;
			m_FilterRadius = filter.m_FilterRadius;
			m_PixelAspectRatio = filter.m_PixelAspectRatio;
			m_FilterType = filter.m_FilterType;
			m_Filter = filter.m_Filter;
		}

		return *this;
	}

	/// <summary>
	/// Allocates and populates the filter buffer with virtual calls to derived Filter() functions.
	/// The caller must manually call this after construction.
	/// </summary>
	void Create()
	{
		T fw = T(2.0) * m_Support * m_Supersample * m_FilterRadius / m_PixelAspectRatio;
		T adjust, ii, jj;

		int fwidth = ((int)fw) + 1;
		int i, j;

		//Make sure the filter kernel has same parity as oversample.
		if ((fwidth ^ m_Supersample) & 1)
			fwidth++;

		//Calculate the coordinate scaling factor for the kernel values.
		if (fw > 0.0)
			adjust = m_Support * fwidth / fw;
		else
			adjust = T(1.0);

		m_Filter.resize(fwidth * fwidth);

		//Fill in the coefs.
		for (i = 0; i < fwidth; i++)
		{
			for (j = 0; j < fwidth; j++)
			{
				//Calculate the function inputs for the kernel function.
				ii = ((T(2.0) * i + T(1.0)) / T(fwidth) - T(1.0)) * adjust;
				jj = ((T(2.0) * j + T(1.0)) / T(fwidth) - T(1.0)) * adjust;

				//Adjust for aspect ratio.
				jj /= m_PixelAspectRatio;

				m_Filter[i + j * fwidth] = Filter(ii) * Filter(jj);//Call virtual Filter(), implemented in specific derived filter classes.
			}
		}

		//Normalize, and return a bad value if the values were too small.
		if (!Normalize())
			m_FinalFilterWidth = -1;
		else
			m_FinalFilterWidth = fwidth;
	}

	/// <summary>
	/// Return a string representation of this filter.
	/// </summary>
	/// <returns>The string representation of this filter</returns>
	string ToString() const
	{
		size_t i;
		stringstream ss;

		ss
			<< "Spatial Filter:" << endl
			<< "	       Support: " << m_Support << endl
			<< "     Filter radius: " << m_FilterRadius << endl
			<< "	   Supersample: " << m_Supersample << endl
			<< "Pixel aspect ratio: " << m_PixelAspectRatio << endl
			<< "Final filter width: " << m_FinalFilterWidth << endl
			<< "Filter buffer size: " << m_Filter.size() << endl;

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
	void Apply() { }
	inline int FinalFilterWidth() const { return m_FinalFilterWidth; }
	inline size_t Supersample() const { return m_Supersample; }
	inline size_t BufferSize() const { return m_Filter.size(); }
	inline size_t BufferSizeBytes() const { return BufferSize() * sizeof(T); }
	inline T Support() const { return m_Support; }
	inline T FilterRadius() const { return m_FilterRadius; }
	inline T PixelAspectRatio() const { return m_PixelAspectRatio; }
	inline eSpatialFilterType FilterType() const { return m_FilterType; }
	inline T* Filter() { return m_Filter.data(); }
	inline const T& operator[] (size_t index) const { return m_Filter[index]; }
	virtual T Filter(T t) const = 0;

protected:
	/// <summary>
	/// Calculation function used in Lanczos filters.
	/// </summary>
	/// <param name="x">The x</param>
	/// <returns>The calculated value</returns>
	static T Sinc(T x)
	{
		x *= T(M_PI);

		if (x != 0)
			return sin(x) / x;

		return 1.0;
	}

private:
	/// <summary>
	/// Normalize all filter values.
	/// </summary>
	/// <returns>True if any value was non-zero, else false if all were zero.</returns>
	bool Normalize()
	{
		size_t i;
		T t = T(0.0);

		for (i = 0; i < m_Filter.size(); i++)
			t += m_Filter[i];

		if (t == 0.0)
			return false;

		t = T(1.0) / t;

		for (i = 0; i < m_Filter.size(); i++)
			m_Filter[i] *= t;

		return true;
	}

	int m_FinalFilterWidth;//The final width that the filter ends up being.
	size_t m_Supersample;//The supersample value of the ember using this filter to render.
	T m_Support;//Extra value.
	T m_FilterRadius;//The requested filter radius.
	T m_PixelAspectRatio;//The aspect ratio of the ember using this filter to render, usually 1.
	eSpatialFilterType m_FilterType;//The type of filter this is.
	vector<T> m_Filter;//The vector holding the calculated filter values.
};

/// <summary>
/// Derivation for Gaussian filter.
/// </summary>
template <typename T>
class EMBER_API GaussianFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.5.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	GaussianFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(GAUSSIAN_SPATIAL_FILTER, T(1.5), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Gaussian filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		return exp(-2 * t * t) * sqrt(2 / T(M_PI));
	}
};

/// <summary>
/// Derivation for Hermite filter.
/// </summary>
template <typename T>
class EMBER_API HermiteFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	HermiteFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(HERMITE_SPATIAL_FILTER, T(1.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Hermite filter to t parameter and return.
	/// f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < 0)
			t = -t;

		if (t < 1)
			return ((T(2.0) * t - T(3.0)) * t * t + T(1.0));

		return 0;
	}
};

/// <summary>
/// Derivation for Box filter.
/// </summary>
template <typename T>
class EMBER_API BoxFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 0.5.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	BoxFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(BOX_SPATIAL_FILTER, T(0.5), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Box filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if ((t > T(-0.5)) && (t <= T(0.5)))
			return 1;

		return 0;
	}
};

/// <summary>
/// Derivation for Triangle filter.
/// </summary>
template <typename T>
class EMBER_API TriangleFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	TriangleFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(TRIANGLE_SPATIAL_FILTER, T(1.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Triangle filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < 0)
			t = -t;

		if (t < 1)
			return 1 - t;

		return 0;
	}
};

/// <summary>
/// Derivation for Bell filter.
/// </summary>
template <typename T>
class EMBER_API BellFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.5.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	BellFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(BELL_SPATIAL_FILTER, T(1.5), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Bell filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		//box (*) box (*) box.
		if (t < 0)
			t = -t;

		if (t < T(0.5))
			return (T(0.75) - (t * t));

		if (t < T(1.5))
		{
			t = (t - T(1.5));
			return (T(0.5) * (t * t));
		}

		return 0;
	}
};

/// <summary>
/// Derivation for B Spline filter.
/// </summary>
template <typename T>
class EMBER_API BsplineFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 2.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	BsplineFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(BSPLINE_SPATIAL_FILTER, T(2.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply B Spline filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		//box (*) box (*) box (*) box.
		T tt;

		if (t < 0)
			t = -t;

		if (t < 1)
		{
			tt = t * t;
			return ((T(0.5) * tt * t) - tt + (T(2.0) / T(3.0)));
		}
		else if (t < 2)
		{
			t = 2 - t;
			return ((T(1.0) / T(6.0)) * (t * t * t));
		}

		return 0;
	}
};

/// <summary>
/// Derivation for Lanczos 3 filter.
/// </summary>
template <typename T>
class EMBER_API Lanczos3Filter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 3.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	Lanczos3Filter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(LANCZOS3_SPATIAL_FILTER, T(3.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Lanczos 3 filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < 0)
			t = -t;

		if (t < 3)
			return SpatialFilter<T>::Sinc(t) * SpatialFilter<T>::Sinc(t / 3);

		return 0;
	}
};

/// <summary>
/// Derivation for Lanczos 2 filter.
/// </summary>
template <typename T>
class EMBER_API Lanczos2Filter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 2.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	Lanczos2Filter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(LANCZOS2_SPATIAL_FILTER, T(2.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Lanczos 2 filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < 0)
			t = -t;

		if (t < 2)
			return SpatialFilter<T>::Sinc(t) * SpatialFilter<T>::Sinc(t / 2);

		return 0;
	}
};

/// <summary>
/// Derivation for Mitchell filter.
/// </summary>
template <typename T>
class EMBER_API MitchellFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 2.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	MitchellFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(MITCHELL_SPATIAL_FILTER, T(2.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Mitchell filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		T tt = t * t;
		const T b = T(1) / T(3);
		const T c = T(1) / T(3);

		if (t < 0)
			t = -t;

		if (t < 1)
		{
			t = (((T(12.0) - T(9.0) * b - T(6.0) * c) * (t * tt))
			+ ((T(-18.0) + T(12.0) * b + T(6.0) * c) * tt)
			+ (T(6.0) - 2 * b));

			return t / 6;
		}
		else if (t < 2)
		{
			t = (((T(-1.0) * b - T(6.0) * c) * (t * tt))
			+ ((T(6.0) * b + T(30.0) * c) * tt)
			+ ((T(-12.0) * b - T(48.0) * c) * t)
			+ (T(8.0) * b + 24 * c));

			return t / 6;
		}

		return 0;
	}
};

/// <summary>
/// Derivation for Blackman filter.
/// </summary>
template <typename T>
class EMBER_API BlackmanFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	BlackmanFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(BLACKMAN_SPATIAL_FILTER, T(1.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Blackman filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		return (T(0.42) + T(0.5) * cos(T(M_PI) * t) + T(0.08) * cos(2 * T(M_PI) * t));
	}
};

/// <summary>
/// Derivation for Catmull-Rom filter.
/// </summary>
template <typename T>
class EMBER_API CatromFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 2.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	CatromFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(CATROM_SPATIAL_FILTER, T(2.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Catmull-Rom filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < 0)
			return 0;

		if (t < -1)
			return T(0.5) * (4 + t * (8 + t * (5 + t)));

		if (t < 0)
			return T(0.5) * (2 + t * t * (-5 - 3 * t));

		if (t < 1)
			return T(0.5) * (2 + t * t * (-5 + 3 * t));

		if (t < 2)
			return T(0.5) * (4 + t * (-8 + t * (5 - t)));

		return 0;
	}
};

/// <summary>
/// Derivation for Hamming filter.
/// </summary>
template <typename T>
class EMBER_API HammingFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	HammingFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(HAMMING_SPATIAL_FILTER, T(1.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Hamming filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		return T(0.54) + T(0.46) * cos(T(M_PI) * t);
	}
};

/// <summary>
/// Derivation for Hanning filter.
/// </summary>
template <typename T>
class EMBER_API HanningFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	HanningFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(HANNING_SPATIAL_FILTER, T(1.0), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Hanning filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		return T(0.5) + T(0.5) * cos(T(M_PI) * t);
	}
};

/// <summary>
/// Derivation for Quadratic filter.
/// </summary>
template <typename T>
class EMBER_API QuadraticFilter : public SpatialFilter<T>
{
public:
	/// <summary>
	/// Constructor which does nothing but pass values to the base class. The caller must still call Create().
	/// Support = 1.5.
	/// </summary>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample of the ember being rendered</param>
	/// <param name="pixelAspectRatio">The pixel aspect ratio being used to render. Default: 1.</param>
	QuadraticFilter(T filterRadius, size_t superSample, T pixelAspectRatio = T(1.0))
		: SpatialFilter<T>(QUADRATIC_SPATIAL_FILTER, T(1.5), filterRadius, superSample, pixelAspectRatio) { }

	/// <summary>
	/// Apply Quadratic filter to t parameter and return.
	/// </summary>
	/// <param name="t">The value to apply the filter to</param>
	/// <returns>The filtered value</returns>
	virtual T Filter(T t) const
	{
		if (t < -1.5)
			return 0.0;

		if (t < -0.5)
			return T(0.5) * (t + T(1.5)) * (t + T(1.5));

		if (t < 0.5)
			return T(0.75) - (t * t);

		if (t < 1.5)
			return T(0.5) * (t - T(1.5)) * (t - T(1.5));

		return 0;
	}
};

/// <summary>
/// Convenience class to assist in converting between filter names and the filter objects themselves.
/// </summary>
template <typename T>
class EMBER_API SpatialFilterCreator
{
public:
	/// <summary>
	/// Creates the specified filter type based on the filterType enum parameter.
	/// </summary>
	/// <param name="filterType">Type of the filter</param>
	/// <param name="filterRadius">The filter radius</param>
	/// <param name="superSample">The supersample value of the ember using this filter to render</param>
	/// <param name="pixelAspectRatio">The aspect ratio of the ember using this filter to render. Default: 1.</param>
	/// <returns>A pointer to the newly created filter object</returns>
	static SpatialFilter<T>* Create(eSpatialFilterType filterType, T filterRadius, size_t superSample, T pixelAspectRatio = 1)
	{
		SpatialFilter<T>* filter = nullptr;

		if (filterType == GAUSSIAN_SPATIAL_FILTER)
			filter = new GaussianFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == HERMITE_SPATIAL_FILTER)
			filter = new HermiteFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == BOX_SPATIAL_FILTER)
			filter = new BoxFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == TRIANGLE_SPATIAL_FILTER)
			filter = new TriangleFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == BELL_SPATIAL_FILTER)
			filter = new BellFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == BSPLINE_SPATIAL_FILTER)
			filter = new BsplineFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == LANCZOS3_SPATIAL_FILTER)
			filter = new Lanczos3Filter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == LANCZOS2_SPATIAL_FILTER)
			filter = new Lanczos2Filter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == MITCHELL_SPATIAL_FILTER)
			filter = new MitchellFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == BLACKMAN_SPATIAL_FILTER)
			filter = new BlackmanFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == CATROM_SPATIAL_FILTER)
			filter = new CatromFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == HAMMING_SPATIAL_FILTER)
			filter = new HammingFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == HANNING_SPATIAL_FILTER)
			filter = new HanningFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else if (filterType == QUADRATIC_SPATIAL_FILTER)
			filter = new QuadraticFilter<T>(filterRadius, superSample, pixelAspectRatio);
		else
			filter = new GaussianFilter<T>(filterRadius, superSample, pixelAspectRatio);

		if (filter)
			filter->Create();

		return filter;
	}

	/// <summary>
	/// Return a string vector of the available filter types.
	/// </summary>
	/// <returns>A vector of strings populated with the available filter types</returns>
	static vector<string> FilterTypes()
	{
		vector<string> v;

		v.reserve(14);
		v.push_back("Gaussian");
		v.push_back("Hermite");
		v.push_back("Box");
		v.push_back("Triangle");
		v.push_back("Bell");
		v.push_back("Bspline");
		v.push_back("Lanczos3");
		v.push_back("Lanczos2");
		v.push_back("Mitchell");
		v.push_back("Blackman");
		v.push_back("Catrom");
		v.push_back("Hamming");
		v.push_back("Hanning");
		v.push_back("Quadratic");

		return v;
	}

	/// <summary>
	/// Convert between the filter name string and its type enum.
	/// </summary>
	/// <param name="filterType">The string name of the filter</param>
	/// <returns>The filter type enum</returns>
	static eSpatialFilterType FromString(const string& filterType)
	{
		if (!_stricmp(filterType.c_str(), "Gaussian"))
			return GAUSSIAN_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Hermite"))
			return HERMITE_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Box"))
			return BOX_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Triangle"))
			return TRIANGLE_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Bell"))
			return BELL_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Bspline"))
			return BSPLINE_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Lanczos3"))
			return LANCZOS3_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Lanczos2"))
			return LANCZOS2_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Mitchell"))
			return MITCHELL_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Blackman"))
			return BLACKMAN_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Catrom"))
			return CATROM_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Hamming"))
			return HAMMING_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Hanning"))
			return HANNING_SPATIAL_FILTER;
		else if (!_stricmp(filterType.c_str(), "Quadratic"))
			return QUADRATIC_SPATIAL_FILTER;
		else
			return GAUSSIAN_SPATIAL_FILTER;
	}

	/// <summary>
	/// Convert between the filter type enum and its name string.
	/// </summary>
	/// <param name="eTemporalFilterType">The filter type enum</param>
	/// <returns>The string name of the filter</returns>
	static string ToString(eSpatialFilterType filterType)
	{
		string filter;

		if (filterType == GAUSSIAN_SPATIAL_FILTER)
			filter = "Gaussian";
		else if (filterType == HERMITE_SPATIAL_FILTER)
			filter = "Hermite";
		else if (filterType == BOX_SPATIAL_FILTER)
			filter = "Box";
		else if (filterType == TRIANGLE_SPATIAL_FILTER)
			filter = "Triangle";
		else if (filterType == BELL_SPATIAL_FILTER)
			filter = "Bell";
		else if (filterType == BSPLINE_SPATIAL_FILTER)
			filter = "Bspline";
		else if (filterType == LANCZOS3_SPATIAL_FILTER)
			filter = "Lanczos3";
		else if (filterType == LANCZOS2_SPATIAL_FILTER)
			filter = "Lanczos2";
		else if (filterType == MITCHELL_SPATIAL_FILTER)
			filter = "Mitchell";
		else if (filterType == BLACKMAN_SPATIAL_FILTER)
			filter = "Blackman";
		else if (filterType == CATROM_SPATIAL_FILTER)
			filter = "Catrom";
		else if (filterType == HAMMING_SPATIAL_FILTER)
			filter = "Hamming";
		else if (filterType == HANNING_SPATIAL_FILTER)
			filter = "Hanning";
		else if (filterType == QUADRATIC_SPATIAL_FILTER)
			filter = "Quadratic";
		else
			filter = "Gaussian";

		return filter;
	}
};
}
