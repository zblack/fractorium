#pragma once

#include "Isaac.h"

/// <summary>
/// Global utility classes and functions that don't really fit anywhere else, but are
/// too small to justify being in their own file.
/// </summary>
namespace EmberNs
{
/// <summary>
/// Thin wrapper around std::find_if() to relieve the caller of having to
/// pass the implicitly obvious .begin() and .end(), and then compare the results to .end().
/// </summary>
/// <param name="container">The container to call find_if() on</param>
/// <param name="pred">The lambda to call on each element</param>
/// <returns>True if pred returned true once, else false.</returns>
template<class c, class pr>
bool inline FindIf(c& container, pr pred)
{
	return std::find_if(container.begin(), container.end(), pred) != container.end();
}

/// <summary>
/// Thin wrapper around std::for_each() to relieve the caller of having to
/// pass the implicitly obvious .begin() and .end().
/// </summary>
/// <param name="container">The container to call for_each() on</param>
/// <param name="pred">The lambda to call on each element</param>
template<class c, class fn>
void inline ForEach(c& container, fn func)
{
	std::for_each(container.begin(), container.end(), func);
}

/// <summary>
/// After a run completes, information about what was run can be saved as strings to the comments
/// section of a jpg or png file. This class is just a container for those values.
/// </summary>
class EMBER_API EmberImageComments
{
public:
	/// <summary>
	/// Set all values to the empty string.
	/// </summary>
	void Clear()
	{
		m_Genome = "";
		m_Badvals = "";
		m_NumIters = "";
		m_Runtime = "";
	}

	string m_Genome;
	string m_Badvals;
	string m_NumIters;
	string m_Runtime;
};

/// <summary>
/// Since running is an incredibly complex process with multiple points of possible failure,
/// it's important that as much information as possible is captured if something goes wrong.
/// Classes wishing to capture this failure information will derive from this class and populate
/// the vector of strings with any useful error information. Note that a small complication can occur
/// when a class derives from this class, yet also has one or more members which do too. In that case, they should
/// override the methods to aggregate the error information from themselves, as well as their members.
/// </summary>
class EMBER_API EmberReport
{
public:
	/// <summary>
	/// Write the entire error report as a single string to the console.
	/// Derived classes with members that also derive from EmberReport should override this to capture
	/// their error information as well as that of their members.
	/// </summary>
	virtual void DumpErrorReport() { cout << ErrorReportString(); }

	/// <summary>
	/// Clear the error report string vector.
	/// Derived classes with members that also derive from EmberReport should override this to clear
	/// their error information as well as that of their members.
	/// </summary>
	virtual void ClearErrorReport() { m_ErrorReport.clear(); }

	/// <summary>
	/// Return the entire error report as a single string.
	/// Derived classes with members that also derive from EmberReport should override this to capture
	/// their error information as well as that of their members.
	/// </summary>
	/// <returns>The entire error report as a single string. Empty if no errors.</returns>
	virtual string ErrorReportString() { return StaticErrorReportString(m_ErrorReport); }

	/// <summary>
	/// Return the entire error report as a vector of strings.
	/// Derived classes with members that also derive from EmberReport should override this to capture
	/// their error information as well as that of their members.
	/// </summary>
	/// <returns>The entire error report as a vector of strings. Empty if no errors.</returns>
	virtual vector<string> ErrorReport() { return m_ErrorReport; }

	/// <summary>
	/// Add string to report.
	/// </summary>
	/// <param name="s">The string to add</param>
	virtual void AddToReport(string s) { m_ErrorReport.push_back(s); }

	/// <summary>
	/// Add a vector of strings to report.
	/// </summary>
	/// <param name="vec">The vector of strings to add</param>
	virtual void AddToReport(vector<string>& vec) { m_ErrorReport.insert(m_ErrorReport.end(), vec.begin(), vec.end()); }
	
	/// <summary>
	/// Static function to dump a vector of strings passed in.
	/// </summary>
	/// <param name="errorReport">The vector of strings to dump</param>
	static void StaticDumpErrorReport(vector<string>& errorReport) { cout << StaticErrorReportString(errorReport); }
	
