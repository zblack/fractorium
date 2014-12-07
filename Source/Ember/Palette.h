#pragma once

#include "Utils.h"
#include "Isaac.h"

/// <summary>
/// Palette class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// The palette stores a set of 256 colors which are what get accumulated to the histogram
/// for each iteration. The colors come from either the main palette Xml file or directly
/// from the ember parameter file. Either way, they come in as 0-255 and get normalized to 0-1.
/// In the future, 2D palette support might be added in which case this class will have to be modified.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Palette
{
public:
	/// <summary>
	/// Constructor which sets the palette index to random and allocates space to hold the color entries.
	/// </summary>
	Palette()
	{
		m_Name = "-";
		m_Index = -1;
		m_Entries.resize(COLORMAP_LENGTH);
		Clear();
	}

	/// <summary>
	/// Constructor that takes a name various parameters. If no color buffer is specified, a default is used.
	/// This is a safety fallback, and it's highly recommended to always supply a buffer of color entries.
	/// </summary>
	/// <param name="name">The name of the palette</param>
	/// <param name="index">The index in the palette file</param>
	/// <param name="size">The size of the palette which should be 256</param>
	/// <param name="xmlPaletteEntries">A pointer to 256 color entries</param>
	Palette(const string& name, int index, uint size, v4T* xmlPaletteEntries)
	{
		m_Name = name;
		m_Index = index;
		m_Entries.resize(size);

		if (xmlPaletteEntries)
		{
			memcpy(&m_Entries[0], xmlPaletteEntries, Size() * sizeof(m_Entries[0]));
		}
		else//They passed in null, so just fill with hard coded values so they at least have something.
		{
			//Palette 15 used in the test ember file.
			byte palette15[COLORMAP_LENGTH * 4] = {
0x00, 0xda, 0xde, 0xbc, 0x00, 0xee, 0xe6, 0xc5, 0x00, 0xee, 0xf2, 0xce, 0x00, 0xee, 0xf2, 0xcf, 0x00, 0xe6, 0xee, 0xe1, 0x00, 0xea, 0xee, 0xd8, 0x00, 0xf2, 0xf1, 0xeb, 0x00, 0xf2, 0xf5, 0xd8,
0x00, 0xe6, 0xf2, 0xce, 0x00, 0xde, 0xea, 0xc5, 0x00, 0xd6, 0xda, 0xc6, 0x00, 0xce, 0xd2, 0xbc, 0x00, 0xc2, 0xca, 0xa9, 0x00, 0xbe, 0xca, 0xa0, 0x00, 0xce, 0xd6, 0xaa, 0x00, 0xde, 0xe2, 0xc5,
0x00, 0xea, 0xed, 0xce, 0x00, 0xea, 0xf2, 0xc5, 0x00, 0xde, 0xe2, 0xc5, 0x00, 0xc2, 0xca, 0xaa, 0x00, 0xae, 0xbe, 0xaa, 0x00, 0xa5, 0xb2, 0x96, 0x00, 0xa2, 0xa9, 0x8d, 0x00, 0x96, 0xa2, 0x84,
0x00, 0x8d, 0x8d, 0x7a, 0x00, 0x85, 0x89, 0x71, 0x00, 0x85, 0x8d, 0x71, 0x00, 0x85, 0x85, 0x67, 0x00, 0x79, 0x7d, 0x67, 0x00, 0x79, 0x7d, 0x67, 0x00, 0x71, 0x79, 0x5e, 0x00, 0x65, 0x6d, 0x55,
0x00, 0x4d, 0x5d, 0x42, 0x00, 0x34, 0x40, 0x25, 0x00, 0x30, 0x40, 0x25, 0x00, 0x30, 0x38, 0x1c, 0x00, 0x2c, 0x3c, 0x1c, 0x00, 0x2c, 0x34, 0x1c, 0x00, 0x24, 0x2c, 0x12, 0x00, 0x24, 0x24, 0x00,
0x00, 0x24, 0x2c, 0x09, 0x00, 0x28, 0x34, 0x09, 0x00, 0x38, 0x40, 0x12, 0x00, 0x30, 0x40, 0x1c, 0x00, 0x40, 0x50, 0x2f, 0x00, 0x55, 0x69, 0x42, 0x00, 0x65, 0x75, 0x55, 0x00, 0x6c, 0x7d, 0x5e,
0x00, 0x74, 0x8d, 0x71, 0x00, 0x74, 0x89, 0x84, 0x00, 0x74, 0x8d, 0x84, 0x00, 0x78, 0x8d, 0x84, 0x00, 0x79, 0x89, 0x7a, 0x00, 0x79, 0x85, 0x71, 0x00, 0x75, 0x7d, 0x67, 0x00, 0x71, 0x79, 0x5e,
0x00, 0x6c, 0x71, 0x5e, 0x00, 0x6d, 0x70, 0x5e, 0x00, 0x6c, 0x79, 0x5e, 0x00, 0x68, 0x75, 0x5e, 0x00, 0x69, 0x71, 0x55, 0x00, 0x6d, 0x75, 0x55, 0x00, 0x6d, 0x75, 0x55, 0x00, 0x69, 0x71, 0x55,
0x00, 0x65, 0x71, 0x55, 0x00, 0x69, 0x6d, 0x55, 0x00, 0x64, 0x71, 0x5e, 0x00, 0x68, 0x70, 0x67, 0x00, 0x68, 0x70, 0x67, 0x00, 0x68, 0x6c, 0x67, 0x00, 0x6c, 0x6c, 0x5e, 0x00, 0x71, 0x71, 0x5e,
0x00, 0x79, 0x79, 0x67, 0x00, 0x81, 0x85, 0x71, 0x00, 0x7d, 0x91, 0x71, 0x00, 0x85, 0x92, 0x7a, 0x00, 0x85, 0x92, 0x7a, 0x00, 0x7d, 0x92, 0x84, 0x00, 0x79, 0x92, 0x84, 0x00, 0x78, 0x92, 0x8d,
0x00, 0x78, 0x8d, 0x8d, 0x00, 0x74, 0x8d, 0x84, 0x00, 0x74, 0x92, 0x84, 0x00, 0x75, 0x92, 0x7a, 0x00, 0x6c, 0x85, 0x67, 0x00, 0x64, 0x79, 0x5e, 0x00, 0x59, 0x69, 0x4b, 0x00, 0xaa, 0x57, 0x00,
0x00, 0x38, 0x44, 0x1c, 0x00, 0x30, 0x3c, 0x1c, 0x00, 0x2c, 0x3c, 0x1c, 0x00, 0x34, 0x40, 0x25, 0x00, 0x50, 0x61, 0x4b, 0x00, 0x5d, 0x6d, 0x5e, 0x00, 0x64, 0x71, 0x5e, 0x00, 0x60, 0x71, 0x5e,
0x00, 0x60, 0x75, 0x5e, 0x00, 0x68, 0x75, 0x5e, 0x00, 0x6c, 0x79, 0x5e, 0x00, 0x6c, 0x79, 0x5e, 0x00, 0x71, 0x79, 0x67, 0x00, 0x70, 0x79, 0x67, 0x00, 0x6c, 0x7d, 0x67, 0x00, 0x68, 0x79, 0x67,
0x00, 0x6c, 0x79, 0x67, 0x00, 0x6c, 0x75, 0x67, 0x00, 0x71, 0x75, 0x5e, 0x00, 0x71, 0x75, 0x5e, 0x00, 0x75, 0x79, 0x5e, 0x00, 0x75, 0x7d, 0x5e, 0x00, 0x81, 0x8d, 0x5e, 0x00, 0x8d, 0x92, 0x5e,
0x00, 0x8d, 0x92, 0x67, 0x00, 0x9a, 0x9a, 0x71, 0x00, 0x9a, 0xa2, 0x7a, 0x00, 0x9a, 0xa2, 0x7a, 0x00, 0x9a, 0xa1, 0x7a, 0x00, 0x92, 0x9a, 0x71, 0x00, 0x89, 0x92, 0x67, 0x00, 0x81, 0x85, 0x5e,
0x00, 0x7d, 0x7d, 0x55, 0x00, 0x69, 0x79, 0x4b, 0x00, 0x61, 0x6d, 0x42, 0x00, 0x44, 0x4c, 0x25, 0x00, 0x38, 0x44, 0x1c, 0x00, 0x40, 0x51, 0x25, 0x00, 0x45, 0x4d, 0x25, 0x00, 0x71, 0x6d, 0x42,
0x00, 0x79, 0x7d, 0x4b, 0x00, 0x81, 0x7d, 0x55, 0x00, 0x79, 0x79, 0x55, 0x00, 0x6d, 0x75, 0x55, 0x00, 0x69, 0x7d, 0x55, 0x00, 0x6c, 0x79, 0x5e, 0x00, 0x65, 0x79, 0x54, 0x00, 0x68, 0x79, 0x5e,
0x00, 0x64, 0x79, 0x67, 0x00, 0x64, 0x79, 0x67, 0x00, 0x68, 0x75, 0x5e, 0x00, 0x64, 0x71, 0x5e, 0x00, 0x64, 0x6c, 0x5e, 0x00, 0x65, 0x6d, 0x55, 0x00, 0x4d, 0x58, 0x42, 0x00, 0x34, 0x40, 0x25,
0x00, 0x2c, 0x38, 0x1c, 0x00, 0x20, 0x28, 0x1c, 0x00, 0x1c, 0x14, 0x09, 0x00, 0x18, 0x18, 0x00, 0x00, 0x04, 0x14, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x0c, 0x18, 0x00, 0x00, 0x1c, 0x28, 0x09,
0x00, 0x24, 0x30, 0x12, 0x00, 0x3c, 0x44, 0x25, 0x00, 0x5d, 0x65, 0x55, 0x00, 0x75, 0x79, 0x55, 0x00, 0x85, 0x89, 0x5e, 0x00, 0x89, 0x91, 0x71, 0x00, 0x96, 0xa2, 0x71, 0x00, 0x9a, 0xa2, 0x7a,
0x00, 0x9e, 0xaa, 0x7a, 0x00, 0x9e, 0xaa, 0x7a, 0x00, 0xaa, 0xae, 0x71, 0x00, 0xa6, 0xaa, 0x7a, 0x00, 0xa2, 0xaa, 0x7a, 0x00, 0xa1, 0xa5, 0x7a, 0x00, 0x96, 0x9e, 0x7a, 0x00, 0x85, 0x96, 0x7a,
0x00, 0x81, 0x92, 0x7a, 0x00, 0x78, 0x92, 0x7a, 0x00, 0x75, 0x92, 0x7a, 0x00, 0x75, 0x8d, 0x7a, 0x00, 0x70, 0x81, 0x67, 0x00, 0x7d, 0x7d, 0x67, 0x00, 0x89, 0x89, 0x67, 0x00, 0x92, 0x9a, 0x71,
0x00, 0x9e, 0xaa, 0x7a, 0x00, 0xaa, 0xb6, 0x84, 0x00, 0xb2, 0xb6, 0x8d, 0x00, 0xb6, 0xba, 0x97, 0x00, 0xc2, 0xca, 0x97, 0x00, 0xb2, 0xbe, 0x8d, 0x00, 0xb2, 0xb6, 0x8d, 0x00, 0xaa, 0xb2, 0x8d,
0x00, 0xa2, 0xae, 0x84, 0x00, 0x9a, 0xa6, 0x7a, 0x00, 0x92, 0x9e, 0x7a, 0x00, 0x85, 0x9a, 0x7a, 0x00, 0x7d, 0x96, 0x7a, 0x00, 0x7d, 0x92, 0x7a, 0x00, 0x7d, 0x92, 0x84, 0x00, 0x7d, 0x92, 0x84,
0x00, 0x81, 0x96, 0x84, 0x00, 0x85, 0x96, 0x84, 0x00, 0x85, 0x96, 0x84, 0x00, 0x81, 0x92, 0x84, 0x00, 0x85, 0x9a, 0x84, 0x00, 0x85, 0x9a, 0x84, 0x00, 0x8d, 0x9a, 0x84, 0x00, 0x92, 0x96, 0x84,
0x00, 0x9e, 0xa9, 0x84, 0x00, 0xae, 0xb2, 0x84, 0x00, 0xaa, 0xba, 0x84, 0x00, 0xb2, 0xbe, 0x8d, 0x00, 0xb6, 0xc2, 0xa0, 0x00, 0xc6, 0xca, 0xa0, 0x00, 0xc6, 0xce, 0xaa, 0x00, 0xd6, 0xda, 0xb3,
0x00, 0xda, 0xe2, 0xc5, 0x00, 0xd2, 0xd6, 0xbc, 0x00, 0xbe, 0xc2, 0xa0, 0x00, 0xaa, 0xb6, 0x8d, 0x00, 0x9e, 0xa6, 0x7a, 0x00, 0x92, 0x9a, 0x71, 0x00, 0x89, 0x89, 0x71, 0x00, 0x81, 0x7d, 0x67,
0x00, 0x7d, 0x7d, 0x67, 0x00, 0x81, 0x78, 0x67, 0x00, 0x7d, 0x7d, 0x5e, 0x00, 0x79, 0x79, 0x5e, 0x00, 0x79, 0x81, 0x5e, 0x00, 0x81, 0x7d, 0x67, 0x00, 0x81, 0x7d, 0x67, 0x00, 0x81, 0x81, 0x67,
0x00, 0x81, 0x89, 0x71, 0x00, 0x85, 0x91, 0x7a, 0x00, 0x89, 0x92, 0x7a, 0x00, 0x96, 0x9d, 0x7a, 0x00, 0x96, 0x9e, 0x7a, 0x00, 0x92, 0x96, 0x84, 0x00, 0x96, 0x9a, 0x8d, 0x00, 0x92, 0x92, 0x84,
0x00, 0x89, 0x91, 0x84, 0x00, 0x81, 0x92, 0x84, 0x00, 0x7d, 0x92, 0x8d, 0x00, 0x78, 0x92, 0x8d, 0x00, 0x74, 0x92, 0x8d, 0x00, 0x78, 0x92, 0x8d, 0x00, 0x78, 0x96, 0x97, 0x00, 0x81, 0x96, 0x8d,
0x00, 0x81, 0x96, 0x8d, 0x00, 0x81, 0x9a, 0x8d, 0x00, 0x85, 0x9a, 0x8d, 0x00, 0x89, 0x9e, 0x8d, 0x00, 0x89, 0x9e, 0x8d, 0x00, 0x8d, 0xa2, 0x97, 0x00, 0x95, 0xa2, 0x97, 0x00, 0x8d, 0xa2, 0x97,
0x00, 0x96, 0xa6, 0x8d, 0x00, 0x9a, 0xa1, 0x8d, 0x00, 0x9e, 0xa9, 0x84, 0x00, 0x9e, 0xa6, 0x7a, 0x00, 0xa2, 0xa5, 0x71, 0x00, 0x9e, 0xa6, 0x71, 0x00, 0x9a, 0xa6, 0x71, 0x00, 0x95, 0x9d, 0x71 };

			for (uint i = 0; i < size; i++)
			{
				m_Entries[i].a = T(palette15[i * 4 + 0]);
				m_Entries[i].r = T(palette15[i * 4 + 1]);
				m_Entries[i].g = T(palette15[i * 4 + 2]);
				m_Entries[i].b = T(palette15[i * 4 + 3]);
			}
		}
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="palette">The Palette object to copy</param>
	Palette(const Palette<T>& palette)
	{
		Palette<T>::operator=<T>(palette);
	}

	/// <summary>
	/// Copy constructor to copy a Palette object of type U.
	/// </summary>
	/// <param name="palette">The Palette object to copy</param>
	template <typename U>
	Palette(const Palette<U>& palette)
	{
		Palette<T>::operator=<U>(palette);
	}

	/// <summary>
	/// Empty destructor.
	/// Needed to eliminate warnings about inlining.
	/// </summary>
	~Palette()
	{
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="palette">The Palette object to copy</param>
	Palette<T>& operator = (const Palette<T>& palette)
	{
		if (this != &palette)
			Palette<T>::operator=<T>(palette);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Palette object of type U.
	/// </summary>
	/// <param name="palette">The Palette object to copy</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Palette<T>& operator = (const Palette<U>& palette)
	{
		m_Index = palette.m_Index;
		m_Name = palette.m_Name;
		CopyVec(m_Entries, palette.m_Entries);

		return *this;
	}

	/// <summary>
	/// Convenience [] operator to index into the color entries vector.
	/// </summary>
	/// <param name="i">The index to get</param>
	/// <returns>The color value at the specified index</returns>
	v4T& operator[] (size_t i)
	{
		return m_Entries[i];
	}

	/// <summary>
	/// Convenience * operator to get a pointer to the beginning of the color entries vector.
	/// </summary>
	/// <returns>The address of the first element in the color entries vector</returns>
	inline v4T* operator() (void)
	{
		return &m_Entries[0];
	}

	/// <summary>
	/// The size of the color entries vector.
	/// </summary>
	/// <returns>The size of the color entries vector</returns>
	size_t Size() { return m_Entries.size(); }

	/// <summary>
	/// Set all colors to either black or white, including the alpha channel.
	/// </summary>
	/// <param name="black">Set all colors to black if true, else white</param>
	void Clear(bool black = true)
	{
		for (glm::length_t i = 0; i < Size(); i++)
		{
			for (glm::length_t j = 0; j < 4; j++)
			{
				if (black)
					m_Entries[i][j] = 0;
				else
					m_Entries[i][j] = 1;
			}
		}
	}

	/// <summary>
	/// Make a copy of this palette, adjust for hue and store in the passed in palette.
	/// This is used because one way an ember Xml can specify color is with an index in the
	/// palette Xml file and a hue rotation value.
	/// </summary>
	/// <param name="palette">The palette to store the results in</param>
	/// <param name="hue">The hue rotation to apply</param>
	void MakeHueAdjustedPalette(Palette<T>& palette, T hue)
	{
		palette.m_Index = m_Index;
		palette.m_Name = m_Name;
		palette.m_Entries.resize(Size());

		for (uint i = 0; i < Size(); i++)
		{
			size_t ii = (i * 256) / COLORMAP_LENGTH;
			T rgb[3], hsv[3];

			rgb[0] = m_Entries[ii].r;
			rgb[1] = m_Entries[ii].g;
			rgb[2] = m_Entries[ii].b;

			RgbToHsv(rgb, hsv);
			hsv[0] += hue * T(6.0);
			HsvToRgb(hsv, rgb);

			//Alpha serves as merely a hit counter that gets incremented by 1 each time, see Renderer::Accumulate() for its usage.
			//Removing it saves no memory since it's 16 byte aligned. This also means alpha is not used.
			palette[i].r = rgb[0];
			palette[i].g = rgb[1];
			palette[i].b = rgb[2];
			palette[i].a = 1;
		}
	}

	/// <summary>
	/// More advanced adjustment than MakeHueAdjustedPalette() provides.
	/// Adjustments are applied in the order:
	/// Frequency, index rotation, hue rotation, saturation, brightness, contrast, blur.
	/// </summary>
	/// <param name="palette">The palette to store the result in</param>
	/// <param name="rot">Index rotation.</param>
	/// <param name="hue">Hue rotation -5 - 5</param>
	/// <param name="sat">Saturation 0 - 1</param>
	/// <param name="bright">Brightness 0 - 1</param>
	/// <param name="cont">Contrast -1 - 2</param>
	/// <param name="blur">Blur 0 - 127</param>
	/// <param name="freq">Frequency 1 - 10</param>
	void MakeAdjustedPalette(Palette<T>& palette, int rot, T hue, T sat, T bright, T cont, uint blur, uint freq)
	{
		T rgb[3], hsv[3];

		if (freq > 1)
		{
			size_t n = Size() / freq;

			for (size_t j = 0; j <= freq; j++)
			{
				for (size_t i = 0; i <= n; i++)
				{
					if ((i + j * n) < Size())
					{
						palette[i + j * n].r = m_Entries[i * freq].r;
						palette[i + j * n].g = m_Entries[i * freq].g;
						palette[i + j * n].b = m_Entries[i * freq].b;
					}
				}
			}

			palette.m_Name = m_Name;
		}
		else
		{
			palette = *this;
		}

		for (size_t i = 0; i < Size(); i++)
		{
			size_t ii = (i * 256) / COLORMAP_LENGTH;

			rgb[0] = palette[(COLORMAP_LENGTH + ii - rot) % COLORMAP_LENGTH].r;//Rotation.
			rgb[1] = palette[(COLORMAP_LENGTH + ii - rot) % COLORMAP_LENGTH].g;
			rgb[2] = palette[(COLORMAP_LENGTH + ii - rot) % COLORMAP_LENGTH].b;
			RgbToHsv(rgb, hsv);
			hsv[0] += hue * T(6.0);//Hue.
			hsv[1] = Clamp<T>(hsv[1] + sat, 0, 1);//Saturation.
			HsvToRgb(hsv, rgb);
			rgb[0] = Clamp<T>(rgb[0] + bright, 0, 1);//Brightness.
			rgb[1] = Clamp<T>(rgb[1] + bright, 0, 1);
			rgb[2] = Clamp<T>(rgb[2] + bright, 0, 1);
			rgb[0] = Clamp<T>(((rgb[0] - T(0.5)) * (cont + T(1.0))) + T(0.5), 0, 1);//Contrast.
			rgb[1] = Clamp<T>(((rgb[1] - T(0.5)) * (cont + T(1.0))) + T(0.5), 0, 1);
			rgb[2] = Clamp<T>(((rgb[2] - T(0.5)) * (cont + T(1.0))) + T(0.5), 0, 1);

			//Alpha serves as merely a hit counter that gets incremented by 1 each time, see Renderer::Accumulate() for its usage.
			//Removing it saves no memory since it's 16 byte aligned.
			palette[i].r = rgb[0];
			palette[i].g = rgb[1];
			palette[i].b = rgb[2];
			palette[i].a = 1;
		}

		if (blur > 0)
		{
			Palette<T> blurPal = palette;

			for (int i = 0; i < 256; i++)
			{
				int n = -1;

				rgb[0] = 0;
				rgb[1] = 0;
				rgb[2] = 0;

				for (int j = i - int(blur); j <= i + int(blur); j++)
				{
					n++;
					int k = (256 + j) % 256;

					if (k != i)
					{
						rgb[0] = rgb[0] + blurPal[k].r;
						rgb[1] = rgb[1] + blurPal[k].g;
						rgb[2] = rgb[2] + blurPal[k].b;
					}
				}

				if (n != 0)
				{
					palette[i].r = rgb[0] / n;
					palette[i].g = rgb[1] / n;
					palette[i].b = rgb[2] / n;
				}
			}
		}
	}

	/// <summary>
	/// Make a copy of this palette and multiply all RGB values by a scalar.
	/// </summary>
	/// <param name="palette">The palette to store the result in</param>
	/// <param name="colorScalar">The color scalar to multiply each RGB value by</param>
	template<typename bucketT>
	void MakeDmap(Palette<bucketT>& palette, T colorScalar = 1)
	{
		palette.m_Index = m_Index;
		palette.m_Name = m_Name;

		if (palette.Size() != Size())
			palette.m_Entries.resize(Size());

		for (uint j = 0; j < palette.Size(); j++)
		{
			palette.m_Entries[j] = m_Entries[j] * colorScalar;
			palette.m_Entries[j].a = 1;
		}
	}

	/// <summary>
	/// Make a buffer with the color values of this palette scaled to 255
	/// and repeated for a number of rows.
	/// Convenience function for displaying this palette on a GUI.
	/// </summary>
	/// <param name="height">The height of the output block</param>
	/// <returns>A vector holding the color values</returns>
	vector<byte> MakeRgbPaletteBlock(uint height)
	{
		size_t width = Size();
		vector<byte> v(height * width * 3);

		if (v.size() == (height * Size() * 3))
		{
			for (uint i = 0; i < height; i++)
			{
				for (uint j = 0; j < width; j++)
				{
					v[(width * 3 * i) + (j * 3)]     = byte(m_Entries[j][0] * T(255));//Palettes are as [0..1], so convert to [0..255] here since it's for GUI display.
					v[(width * 3 * i) + (j * 3) + 1] = byte(m_Entries[j][1] * T(255));
					v[(width * 3 * i) + (j * 3) + 2] = byte(m_Entries[j][2] * T(255));
				}
			}
		}

		return v;
	}

	/// <summary>
	/// Convert RGB to HSV.
	/// </summary>
	/// <param name="r">Red 0 - 1</param>
	/// <param name="g">Green 0 - 1</param>
	/// <param name="b">Blue 0 - 1</param>
	/// <param name="h">Hue 0 - 6</param>
	/// <param name="s">Saturation 0 - 1</param>
	/// <param name="v">Value 0 - 1</param>
	static void RgbToHsv(T r, T g, T b, T& h, T& s, T& v)
	{
		T max, min, del, rc, gc, bc;

		max = std::max(std::max(r, g), b);//Compute maximum of r, g, b.
		min = std::min(std::min(r, g), b);//Compute minimum of r, g, b.

		del = max - min;
		v = max;
		s = (max != 0) ? (del / max) : 0;
		h = 0;

		if (s != 0)
		{
			rc = (max - r) / del;
			gc = (max - g) / del;
			bc = (max - b) / del;

			if (r == max)
				h = bc - gc;
			else if (g == max)
				h = 2 + rc - bc;
			else if (b == max)
				h = 4 + gc - rc;

			if (h < 0)
				h += 6;
		}
	}

	/// <summary>
	/// Wrapper around RgbToHsv() which takes buffers as parameters instead of individual components.
	/// </summary>
	/// <param name="rgb">The RGB buffer</param>
	/// <param name="hsv">The HSV buffer</param>
	static void RgbToHsv(T* rgb, T* hsv)
	{
		RgbToHsv(rgb[0], rgb[1], rgb[2], hsv[0], hsv[1], hsv[2]);
	}

	/// <summary>
	/// Convert HSV to RGB.
	/// </summary>
	/// <param name="h">Hue 0 - 6</param>
	/// <param name="s">Saturation 0 - 1</param>
	/// <param name="v">Value 0 - 1</param>
	/// <param name="r">Red 0 - 1</param>
	/// <param name="g">Green 0 - 1</param>
	/// <param name="b">Blue 0 - 1</param>
	static void HsvToRgb(T h, T s, T v, T& r, T& g, T& b)
	{
		int j;
		T f, p, q, t;

		while (h >= 6)
			h -= 6;

		while (h <  0)
			h += 6;

		j = Floor<T>(h);
		f = h - j;
		p = v * (1 - s);
		q = v * (1 - (s * f));
		t = v * (1 - (s * (1 - f)));

		switch (j)
		{
			case 0:  r = v;  g = t;  b = p;  break;
			case 1:  r = q;  g = v;  b = p;  break;
			case 2:  r = p;  g = v;  b = t;  break;
			case 3:  r = p;  g = q;  b = v;  break;
			case 4:  r = t;  g = p;  b = v;  break;
			case 5:  r = v;  g = p;  b = q;  break;
			default: r = v;  g = t;  b = p;  break;
		}
	}

	/// <summary>
	/// Wrapper around HsvToRgb() which takes buffers as parameters instead of individual components.
	/// </summary>
	/// <param name="hsv">The HSV buffer</param>
	/// <param name="rgb">The RGB buffer</param>
	static void HsvToRgb(T* hsv, T* rgb)
	{
		HsvToRgb(hsv[0], hsv[1], hsv[2], rgb[0], rgb[1], rgb[2]);
	}

	/// <summary>
	/// Calculates the alpha.
	/// Used for gamma correction in final accumulation.
	/// Not the slightest clue what this is doing.
	/// </summary>
	/// <param name="density">Density</param>
	/// <param name="gamma">Gamma</param>
	/// <param name="linrange">Linear range</param>
	/// <returns>Alpha</returns>
	static T CalcAlpha(T density, T gamma, T linrange)
	{
		T frac, alpha;
		T funcval = pow(linrange, gamma);

		if (density > 0)
		{
			if (density < linrange)
			{
				frac = density / linrange;
				alpha = (T(1.0) - frac) * density * (funcval / linrange) + frac * pow(density, gamma);
			}
			else
				alpha = pow(density, gamma);
		}
		else
			alpha = 0;

		return alpha;
	}

	/// <summary>
	/// Calculates the new RGB and stores in the supplied buffer.
	/// Used for gamma correction in final accumulation.
	/// Not the slightest clue what this is doing.
	/// </summary>
	/// <param name="cBuf">The input RGB color buffer 0 - 1</param>
	/// <param name="ls">Log scaling</param>
	/// <param name="highPow">Highlight power, -1 - 1</param>
	/// <param name="newRgb">Newly computed RGB value</param>
	template<typename bucketT>
	static void CalcNewRgb(bucketT* cBuf, T ls, T highPow, bucketT* newRgb)
	{
		int rgbi;
		T newls, lsratio;
		bucketT newhsv[3];
		T maxa, maxc;
		T adjustedHighlight;

		if (ls == 0 || (cBuf[0] == 0 && cBuf[1] == 0 && cBuf[2] == 0))
		{
			newRgb[0] = 0;
			newRgb[1] = 0;
			newRgb[2] = 0;
			return;
		}

		//Identify the most saturated channel.
		maxc = max(max(cBuf[0], cBuf[1]), cBuf[2]);
		maxa = ls * maxc;

		//If a channel is saturated and highlight power is non-negative
		//modify the color to prevent hue shift.
		if (maxa > 255 && highPow >= 0)
		{
			newls = T(255.0) / maxc;
			lsratio = pow(newls / ls, highPow);

			//Calculate the max-value color (ranged 0 - 1).
			for (rgbi = 0; rgbi < 3; rgbi++)
				newRgb[rgbi] = bucketT(newls) * cBuf[rgbi] / bucketT(255.0);

			//Reduce saturation by the lsratio.
			Palette<bucketT>::RgbToHsv(newRgb, newhsv);
			newhsv[1] *= bucketT(lsratio);
			Palette<bucketT>::HsvToRgb(newhsv, newRgb);

			for (rgbi = 0; rgbi < 3; rgbi++)
				newRgb[rgbi] *= T(255.0);
		}
		else
		{
			newls = T(255.0) / maxc;
			adjustedHighlight = -highPow;

			if (adjustedHighlight > 1)
				adjustedHighlight = 1;

			if (maxa <= 255)
				adjustedHighlight = 1;

			//Calculate the max-value color (ranged 0 - 1) interpolated with the old behavior.
			for (rgbi = 0; rgbi < 3; rgbi++)
				newRgb[rgbi] = bucketT((T(1.0) - adjustedHighlight) * newls + adjustedHighlight * ls) * cBuf[rgbi];
		}
	}

	int m_Index;//Index in the xml palette file of this palette, use -1 for random.
	string m_Name;//Name of this palette.
	vector<v4T> m_Entries;//Storage for the color values.
};
}
