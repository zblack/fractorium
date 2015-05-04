#pragma once

#include "Utils.h"
#include "PaletteList.h"
#include "VariationList.h"
#include "Ember.h"

/// <summary>
/// EmberToXml class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Class for converting ember objects to Xml documents.
/// Support for saving one or more to a single file.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API EmberToXml : public EmberReport
{
public:
	/// <summary>
	/// Empty constructor.
	/// </summary>
	EmberToXml()
	{
	}

	/// <summary>
	/// Save the ember to the specified file.
	/// </summary>
	/// <param name="filename">Full path and filename</param>
	/// <param name="ember">The ember to save</param>
	/// <param name="printEditDepth">How deep the edit depth goes</param>
	/// <param name="doEdits">If true included edit tags, else don't.</param>
	/// <param name="intPalette">If true use integers instead of floating point numbers when embedding a non-hex formatted palette, else use floating point numbers.</param>
	/// <param name="hexPalette">If true, embed a hexadecimal palette instead of Xml Color tags, else use Xml color tags.</param>
	/// <param name="append">If true, append to the file if it already exists, else create a new file.</param>
	/// <param name="start">Whether a new file is to be started</param>
	/// <param name="finish">Whether an existing file is to be ended</param>
	/// <returns>True if successful, else false</returns>
	bool Save(const string& filename, Ember<T>& ember, size_t printEditDepth, bool doEdits, bool intPalette, bool hexPalette, bool append = false, bool start = false, bool finish = false)
	{
		vector<Ember<T>> vec;

		vec.push_back(ember);
		return Save(filename, vec, printEditDepth, doEdits, intPalette, hexPalette, append, start, finish);
	}

	/// <summary>
	/// Save a vector of embers to the specified file.
	/// </summary>
	/// <param name="filename">Full path and filename</param>
	/// <param name="embers">The vector of embers to save</param>
	/// <param name="printEditDepth">How deep the edit depth goes</param>
	/// <param name="doEdits">If true included edit tags, else don't.</param>
	/// <param name="intPalette">If true use integers instead of floating point numbers when embedding a non-hex formatted palette, else use floating point numbers.</param>
	/// <param name="hexPalette">If true, embed a hexadecimal palette instead of Xml Color tags, else use Xml color tags.</param>
	/// <param name="append">If true, append to the file if it already exists, else create a new file.</param>
	/// <param name="start">Whether a new file is to be started</param>
	/// <param name="finish">Whether an existing file is to be ended</param>
	/// <returns>True if successful, else false</returns>
	bool Save(const string& filename, vector<Ember<T>>& embers, size_t printEditDepth, bool doEdits, bool intPalette, bool hexPalette, bool append = false, bool start = false, bool finish = false)
	{
		bool b = false;
		string temp;
		ofstream f;

		try
		{
			if (append)
				f.open(filename, std::ofstream::out | std::ofstream::app);//Appending allows us to write multiple embers to a single file.
			else
				f.open(filename);

			if (f.is_open())
			{
				if ((append && start) || !append)
				{
					temp = "<flames>\n";
					//temp = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<flames>\n";
					f.write(temp.c_str(), temp.size());
				}

				for (auto& ember : embers)
				{
					string s = ToString(ember, "", printEditDepth, doEdits, intPalette, hexPalette);
					f.write(s.c_str(), s.size());
				}

				if ((append && finish) || !append)
				{
					temp = "</flames>\n";
					f.write(temp.c_str(), temp.size());
				}

				f.close();
				b = true;
			}
			else
			{
				cout << "Error: Writing flame " << filename << " failed." << endl;
				b = false;
			}
		}
		catch (...)
		{
			if (f.is_open())
				f.close();

			b = false;
		}

		return b;
	}

	/// <summary>
	/// Return the Xml string representation of an ember.
	/// </summary>
	/// <param name="ember">The ember to create the Xml with</param>
	/// <param name="extraAttributes">If true, add extra attributes, else don't</param>
	/// <param name="printEditDepth">How deep the edit depth goes</param>
	/// <param name="doEdits">If true included edit tags, else don't.</param>
	/// <param name="intPalette">If true use integers instead of floating point numbers when embedding a non-hex formatted palette, else use floating point numbers.</param>
	/// <param name="hexPalette">If true, embed a hexadecimal palette instead of Xml Color tags, else use Xml color tags.</param>
	/// <returns>The Xml string representation of the passed in ember</returns>
	string ToString(Ember<T>& ember, string extraAttributes, size_t printEditDepth, bool doEdits, bool intPalette, bool hexPalette = true)
	{
		size_t i, j;
		string s;
		ostringstream os;
		vector<Variation<T>*> variations;

		os << "<flame version=\"EMBER-" << EmberVersion() << "\" time=\"" << ember.m_Time << "\"";

		if (!ember.m_Name.empty())
			os << " name=\"" << ember.m_Name << "\"";

		os << " size=\"" << ember.m_FinalRasW << " " << ember.m_FinalRasH << "\"";
		os << " center=\"" << ember.m_CenterX << " " << ember.m_CenterY << "\"";
		os << " scale=\"" << ember.m_PixelsPerUnit << "\"";

		if (ember.m_Zoom != 0)
			os << " zoom=\"" << ember.m_Zoom << "\"";

		os << " rotate=\"" << ember.m_Rotate << "\"";
		os << " supersample=\"" << std::max<size_t>(1, ember.m_Supersample) << "\"";
		os << " filter=\"" << ember.m_SpatialFilterRadius << "\"";

		os << " filter_shape=\"" << ToLower(SpatialFilterCreator<T>::ToString(ember.m_SpatialFilterType)) << "\"";
		os << " temporal_filter_type=\"" << ToLower(TemporalFilterCreator<T>::ToString(ember.m_TemporalFilterType)) << "\"";

		if (ember.m_TemporalFilterType == EXP_TEMPORAL_FILTER)
			os << " temporal_filter_exp=\"" << ember.m_TemporalFilterExp << "\"";

		os << " temporal_filter_width=\"" << ember.m_TemporalFilterWidth << "\"";
		os << " quality=\"" << ember.m_Quality << "\"";
		os << " temporal_samples=\"" << ember.m_TemporalSamples << "\"";
		os << " sub_batch_size=\"" << ember.m_SubBatchSize << "\"";
		os << " fuse=\"" << ember.m_FuseCount << "\"";
		os << " background=\"" << ember.m_Background.r << " " << ember.m_Background.g << " " << ember.m_Background.b << "\"";
		os << " brightness=\"" << ember.m_Brightness << "\"";
		os << " gamma=\"" << ember.m_Gamma << "\"";
		os << " highlight_power=\"" << ember.m_HighlightPower << "\"";
		os << " vibrancy=\"" << ember.m_Vibrancy << "\"";
		//os << " hue=\"" << ember.m_Hue << "\"";//Oddly enough, flam3 never wrote this value out.//ORIG
		os << " estimator_radius=\"" << ember.m_MaxRadDE << "\"";
		os << " estimator_minimum=\"" << ember.m_MinRadDE << "\"";
		os << " estimator_curve=\"" << ember.m_CurveDE << "\"";
		os << " gamma_threshold=\"" << ember.m_GammaThresh << "\"";
		os << " cam_zpos=\"" << ember.m_CamZPos << "\"";
		os << " cam_persp=\"" << ember.m_CamPerspective << "\"";
		os << " cam_yaw=\"" << ember.m_CamYaw << "\"";
		os << " cam_pitch=\"" << ember.m_CamPitch << "\"";
		os << " cam_dof=\"" << ember.m_CamDepthBlur << "\"";

		if (ember.m_PaletteMode == PALETTE_STEP)
			os << " palette_mode=\"step\"";
		else if (ember.m_PaletteMode == PALETTE_LINEAR)
			os << " palette_mode=\"linear\"";

		if (ember.m_Interp == EMBER_INTERP_SMOOTH)
			os << " interpolation=\"smooth\"";

		if (ember.m_AffineInterp == INTERP_LINEAR)
			os << " interpolation_type=\"linear\"";
		else if (ember.m_AffineInterp == INTERP_LOG)
			os << " interpolation_type=\"log\"";
		else if (ember.m_AffineInterp == INTERP_COMPAT)
			os << " interpolation_type=\"old\"";
		else if (ember.m_AffineInterp == INTERP_OLDER)
			os << " interpolation_type=\"older\"";

		if (ember.m_PaletteInterp == INTERP_SWEEP)
			os << " palette_interpolation=\"sweep\"";

		if (!extraAttributes.empty())
			os << " " << extraAttributes;

		os << " plugins=\"";
		ember.GetPresentVariations(variations, false);

		if (!variations.empty())
			for (auto var : variations) os << var->Name() << (var != variations.back() ? " " : "\"");
		else
			os << "\"";

		os << " new_linear=\"1\"";
		os << " curves=\"";

		for (glm::length_t ci = 0; ci < 4; ci++)
		{
			for (glm::length_t cj = 0; cj < 4; cj++)
			{
				os << ember.m_Curves.m_Points[ci][cj].x << " ";
				os << ember.m_Curves.m_Points[ci][cj].y << " ";
				os << ember.m_Curves.m_Weights[ci][cj] << " ";
			}
		}

		os << "\">\n";

		//This is a grey area, what to do about symmetry to avoid duplicating the symmetry xforms when reading back?//TODO//BUG.
		//if (ember.m_Symmetry)
		//	os << "   <symmetry kind=\"" << ember.m_Symmetry << "\"/>\n";

		for (i = 0; i < ember.XformCount(); i++)
			os << ToString(*ember.GetXform(i), ember.XformCount(), false, false);//Not final, don't do motion.

		if (ember.UseFinalXform())
			os << ToString(*ember.NonConstFinalXform(), ember.XformCount(), true, false);//Final, don't do motion.

		if (hexPalette)
		{
			os << "   <palette count=\"256\" format=\"RGB\">\n";

			for (i = 0; i < 32; i++)
			{
				os << "      ";

				for (j = 0; j < 8; j++)
				{
					size_t idx = 8 * i + j;

					os << hex << setw(2) << setfill('0') << int(Rint(ember.m_Palette[idx][0] * 255));
					os << hex << setw(2) << setfill('0') << int(Rint(ember.m_Palette[idx][1] * 255));
					os << hex << setw(2) << setfill('0') << int(Rint(ember.m_Palette[idx][2] * 255));
				}

				os << endl;
			}

			os << "   </palette>\n";
		}
		else
		{
			for (i = 0; i < 256; i++)
			{
				double r = ember.m_Palette[i][0] * 255;
				double g = ember.m_Palette[i][1] * 255;
				double b = ember.m_Palette[i][2] * 255;
				double a = ember.m_Palette[i][3] * 255;

				os << "   ";
				//The original used a precision of 6 which is totally unnecessary, use 2.
				if (IsClose(a, 255.0))
				{
					if (intPalette)
						os << "<color index=\"" << i << "\" rgb=\"" << int(Rint(r)) << " " << int(Rint(g)) << " " << int(Rint(b)) << "\"/>";
					else
						os << "<color index=\"" << i << "\" rgb=\"" << std::fixed << std::setprecision(2) << r << " " << g << " " << b << "\"/>";
				}
				else
				{
					if (intPalette)
						os << "   <color index=\"" << i << "\" rgba=\"" << int(Rint(r)) << " " << int(Rint(g)) << " " << int(Rint(b)) << " " << int(Rint(a)) << "\"/>";
					else
						os << "   <color index=\"" << i << "\" rgba=\"" << std::fixed << std::setprecision(2) << r << " " << g << " " << b << " " << a << "\"/>";
				}

				os << "\n";
			}
		}

		if (doEdits && ember.m_Edits != nullptr)
			os << ToString(xmlDocGetRootElement(ember.m_Edits), 1, true, printEditDepth);

		os << "</flame>\n";

		return os.str();
	}

	/// <summary>
	/// Create a new editdoc optionally based on parents passed in.
	/// This is used when an ember is made out of some mutation or edit from one or two existing embers and
	/// the user wants to capture the genetic lineage history information in the edit doc of the new ember.
	/// </summary>
	/// <param name="parent0">The first parent, optionally nullptr.</param>
	/// <param name="parent1">The second parent, optionally nullptr.</param>
	/// <param name="action">The action that was taken to create the new ember</param>
	/// <param name="nick">The nickname of the author</param>
	/// <param name="url">The Url of the author</param>
	/// <param name="id">The id of the author</param>
	/// <param name="comment">The comment to include</param>
	/// <param name="sheepGen">The sheep generation used if > 0. Default: 0.</param>
	/// <param name="sheepId">The sheep id used if > 0. Default: 0.</param>
	/// <returns></returns>
	xmlDocPtr CreateNewEditdoc(Ember<T>* parent0, Ember<T>* parent1, string action, string nick, string url, string id, string comment, int sheepGen = 0, int sheepId = 0)
	{
		char timeString[128];
		time_t myTime;
		string s;
		xmlDocPtr commentDoc = nullptr;
		xmlDocPtr doc = xmlNewDoc(XC("1.0"));
		xmlNodePtr rootNode = nullptr, node = nullptr, nodeCopy = nullptr;
		xmlNodePtr rootComment = nullptr;
		ostringstream os;

		//Create the root node, called "edit".
		rootNode = xmlNewNode(nullptr, XC("edit"));
		xmlDocSetRootElement(doc, rootNode);

		//Add the edit attributes.
		//Date.
		myTime = time(nullptr);
#ifdef WIN32
		tm localt;
		localtime_s(&localt, &myTime);
		strftime(timeString, 128, "%a %b %d %H:%M:%S %z %Y", &localt);//XXX use standard time format including timezone.
#else
		tm* localt;
		localt = localtime(&myTime);
		strftime(timeString, 128, "%a %b %d %H:%M:%S %z %Y", localt);//XXX use standard time format including timezone.
#endif
		xmlNewProp(rootNode, XC("date"), XC(timeString));

		//Nick.
		if (nick != "")
			xmlNewProp(rootNode, XC("nick"), XC(nick.c_str()));

		//Url.
		if (url != "")
			xmlNewProp(rootNode, XC("url"), XC(url.c_str()));

		if (id != "")
			xmlNewProp(rootNode, XC("id"), XC(id.c_str()));

		//Action.
		xmlNewProp(rootNode, XC("action"), XC(action.c_str()));

		//Sheep info.
		if (sheepGen > 0 && sheepId > 0)
		{
			//Create a child node of the root node called sheep.
			node = xmlNewChild(rootNode, nullptr, XC("sheep"), nullptr);

			//Create the sheep attributes.
			os << sheepGen;
			s = os.str();
			xmlNewProp(node, XC("generation"), XC(s.c_str()));
			os.str("");

			os << sheepId;
			s = os.str();
			xmlNewProp(node, XC("id"), XC(s.c_str()));
			os.str("");
		}

		//Check for the parents.
		//If parent 0 not specified, this is a randomly generated genome.
		if (parent0)
		{
			os << parent0->m_Index;
			s = os.str();

			if (parent0->m_Edits)
			{
				//Copy the node from the parent.
				node = xmlDocGetRootElement(parent0->m_Edits);
				nodeCopy = xmlCopyNode(node, 1);
				AddFilenameWithoutAmpersand(nodeCopy, parent0->m_ParentFilename);

				xmlNewProp(nodeCopy, XC("index"), XC(s.c_str()));
				xmlAddChild(rootNode, nodeCopy);
			}
			else
			{
				//Insert a (parent has no edit) message.
				nodeCopy = xmlNewChild(rootNode, nullptr, XC("edit"), nullptr);
				AddFilenameWithoutAmpersand(nodeCopy, parent0->m_ParentFilename);
				xmlNewProp(nodeCopy, XC("index"), XC(s.c_str()));
			}

			os.str("");
		}

		if (parent1)
		{
			os << parent1->m_Index;
			s = os.str();

			if (parent1->m_Edits)
			{
				//Copy the node from the parent.
				node = xmlDocGetRootElement(parent1->m_Edits);
				nodeCopy = xmlCopyNode(node, 1);
				AddFilenameWithoutAmpersand(nodeCopy, parent1->m_ParentFilename);
				xmlNewProp(nodeCopy, XC("index"), XC(s.c_str()));
				xmlAddChild(rootNode, nodeCopy);
			}
			else
			{
				//Insert a (parent has no edit) message.
				nodeCopy = xmlNewChild(rootNode, nullptr, XC("edit"),nullptr);
				AddFilenameWithoutAmpersand(nodeCopy, parent1->m_ParentFilename);
				xmlNewProp(nodeCopy, XC("index"), XC(s.c_str()));
			}

			os.str("");
		}

		//Comment string:
		//This one's hard, since the comment string must be treated as
		//a valid XML document. Create a new document using the comment
		//string as the in-memory document, and then copy all children of
		//the root node into the edit structure
		//Parsing the comment string should be done once and then copied
		//for each new edit doc, but that's for later.
		if (comment != "")
		{
			os << "<comm>" << comment << "</comm>";
			s = os.str();
			commentDoc = xmlReadMemory(s.c_str(), int(s.length()), "comment.env", nullptr, XML_PARSE_NONET);
			os.str("");

			//Check for errors.
			if (commentDoc != nullptr)
			{

				//Loop through the children of the new document and copy them into the rootNode.
				rootComment = xmlDocGetRootElement(commentDoc);

				for (node = rootComment->children; node; node = node->next)
				{
					nodeCopy = xmlCopyNode(node, 1);
					xmlAddChild(rootNode, nodeCopy);
				}

				//Free the created document.
				xmlFreeDoc(commentDoc);
			}
			else
			{
				cout << "Failed to parse comment into Xml." << endl;
			}
		}

		//Return the Xml doc.
		return doc;
	}

private:
	/// <summary>
	/// Return the Xml string representation of an xform.
	/// </summary>
	/// <param name="xform">The xform to create the Xml with</param>
	/// <param name="xformCount">The number of non-final xforms in the ember to which this xform belongs. Used for xaos.</param>
	/// <param name="isFinal">True if the xform is the final xform in the ember, else false.</param>
	/// <param name="doMotion">If true, include motion elements in the Xml string, else omit.</param>
	/// <returns>The Xml string representation of the passed in xform</returns>
	string ToString(Xform<T>& xform, size_t xformCount, bool isFinal, bool doMotion)
	{
		size_t i, j;
		ostringstream os;

		if (doMotion)
		{
			os << "      <motion motion_frequency=\"" << xform.m_MotionFreq << "\" ";

			if (xform.m_MotionFunc == MOTION_SIN)
				os << "motion_function=\"sin\" ";
			else if (xform.m_MotionFunc == MOTION_TRIANGLE)
				os << "motion_function=\"triangle\" ";
			else if (xform.m_MotionFunc== MOTION_HILL)
				os << "motion_function=\"hill\" ";
		}
		else
		{
			if (isFinal)
				os << "   <finalxform ";
			else
				os << "   <xform weight=\"" << xform.m_Weight << "\" ";
		}

		if (!doMotion || xform.m_ColorX != EMPTYFIELD) os << "color=\"" << xform.m_ColorX << "\" ";
		//if (!doMotion || xform.m_ColorY != EMPTYFIELD) os << "color=\"" << xform.m_ColorX << " " << xform.m_ColorY << "\" ";
		if (!doMotion || xform.m_DirectColor != EMPTYFIELD) os << "var_color=\"" << xform.m_DirectColor << "\" ";
		if (!doMotion || xform.m_ColorSpeed != EMPTYFIELD) os << "color_speed=\"" << xform.m_ColorSpeed << "\" ";
		//os << "symmetry=\"" << fabs(xform.m_ColorSpeed - 1) * 2 << "\" ";//Legacy support.

		if (!doMotion)
		{
			string s = xform.m_Name;

			std::replace(s.begin(), s.end(), ' ', '_');
			os << "name=\"" << s << "\" ";//Flam3 didn't do this, but Apo does.

			if (!isFinal)
				os << "animate=\"" << xform.m_Animate << "\" ";
		}

		//Variation writing order differs slightly from the original to make it a bit more readable.
		//The original wrote out all of the variation names and weights. Then wrote out the parameters for
		//the parametric variations. Here, write out the params immediately after each parametric variation
		//so they are more closely grouped with the variation they apply to, rather than being all grouped at the end.
		for (i = 0; i < xform.TotalVariationCount(); i++)
		{
			Variation<T>* var = xform.GetVariation(i);
			ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);

			if (var->m_Weight != 0)
			{
				os << var->Name() << "=\"" << var->m_Weight << "\" ";

				if (parVar)
				{
					ParamWithName<T>* params = parVar->Params();

					for (j = 0; j < parVar->ParamCount(); j++)
					{
						if ((!doMotion || (doMotion && (params[j].ParamVal() != 0))) && !params[j].IsPrecalc())
							os << params[j].Name() << "=\"" << params[j].ParamVal() << "\" ";
					}
				}
			}
		}

		if (!doMotion || (doMotion && !xform.m_Affine.IsZero() && !xform.m_Affine.IsEmpty()))
		{
			os << "coefs=\"" << xform.m_Affine.A() << " " << xform.m_Affine.D() << " " << xform.m_Affine.B() << " "
				 << xform.m_Affine.E() << " " << xform.m_Affine.C() << " " << xform.m_Affine.F() << "\"";
		}

		if ((!doMotion && !xform.m_Post.IsID()) || (doMotion && !xform.m_Post.IsZero() && !xform.m_Post.IsEmpty()))
		{
			os << " post=\"" << xform.m_Post.A() << " " << xform.m_Post.D() << " " << xform.m_Post.B() << " "
				<< xform.m_Post.E() << " " << xform.m_Post.C() << " " << xform.m_Post.F() << "\"";
		}

		//Original only printed xaos values that were not 1. Here, print them all out if any are present.
		if (!isFinal && !doMotion && xform.XaosPresent())//Applying motion to xaos not supported.
		{
			os << " chaos=\"";

			for (i = 0; i < xformCount; i++)
				os << xform.Xaos(i) << " ";

			os << "\"";
		}

		if (!doMotion || xform.m_Opacity != EMPTYFIELD) os << " opacity=\"" << xform.m_Opacity << "\"";

		if (!doMotion && !xform.m_Motion.empty())
		{
			os << ">\n";

			for (i = 0; i < xform.m_Motion.size(); i++)
				os << ToString(xform.m_Motion[i], 0, false, true);

			if (isFinal)//Fixed to properly close final.//SMOULDER
				os << "   </finalxform>\n";
			else
				os << "   </xform>\n";
		}
		else
			os << "/>\n";

		return os.str();
	}

	/// <summary>
	/// Return an edit node Xml string.
	/// </summary>
	/// <param name="editNode">The edit node to get the string for</param>
	/// <param name="tabs">How many tabs to use</param>
	/// <param name="formatting">If true, include newlines and tabs, else don't.</param>
	/// <param name="printEditDepth">How deep the edit depth goes</param>
	/// <returns>The edit node Xml string</returns>
	string ToString(xmlNodePtr editNode, size_t tabs, bool formatting, size_t printEditDepth)
	{
		bool indentPrinted = false;
		const char* tabString = "   ", *attStr;
		const char* editString = "edit";
		const char* sheepString = "sheep";
		size_t ti;//, editOrSheep = 0;
		xmlAttrPtr attPtr = nullptr, curAtt = nullptr;
		xmlNodePtr childPtr = nullptr, curChild = nullptr;
		ostringstream os;

		if (printEditDepth > 0 && tabs > printEditDepth)
			return "";

		//If this node is an XML_ELEMENT_NODE, print it and its attributes.
		if (editNode->type == XML_ELEMENT_NODE)
		{
			//Print the node at the tab specified.
			if (formatting)
				for (ti = 0; ti < tabs; ti++)
					os << tabString;

			os << "<" << editNode->name;

			//This can either be an edit node or a sheep node.
			//If it's an edit node, add one to the tab.
			if (!Compare(editNode->name, editString))
			{
				//editOrSheep = 1;
				tabs++;
			}
			else if (!Compare(editNode->name, sheepString)) { }
				//editOrSheep = 2;
			else { }
				//editOrSheep = 0;

			//Print the attributes.
			attPtr = editNode->properties;

			for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
			{
				attStr = CX(xmlGetProp(editNode, curAtt->name));
				os << " " << curAtt->name << "=\"" << attStr << "\"";
				xmlFree(reinterpret_cast<void*>(const_cast<char*>(attStr)));
			}

			//Does this node have children?
			if (!editNode->children || (printEditDepth > 0 && tabs > printEditDepth))
			{
				//Close the tag and subtract the tab.
				os << "/>";

				if (formatting)
					os << "\n";

				tabs--;
			}
			else
			{
				//Close the tag.
				os << ">";

				if (formatting)
					os << "\n";

				//Loop through the children and print them.
				childPtr = editNode->children;
				indentPrinted = false;

				for (curChild = childPtr; curChild; curChild = curChild->next)
				{
					//If child is an element, indent first and then print it.
					if (curChild->type == XML_ELEMENT_NODE &&
					   (!Compare(curChild->name, editString) || !Compare(curChild->name, sheepString)))
					{
						if (indentPrinted)
						{
							indentPrinted = false;
							os << "\n";
						}

						os << ToString(curChild, tabs, true, printEditDepth);
					}
					else
					{
						//Child is a text node, don't want to indent more than once.
						if (xmlIsBlankNode(curChild))
							continue;

						if (!indentPrinted && formatting)
						{
							for (ti = 0; ti < tabs; ti++)
								os << tabString;

							indentPrinted = true;
						}

						//Print nodes without formatting.
						os << ToString(curChild, tabs, false, printEditDepth);
					}
				}

				if (indentPrinted && formatting)
					os << "\n";

				tabs--;//Tab out.

				if (formatting)
					for (ti = 0; ti < tabs; ti++)
						os << tabString;

				os << "</" << editNode->name << ">";//Close the tag.

				if (formatting)
					os << "\n";
			}
		}
		else if (editNode->type == XML_TEXT_NODE)
		{
			string s(reinterpret_cast<char*>(xmlNodeGetContent(editNode)));
			os << Trim(s);
		}

		return os.str();
	}

	void AddFilenameWithoutAmpersand(xmlNodePtr node, string& filename)
	{
		if (filename.find_first_of('&') != std::string::npos)
		{
			string filenameWithoutAmpersands = filename;

			FindAndReplace<string>(filenameWithoutAmpersands, "&", "&amp;");
			xmlNewProp(node, XC("filename"), XC(filenameWithoutAmpersands.c_str()));
		}
		else
		{
			xmlNewProp(node, XC("filename"), XC(filename.c_str()));
		}
	}
};
}
