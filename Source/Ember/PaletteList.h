#pragma once

#include "Palette.h"

/// <summary>
/// PaletteList class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Holds a list of palettes read from an Xml file. Since the default list from flam3-palettes.xml is fairly large at 700 palettes,
/// the list member is kept as a static. This class derives from EmberReport in order to report any errors that occurred while reading the Xml.
/// Note that although the Xml color values are expected to be 0-255, they are converted and stored as normalized colors, with values from 0-1.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API PaletteList : public EmberReport
{
public:
	/// <summary>
	/// Empty constructor which does nothing.
	/// </summary>
	PaletteList()
	{
	}

	/// <summary>
	/// Read an Xml palette file into memory.
	/// This must be called before any palette file usage.
	/// </summary>
	/// <param name="filename">The full path to the file to read</param>
	/// <param name="force">If true, override the initialization state and force a read, else observe the initialization state.</param>
	/// <returns>The initialization state</returns>
	bool Init(const string& filename, bool force = false)
	{
		if (!m_Init || force)
		{
			const char* loc = __FUNCTION__;

			m_Init = false;
			m_Palettes.clear();
			m_ErrorReport.clear();
			string buf;

			if (ReadFile(filename.c_str(), buf))
			{
				xmlDocPtr doc = xmlReadMemory((const char*)buf.data(), (int)buf.size(), filename.c_str(), nullptr, XML_PARSE_NONET);

				if (doc != nullptr)
				{
					xmlNode* rootNode = xmlDocGetRootElement(doc);

					m_Palettes.reserve(buf.size() / 2048);//Roughly what it takes per palette.
					ParsePalettes(rootNode);
					xmlFreeDoc(doc);
					m_Init = m_ErrorReport.empty();
				}
				else
				{
					m_ErrorReport.push_back(string(loc) + " : Couldn't load xml doc");
				}
			}
			else
			{
				m_ErrorReport.push_back(string(loc) + " : Couldn't read palette file " + filename);
			}
		}

		return m_Init;
	}

	/// <summary>
	/// Gets the palette at a specified index.
	/// </summary>
	/// <param name="i">The index of the palette to read. A value of -1 indicates a random palette.</param>
	/// <returns>A pointer to the requested palette if the index was in range, else nullptr.</returns>
	Palette<T>* GetPalette(int i)
	{
		if (!m_Palettes.empty())
		{
			if (i == -1)
				return &m_Palettes[QTIsaac<ISAAC_SIZE, ISAAC_INT>::GlobalRand->Rand() % Size()];
			else if (i < (int)m_Palettes.size())
				return &m_Palettes[i];
		}

		return nullptr;
	}

	/// <summary>
	/// Gets a pointer to a palette with a specified name.
	/// </summary>
	/// <param name="name">The name of the palette to retrieve</param>
	/// <returns>A pointer to the palette if found, else nullptr</returns>
	Palette<T>* GetPaletteByName(const string&& name)
	{
		for (uint i = 0; i < Size(); i++)
			if (m_Palettes[i].m_Name == name)
				return &m_Palettes[i];

		return nullptr;
	}

	/// <summary>
	/// Gets a copy of the palette at a specified index with its hue adjusted by the specified amount.
	/// </summary>
	/// <param name="i">The index of the palette to read. A value of -1 indicates a random palette.</param>
	/// <param name="hue">The hue adjustment to apply</param>
	/// <param name="palette">The palette to store the output</param>
	/// <returns>True if successful, else false.</returns>
	bool GetHueAdjustedPalette(int i, T hue, Palette<T>& palette)
	{
		bool b = false;
		Palette<T>* unadjustedPal = GetPalette(i);

		if (unadjustedPal)
		{
			unadjustedPal->MakeHueAdjustedPalette(palette, hue);
			b = true;
		}

		return b;
	}

	/// <summary>
	/// Clear the palette list and reset the initialization state.
	/// </summary>
	void Clear()
	{
		m_Palettes.clear();
		m_Init = false;
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	bool Init() { return m_Init; }
	size_t Size() { return m_Palettes.size(); }

private:
	/// <summary>
	/// Parses an Xml node for all palettes present and stores in the palette list.
	/// Note that although the Xml color values are expected to be 0-255, they are converted and
	/// stored as normalized colors, with values from 0-1.
	/// </summary>
	/// <param name="node">The parent note of all palettes in the Xml file.</param>
	void ParsePalettes(xmlNode* node)
	{
		bool hexError = false;
		char* val;
		const char* loc = __FUNCTION__;
		xmlAttrPtr attr;

		while (node)
		{
			if (node->type == XML_ELEMENT_NODE && !Compare(node->name, "palette"))
			{
				attr = node->properties;
				Palette<T> palette;

				while (attr)
				{
					val = (char*)xmlGetProp(node, attr->name);

					if (!Compare(attr->name, "data"))
					{
						int colorIndex = 0;
						uint r, g, b;
						int colorCount = 0;
						hexError = false;

						do
						{
							int ret = sscanf_s((char*)&(val[colorIndex]),"00%2x%2x%2x", &r, &g, &b);

							if (ret != 3)
							{
								m_ErrorReport.push_back(string(loc) + " : Problem reading hexadecimal color data " + string(&val[colorIndex]));
								hexError = true;
								break;
							}

							colorIndex += 8;

							while (isspace((int)val[colorIndex]))
								colorIndex++;

							palette[colorCount].r = T(r) / T(255);//Store as normalized colors in the range of 0-1.
							palette[colorCount].g = T(g) / T(255);
							palette[colorCount].b = T(b) / T(255);

							colorCount++;
						} while (colorCount < COLORMAP_LENGTH);
					}
					else if (!Compare(attr->name, "number"))
					{
						palette.m_Index = atoi(val);
					}
					else if (!Compare(attr->name, "name"))
					{
						palette.m_Name = string(val);
					}

					xmlFree(val);
					attr = attr->next;
				}

				if (!hexError)
				{
					m_Palettes.push_back(palette);
				}
			}
			else
			{
				ParsePalettes(node->children);
			}

			node = node->next;
		}
	}

	static bool m_Init;//Initialized to false in Ember.cpp, and will be set to true upon successful reading of an Xml palette file.
	static vector<Palette<T>> m_Palettes;//The vector that stores the palettes.
};
}
