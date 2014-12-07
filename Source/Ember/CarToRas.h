#pragma once

#include "Point.h"

/// <summary>
/// CarToRas class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// When iterating, everything is positioned in terms of a carteseian plane with 0,0 in the center like so:
/// [-1,1]			[1,1]
/// [-1,-1]			[1,-1]
/// However, when accumulating to the histogram, the data is stored in the traditional raster coordinate system
/// of 0,0 at the top left and x,y at the bottom right. This class provides functionality to convert from one
/// to the other and is used when accumulating a sub batch of iteration results to the histogram.
/// Note the functions use reference arguments for the converted values because they are slightly faster than returning a value.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API CarToRas
{
public:
	/// <summary>
	/// Empty constructor. This class should never be used unless it's been properly constructed with the constructor that takes arguments.
	/// </summary>
	CarToRas()
	{
	}

	/// <summary>
	/// Constructor that takes arguments to set up the bounds and passes them to Init().
	/// </summary>
	/// <param name="carLlX">The lower left x of the cartesian plane</param>
	/// <param name="carLlY">The lower left y of the cartesian plane</param>
	/// <param name="carUrX">The upper right x of the cartesian plane</param>
	/// <param name="carUrY">The upper right y of the cartesian plane</param>
	/// <param name="rasW">The width in pixels of the raster image/histogram</param>
	/// <param name="rasH">The height in pixels of the raster image/histogram</param>
	/// <param name="aspectRatio">The aspect ratio, generally 1</param>
	CarToRas(T carLlX, T carLlY, T carUrX, T carUrY, size_t rasW, size_t rasH, T aspectRatio)
	{
		Init(carLlX, carLlY, carUrX, carUrY, rasW, rasH, aspectRatio);
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="carToRas">The CarToRas object to copy</param>
	CarToRas(const CarToRas<T>& carToRas)
	{
		CarToRas<T>::operator=<T>(carToRas);
	}

	/// <summary>
	/// Copy constructor to copy a CarToRas object of type U.
	/// </summary>
	/// <param name="carToRas">The CarToRas object to copy</param>
	template <typename U>
	CarToRas(const CarToRas<U>& carToRas)
	{
		CarToRas<T>::operator=<U>(carToRas);
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="carToRas">The CarToRas object to copy</param>
	CarToRas<T>& operator = (const CarToRas<T>& carToRas)
	{
		if (this != &carToRas)
			CarToRas<T>::operator=<T>(carToRas);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a CarToRas object of type U.
	/// </summary>
	/// <param name="carToRas">The CarToRas object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	CarToRas<T>& operator = (const CarToRas<U>& carToRas)
	{
		m_RasWidth = carToRas.RasWidth();
		m_RasHeight = carToRas.RasHeight();
		m_OneRow = T(carToRas.OneRow());
		m_OneCol = T(carToRas.OneCol());
		m_PixPerImageUnitW = T(carToRas.PixPerImageUnitW());
		m_RasLlX = T(carToRas.RasLlX());
		m_PixPerImageUnitH = T(carToRas.PixPerImageUnitH());
		m_RasLlY = T(carToRas.RasLlY());
		m_CarLlX = T(carToRas.CarLlX());
		m_CarLlY = T(carToRas.CarLlY());
		m_CarUrX = T(carToRas.CarUrX());
		m_CarUrY = T(carToRas.CarUrY());
		m_PadCarLlX = T(carToRas.PadCarLlX());
		m_PadCarLlY = T(carToRas.PadCarLlY());
		m_PadCarUrX = T(carToRas.PadCarUrX());
		m_PadCarUrY = T(carToRas.PadCarUrY());

		return *this;
	}

	/// <summary>
	/// Initialize the dimensions with the specified bounds.
	/// </summary>
	/// <param name="carLlX">The lower left x of the cartesian plane</param>
	/// <param name="carLlY">The lower left y of the cartesian plane</param>
	/// <param name="carUrX">The upper right x of the cartesian plane</param>
	/// <param name="carUrY">The upper right y of the cartesian plane</param>
	/// <param name="rasW">The width in pixels of the raster image/histogram</param>
	/// <param name="rasH">The height in pixels of the raster image/histogram</param>
	/// <param name="aspectRatio">The aspect ratio, generally 1</param>
	void Init(T carLlX, T carLlY, T carUrX, T carUrY, size_t rasW, size_t rasH, T aspectRatio)
	{
		m_RasWidth = rasW;
		m_RasHeight = rasH;

		m_CarLlX = carLlX;
		m_CarLlY = carLlY;
		m_CarUrX = carUrX;
		m_CarUrY = carUrY;

		T carW = m_CarUrX - m_CarLlX;//Right minus left.
		T carH = m_CarUrY - m_CarLlY;//Top minus bottom.
		T invSizeW = T(1.0) / carW;
		T invSizeH = T(1.0) / carH;

		m_PixPerImageUnitW = static_cast<T>(rasW) * invSizeW;
		m_RasLlX = m_PixPerImageUnitW * carLlX;

		m_PixPerImageUnitH = static_cast<T>(rasH) * invSizeH;
		m_RasLlY = m_PixPerImageUnitH * carLlY;

		m_OneRow = abs(m_CarUrY - m_CarLlY) / m_RasHeight;
		m_OneCol = abs(m_CarUrX - m_CarLlX) / m_RasWidth;

		m_PadCarLlX = m_CarLlX + m_OneCol;
		m_PadCarUrX = m_CarUrX - m_OneCol;

		m_PadCarLlY = m_CarLlY + m_OneRow;
		m_PadCarUrY = m_CarUrY - m_OneRow;
	}

	/// <summary>
	/// Convert a cartesian x, y coordinate to a raster x, y coordinate.
	/// This will flip the Y coordinate, so points that hit the bottom of the cartesian plane will
	/// be mapped to the top of the histogram and vice versa.
	/// There is a very slim chance that a point will be right on the border and will technically be in bounds, passing the InBounds() test,
	/// but ends up being mapped to a histogram bucket that is out of bounds due to roundoff error. Perform an additional check after this call to make sure the
	/// mapped point is in bounds.
	/// </summary>
	/// <param name="cartX">The cartesian x</param>
	/// <param name="cartY">The cartesian y</param>
	/// <param name="rasX">The converted raster x</param>
	/// <param name="rasY">The converted raster y</param>
	inline void Convert(T cartX, T cartY, size_t& rasX, size_t& rasY)
	{
		rasX = static_cast<size_t>(m_PixPerImageUnitW * cartX - m_RasLlX);
		rasY = static_cast<size_t>(m_RasLlY - (m_PixPerImageUnitH * cartY));
	}

	/// <summary>
	/// Convert a cartesian x, y coordinate to a single raster buffer index.
	/// This will flip the Y coordinate, so points that hit the bottom of the cartesian plane will
	/// be mapped to the top of the histogram and vice versa.
	/// There is a very slim chance that a point will be right on the border and will technically be in bounds, passing the InBounds() test,
	/// but ends up being mapped to a histogram bucket that is out of bounds due to roundoff error. Perform an additional check after this call to make sure the
	/// mapped point is in bounds.
	/// </summary>
	/// <param name="cartX">The cartesian x</param>
	/// <param name="cartY">The cartesian y</param>
	/// <param name="singleBufferIndex">The converted single raster buffer index</param>
	inline void Convert(T cartX, T cartY, size_t& singleBufferIndex)
	{
		singleBufferIndex = static_cast<size_t>(m_PixPerImageUnitW * cartX - m_RasLlX) + (m_RasWidth * static_cast<size_t>(m_PixPerImageUnitH * cartY - m_RasLlY));
	}

	/// <summary>
	/// Convert a cartesian x, y point to a single raster buffer index.
	/// This will flip the Y coordinate, so points that hit the bottom of the cartesian plane will
	/// be mapped to the top of the histogram and vice versa.
	/// This is the most efficient possible way of converting, consisting of only
	/// a multiply and subtract per coordinate element.
	/// There is a very slim chance that a point will be right on the border and will technically be in bounds, passing the InBounds() test,
	/// but ends up being mapped to a histogram bucket that is out of bounds due to roundoff error. Perform an additional check after this call to make sure the
	/// mapped point is in bounds.
	/// </summary>
	/// <param name="point">The cartesian y</param>
	/// <param name="singleBufferIndex">The converted single raster buffer index</param>
	inline void Convert(Point<T>& point, size_t& singleBufferIndex)
	{
		singleBufferIndex = static_cast<size_t>(m_PixPerImageUnitW * point.m_X - m_RasLlX) + (m_RasWidth * static_cast<size_t>(m_PixPerImageUnitH * point.m_Y - m_RasLlY));
	}

	/// <summary>
	/// Determine if a point in the cartesian plane can be converted to a point within the raster plane.
	/// There is a very slim chance that a point will be right on the border and will technically be in bounds, passing the InBounds() test,
	/// but ends up being mapped to a histogram bucket that is out of bounds due to roundoff error. Perform an additional check after this call to make sure the
	/// mapped point is in bounds.
	/// </summary>
	/// <param name="point">The point to test</param>
	/// <returns>True if within bounds, else false</returns>
	inline bool InBounds(Point<T>& point)
	{
		//Debug check for hitting the very first pixel in the image.
		//if (point.m_Y > m_CarLlY && point.m_Y <= m_PadCarLlY && //Mapped to top row...
		//	point.m_X > m_CarLlX && point.m_X <= m_PadCarLlX)//...first col.
		//{
		//	cout << "First pixel hit." << endl;
		//}

		return point.m_X >= m_CarLlX &&
			   point.m_X < m_CarUrX &&
			   point.m_Y < m_CarUrY &&
			   point.m_Y >= m_CarLlY;
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	inline size_t RasWidth() const { return m_RasWidth; }
	inline size_t RasHeight() const { return m_RasHeight; }
	inline T OneRow() const { return m_OneRow; }
	inline T OneCol() const { return m_OneCol; }
	inline T PixPerImageUnitW() const { return m_PixPerImageUnitW; }
	inline T RasLlX() const { return m_RasLlX; }
	inline T PixPerImageUnitH() const { return m_PixPerImageUnitH; }
	inline T RasLlY() const { return m_RasLlY; }
	inline T CarLlX() const { return m_CarLlX; }
	inline T CarLlY() const { return m_CarLlY; }
	inline T CarUrX() const { return m_CarUrX; }
	inline T CarUrY() const { return m_CarUrY; }
	inline T PadCarLlX() const { return m_PadCarLlX; }
	inline T PadCarLlY() const { return m_PadCarLlY; }
	inline T PadCarUrX() const { return m_PadCarUrX; }
	inline T PadCarUrY() const { return m_PadCarUrY; }

private:
	size_t m_RasWidth, m_RasHeight;//The width and height of the raster image.
	T m_OneRow;//The distance that one raster row represents in the cartesian plane.
	T m_OneCol;//The distance that one raster column represents in the cartesian plane.
	T m_PixPerImageUnitW;//The number of columns in the raster plane that a horizontal distance of 1 in the cartesian plane represents. The higher the number, the more zoomed in.
	T m_RasLlX;//The lower left x of the raster image plane.
	T m_PixPerImageUnitH;//The number of rows in the raster plane that a vertical distance of 1 in the cartesian plane represents. The higher the number, the more zoomed in.
	T m_RasLlY;//The lower left y of the raster image plane.
	T m_CarLlX, m_CarLlY, m_CarUrX, m_CarUrY;//The bounds of the cartesian plane.
	T m_PadCarLlX, m_PadCarLlY, m_PadCarUrX, m_PadCarUrY;//The bounds of the cartesian plane padded by one raster row and column on each side.
};
}
