#pragma once

#include "Utils.h"
#include "PaletteList.h"
#include "VariationList.h"

/// <summary>
/// XmlToEmber and Locale classes.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Convenience class for setting and resetting the locale.
/// It's set up in the constructor and restored in the destructor.
/// This relieves the caller of having to manually do it everywhere.
/// </summary>
class EMBER_API Locale
{
public:
	/// <summary>
	/// Constructor which saves the state of the current locale and
	/// sets the new one based on the parameters passed in.
	/// </summary>
	/// <param name="category">The locale category. Default: LC_NUMERIC.</param>
	/// <param name="loc">The locale. Default: "C".</param>
	Locale(int category = LC_NUMERIC, const char* loc = "C")
	{
		m_Category = category;
		m_NewLocale = string(loc);
		m_OriginalLocale = setlocale(category, nullptr);//Query.

		if (m_OriginalLocale.empty())
			cout << "Couldn't get original locale." << endl;

		if (setlocale(category, loc) == nullptr)//Set.
			cout << "Couldn't set new locale " << category << ", " << loc << "." << endl;
	}

	/// <summary>
	/// Reset the locale to the value stored during construction.
	/// </summary>
	~Locale()
	{
		if (!m_OriginalLocale.empty())
			if (setlocale(m_Category, m_OriginalLocale.c_str()) == nullptr)//Restore.
				cout << "Couldn't restore original locale " << m_Category << ", " << m_OriginalLocale << "." << endl;
	}

private:
	int m_Category;
	string m_NewLocale;
	string m_OriginalLocale;
};