	/// <summary>
	/// Static function to return the entire error report passed in as a single string.
	/// </summary>
	/// <param name="errorReport">The vector of strings to concatenate</param>
	/// <returns>A string containing all strings in the vector passed in separated by newlines</returns>
	static string StaticErrorReportString(vector<string>& errorReport)
	{
		stringstream ss;

		ForEach(errorReport, [&](string s) { ss << s << endl; });
		
		return ss.str();
	}

protected:
	vector<string> m_ErrorReport;
};

/// <summary>
/// Open a file in binary mode and read its entire contents into a vector of unsigned chars. Optionally null terminate.
/// </summary>
/// <param name="filename">The full path to the file to read</param>
/// <param name="buf">The vector which will be populated with the file's contents</param>
/// <param name="nullTerminate">Whether to append a NULL character as the last element of the vector. Needed when reading text files. Default: true.</param>
/// <returns>True if successfully read and populated, else false</returns>
static bool ReadFile(const char* filename, string& buf, bool nullTerminate = true)
{
	bool b = false;
	FILE* f;

	try
	{
		fopen_s(&f, filename, "rb");//Open in binary mode.

		if (f != NULL)
		{
			struct _stat statBuf;
			int statResult = _fstat(f->_file, &statBuf);//Get data associated with file.

			if (statResult == 0)//Check if statistics are valid.
			{
				buf.resize(statBuf.st_size + (nullTerminate ? 1 : 0));//Allocate vector to be the size of the entire file, with an optional additional character for NULL.

				if (buf.size() == statBuf.st_size + 1)//Ensure allocation succeeded.
				{
					size_t bytesRead = fread(&buf[0], 1, statBuf.st_size, f);//Read the entire file at once.

					if (bytesRead == statBuf.st_size)//Ensure the number of bytes read matched what was requested.
					{
						if (nullTerminate)//Optionally NULL terminate if they want to treat it as a string.
							buf[buf.size() - 1] = NULL;

						b = true;//Success.
					}
				}
			}

			fclose(f);
		}
	}
	catch (...)
	{
		if (f != NULL)
			fclose(f);

		b = false;
	}

	return b;
}

/// <summary>
/// Clear dest and copy all of the elements of vector source with elements of type U to the vector
/// dest with elements of type T.
/// </summary>
/// <param name="dest">The vector of type T to copy to</param>
/// <param name="source">The vector of type U to copy from</param>
template <typename T, typename U>
void CopyVec(vector<T>& dest, const vector<U>& source)
{
	dest.clear();
	dest.resize(source.size());

	for (size_t i = 0; i < source.size(); i++)
		dest[i] = T(source[i]);//Valid assignment operator between T and U types must be defined somewhere.
}

/// <summary>
/// Clear a vector of pointers to any type by checking each element for NULL and calling delete on it, then clearing the entire vector.
/// Optionally call array delete if the elements themselves are pointers to dynamically allocated arrays.
/// </summary>
/// <param name="vec">The vector to be cleared</param>
/// <param name="arrayDelete">Whether to call delete or delete []. Default: false.</param>
template <typename T>
static void ClearVec(vector<T*>& vec, bool arrayDelete = false)
{
	for (unsigned int i = 0; i < vec.size(); i++)
	{
		if (vec[i] != NULL)
		{
			if (arrayDelete)
				delete [] vec[i];
			else
				delete vec[i];
		}

		vec[i] = NULL;
	}

	vec.clear();
}

/// <summary>
/// Convert an RGBA buffer to an RGB buffer.
/// </summary>
/// <param name="rgba">The RGBA buffer</param>
/// <param name="rgb">The RGB buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
static void RgbaToRgb(vector<unsigned char>& rgba, vector<unsigned char>& rgb, unsigned int width, unsigned int height)
{
	rgb.resize(width * height * 3);

	for (unsigned int i = 0, j = 0; i < (width * height * 4); i += 4, j += 3)
	{
		rgb[j]	   = rgba[i];
		rgb[j + 1] = rgba[i + 1];
		rgb[j + 2] = rgba[i + 2];
	}
}

/// <summary>
/// Clamp and return a value to be greater than or equal to a specified minimum and less than
/// or equal to a specified maximum.
/// </summary>
/// <param name="val">The value to be clamped</param>
/// <param name="min">A value which the clamped value must be greater than or equal to</param>
/// <param name="max">A value which the clamped value must be less than or equal to</param>
/// <returns>The clamped value</returns>
template <typename T>
static inline T Clamp(T val, T min, T max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;
	else
		return val;
}

/// <summary>
/// Clamp and return a value to be greater than or equal to a specified minimum and less than
/// or equal to a specified maximum. If lesser, the value is fmod(val - min, max - min). If greater,
/// the value is max - fmod(max - val, max - min).
/// </summary>
/// <param name="val">The value to be clamped</param>
/// <param name="min">A value which the clamped value must be greater than or equal to</param>
/// <param name="max">A value which the clamped value must be less than or equal to</param>
/// <returns>The clamped and modded value</returns>
template <typename T>
static inline T ClampMod(T val, T min, T max)
{
	if (val < min)
		return min + fmod(val - min, max - min);
	else if (val > max)
		return max - fmod(max - val, max - min);
	else
		return val;
}

/// <summary>
/// Similar to Clamp(), but clamps a reference value in place rather than returning.
/// </summary>
/// <param name="val">The reference value to be clamped in place</param>
/// <param name="min">A value which the clamped value must be greater than or equal to</param>
/// <param name="max">A value which the clamped value must be less than or equal to</param>
template <typename T>
static inline void ClampRef(T& val, T min, T max)
{
	if (val < min)
		val = min;
	else if (val > max)
		val = max;
}

/// <summary>
/// Similar to Clamp(), but clamps a reference value in place rather than returning.
/// </summary>
/// <param name="val">The reference value to be clamped in place</param>
/// <param name="gte">A value which the clamped value must be less than or equal to</param>
template <typename T>
static inline void ClampLteRef(T& val, T lte)
{
	if (val > lte)
		val = lte;
}

/// <summary>
/// Clamp and return a value to be greater than or equal to a specified value.
/// Useful for ensuring something is not less than zero.
/// </summary>
/// <param name="val">The value to be clamped</param>
/// <param name="gte">A value which the clamped value must be greater than or equal to</param>
/// <returns>The clamped value</returns>
template <typename T>
static inline T ClampGte(T val, T gte)
{
	return (val < gte) ? gte : val;
}

/// <summary>
/// Similar to Clamp(), but clamps a reference value in place rather than returning.
/// </summary>
/// <param name="val">The reference value to be clamped in place</param>
/// <param name="gte">A value which the clamped value must be greater than or equal to</param>
template <typename T>
static inline void ClampGteRef(T& val, T gte)
{
	if (val < gte)
		val = gte;
}

/// <summary>
/// Thin wrapper around a call to ClampGte() with a gte value of zero.
/// </summary>
/// <param name="val">The value to be clamped</param>
/// <returns>The clamped value</returns>
template <typename T>
static inline T ClampGte0(T val)
{
	return ClampGte<T>(val, 0);
}

/// <summary>
/// Thin wrapper around a call to ClampGteRef() with a gte value of zero.
/// </summary>
/// <param name="val">The reference value to be clamped in place</param>
template <typename T>
static inline void ClampGte0Ref(T& val)
{
	ClampGteRef<T>(val, 0);
}

/// <summary>
/// Return a value rounded up or down. Works for positive and negative numbers.
/// </summary>
/// <param name="r">The value to round</param>
/// <returns>The rounded value</returns>
template <typename T>
T Round(T r)
{
	return (r > 0) ? (T)Floor<T>(r + T(0.5)) : ceil(r - T(0.5));
}

/// <summary>
/// Special rounding for certain variations, gotten from Apophysis.
/// </summary>
/// <param name="x">The value to round</param>
/// <returns>The rounded value</returns>
inline float LRint(float x)
{
	int temp = (x >= 0 ? (int)(x + 0.5f) : (int)(x - 0.5f));
	return (float)temp;
}

/// <summary>
/// Special rounding for certain variations, gotten from Apophysis.
/// </summary>
/// <param name="x">The value to round</param>
/// <returns>The rounded value</returns>
inline double LRint(double x)
{
	__int64 temp = (x >= 0 ? (__int64)(x + 0.5) : (__int64)(x - 0.5));
	return (double)temp;
}

/// <summary>
/// System floor() extremely slow because it accounts for various error conditions.
/// This is a much faster version that works on data that is not NaN.
/// </summary>
/// <param name="x">The value to return the floor of</param>
/// <returns>The floored value</returns>
template <typename T>
static inline int Floor(T val)
{
	if (val >= 0)
	{
		return (int)val;
	}
	else
	{
		int i = (int)val;//Truncate.
		return i - (i > val);//Convert trunc to floor.
	}
}

/// <summary>
/// Never really understood what this did.
/// </summary>
/// <param name="r">The value to round</param>
/// <returns>The rounded value</returns>
template <typename T>
static inline T Round6(T r)
{
	r *= 1e6;
	
	if (r < 0)
		r -= 1;

	return T(1e-6 * (int)(r + T(0.5)));
}

/// <summary>
/// Return -1 if the value is less than 0, 1 if it's greater and
/// 0 if it's equal to 0.
/// </summary>
/// <param name="v">The value to inspect</param>
/// <returns>-1, 0 or 1</returns>
template <typename T>
static inline T Sign(T v)
{
	return (v < 0) ? T(-1) : (v > 0) ? T(1) : T(0);
}

/// <summary>
/// Return -1 if the value is less than 0, 1 if it's greater.
/// This differs from Sign() in that it doesn't return 0.
/// </summary>
/// <param name="v">The value to inspect</param>
/// <returns>-1 or 1</returns>
template <typename T>
static inline T SignNz(T v)
{
	return (v < 0) ? T(-1) : T(1);
}

/// <summary>
/// Return the square of the passed in value.
/// This is useful when the value is a result of a computation
/// rather than a fixed number. Otherwise, use the SQR macro.
/// </summary>
/// <param name="v">The value to square</param>
/// <returns>The squared value</returns>
template <typename T>
static inline T Sqr(T t)
{
	return t * t;
}

/// <summary>
/// Taking the square root of numbers close to zero is dangerous.  If x is negative
/// due to floating point errors, it can return NaN results.
/// </summary>
template <typename T>
static inline T SafeSqrt(T x)
{
	if (x <= 0)
		return 0;

	return sqrt(x);
}

/// <summary>
/// Return the cube of the passed in value.
/// This is useful when the value is a result of a computation
/// rather than a fixed number. Otherwise, use the CUBE macro.
/// </summary>
/// <param name="v">The value to cube</param>
/// <returns>The cubed value</returns>
template <typename T>
static inline T Cube(T t)
{
	return t * t * t;
}

/// <summary>
/// Return the hypotenuse of the passed in values.
/// </summary>
/// <param name="x">The x distance</param>
/// <param name="y">The y distance</param>
/// <returns>The hypotenuse</returns>
template <typename T>
static inline T Hypot(T x, T y)
{
	return sqrt(SQR(x) + SQR(y));
}

/// <summary>
/// Spread the values.
/// </summary>
/// <param name="x">The x distance</param>
/// <param name="y">The y distance</param>
/// <returns>The spread</returns>
template <typename T>
static inline T Spread(T x, T y)
{
	return Hypot<T>(x, y) * ((x) > 0 ? 1 : -1);
}

/// <summary>
/// Unsure.
/// </summary>
/// <param name="x">The x distance</param>
/// <param name="y">The y distance</param>
/// <returns>The powq4</returns>
template <typename T>
static inline T Powq4(T x, T y)
{
	return pow(fabs(x), y) * SignNz(x);
}

/// <summary>
/// Unsure.
/// </summary>
/// <param name="x">The x distance</param>
/// <param name="y">The y distance</param>
/// <returns>The powq4c</returns>
template <typename T>
static inline T Powq4c(T x, T y)
{
	return y == 1 ? x : Powq4(x, y);
}

/// <summary>
/// Return EPS if the passed in value was zero, else return the value.
/// </summary>
/// <param name="x">The value</param>
/// <param name="y">The y distance</param>
/// <returns>EPS or the value if it was non-zero</returns>
template <typename T>
static inline T Zeps(T x)
{
	return x == 0 ? EPS : x;
}

/// <summary>
/// Interpolate a given percentage between two values.
/// </summary>
/// <param name="a">The first value to interpolate between.</param>
/// <param name="b">The secod value to interpolate between.</param>
/// <param name="p">The percentage between the two values to calculate.</param>
/// <returns>The interpolated value.</returns>
template <typename T>
static inline T Lerp(T a, T b, T p)
{
	return a + (b - a) * p;
}

/// <summary>
/// Thin wrapper around a call to modf that discards the integer portion
/// and returns the signed fractional portion.
/// </summary>
/// <param name="v">The value to retrieve the signed fractional portion of.</param>
/// <returns>The signed fractional portion of v.</returns>
template <typename T>
static inline T Fabsmod(T v)
{
	T dummy;

	return modf(v, &dummy);
}

/// <summary>
/// Unsure.
/// </summary>
/// <param name="p">Unsure.</param>
/// <param name="amp">Unsure.</param>
/// <param name="ph">Unsure.</param>
/// <returns>Unsure.</returns>
template <typename T>
static inline T Fosc(T p, T amp, T ph)
{
	return T(0.5) - cos(p * amp + ph) * T(0.5);
}

/// <summary>
/// Unsure.
/// </summary>
/// <param name="p">Unsure.</param>
/// <param name="ph">Unsure.</param>
/// <returns>Unsure.</returns>
template <typename T>
static inline T Foscn(T p, T ph)
{
	return T(0.5) - cos(p + ph) * T(0.5);
}

/// <summary>
/// Log scale from Apophysis.
/// </summary>
/// <param name="x">The value to log scale</param>
/// <returns>The log scaled value</returns>
template <typename T>
static inline T LogScale(T x)
{
	return x == 0 ? 0 : log((fabs(x) + 1) * T(M_E)) * SignNz(x) / T(M_E);
}

/// <summary>
/// Log map from Apophysis.
/// </summary>
/// <param name="x">The value to log map</param>
/// <returns>The log mapped value</returns>
template <typename T>
static inline T LogMap(T x)
{
	return x == 0 ? 0 : (T(M_E) + log(x * T(M_E))) * T(0.25) * SignNz(x);
}

/// <summary>
/// Thin wrapper around calling xmlStrcmp() on an Xml tag to tell
/// if its name is a given value.
/// </summary>
/// <param name="name">The name of the tag of the to inspect</param>
/// <param name="val">The value compare against</param>
/// <returns>True if the comparison matched, else false</returns>
static inline bool Compare(const xmlChar* name, char* val)
{
	return xmlStrcmp(name, XC val) != 0;
}

/// <summary>
/// Determine whether the specified value is very close to zero.
/// This is useful for determining equality of float/double types.
/// </summary>
/// <param name="val">The value to compare against</param>
/// <param name="tolerance">The tolerance. Default: 1e-6.</param>
/// <returns>True if the value was very close to zero, else false</returns>
template <typename T>
static inline bool IsNearZero(T val, T tolerance = 1e-6)
{
	return (val > -tolerance && val < tolerance);
}

/// <summary>
/// Determine whether a specified value is very close to another value.
/// This is useful for determining equality of float/double types.
/// </summary>
/// <param name="val1">The first value.</param>
/// <param name="val2">The second value.</param>
/// <param name="tolerance">The tolerance. Default: 1e-6.</param>
/// <returns>True if the values were very close to each other, else false</returns>
template <typename T>
static bool IsClose(T val1, T val2, T tolerance = 1e-6)
{
	return IsNearZero(val1 - val2, tolerance);
}

/// <summary>
/// Put an angular measurement in degrees into the range of -180 - 180.
/// </summary>
/// <param name="angle">The angle to normalize</param>
/// <returns>The normalized angle in a range of -180 - 180</returns>
template <typename T>
static inline T NormalizeDeg180(T angle)
{
	angle = fmod(angle, 360);

	if (angle > 180)
	{
		angle -= 360;
	}
	else if (angle < -180)
	{
		angle += 360;
	}

	return angle;
}

/// <summary>
/// Put an angular measurement in degrees into the range of 0 - 360.
/// </summary>
/// <param name="angle">The angle to normalize</param>
/// <returns>The normalized angle in a range of 0 - 360</returns>
template <typename T>
static inline T NormalizeDeg360(T angle)
{
	if (angle > 360 || angle < -360)
		angle = fmod(angle, 360);

	if (angle < 0)
		angle += 360;

	return angle;
}

/// <summary>
/// Return a lower case copy of a string.
/// </summary>
/// <param name="str">The string to copy and make lower case</param>
/// <returns>The lower case string</returns>
static inline string ToLower(string& str)
{
	string lower;

	lower.resize(str.size());//Allocate the destination space.
	std::transform(str.begin(), str.end(), lower.begin(), ::tolower);//Convert the source string to lower case storing the result in the destination string.
	return lower;
}

/// <summary>
/// Return an upper case copy of a string.
/// </summary>
/// <param name="str">The string to copy and make upper case</param>
/// <returns>The upper case string</returns>
static inline string ToUpper(string& str)
{
	string upper;

	upper.resize(str.size());//Allocate the destination space.
	std::transform(str.begin(), str.end(), upper.begin(), ::toupper);//Convert the source string to lower case storing the result in the destination string.
	return upper;
}

/// <summary>
/// Return a copy of a string with leading and trailing occurrences of a specified character removed.
/// The default character is a space.
/// </summary>
/// <param name="str">The string to trim</param>
/// <param name="ch">The character to trim. Default: space.</param>
/// <returns>The trimmed string</returns>
static inline string Trim(string& str, char ch = ' ')
{
	string ret;

	if (str != "")
	{
		size_t firstChar = str.find_first_not_of(ch);
		size_t lastChar = str.find_last_not_of(ch);
		
		if (firstChar == string::npos)
			firstChar = 0;

		if (lastChar == string::npos)
			lastChar = str.size();

		ret = str.substr(firstChar, lastChar - firstChar + 1);
	}

	return ret;
}

/// <summary>
/// Placeholder for a templated function to query the value of a specified system environment variable
/// of a specific type. This function does nothing as the functions for specific types implement the behavior
/// via template specialization.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <typename T>
static T Arg(char* name, T def)
{
	T t;
	return t;
}

/// <summary>
/// Template specialization for Arg<>() with a type of int.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <>
static int Arg<int>(char* name, int def)
{
	char* ch;
	int returnVal;
	size_t len;
	errno_t err = _dupenv_s(&ch, &len, name);

	if (err || !ch)
		returnVal = def;
	else
		returnVal = atoi(ch);

	free(ch);
	return returnVal;
}

/// <summary>
/// Template specialization for Arg<>() with a type of unsigned int.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <>
static unsigned int Arg<unsigned int>(char* name, unsigned int def)
{
	return Arg<int>(name, (int)def);
}

/// <summary>
/// Template specialization for Arg<>() with a type of bool.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <>
static bool Arg<bool>(char* name, bool def)
{
	return (Arg<int>(name, -999) != -999) ? true : def;
}

/// <summary>
/// Template specialization for Arg<>() with a type of double.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <>
static double Arg<double>(char* name, double def)
{
	char* ch;
	double returnVal;
	size_t len;
	errno_t err = _dupenv_s(&ch, &len, name);

	if (err || !ch)
		returnVal = def;
	else
		returnVal = atof(ch);

	free(ch);
	return returnVal;
}

/// <summary>
/// Template specialization for Arg<>() with a type of string.
/// </summary>
/// <param name="name">The name of the environment variable to query</param>
/// <param name="def">The default value to return if the environment variable was not present</param>
/// <returns>The value of the specified environment variable if found, else default</returns>
template <>
static string Arg<string>(char* name, string def)
{
	char* ch;
	string returnVal;
	size_t len;
	errno_t err = _dupenv_s(&ch, &len, name);

	if (err || !ch)
	{
		if (def != "")
			returnVal = def;
	}
	else
		returnVal = string(ch);

	free(ch);
	return returnVal;
}

/// <summary>
/// Replaces all instances of a value within a collection, with the specified value.
/// Taken from a StackOverflow.com post.
/// Modified to account for the scenario where the find and replace strings each start with
/// the same character.
/// Template argument should be any STL container.
/// </summary>
/// <param name="source">Collection to replace values in</param>
/// <param name="find">The value to replace</param>
/// <param name="replace">The value to replace with</param>
/// <returns>The number of instances replaced</returns>
template<typename T>
unsigned int inline FindAndReplace(T& source, const T& find, const T& replace)
{
	unsigned int replaceCount = 0;
	typename T::size_type fLen = find.size();
	typename T::size_type rLen = replace.size();

	for (typename T::size_type pos = 0; (pos = source.find(find, pos)) != T::npos; pos += rLen)
	{
		typename T::size_type pos2 = source.find(replace, pos);

		if (pos != pos2)
		{
			replaceCount++;
			source.replace(pos, fLen, replace);
		}
	}

	return replaceCount;
}

/// <summary>
/// Return a character pointer to a version string composed of the EMBER_OS and EMBER_VERSION values.
/// </summary>
static char* EmberVersion()
{
	return EMBER_OS "-" EMBER_VERSION;
}
}