/// <summary>
/// Class for reading Xml files into ember objects.
/// This class derives from EmberReport, so the caller is able
/// to retrieve a text dump of error information if any errors occur.
/// Since this class contains a VariationList object, it's important to declare one
/// instance and reuse it for the duration of the program instead of creating and deleting
/// them as local variables.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API XmlToEmber : public EmberReport
{
public:
	/// <summary>
	/// Constructor that initializes the random context.
	/// </summary>
	XmlToEmber()
	{
		Timing t;

		if (!m_Init)
		{
			m_BadParamNames.reserve(100);
			m_BadParamNames.push_back(pair<string, string>("swtin_distort",   "stwin_distort"));//stwin.
			m_BadParamNames.push_back(pair<string, string>("pow_numerator",   "pow_block_numerator"));//pow_block.
			m_BadParamNames.push_back(pair<string, string>("pow_denominator", "pow_block_denominator"));
			m_BadParamNames.push_back(pair<string, string>("pow_root",        "pow_block_root"));
			m_BadParamNames.push_back(pair<string, string>("pow_correctn",    "pow_block_correctn"));
			m_BadParamNames.push_back(pair<string, string>("pow_correctd",    "pow_block_correctd"));
			m_BadParamNames.push_back(pair<string, string>("pow_power",       "pow_block_power"));
			m_BadParamNames.push_back(pair<string, string>("lT", "linearT_powX"));//linearT.
			m_BadParamNames.push_back(pair<string, string>("lT", "linearT_powY"));
			m_BadParamNames.push_back(pair<string, string>("Re_A", "Mobius_Re_A"));//Mobius.
			m_BadParamNames.push_back(pair<string, string>("Im_A", "Mobius_Im_A"));
			m_BadParamNames.push_back(pair<string, string>("Re_B", "Mobius_Re_B"));
			m_BadParamNames.push_back(pair<string, string>("Im_B", "Mobius_Im_B"));
			m_BadParamNames.push_back(pair<string, string>("Re_C", "Mobius_Re_C"));
			m_BadParamNames.push_back(pair<string, string>("Im_C", "Mobius_Im_C"));
			m_BadParamNames.push_back(pair<string, string>("Re_D", "Mobius_Re_D"));
			m_BadParamNames.push_back(pair<string, string>("Im_D", "Mobius_Im_D"));
			m_BadParamNames.push_back(pair<string, string>("rx_sin", "rotate_x_sin"));//rotate_x.
			m_BadParamNames.push_back(pair<string, string>("rx_cos", "rotate_x_cos"));
			m_BadParamNames.push_back(pair<string, string>("ry_sin", "rotate_y_sin"));//rotate_y.
			m_BadParamNames.push_back(pair<string, string>("ry_cos", "rotate_y_cos"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_a1", "interference2_a1"));//interference2.
			m_BadParamNames.push_back(pair<string, string>("intrfr2_b1", "interference2_b1"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_c1", "interference2_c1"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_p1", "interference2_p1"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_t1", "interference2_t1"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_a2", "interference2_a2"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_b2", "interference2_b2"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_c2", "interference2_c2"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_p2", "interference2_p2"));
			m_BadParamNames.push_back(pair<string, string>("intrfr2_t2", "interference2_t2"));
			m_BadParamNames.push_back(pair<string, string>("octa_x", "octagon_x"));//octagon.
			m_BadParamNames.push_back(pair<string, string>("octa_y", "octagon_y"));
			m_BadParamNames.push_back(pair<string, string>("octa_z", "octagon_z"));
			m_BadParamNames.push_back(pair<string, string>("bubble_x", "bubble2_x"));//bubble2.
			m_BadParamNames.push_back(pair<string, string>("bubble_y", "bubble2_y"));
			m_BadParamNames.push_back(pair<string, string>("bubble_z", "bubble2_z"));
			m_BadParamNames.push_back(pair<string, string>("cubic3D_xpand", "cubicLattice_3D_xpand"));//cubicLattice_3D.
			m_BadParamNames.push_back(pair<string, string>("cubic3D_style", "cubicLattice_3D_style"));
			m_BadParamNames.push_back(pair<string, string>("splitb_x",  "SplitBrdr_x"));//SplitBrdr.
			m_BadParamNames.push_back(pair<string, string>("splitb_y",  "SplitBrdr_y"));
			m_BadParamNames.push_back(pair<string, string>("splitb_px", "SplitBrdr_px"));
			m_BadParamNames.push_back(pair<string, string>("splitb_py", "SplitBrdr_py"));
			m_BadParamNames.push_back(pair<string, string>("dc_cyl_offset", "dc_cylinder_offset"));//dc_cylinder.
			m_BadParamNames.push_back(pair<string, string>("dc_cyl_angle",  "dc_cylinder_angle"));
			m_BadParamNames.push_back(pair<string, string>("dc_cyl_scale",  "dc_cylinder_scale"));
			m_BadParamNames.push_back(pair<string, string>("cyl_x",         "dc_cylinder_x"));
			m_BadParamNames.push_back(pair<string, string>("cyl_y",         "dc_cylinder_y"));
			m_BadParamNames.push_back(pair<string, string>("cyl_blur",      "dc_cylinder_blur"));
			m_BadParamNames.push_back(pair<string, string>("mobius_radius",   "mobius_strip_radius"));//mobius_strip.
			m_BadParamNames.push_back(pair<string, string>("mobius_width",    "mobius_strip_width"));
			m_BadParamNames.push_back(pair<string, string>("mobius_rect_x",   "mobius_strip_rect_x"));
			m_BadParamNames.push_back(pair<string, string>("mobius_rect_y",   "mobius_strip_rect_y"));
			m_BadParamNames.push_back(pair<string, string>("mobius_rotate_x", "mobius_strip_rotate_x"));
			m_BadParamNames.push_back(pair<string, string>("mobius_rotate_y", "mobius_strip_rotate_y"));
			m_BadParamNames.push_back(pair<string, string>("bwraps2_cellsize",    "bwraps_cellsize"));//bwraps2.
			m_BadParamNames.push_back(pair<string, string>("bwraps2_space",       "bwraps_space"));
			m_BadParamNames.push_back(pair<string, string>("bwraps2_gain",        "bwraps_gain"));
			m_BadParamNames.push_back(pair<string, string>("bwraps2_inner_twist", "bwraps_inner_twist"));
			m_BadParamNames.push_back(pair<string, string>("bwraps2_outer_twist", "bwraps_outer_twist"));
			m_BadParamNames.push_back(pair<string, string>("bwraps7_cellsize",    "bwraps_cellsize"));//bwraps7.
			m_BadParamNames.push_back(pair<string, string>("bwraps7_space",       "bwraps_space"));
			m_BadParamNames.push_back(pair<string, string>("bwraps7_gain",        "bwraps_gain"));
			m_BadParamNames.push_back(pair<string, string>("bwraps7_inner_twist", "bwraps_inner_twist"));
			m_BadParamNames.push_back(pair<string, string>("bwraps7_outer_twist", "bwraps_outer_twist"));
			m_BadParamNames.push_back(pair<string, string>("pre_bwraps2_cellsize",    "pre_bwraps_cellsize"));
			m_BadParamNames.push_back(pair<string, string>("pre_bwraps2_space",       "pre_bwraps_space"));
			m_BadParamNames.push_back(pair<string, string>("pre_bwraps2_gain",        "pre_bwraps_gain"));
			m_BadParamNames.push_back(pair<string, string>("pre_bwraps2_inner_twist", "pre_bwraps_inner_twist"));
			m_BadParamNames.push_back(pair<string, string>("pre_bwraps2_outer_twist", "pre_bwraps_outer_twist"));
			m_BadParamNames.push_back(pair<string, string>("post_bwraps2_cellsize",    "post_bwraps_cellsize"));
			m_BadParamNames.push_back(pair<string, string>("post_bwraps2_space",       "post_bwraps_space"));
			m_BadParamNames.push_back(pair<string, string>("post_bwraps2_gain",        "post_bwraps_gain"));
			m_BadParamNames.push_back(pair<string, string>("post_bwraps2_inner_twist", "post_bwraps_inner_twist"));
			m_BadParamNames.push_back(pair<string, string>("post_bwraps2_outer_twist", "post_bwraps_outer_twist"));

			m_FlattenNames.reserve(24);
			m_FlattenNames.push_back("pre_crop");
			m_FlattenNames.push_back("pre_falloff2");
			m_FlattenNames.push_back("pre_rotate_x");
			m_FlattenNames.push_back("pre_rotate_y");
			m_FlattenNames.push_back("pre_ztranslate");

			m_FlattenNames.push_back("blur3D");
			m_FlattenNames.push_back("bubble");
			m_FlattenNames.push_back("bwraps");
			m_FlattenNames.push_back("bwraps2");
			m_FlattenNames.push_back("crop");
			m_FlattenNames.push_back("cylinder");
			m_FlattenNames.push_back("falloff2");
			m_FlattenNames.push_back("hemisphere");
			m_FlattenNames.push_back("julia3D");
			m_FlattenNames.push_back("julia3Dz");
			m_FlattenNames.push_back("linear3D");
			m_FlattenNames.push_back("zblur");
			m_FlattenNames.push_back("zcone");
			m_FlattenNames.push_back("ztranslate");

			m_FlattenNames.push_back("post_crop");
			m_FlattenNames.push_back("post_falloff2");
			m_FlattenNames.push_back("post_rotate_x");
			m_FlattenNames.push_back("post_rotate_y");
			
			m_FlattenNames.push_back("curl3D_cz");

			//This is a vector of the param names as they are in the legacy, badly named flam3/Apophysis code.
			vector<string> badParams;

			badParams.reserve(6);

			badParams.push_back("bwraps7_cellsize");
			badParams.push_back("bwraps7_space");
			badParams.push_back("bwraps7_gain");
			badParams.push_back("bwraps7_inner_twist");
			badParams.push_back("bwraps7_outer_twist");
			m_BadVariationNames.push_back(make_pair(make_pair(string("bwraps7"), string("bwraps")), badParams));//bwraps7 is the same as bwraps.
			badParams.clear();

			badParams.push_back("bwraps2_cellsize");
			badParams.push_back("bwraps2_space");
			badParams.push_back("bwraps2_gain");
			badParams.push_back("bwraps2_inner_twist");
			badParams.push_back("bwraps2_outer_twist");
			m_BadVariationNames.push_back(make_pair(make_pair(string("bwraps2"), string("bwraps")), badParams));//bwraps2 is the same as bwraps.
			badParams.clear();

			badParams.push_back("pre_bwraps2_cellsize");
			badParams.push_back("pre_bwraps2_space");
			badParams.push_back("pre_bwraps2_gain");
			badParams.push_back("pre_bwraps2_inner_twist");
			badParams.push_back("pre_bwraps2_outer_twist");
			m_BadVariationNames.push_back(make_pair(make_pair(string("pre_bwraps2"), string("pre_bwraps")), badParams));
			badParams.clear();

			badParams.push_back("post_bwraps2_cellsize");
			badParams.push_back("post_bwraps2_space");
			badParams.push_back("post_bwraps2_gain");
			badParams.push_back("post_bwraps2_inner_twist");
			badParams.push_back("post_bwraps2_outer_twist");
			m_BadVariationNames.push_back(make_pair(make_pair(string("post_bwraps2"), string("post_bwraps")), badParams));
			badParams.clear();

			badParams.push_back("mobius_radius");
			badParams.push_back("mobius_width");
			badParams.push_back("mobius_rect_x");
			badParams.push_back("mobius_rect_y");
			badParams.push_back("mobius_rotate_x");
			badParams.push_back("mobius_rotate_y");
			m_BadVariationNames.push_back(make_pair(make_pair(string("mobius"),	string("mobius_strip")), badParams));//mobius_strip clashes with Mobius.
			badParams.clear();

			badParams.push_back("post_dcztransl_x0");
			badParams.push_back("post_dcztransl_x1");
			badParams.push_back("post_dcztransl_factor");
			badParams.push_back("post_dcztransl_overwrite");
			badParams.push_back("post_dcztransl_clamp");
			m_BadVariationNames.push_back(make_pair(make_pair(string("post_dcztransl"), string("post_dc_ztransl")), badParams));
			badParams.clear();

			m_BadVariationNames.push_back(make_pair(make_pair(string("pre_blur"),    string("pre_gaussian_blur")), badParams));
			m_BadVariationNames.push_back(make_pair(make_pair(string("pre_spin_z"),  string("pre_rotate_z")),      badParams));
			m_BadVariationNames.push_back(make_pair(make_pair(string("post_spin_z"), string("post_rotate_z")),     badParams));

			m_Init = true;
		}
	}

	/// <summary>
	/// Parse the specified buffer and place the results in the vector of embers passed in.
	/// </summary>
	/// <param name="buf">The buffer to parse</param>
	/// <param name="filename">Full path and filename, optionally empty</param>
	/// <param name="embers">The newly constructed embers based on what was parsed</param>
	/// <returns>True if there were no errors, else false.</returns>
	bool Parse(unsigned char* buf, const char* filename, vector<Ember<T>>& embers)
	{
		char* bn;
		const char* xmlPtr;
		const char* loc = __FUNCTION__;
		size_t emberSize;
		size_t bufSize;
		xmlDocPtr doc;//Parsed XML document tree.
		xmlNodePtr rootnode;
		Locale locale;//Sets and restores on exit.
		//Timing t;
		m_ErrorReport.clear();

		//Parse XML string into internal document.
		xmlPtr = (const char*)(&buf[0]);
		bufSize = strlen(xmlPtr);
		embers.reserve(bufSize / 2500);//The Xml text for an ember is around 2500 bytes, but can be much more. Pre-allocate to aovid unnecessary resizing.
		doc = xmlReadMemory(xmlPtr, (int)bufSize, filename, "ISO-8859-1", XML_PARSE_NONET);//Forbid network access during read.
		//t.Toc("xmlReadMemory");

		if (doc == nullptr)
		{
			m_ErrorReport.push_back(string(loc) + " : Error parsing xml file " + string(filename));
			return false;
		}

		//What is the root node of the document?
		rootnode = xmlDocGetRootElement(doc);

		//Scan for <flame> nodes, starting with this node.
		//t.Tic();
		bn = basename((char*)filename);
		ScanForEmberNodes(rootnode, bn, embers);
		xmlFreeDoc(doc);
		emberSize = embers.size();
		//t.Toc("ScanForEmberNodes");

		//Check to see if the first control point or the second-to-last
		//control point has interpolation="smooth".  This is invalid
		//and should be reset to linear (with a warning).
		if (emberSize > 0)
		{
			if (embers[0].m_Interp == EMBER_INTERP_SMOOTH)
			{
				cout << "Warning: smooth interpolation cannot be used for first segment.\n         switching to linear.\n" << endl;
				embers[0].m_Interp = EMBER_INTERP_LINEAR;
			}

			if (emberSize >= 2 && embers[emberSize - 2].m_Interp == EMBER_INTERP_SMOOTH)
			{
				cout << "Warning: smooth interpolation cannot be used for last segment.\n         switching to linear.\n" << endl;
				embers[emberSize - 2].m_Interp = EMBER_INTERP_LINEAR;
			}
		}

		//Finally, ensure that consecutive 'rotate' parameters never exceed
		//a difference of more than 180 degrees (+/-) for interpolation.
		//An adjustment of +/- 360 degrees is made until this is true.
		if (emberSize > 1)
		{
			for (unsigned int i = 1; i < emberSize; i++)
			{
				//Only do this adjustment if not in compat mode..
				if (embers[i - 1].m_AffineInterp != INTERP_COMPAT && embers[i - 1].m_AffineInterp != INTERP_OLDER)
				{
					while (embers[i].m_Rotate < embers[i - 1].m_Rotate - 180)
						embers[i].m_Rotate += 360;

					while (embers[i].m_Rotate > embers[i - 1].m_Rotate + 180)
						embers[i].m_Rotate -= 360;
				}
			}
		}

		return true;
	}

	/// <summary>
	/// Parse the specified file and place the results in the vector of embers passed in.
	/// This will strip out ampersands because the Xml parser can't handle them.
	/// </summary>
	/// <param name="filename">Full path and filename</param>
	/// <param name="embers">The newly constructed embers based on what was parsed</param>
	/// <returns>True if there were no errors, else false.</returns>
	bool Parse(const char* filename, vector<Ember<T>>& embers)
	{
		const char* loc = __FUNCTION__;
		string buf;

		//Ensure palette list is setup first.
		if (!m_PaletteList.Init())
		{
			m_ErrorReport.push_back(string(loc) + " : Palette list must be initialized before parsing embers.");
			return false;
		}

		if (ReadFile(filename, buf))
		{
			std::replace(buf.begin(), buf.end(), '&', '+');
			return Parse((unsigned char*)buf.data(), filename, embers);
		}
		else
			return false;
	}

	/// <summary>
	/// Convert the string to a floating point value and return a bool indicating success.
	/// See error report for errors.
	/// </summary>
	/// <param name="str">The string to convert</param>
	/// <param name="val">The converted value</param>
	/// <returns>True if success, else false.</returns>
	bool Atof(const char* str, T& val)
	{
		bool b = true;
		char* endp;
		const char* loc = __FUNCTION__;

		//Reset errno.
		errno = 0;//Note that this is not thread-safe.

		//Convert the string using strtod().
		val = (T)strtod(str, &endp);

		//Check errno & return string.
		if (endp != str + strlen(str))
		{
			m_ErrorReport.push_back(string(loc) + " : Error converting " + string(str) + ", extra chars");
			b = false;
		}

		if (errno)
		{
			m_ErrorReport.push_back(string(loc) + " : Error converting " + string(str));
			b = false;
		}

		return b;
	}

	/// <summary>
	/// Thin wrapper around Atoi().
	/// See error report for errors.
	/// </summary>
	/// <param name="str">The string to convert</param>
	/// <param name="val">The converted unsigned integer value</param>
	/// <returns>True if success, else false.</returns>
	bool Atoi(const char* str, unsigned int& val)
	{
		return Atoi(str, (int&)val);
	}

	/// <summary>
	/// Convert the string to an unsigned integer value and return a bool indicating success.
	/// See error report for errors.
	/// </summary>
	/// <param name="str">The string to convert</param>
	/// <param name="val">The converted unsigned integer value</param>
	/// <returns>True if success, else false.</returns>
	bool Atoi(const char* str, int& val)
	{
		bool b = true;
		char* endp;
		const char* loc = __FUNCTION__;

		//Reset errno.
		errno = 0;//Note that this is not thread-safe.

		//Convert the string using strtod().
		val = strtol(str, &endp, 10);

		//Check errno & return string.
		if (endp != str + strlen(str))
		{
			m_ErrorReport.push_back(string(loc) + " : Error converting " + string(str) + ", extra chars");
			b = false;
		}

		if (errno)
		{
			m_ErrorReport.push_back(string(loc) + " : Error converting " + string(str));
			b = false;
		}

		return b;
	}

	/// <summary>
	/// Convert an integer to a string.
	/// Just a wrapper around _itoa_s() which wraps the result in a std::string.
	/// </summary>
	/// <param name="i">The integer to convert</param>
	/// <param name="radix">The radix of the integer. Default: 10.</param>
	/// <returns>The converted string</returns>
	static string Itos(int i, int radix = 10)
	{
		char ch[16];

#ifdef WIN32
		_itoa_s(i, ch, 16, radix);
#else
		sprintf(ch, "%d", i);
#endif
		return string(ch);
	}

	/// <summary>
	/// Convert an unsigned 64-bit integer to a string.
	/// Just a wrapper around _ui64toa_s() which wraps the result in a std::string.
	/// </summary>
	/// <param name="i">The unsigned 64-bit integer to convert</param>
	/// <param name="radix">The radix of the integer. Default: 10.</param>
	/// <returns>The converted string</returns>
	static string Itos64(size_t i, int radix = 10)
	{
		char ch[64];

#ifdef WIN32
		_ui64toa_s(i, ch, 64, radix);
#else
		sprintf(ch, "%lu", i);
#endif
		return string(ch);
	}

	static vector<string> m_FlattenNames;

private:
	/// <summary>
	/// Scan the file for ember nodes, and parse them out into the vector of embers.
	/// </summary>
	/// <param name="curNode">The current node to parse</param>
	/// <param name="parentFile">The full path and filename</param>
	/// <param name="embers">The newly constructed embers based on what was parsed</param>
	void ScanForEmberNodes(xmlNode* curNode, char* parentFile, vector<Ember<T>>& embers)
	{
		bool parseEmberSuccess;
		xmlNodePtr thisNode = nullptr;
		const char* loc = __FUNCTION__;
		string parentFileString = string(parentFile);

		//Original memset to 0, but the constructors should handle that.
		//Loop over this level of elements.
		for (thisNode = curNode; thisNode; thisNode = thisNode->next)
		{
			//Check to see if this element is a <ember> element.
			if (thisNode->type == XML_ELEMENT_NODE && !Compare(thisNode->name, "flame"))
			{
				Ember<T> currentEmber;//Place this inside here so its constructor is called each time.

				parseEmberSuccess = ParseEmberElement(thisNode, currentEmber);

				if (!parseEmberSuccess)
				{
					//Original leaked memory here, ours doesn't.
					m_ErrorReport.push_back(string(loc) + " : Error parsing ember element");
					return;
				}

				if (currentEmber.PaletteIndex() != -1)
				{
					if (!m_PaletteList.GetHueAdjustedPalette(currentEmber.PaletteIndex(), currentEmber.m_Hue, currentEmber.m_Palette))
					{
						m_ErrorReport.push_back(string(loc) + " : Error assigning palette with index " + Itos(currentEmber.PaletteIndex()));
					}
				}

				//if (!Interpolater<T>::InterpMissingColors(currentEmber.m_Palette.m_Entries))
				//	m_ErrorReport.push_back(string(loc) + " : Error interpolating missing palette colors");

				currentEmber.CacheXforms();
				currentEmber.m_Index = embers.size();
				currentEmber.m_ParentFilename = parentFileString;
				embers.push_back(currentEmber);
			}
			else
			{
				//Check all of the children of this element.
				ScanForEmberNodes(thisNode->children, parentFile, embers);
			}
		}
	}

	/// <summary>
	/// Parse an ember element.
	/// </summary>
	/// <param name="emberNode">The current node to parse</param>
	/// <param name="currentEmber">The newly constructed ember based on what was parsed</param>
	/// <returns>True if there were no errors, else false.</returns>
	bool ParseEmberElement(xmlNode* emberNode, Ember<T>& currentEmber)
	{
		bool ret = true;
		unsigned int newLinear = 0;
		char* attStr;
		const char* loc = __FUNCTION__;
		int soloXform = -1;
		unsigned int i, count, index = 0;
		double vals[10];
		xmlAttrPtr att, curAtt;
		xmlNodePtr editNode, childNode, motionNode;

		currentEmber.m_Palette.Clear();//Wipe out the current palette.
		att = emberNode->properties;//The top level element is a ember element, read the attributes of it and store them.

		if (att == nullptr)
		{
			m_ErrorReport.push_back(string(loc) + " : <flame> element has no attributes");
			return false;
		}

		for (curAtt = att; curAtt; curAtt = curAtt->next)
		{
			attStr = (char*)xmlGetProp(emberNode, curAtt->name);

			//First parse out simple float reads.
			if		(ParseAndAssignFloat(curAtt->name, attStr, "time",                  currentEmber.m_Time,                ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "scale",					currentEmber.m_PixelsPerUnit,		ret)) { currentEmber.m_OrigPixPerUnit = currentEmber.m_PixelsPerUnit; }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "rotate",                currentEmber.m_Rotate,              ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "zoom",                  currentEmber.m_Zoom,                ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "filter",                currentEmber.m_SpatialFilterRadius, ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "temporal_filter_width", currentEmber.m_TemporalFilterWidth, ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "temporal_filter_exp",   currentEmber.m_TemporalFilterExp,   ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "quality",               currentEmber.m_Quality,             ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "brightness",            currentEmber.m_Brightness,          ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "gamma",                 currentEmber.m_Gamma,               ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "highlight_power",       currentEmber.m_HighlightPower,      ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "vibrancy",              currentEmber.m_Vibrancy,            ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "estimator_radius",      currentEmber.m_MaxRadDE,            ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "estimator_minimum",     currentEmber.m_MinRadDE,            ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "estimator_curve",       currentEmber.m_CurveDE,             ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "gamma_threshold",       currentEmber.m_GammaThresh,         ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_zpos",				currentEmber.m_CamZPos,				ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_persp",				currentEmber.m_CamPerspective,      ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_perspective",		currentEmber.m_CamPerspective,      ret)) { }//Apo bug.
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_yaw",				currentEmber.m_CamYaw,				ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_pitch",				currentEmber.m_CamPitch,			ret)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "cam_dof",				currentEmber.m_CamDepthBlur,        ret)) { }

			//Parse simple int reads.
			else if (ParseAndAssignInt(curAtt->name, attStr, "palette",          currentEmber.m_Palette.m_Index, ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "oversample",       currentEmber.m_Supersample    , ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "supersample",      currentEmber.m_Supersample    , ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "temporal_samples", currentEmber.m_TemporalSamples, ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "sub_batch_size",	 currentEmber.m_SubBatchSize   , ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "fuse",			 currentEmber.m_FuseCount	   , ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "soloxform",		 soloXform                     , ret)) { }
			else if (ParseAndAssignInt(curAtt->name, attStr, "new_linear",		 newLinear					   , ret)) { }

			//Parse more complicated reads that have multiple possible values.
			else if (!Compare(curAtt->name, "interpolation"))
			{
				if (!_stricmp("linear", attStr))
					currentEmber.m_Interp = EMBER_INTERP_LINEAR;
				else if (!_stricmp("smooth", attStr))
					currentEmber.m_Interp = EMBER_INTERP_SMOOTH;
				else
					m_ErrorReport.push_back(string(loc) + " : Unrecognized interpolation type " + string(attStr));
			}
			else if (!Compare(curAtt->name, "palette_interpolation"))
			{
				if (!_stricmp("hsv", attStr))
					currentEmber.m_PaletteInterp = INTERP_HSV;
				else if (!_stricmp("sweep", attStr))
					currentEmber.m_PaletteInterp = INTERP_SWEEP;
				else
					m_ErrorReport.push_back(string(loc) + " : Unrecognized palette interpolation type " + string(attStr));
			}
			else if (!Compare(curAtt->name, "interpolation_space") || !Compare(curAtt->name, "interpolation_type"))
			{
				if (!_stricmp("linear", attStr))
					currentEmber.m_AffineInterp = INTERP_LINEAR;
				else if (!_stricmp("log", attStr))
					currentEmber.m_AffineInterp = INTERP_LOG;
				else if (!_stricmp("old", attStr))
					currentEmber.m_AffineInterp = INTERP_COMPAT;
				else if (!_stricmp("older", attStr))
					currentEmber.m_AffineInterp = INTERP_OLDER;
				else
					m_ErrorReport.push_back(string(loc) + " : Unrecognized interpolation type " + string(attStr));
			}
			else if (!Compare(curAtt->name, "name"))
			{
				currentEmber.m_Name = string(attStr);
				std::replace(currentEmber.m_Name.begin(), currentEmber.m_Name.end(), ' ', '_');
			}
			else if (!Compare(curAtt->name, "size"))
			{
				if (sscanf_s(attStr, "%lu %lu", &currentEmber.m_FinalRasW, &currentEmber.m_FinalRasH) != 2)
				{
					m_ErrorReport.push_back(string(loc) + " : Invalid size attribute " + string(attStr));
					xmlFree(attStr);

					//These return statements are bad. One because they are inconsistent with others that just assign defaults.
					//Two, because assigning easily guessable defaults is easy and less drastic.
					return false;
				}

				currentEmber.m_OrigFinalRasW = currentEmber.m_FinalRasW;
				currentEmber.m_OrigFinalRasH = currentEmber.m_FinalRasH;
			}
			else if (!Compare(curAtt->name, "center"))
			{
				if (sscanf_s(attStr, "%lf %lf", &vals[0], &vals[1]) != 2)
				{
					m_ErrorReport.push_back(string(loc) + " : Invalid center attribute " + string(attStr));
					xmlFree(attStr);
					return false;
				}

				currentEmber.m_CenterX = T(vals[0]);
				currentEmber.m_CenterY = currentEmber.m_RotCenterY = T(vals[1]);
			}
			else if (!Compare(curAtt->name, "filter_shape"))
			{
				currentEmber.m_SpatialFilterType = SpatialFilterCreator<T>::FromString(string(attStr));
			}
			else if (!Compare(curAtt->name, "temporal_filter_type"))
			{
				currentEmber.m_TemporalFilterType = TemporalFilterCreator<T>::FromString(string(attStr));
			}
			else if (!Compare(curAtt->name, "palette_mode"))
			{
				if (!_stricmp("step", attStr))
					currentEmber.m_PaletteMode = PALETTE_STEP;
				else if (!_stricmp("linear", attStr))
					currentEmber.m_PaletteMode = PALETTE_LINEAR;
				else
				{
					currentEmber.m_PaletteMode = PALETTE_STEP;
					m_ErrorReport.push_back(string(loc) + " : Unrecognized palette mode " + string(attStr) + ", using step");
				}
			}
			else if (!Compare(curAtt->name, "background"))
			{
				if (sscanf_s(attStr, "%lf %lf %lf", &vals[0], &vals[1], &vals[2]) != 3)
				{
					m_ErrorReport.push_back(string(loc) + " : Invalid background attribute " + string(attStr));
					xmlFree(attStr);
					return false;
				}

				currentEmber.m_Background[0] = T(vals[0]);//[0..1]
				currentEmber.m_Background[1] = T(vals[1]);
				currentEmber.m_Background[2] = T(vals[2]);
			}
			else if (!Compare(curAtt->name, "hue"))
			{
				Atof(attStr, currentEmber.m_Hue);
				currentEmber.m_Hue = fmod(currentEmber.m_Hue, T(0.5));//Orig did fmod 1, but want it in the range -0.5 - 0.5.
			}

			xmlFree(attStr);
		}

		//Finished with ember attributes. Now look at the children of the ember element.
		for (childNode = emberNode->children; childNode; childNode = childNode->next)
		{
			if (!Compare(childNode->name, "color"))
			{
				index = -1;
				double r = 0, g = 0, b = 0, a = 0;

				//Loop through the attributes of the color element.
				att = childNode->properties;

				if (att == nullptr)
				{
					m_ErrorReport.push_back(string(loc) + " : No attributes for color element");
					continue;
				}

				for (curAtt = att; curAtt; curAtt = curAtt->next)
				{
					attStr = (char*)xmlGetProp(childNode, curAtt->name);
					a = 255;

					//This signifies that a palette is not being retrieved from the palette file, rather it's being parsed directly out of the ember xml.
					//This also means the palette has already been hue adjusted and it doesn't need to be done again, which would be necessary if it were
					//coming from the palette file.
					currentEmber.m_Palette.m_Index = -1;

					if (!Compare(curAtt->name, "index"))
					{
						Atoi(attStr, index);
					}
					else if(!Compare(curAtt->name, "rgb"))
					{
						if (sscanf_s(attStr, "%lf %lf %lf", &r, &g, &b) != 3)
							m_ErrorReport.push_back(string(loc) + " : Invalid rgb attribute " + string(attStr));
					}
					else if(!Compare(curAtt->name, "rgba"))
					{
						if (sscanf_s(attStr, "%lf %lf %lf %lf", &r, &g, &b, &a) != 4)
							m_ErrorReport.push_back(string(loc) + " : Invalid rgba attribute " + string(attStr));
					}
					else if(!Compare(curAtt->name, "a"))
					{
						if (sscanf_s(attStr, "%lf", &a) != 1)
							m_ErrorReport.push_back(string(loc) + " : Invalid a attribute " + string(attStr));
					}
					else
					{
						m_ErrorReport.push_back(string(loc) + " : Unknown color attribute " + string((const char*)curAtt->name));
					}

					xmlFree(attStr);
				}

				//Palette colors are [0..255], convert to [0..1].
				if (index >= 0 && index <= 255)
				{
					T alphaPercent = T(a) / T(255);//Aplha percentage in the range of 0 to 1.

					//Premultiply the palette.
					currentEmber.m_Palette.m_Entries[index].r = alphaPercent * (T(r) / T(255));
					currentEmber.m_Palette.m_Entries[index].g = alphaPercent * (T(g) / T(255));
					currentEmber.m_Palette.m_Entries[index].b = alphaPercent * (T(b) / T(255));
					currentEmber.m_Palette.m_Entries[index].a = T(a) / 255;//Will be one for RGB, and other than one if RGBA with A != 255.
				}
				else
				{
					stringstream ss;
					ss << "ParseEmberElement() : Color element with bad/missing index attribute " << index;
					m_ErrorReport.push_back(ss.str());
				}
			}
			else if (!Compare(childNode->name, "colors"))
			{
				//Loop through the attributes of the color element.
				att = childNode->properties;

				if (att == nullptr)
				{
					m_ErrorReport.push_back(string(loc) + " : No attributes for colors element");
					continue;
				}

				for (curAtt = att; curAtt; curAtt = curAtt->next)
				{
					attStr = (char*)xmlGetProp(childNode, curAtt->name);

					if (!Compare(curAtt->name, "count"))
					{
						Atoi(attStr, count);
					}
					else if (!Compare(curAtt->name, "data"))
					{
						if (!ParseHexColors(attStr, currentEmber, count, -4))
						{
							m_ErrorReport.push_back(string(loc) + " : Error parsing hexformatted colors, some may be set to zero");
						}
					}
					else
					{
						m_ErrorReport.push_back(string(loc) + " : Unknown color attribute " + string((const char*)curAtt->name));
					}

					xmlFree(attStr);
				}
			}
			else if (!Compare(childNode->name, "palette"))
			{
				//This could be either the old form of palette or the new form.
				//Make sure BOTH are not specified, otherwise either are ok.
				int numColors = 0;
				int numBytes = 0;
				bool oldFormat = false;
				bool newFormat = false;
				int index0, index1;
				T hue0, hue1;
				T blend = 0.5;
				index0 = index1 = -1;
				hue0 = hue1 = 0.0;

				//Loop through the attributes of the palette element.
				att = childNode->properties;

				if (att == nullptr)
				{
					m_ErrorReport.push_back(string(loc) + " : No attributes for palette element");
					continue;
				}

				for (curAtt = att; curAtt; curAtt = curAtt->next)
				{
					attStr = (char*)xmlGetProp(childNode, curAtt->name);

					if (!Compare(curAtt->name, "index0"))
					{
						oldFormat = true;
						Atoi(attStr, index0);
					}
					else if (!Compare(curAtt->name, "index1"))
					{
						oldFormat = true;
						Atoi(attStr, index1);
					}
					else if (!Compare(curAtt->name, "hue0"))
					{
						oldFormat = true;
						Atof(attStr, hue0);
					}
					else if (!Compare(curAtt->name, "hue1"))
					{
						oldFormat = true;
						Atof(attStr, hue1);
					}
					else if (!Compare(curAtt->name, "blend"))
					{
						oldFormat = true;
						Atof(attStr, blend);
					}
					else if (!Compare(curAtt->name, "count"))
					{
						newFormat = true;
						Atoi(attStr, numColors);
					}
					else if (!Compare(curAtt->name, "format"))
					{
						newFormat = true;

						if (!_stricmp(attStr, "RGB"))
							numBytes = 3;
						else if (!_stricmp(attStr, "RGBA"))
							numBytes = 4;
						else
						{
							m_ErrorReport.push_back(string(loc) + " : Unrecognized palette format string " + string(attStr) + ", defaulting to RGB");
							numBytes = 3;
						}
					}
					else
					{
						m_ErrorReport.push_back(string(loc) + " : Unknown palette attribute " + string((const char*)curAtt->name));
					}

					xmlFree(attStr);
				}

				//Old or new format?
				if (newFormat && oldFormat)
				{
					oldFormat = false;
					m_ErrorReport.push_back(string(loc) + " : Mixing of old and new palette tag syntax not allowed, defaulting to new");
				}

				if (oldFormat)
				{
					InterpolateCmap(currentEmber.m_Palette, blend, index0, hue0, index1, hue1);
				}
				else
				{
					//Read formatted string from contents of tag.
					char* palStr = (char*)xmlNodeGetContent(childNode);

					if (!ParseHexColors(palStr, currentEmber, numColors, numBytes))
					{
						m_ErrorReport.push_back(string(loc) + " : Problem reading hexadecimal color data in palette");
					}

					xmlFree(palStr);
				}
			}
			else if (!Compare(childNode->name, "symmetry"))
			{
				int symKind = INT_MAX;

				//Loop through the attributes of the palette element.
				att = childNode->properties;

				if (att == nullptr)
				{
					m_ErrorReport.push_back(string(loc) + " : No attributes for palette element");
					continue;
				}

				for (curAtt = att; curAtt; curAtt = curAtt->next)
				{
					attStr = (char*)xmlGetProp(childNode, curAtt->name);

					if (!Compare(curAtt->name, "kind"))
					{
						Atoi(attStr, symKind);
					}
					else
					{
						m_ErrorReport.push_back(string(loc) + " : Unknown symmetry attribute " + string(attStr));
						continue;
					}

					xmlFree(attStr);
				}

				//if (symKind != INT_MAX)//What to do about this? Should sym not be saved? Or perhaps better intelligence when adding?//TODO//BUG.
				//{
				//	currentEmber.AddSymmetry(symKind, *(GlobalRand.get()));//Determine what to do here.
				//}
			}
			else if (!Compare(childNode->name, "xform") || !Compare(childNode->name, "finalxform"))
			{
				Xform<T>* theXform = nullptr;

				if (!Compare(childNode->name, "finalxform"))
				{
					Xform<T> finalXform;

					if (!ParseXform(childNode, finalXform, false))
					{
						m_ErrorReport.push_back(string(loc) + " : Error parsing final xform");
					}
					else
					{
						if (finalXform.m_Weight != 0)
						{
							finalXform.m_Weight = 0;
							m_ErrorReport.push_back(string(loc) + " : Final xforms should not have weight specified, setting to zero");
						}

						currentEmber.SetFinalXform(finalXform);
						theXform = currentEmber.NonConstFinalXform();
					}
				}
				else
				{
					Xform<T> xform;

					if (!ParseXform(childNode, xform, false))
					{
						m_ErrorReport.push_back(string(loc) + " : Error parsing xform");
					}
					else
					{
						currentEmber.AddXform(xform);
						theXform = currentEmber.GetXform(currentEmber.XformCount() - 1);
					}
				}

				if (theXform)
				{
					//Check for non-zero motion params.
					if (theXform->m_MotionFreq != 0)//Original checked for motion func being non-zero, but it was set to MOTION_SIN (1) in Xform::Init(), so don't check for 0 here.
					{
						m_ErrorReport.push_back(string(loc) + " : Motion parameters should not be specified in regular, non-motion xforms");
					}

					//Motion Language:  Check the xform element for children - should be named 'motion'.
					for (motionNode = childNode->children; motionNode; motionNode = motionNode->next)
					{
						if (!Compare(motionNode->name, "motion"))
						{
							Xform<T> xform;

							if (!ParseXform(motionNode, xform, true))
								m_ErrorReport.push_back(string(loc) + " : Error parsing motion xform");
							else
								theXform->m_Motion.push_back(xform);
						}
					}
				}
			}
			else if (!Compare(childNode->name, "edit"))
			{
				//Create a new XML document with this edit node as the root node.
				currentEmber.m_Edits = xmlNewDoc((const xmlChar*)"1.0");
				editNode = xmlCopyNode(childNode, 1);
				xmlDocSetRootElement(currentEmber.m_Edits, editNode);
			}
		}

		//if (!newLinear)
		//	currentEmber.Flatten(m_FlattenNames);

		for (i = 0; i < currentEmber.XformCount(); i++)
			if (soloXform >= 0 && i != soloXform)
				currentEmber.GetXform(i)->m_Opacity = 0;//Will calc the cached adjusted viz value later.

		return m_ErrorReport.empty();
	}

	/// <summary>
	/// Parse an xform element.
	/// </summary>
	/// <param name="childNode">The current node to parse</param>
	/// <param name="xform">The newly constructed xform based on what was parsed</param>
	/// <param name="motion">True if this xform is a motion within a parent xform, else false</param>
	/// <returns>True if there were no errors, else false.</returns>
	bool ParseXform(xmlNode* childNode, Xform<T>& xform, bool motion)
	{
		bool success = true;
		char* attStr;
		const char* loc = __FUNCTION__;
		unsigned int j;
		T temp;
		double a, b, c, d, e, f;
		double vals[10];
		xmlAttrPtr attPtr, curAtt;

		//Loop through the attributes of the xform element.
		attPtr = childNode->properties;

		if (attPtr == nullptr)
		{
			m_ErrorReport.push_back(string(loc) + " : Error: No attributes for element");
			return false;
		}

		for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
		{
			attStr = (char*)xmlGetProp(childNode, curAtt->name);

			//First parse out simple float reads.
			if (ParseAndAssignFloat(curAtt->name, attStr, "weight", xform.m_Weight, success)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "color_speed", xform.m_ColorSpeed, success)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "symmetry", xform.m_ColorSpeed, success)) { xform.m_ColorSpeed = (1 - xform.m_ColorSpeed) / 2; }//Legacy support.
			else if (ParseAndAssignFloat(curAtt->name, attStr, "animate", xform.m_Animate, success)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "opacity", xform.m_Opacity, success)) { }
			else if (ParseAndAssignFloat(curAtt->name, attStr, "var_color", xform.m_DirectColor, success)) { }

			//Parse simple int reads.
			else if (ParseAndAssignInt(curAtt->name, attStr, "motion_frequency", xform.m_MotionFreq, success)) { }

			//Parse more complicated reads that have multiple possible values.
			else if (!Compare(curAtt->name, "name"))
			{
				xform.m_Name = string(attStr);
				std::replace(xform.m_Name.begin(), xform.m_Name.end(), ' ', '_');
			}
			else if (!Compare(curAtt->name, "symmetry"))
			{
				//Deprecated, set both color_speed and animate to this value.
				//Huh? Either set it or not?
				Atof(attStr, temp);
				xform.m_ColorSpeed = (1 - temp) / 2;
				xform.m_Animate = T(temp > 0 ? 0 : 1);
			}
			else if (!Compare(curAtt->name, "motion_function"))
			{
				if (!_stricmp("sin", attStr))
					xform.m_MotionFunc = MOTION_SIN;
				else if (!_stricmp("triangle", attStr))
					xform.m_MotionFunc = MOTION_TRIANGLE;
				else if (!_stricmp("hill", attStr))
					xform.m_MotionFunc = MOTION_HILL;
				else
				{
					xform.m_MotionFunc = MOTION_SIN;
					m_ErrorReport.push_back(string(loc) + " : Unknown motion function " + string(attStr) + ", using sin");
				}
			}
			else if (!Compare(curAtt->name, "color"))
			{
				xform.m_ColorX = xform.m_ColorY = 0;

				//Try two coords first .
				if (sscanf_s(attStr, "%lf %lf", &vals[0], &vals[1]) == 2)
				{
					xform.m_ColorX = T(vals[0]);
					xform.m_ColorY = T(vals[1]);
				}
				else if (sscanf_s(attStr, "%lf", &vals[0]) == 1)//Try one color.
				{
					xform.m_ColorX = T(vals[0]);
				}
				else
				{
					xform.m_ColorX = xform.m_ColorY = T(0.5);
					m_ErrorReport.push_back(string(loc) + " : Malformed xform color attribute " + string(attStr) + ", using 0.5, 0.5");
				}
			}
			else if (!Compare(curAtt->name, "chaos"))
			{
				stringstream ss(attStr);
				j = 0;

				while (ss >> temp)
				{
					xform.SetXaos(j, temp);
					j++;
				}
			}
			else if (!Compare(curAtt->name, "plotmode"))
			{
				if (motion == 1)
				{
					m_ErrorReport.push_back(string(loc) + " : Motion element cannot have a plotmode attribute");
				}
				else if (!_stricmp("off", attStr))
					xform.m_Opacity = 0;
			}
			else if (!Compare(curAtt->name, "coefs"))
			{
				if (sscanf_s(attStr, "%lf %lf %lf %lf %lf %lf", &a, &d, &b, &e, &c, &f) != 6)//Original did a complicated parsing scheme. This is easier.//ORIG
				{
					a = d = b = e = c = f = 0;
					m_ErrorReport.push_back(string(loc) + " : Bad coeffs attribute " + string(attStr));
				}

				xform.m_Affine.A(T(a));
				xform.m_Affine.B(T(b));
				xform.m_Affine.C(T(c));
				xform.m_Affine.D(T(d));
				xform.m_Affine.E(T(e));
				xform.m_Affine.F(T(f));
			}
			else if (!Compare(curAtt->name, "post"))
			{
				if (sscanf_s(attStr, "%lf %lf %lf %lf %lf %lf", &a, &d, &b, &e, &c, &f) != 6)//Original did a complicated parsing scheme. This is easier.//ORIG
				{
					a = d = b = e = c = f = 0;
					m_ErrorReport.push_back(string(loc) + " : Bad post coeffs attribute " + string(attStr));
				}

				xform.m_Post.A(T(a));
				xform.m_Post.B(T(b));
				xform.m_Post.C(T(c));
				xform.m_Post.D(T(d));
				xform.m_Post.E(T(e));
				xform.m_Post.F(T(f));
			}
			else
			{
				string s = GetCorrectedVariationName(m_BadVariationNames, curAtt);

				if (Variation<T>* var = m_VariationList.GetVariation(s))
				{
					Variation<T>* varCopy = var->Copy();

					Atof(attStr, varCopy->m_Weight);
					xform.AddVariation(varCopy);
				}
				//else
				//{
				//	m_ErrorReport.push_back("Unsupported variation: " + string((const char*)curAtt->name));
				//}
			}

			xmlFree(attStr);
		}

		//Handle var1.
		for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
		{
			bool var1 = false;

			if (!Compare(curAtt->name, "var1"))
			{
				attStr = (char*)xmlGetProp(childNode, curAtt->name);

				for (j = 0; j < xform.TotalVariationCount(); j++)
					xform.GetVariation(j)->m_Weight = 0;

				if (Atof(attStr, temp))
				{
					unsigned int iTemp = (unsigned int)temp;

					if (iTemp < xform.TotalVariationCount())
					{
						xform.GetVariation(iTemp)->m_Weight = 1;
						var1 = true;
					}
				}

				if (!var1)
					m_ErrorReport.push_back(string(loc) + " : Bad value for var1 " + string(attStr));

				xmlFree(attStr);
				break;
			}
		}

		//Handle var.
		for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
		{
			bool var = false;

			if (!Compare(curAtt->name, "var"))
			{
				attStr = (char*)xmlGetProp(childNode, curAtt->name);

				if (Atof(attStr, temp))
				{
					for (j = 0; j < xform.TotalVariationCount(); j++)
						xform.GetVariation(j)->m_Weight = temp;

					var = true;
				}

				if (!var)
					m_ErrorReport.push_back(string(loc) + " : Bad value for var " + string(attStr));

				xmlFree(attStr);
				break;
			}
		}

		//Now that all xforms have been parsed, go through and try to find params for the parametric variations.
		for (unsigned int i = 0; i < xform.TotalVariationCount(); i++)
		{
			if (ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(xform.GetVariation(i)))
			{
				for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
				{
					string s = GetCorrectedParamName(m_BadParamNames, (const char*)curAtt->name);
					const char* name = s.c_str();

					if (parVar->ContainsParam(name))
					{
						T val = 0;
						attStr = (char*)xmlGetProp(childNode, curAtt->name);

						if (Atof(attStr, val))
						{
							parVar->SetParamVal(name, val);
						}
						else
						{
							m_ErrorReport.push_back(string(loc) + " : Failed to parse parametric variation parameter " + s + " - " + string(attStr));
						}

						xmlFree(attStr);
					}
				}
			}
		}

		return true;
	}

	/// <summary>
	/// Some Apophysis plugins use an inconsistent naming scheme for the parametric variation variables.
	/// This function identifies and converts them to Ember's consistent naming convention.
	/// </summary>
	/// <param name="vec">The vector of corrected names to search</param>
	/// <param name="att">The current Xml node to check</param>
	/// <returns>The corrected name if one was found, else the passed in name.</returns>
	static string GetCorrectedParamName(vector<pair<string, string>>& vec, const char* name)
	{
		for (size_t i = 0; i < vec.size(); i++)
		{
			if (!_stricmp(vec[i].first.c_str(), name))
				return vec[i].second;
		}

		return name;
	}

	/// <summary>
	/// Some Apophysis plugins use an inconsistent naming scheme for variation names.
	/// This function identifies and converts them to Ember's consistent naming convention.
	/// It uses some additional intelligence to ensure the variation is the expected one,
	/// by examining the rest of the xform for the existence of parameter names.
	/// </summary>
	/// <param name="vec">The vector of corrected names to search</param>
	/// <param name="att">The current Xml node to check</param>
	/// <returns>The corrected name if one was found, else the passed in name.</returns>
	static string GetCorrectedVariationName(vector<pair<pair<string, string>, vector<string>>>& vec, xmlAttrPtr att)
	{
		for (size_t i = 0; i < vec.size(); i++)
		{
			if (!_stricmp(vec[i].first.first.c_str(), (const char*)att->name))//Do case insensitive here.
			{
				if (!vec[i].second.empty())
				{
					for (size_t j = 0; j < vec[i].second.size(); j++)
					{
						if (XmlContainsTag(att, vec[i].second[j].c_str()))
							return vec[i].first.second;
					}
				}
				else
				{
					return vec[i].first.second;
				}
			}
		}

		return string((const char*)att->name);
	}

	/// <summary>
	/// Determine if an Xml node contains a given tag.
	/// </summary>
	/// <param name="att">The Xml node to search</param>
	/// <param name="name">The node name to search for</param>
	/// <returns>True if name was found, else false.</returns>
	static bool XmlContainsTag(xmlAttrPtr att, const char* name)
	{
		xmlAttrPtr temp = att;

		do
		{
			if (!_stricmp(name, (const char*)temp->name))
				return true;
		} while ((temp = temp->next));

		return false;
	}

	/// <summary>
	/// Parse hexadecimal colors.
	/// This can read RGB and RGBA, however only RGB will be stored.
	/// </summary>
	/// <param name="colstr">The string of hex colors to parse</param>
	/// <param name="ember">The ember whose palette will be assigned the colors</param>
	/// <param name="numColors">The number of colors present</param>
	/// <param name="chan">The number of channels in each color</param>
	/// <returns>True if there were no errors, else false.</returns>
	bool ParseHexColors(char* colstr, Ember<T>& ember, int numColors, int chan)
	{
		int colorIndex = 0;
		int colorCount = 0;
		unsigned int r, g, b, a;
		int ret;
		char tmps[2];
		int skip = (int)abs(chan);
		bool ok = true;
		const char* loc = __FUNCTION__;

		//Strip whitespace prior to first color.
		while (isspace((int)colstr[colorIndex]))
			colorIndex++;

		do
		{
			//Parse an RGB triplet at a time.
			if (chan == 3)
				ret = sscanf_s(&(colstr[colorIndex]),"%2x%2x%2x", &r, &g, &b);
			else if (chan == -4)
				ret = sscanf_s(&(colstr[colorIndex]),"00%2x%2x%2x", &r, &g, &b);
			else // chan==4
				ret = sscanf_s(&(colstr[colorIndex]),"%2x%2x%2x%2x", &r,&g, &b, &a);

			a = 1;//Original allows for alpha, even though it will most likely never happen. Ember omits support for it.

			if ((chan != 4 && ret != 3) || (chan == 4 && ret != 4))
			{
				ok = false;
				r = g = b = 0;
				m_ErrorReport.push_back(string(loc) + " : Problem reading hexadecimal color data, assigning to 0");
				break;
			}

			colorIndex += 2 * skip;

			while (isspace((int)colstr[colorIndex]))
				colorIndex++;

			ember.m_Palette.m_Entries[colorCount].r = T(r) / T(255);//Hex palette is [0..255], convert to [0..1].
			ember.m_Palette.m_Entries[colorCount].g = T(g) / T(255);
			ember.m_Palette.m_Entries[colorCount].b = T(b) / T(255);
			ember.m_Palette.m_Entries[colorCount].a = T(a);

			colorCount++;

		} while (colorCount < numColors && colorCount < ember.m_Palette.m_Entries.size());

#ifdef WIN32
		if (sscanf_s(&(colstr[colorIndex]),"%1s", tmps, sizeof(tmps)) > 0)//Really need to migrate all of this parsing to C++.//TODO
#else
		if (sscanf_s(&(colstr[colorIndex]),"%1s", tmps) > 0)
#endif
		{
			m_ErrorReport.push_back(string(loc) + " : Extra data at end of hex color data " + string(&(colstr[colorIndex])));
			ok = false;
		}

		return ok;
	}

	/// <summary>
	/// Interpolate the palette.
	/// Used with older formats, deprecated.
	/// </summary>
	/// <param name="palette">The palette to interpolate</param>
	/// <param name="blend">The blend</param>
	/// <param name="index0">The first index</param>
	/// <param name="hue0">The first hue</param>
	/// <param name="index1">The second index</param>
	/// <param name="hue1">The second hue/param>
	void InterpolateCmap(Palette<T>& palette, T blend, int index0, T hue0, int index1, T hue1)
	{
		int i, j;
		const char* loc = __FUNCTION__;
		Palette<T> adjustedPal0, adjustedPal1;

		if (m_PaletteList.GetHueAdjustedPalette(index0, hue0, adjustedPal0) &&
			m_PaletteList.GetHueAdjustedPalette(index1, hue1, adjustedPal1))
		{
			v4T* hueAdjusted0 = adjustedPal0.m_Entries.data();
			v4T* hueAdjusted1 = adjustedPal1.m_Entries.data();

			for (i = 0; i < 256; i++)
			{
				T t[4], s[4];

				Palette<T>::RgbToHsv(glm::value_ptr(hueAdjusted0[i]), s);
				Palette<T>::RgbToHsv(glm::value_ptr(hueAdjusted1[i]), t);

				s[3] = hueAdjusted0[i][3];
				t[3] = hueAdjusted1[i][3];

				for (j = 0; j < 4; j++)
					t[j] = ((1 - blend) * s[j]) + (blend * t[j]);

				Palette<T>::HsvToRgb(t, glm::value_ptr(palette.m_Entries[i]));
				palette.m_Entries[i][3] = t[3];
			}
		}
		else
		{
			m_ErrorReport.push_back(string(loc) + " : Unable to retrieve palettes");
		}
	}

	/// <summary>
	/// Wrapper to parse a floating point Xml value and convert it to float.
	/// </summary>
	/// <param name="name">The xml tag to parse</param>
	/// <param name="attStr">The name of the Xml attribute</param>
	/// <param name="str">The name of the Xml tag</param>
	/// <param name="val">The parsed value</param>
	/// <param name="b">Bitwise ANDed with true if name matched str and the call to Atof() succeeded, else false. Used for keeping a running value between successive calls.</param>
	/// <returns>True if the tag was matched, else false</returns>
	bool ParseAndAssignFloat(const xmlChar* name, const char* attStr, const char* str, T& val, bool& b)
	{
		bool ret = false;

		if (!Compare(name, str))
		{
			b &= Atof(attStr, val);
			ret = true;//Means the strcmp() was right, but doesn't necessarily mean the conversion went ok.
		}

		return ret;
	}

	/// <summary>
	/// Wrapper to parse an int Xml string value and convert it to an int.
	/// </summary>
	/// <param name="name">The xml tag to parse</param>
	/// <param name="attStr">The name of the Xml attribute</param>
	/// <param name="str">The name of the Xml tag</param>
	/// <param name="val">The parsed value</param>
	/// <param name="b">Bitwise ANDed with true if name matched str and the call to Atoi() succeeded, else false. Used for keeping a running value between successive calls.</param>
	/// <returns>True if the tag was matched, else false</returns>
	template <typename intT>
	bool ParseAndAssignInt(const xmlChar* name, const char* attStr, const char* str, intT& val, bool& b)
	{
		bool ret = false;
		T fval = 0;

		if (!Compare(name, str))
		{
			b &= Atof(attStr, fval);
			val = (intT)fval;
			ret = true;//Means the strcmp() was right, but doesn't necessarily mean the conversion went ok.
		}

		return ret;
	}

	static bool m_Init;
	static vector<pair<string, string>> m_BadParamNames;
	static vector<pair<pair<string, string>, vector<string>>> m_BadVariationNames;
	VariationList<T> m_VariationList;//The variation list used to make copies of variations to populate the embers with.
	PaletteList<T> m_PaletteList;
};
}
