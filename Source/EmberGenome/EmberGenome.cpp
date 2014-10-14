#include "EmberCommonPch.h"
#include "EmberGenome.h"
#include "JpegUtils.h"

/// <summary>
/// Set various default test values on the passed in ember.
/// </summary>
/// <param name="ember">The ember to test</param>
template <typename T>
void SetDefaultTestValues(Ember<T>& ember)
{
   ember.m_Time = 0.0;
   ember.m_Interp = EMBER_INTERP_LINEAR;
   ember.m_PaletteInterp = INTERP_HSV;
   ember.m_Background[0] = 0;
   ember.m_Background[1] = 0;
   ember.m_Background[2] = 0;
   ember.m_Background[3] = 255;
   ember.m_CenterX = 0;
   ember.m_CenterY = 0;
   ember.m_Rotate = 0;
   ember.m_PixelsPerUnit = 64;
   ember.m_FinalRasW = 128;
   ember.m_FinalRasH = 128;
   ember.m_Supersample = 1;
   ember.m_SpatialFilterRadius = T(0.5);
   ember.m_SpatialFilterType = GAUSSIAN_SPATIAL_FILTER;
   ember.m_Zoom = 0;
   ember.m_Quality = 1;
   ember.m_Passes = 1;
   ember.m_TemporalSamples = 1;
   ember.m_MaxRadDE = 0;
   ember.m_MinRadDE = 0;
   ember.m_CurveDE = T(0.6);
}

/// <summary>
/// The core of the EmberGenome.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
bool EmberGenome(EmberOptions& opt)
{
	OpenCLWrapper wrapper;
	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cout << opt.GetValues(OPT_USE_GENOME) << endl;
	
	if (opt.OpenCLInfo())
	{
		cout << "\nOpenCL Info: " << endl;
		cout << wrapper.DumpInfo();
		return true;
	}

	//Regular variables.
	Timing t;
	bool exactTimeMatch, randomMode, didColor, seqFlag;
	unsigned int i, j, i0, i1, rep, val, frame, frameCount, count = 0;
	unsigned int ftime, firstFrame, lastFrame;
	size_t n, tot, totb, totw;
	T avgPix, fractionBlack, fractionWhite, blend, spread, mix0, mix1;
	string token, filename;
	ostringstream os, os2;
	vector<Ember<T>> embers, embers2, templateEmbers;
	vector<eVariationId> vars, noVars;
	vector<unsigned char> finalImage;
	eCrossMode crossMeth;
	eMutateMode mutMeth;
	Ember<T> orig, save, selp0, selp1, parent0, parent1;
	Ember<T> result, result1, result2, result3, interpolated;
	Ember<T>* aselp0, *aselp1, *pTemplate = NULL;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	VariationList<T> varList;
	EmberReport emberReport, emberReport2;
	auto_ptr<RenderProgress<T>> progress(new RenderProgress<T>());
	auto_ptr<Renderer<T, bucketT>> renderer(CreateRenderer<T, bucketT>(opt.EmberCL() ? OPENCL_RENDERER : CPU_RENDERER, opt.Platform(), opt.Device(), false, 0, emberReport));
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand(ISAAC_INT(t.Tic()), ISAAC_INT(t.Tic() * 2), ISAAC_INT(t.Tic() * 3));
	vector<string> errorReport = emberReport.ErrorReport();

	os.imbue(std::locale(""));
	os2.imbue(std::locale(""));

	if (!errorReport.empty())
		emberReport.DumpErrorReport();

	if (!renderer.get())
	{
		cout << "Renderer creation failed, exiting." << endl;
		return false;
	}

	if (!InitPaletteList<T>(opt.PalettePath()))
		return false;

	if (!opt.EmberCL())
	{
		if (opt.ThreadCount() != 0)
			renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : NULL);
	}
	else
	{
		cout << "Using OpenCL to render." << endl;
		
		if (opt.Verbose())
		{
			cout << "Platform: " << wrapper.PlatformName(opt.Platform()) << endl;
			cout << "Device: " << wrapper.DeviceName(opt.Platform(), opt.Device()) << endl;
		}
	}

	//SheepTools will own the created renderer and will take care of cleaning it up.
	SheepTools<T, bucketT> tools(opt.PalettePath(), CreateRenderer<T, bucketT>(opt.EmberCL() ? OPENCL_RENDERER : CPU_RENDERER, opt.Platform(), opt.Device(), false, 0, emberReport2));

	tools.SetSpinParams(!opt.UnsmoothEdge(),
						T(opt.Stagger()),
						T(opt.OffsetX()),
						T(opt.OffsetY()),
						opt.Nick(),
						opt.Url(),
						opt.Id(),
						opt.Comment(),
						opt.SheepGen(),
						opt.SheepId());

	if (opt.UseVars() != "" && opt.DontUseVars() != "")
	{
		cout << "use_vars and dont_use_vars cannot both be specified. Returning without executing." << endl;
		return false;
	}

	//Specify reasonable defaults if nothing is specified.
	if (opt.UseVars() == "" && opt.DontUseVars() == "")
	{
		noVars.push_back(VAR_NOISE);
		noVars.push_back(VAR_BLUR);
		noVars.push_back(VAR_GAUSSIAN_BLUR);
		noVars.push_back(VAR_RADIAL_BLUR);
		noVars.push_back(VAR_NGON);
		noVars.push_back(VAR_SQUARE);
		noVars.push_back(VAR_RAYS);
		noVars.push_back(VAR_CROSS);
		noVars.push_back(VAR_PRE_BLUR);
		noVars.push_back(VAR_SEPARATION);
		noVars.push_back(VAR_SPLIT);
		noVars.push_back(VAR_SPLITS);
	  
		//Loop over the novars and set ivars to the complement.
		for (i = 0; i < varList.Size(); i++)
		{
			for (j = 0; j < noVars.size(); j++)
			{
				if (noVars[j] == varList.GetVariation(i)->VariationId())
					break;
			}

			if (j == noVars.size())
				vars.push_back(varList.GetVariation(i)->VariationId());
		}
	}
	else
	{
		if (opt.UseVars() != "")//Parse comma-separated list of variations to use.
		{
			istringstream iss(opt.UseVars());

			while (std::getline(iss, token, ','))
			{
				if (parser.Atoi((char*)token.c_str(), val))
				{
					if (val < varList.Size())
						vars.push_back((eVariationId)val);
				}
			}
		}
		else if (opt.DontUseVars() != "")
		{
			istringstream iss(opt.DontUseVars());

			while (std::getline(iss, token, ','))
			{
				if (parser.Atoi((char*)token.c_str(), val))
				{
					if (val < varList.Size())
						noVars.push_back((eVariationId)val);
				}
			}

			//Loop over the novars and set ivars to the complement.
			for (i = 0; i < varList.Size(); i++)
			{
				for (j = 0; j < noVars.size(); j++)
				{
					if (noVars[j] == varList.GetVariation(i)->VariationId())
						break;
				}

				if (j == noVars.size())
					vars.push_back(varList.GetVariation(i)->VariationId());
			}
		}
	}

	bool doMutate = opt.Mutate() != "";
	bool doInter  = opt.Inter()  != "";
	bool doRotate = opt.Rotate() != "";
	bool doClone  = opt.Clone()  != "";
	bool doStrip  = opt.Strip()  != "";
	bool doCross0 = opt.Cross0() != "";
	bool doCross1 = opt.Cross1() != "";

	count += (doMutate ? 1 : 0);
	count += (doInter  ? 1 : 0);
	count += (doRotate ? 1 : 0);
	count += (doClone  ? 1 : 0);
	count += (doStrip  ? 1 : 0);
	count += ((doCross0 || doCross1) ? 1 : 0);

	if (count > 1)
	{
		cout << "Can only specify one of mutate, clone, cross, rotate, strip, or inter. Returning without executing." << endl;
		return false;
	}

	if ((!doCross0) ^ (!doCross1))
	{
		cout << "Must specify both crossover arguments. Returning without executing." << endl;
		return false;
	}

	if (opt.Method() != "" && (!doCross0 && !doMutate))
	{
		cout << "Cannot specify method unless doing crossover or mutate. Returning without executing." << endl;
		return false;
	}

	if (opt.TemplateFile() != "")
	{
		if (!ParseEmberFile(parser, opt.TemplateFile(), templateEmbers))
			return false;
		
		if (templateEmbers.size() > 1)
			cout << "More than one control point in template, ignoring all but first." << endl;

		pTemplate = &templateEmbers[0];
	}

	//Methods for genetic manipulation begin here.
	if      (doMutate) filename = opt.Mutate();
	else if (doInter)  filename = opt.Inter();
	else if (doRotate) filename = opt.Rotate();
	else if (doClone)  filename = opt.Clone();
	else if (doStrip)  filename = opt.Strip();
	else if (doCross0) filename = opt.Cross0();
	else if (opt.CloneAll() != "") filename = opt.CloneAll();
	else if (opt.Animate()  != "") filename = opt.Animate();
	else if (opt.Sequence() != "") filename = opt.Sequence();
	else if (opt.Inter()    != "") filename = opt.Inter();
	else if (opt.Rotate()   != "") filename = opt.Rotate();
	else if (opt.Strip()    != "") filename = opt.Strip();
	else if (opt.Clone()    != "") filename = opt.Clone();
	else if (opt.Mutate()   != "") filename = opt.Mutate();

	if (!ParseEmberFile(parser, filename, embers))
		return false;

	if (doCross1 && !ParseEmberFile(parser, opt.Cross1(), embers2))
		return false;

	if (opt.CloneAll() != "")
	{
		cout << "<clone_all version=\"Ember-" << EmberVersion() << "\">" << endl;

		for (i = 0; i < embers.size(); i++)
		{
			if (pTemplate)
				tools.ApplyTemplate(embers[i], *pTemplate);
			
			tools.Offset(embers[i], (T)opt.OffsetX(), (T)opt.OffsetY());
			cout << emberToXml.ToString(embers[i], opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		}

		cout << "</clone_all>" << endl;
		return true;
	}

	if (opt.Animate() != "")
	{
		for (i = 0; i < embers.size(); i++)
		{
			if (i > 0 && embers[i].m_Time <= embers[i - 1].m_Time)
			{
				cout << "Error: control points must be sorted by time, but " << embers[i].m_Time << " <= " << embers[i - 1].m_Time << ", index " << i << "." << endl;
				return false;
			}

			embers[i].DeleteMotionElements();
		}

		firstFrame = (unsigned int)(opt.FirstFrame() == UINT_MAX ? embers[0].m_Time : opt.FirstFrame());
		lastFrame  = (unsigned int)(opt.LastFrame()  == UINT_MAX ? embers.back().m_Time : opt.LastFrame());

		if (lastFrame < firstFrame)
			lastFrame = firstFrame;

		cout << "<animate version=\"EMBER-" << EmberVersion() << "\">" << endl;

		for (ftime = firstFrame; ftime <= lastFrame; ftime++)
		{
			exactTimeMatch = false;

			for (i = 0; i < embers.size(); i++)
			{
				if (ftime == (unsigned int)embers[i].m_Time)
				{
					interpolated = embers[i];
					exactTimeMatch = true;
					break;
				}
			}

			if (!exactTimeMatch)
			{
				Interpolater<T>::Interpolate(embers, T(ftime), T(opt.Stagger()), interpolated);
				
				for (i = 0; i < embers.size(); i++)
				{
					if (ftime == (unsigned int)(embers[i].m_Time - 1))
					{
						exactTimeMatch = true;
						break;
					}
				}

				if (!exactTimeMatch)
					interpolated.m_AffineInterp = INTERP_LINEAR;
			}
		 
			if (pTemplate)
				tools.ApplyTemplate(interpolated, *pTemplate);

			cout << emberToXml.ToString(interpolated, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		}

		cout << "</animate>" << endl;
		return true;
	}

	if (opt.Sequence() != "")
	{
		frame = std::max(opt.Frame(), opt.Time());

		if (opt.Frames() == 0)
		{
			cout << "nframes must be positive and non-zero, not " << opt.Frames() << "." << endl;
			return false;
		}

		if (opt.Enclosed())
			cout << "<sequence version=\"EMBER-" << EmberVersion() << "\">" << endl;

		spread = 1 / T(opt.Frames());
		frameCount = 0;

		for (i = 0; i < embers.size(); i++)
		{
			if (opt.Loops())
			{
				for (frame = 0; frame < opt.Frames(); frame++)
				{
					blend = (T)frame / (T)opt.Frames();
					tools.Spin(embers[i], pTemplate, result, frameCount++, blend);//Result is cleared and reassigned each time inside of Spin().
					cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
				}
			}

			if (i < embers.size() - 1)
			{
				for (frame = 0; frame < opt.Frames(); frame++)
				{
					seqFlag = (frame == 0 || frame == opt.Frames() - 1);
					blend = frame / (T)opt.Frames();
					result.Clear();
					tools.SpinInter(&embers[i], pTemplate, result, frameCount++, seqFlag, blend);
					cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
				}
			}
		}

		result = embers.back();
		tools.Spin(embers.back(), pTemplate, result, frameCount, 0);
		cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		
		if (opt.Enclosed())
			cout << "</sequence>" << endl;

		return true;
	}

	if (doInter || doRotate)
	{
		frame = std::max(opt.Frame(), opt.Time());
		
		if (opt.Frames() == 0)
		{
			cout << "nframes must be positive and non-zero, not " << opt.Frames() << "." << endl;
			return false;
		}

		blend = frame / T(opt.Frames());
		spread = 1 / T(opt.Frames());

		if (opt.Enclosed())
			cout << "<pick version=\"EMBER-" << EmberVersion() << "\">" << endl;

		if (doRotate)
		{
			if (embers.size() != 1)
			{
				cout << "rotation requires one control point, not " << embers.size() << "." << endl;
				return false;
			}

			tools.Spin(embers[0], pTemplate, result1, frame - 1, blend - spread);
			tools.Spin(embers[0], pTemplate, result2, frame    , blend         );
			tools.Spin(embers[0], pTemplate, result3, frame + 1, blend + spread);

			cout << emberToXml.ToString(result1, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
			cout << emberToXml.ToString(result2, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
			cout << emberToXml.ToString(result3, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		}
		else
		{
			if (embers.size() != 2)
			{
				cout << "interpolation requires two control points, not " << embers.size() << "." << endl;
				return false;
			}

			tools.SpinInter(embers.data(), pTemplate, result1, frame - 1, 0, blend - spread);
			tools.SpinInter(embers.data(), pTemplate, result2, frame    , 0, blend         );
			tools.SpinInter(embers.data(), pTemplate, result3, frame + 1, 0, blend + spread);

			cout << emberToXml.ToString(result1, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
			cout << emberToXml.ToString(result2, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
			cout << emberToXml.ToString(result3, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		}

		if (opt.Enclosed())
			cout << "</pick>" << endl;
	  
		return true;
	}

	if (doStrip)
	{
		if (opt.Enclosed())
			cout << "<pick version=\"EMBER-" << EmberVersion() << "\">" << endl;

		for (i = 0; i < embers.size(); i++)
		{
			T oldX, oldY;

			embers[i].DeleteMotionElements();
			
			oldX = embers[i].m_CenterX;
			oldY = embers[i].m_CenterY;
			embers[i].m_FinalRasH = (unsigned int)((T)embers[i].m_FinalRasH / (T)opt.Frames());

			embers[i].m_CenterY = embers[i].m_CenterY - ((opt.Frames() - 1) * embers[i].m_FinalRasH) /
				(2 * embers[i].m_PixelsPerUnit * pow(T(2.0), embers[i].m_Zoom));
			embers[i].m_CenterY += embers[i].m_FinalRasH * opt.Frame() / (embers[i].m_PixelsPerUnit * pow(T(2.0), embers[i].m_Zoom));

			tools.RotateOldCenterBy(embers[i].m_CenterX, embers[i].m_CenterY, oldX, oldY, embers[i].m_Rotate);

			if (pTemplate)
				tools.ApplyTemplate(embers[i], *pTemplate);
			
			tools.Offset(embers[i], T(opt.OffsetX()), T(opt.OffsetY()));
			cout << emberToXml.ToString(embers[i], opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		}

		if (opt.Enclosed())
			cout << "</pick>" << endl;

		return true;
	}

	//Repeat.
	renderer->EarlyClip(opt.EarlyClip());
	renderer->YAxisUp(opt.YAxisUp());
	renderer->LockAccum(opt.LockAccum());
	renderer->SubBatchSize(opt.SubBatchSize());
	renderer->PixelAspectRatio(T(opt.AspectRatio()));
	renderer->Transparency(opt.Transparency());

	if (opt.Repeat() == 0)
	{
		cout << "Repeat must be positive, not " << opt.Repeat() << endl;
		return false;
	}

	if (opt.Enclosed())
		cout << "<pick version=\"EMBER-" << EmberVersion() << "\">" << endl;

	for (rep = 0; rep < opt.Repeat(); rep++)
	{
		count = 0;
		os.str("");
		save.Clear();
		VerbosePrint("Flame = " << rep + 1 << "/" << opt.Repeat() << "..");

		if (opt.Clone() != "")
		{
			os << "clone";//Action is 'clone' with trunc vars concat.

			if (opt.CloneAction() != "")
				os << " " << opt.CloneAction();

			selp0 = embers[rand.Rand() % embers.size()];
			save = selp0;
			aselp0 = &selp0;
			aselp1 = NULL;
			os << tools.TruncateVariations(save, 5);
			save.m_Edits = emberToXml.CreateNewEditdoc(aselp0, aselp1, os.str(), opt.Nick(), opt.Url(), opt.Id(), opt.Comment(), opt.SheepGen(), opt.SheepId());
		}
		else
		{
			do
			{
				randomMode = false;
				didColor = false;
				os.str("");
				VerbosePrint(".");

				if (doMutate)
				{
					selp0 = embers[rand.Rand() % embers.size()];
					orig = selp0;
					aselp0 = &selp0;
					aselp1 = NULL;

					if (opt.Method() == "")
						mutMeth = MUTATE_NOT_SPECIFIED;
					else if (opt.Method() == "all_vars")
						mutMeth = MUTATE_ALL_VARIATIONS;
					else if (opt.Method() == "one_xform")
						mutMeth = MUTATE_ONE_XFORM_COEFS;
					else if (opt.Method() == "add_symmetry")
						mutMeth = MUTATE_ADD_SYMMETRY;
					else if (opt.Method() == "post_xforms")
						mutMeth = MUTATE_POST_XFORMS;
					else if (opt.Method() == "color_palette")
						mutMeth = MUTATE_COLOR_PALETTE;
					else if (opt.Method() == "delete_xform")
						mutMeth = MUTATE_DELETE_XFORM;
					else if (opt.Method() == "all_coefs")
						mutMeth = MUTATE_ALL_COEFS;
					else
					{
						cout << "method " << opt.Method() << " not defined for mutate. Defaulting to random." << endl;
						mutMeth = MUTATE_NOT_SPECIFIED;
					}

					os << tools.Mutate(orig, mutMeth, vars, opt.Symmetry(), T(opt.Speed()));

					//Scan string returned for 'mutate color'.
					if (strstr(os.str().c_str(), "mutate color"))
						didColor = true;
				  
					if (orig.m_Name != "")
					{
						os2.str("");
						os2 << "mutation " << rep << " of " << orig.m_Name;
						orig.m_Name = os2.str();
					}
				}
				else if (doCross0)
				{
					i0 = rand.Rand() % embers.size();
					i1 = rand.Rand() % embers2.size();

					selp0 = embers[i0];
					selp1 = embers2[i1];

					aselp0 = &selp0;
					aselp1 = &selp1;

					if (opt.Method() == "")
						crossMeth = CROSS_NOT_SPECIFIED;
					else if (opt.Method() == "union")
						crossMeth = CROSS_UNION;
					else if (opt.Method() == "interpolate")
						crossMeth = CROSS_INTERPOLATE;
					else if (opt.Method() == "alternate")
						crossMeth = CROSS_ALTERNATE;
					else
					{
						cout << "method '" << opt.Method() << "' not defined for cross. Defaulting to random." << endl;
						crossMeth = CROSS_NOT_SPECIFIED;
					}

					tools.Cross(embers[i0], embers2[i1], orig, crossMeth);

					if (embers[i0].m_Name != "" || embers2[i1].m_Name != "")
					{
						os2.str("");
						os2 << rep << " of " << embers[i0].m_Name << " x " << embers2[i1].m_Name;
						orig.m_Name = os2.str();
					}
				}
				else
				{
					os << "random";
					randomMode = true;
					tools.Random(orig, vars, opt.Symmetry(), 0);
					aselp0 = NULL;
					aselp1 = NULL;
				}

				//Adjust bounding box half the time.
				if (rand.RandBit() || randomMode)
				{
					T bmin[2], bmax[2];
					tools.EstimateBoundingBox(orig, T(0.01), 100000, bmin, bmax);

					if (rand.Frand01<T>() < T(0.3))
					{
						orig.m_CenterX = (bmin[0] + bmax[0]) / 2;
						orig.m_CenterY = (bmin[1] + bmax[1]) / 2;
						os << " recentered";
					}
					else
					{
						if (rand.RandBit())
						{
							mix0 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							mix1 = rand.GoldenBit<T>();
							os << " reframed0";
						}
						else if (rand.RandBit())
						{
							mix0 = rand.GoldenBit<T>();
							mix1 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							os << " reframed1";
						}
						else
						{
							mix0 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							mix1 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							os << " reframed2";
						}

						orig.m_CenterX = mix0 * bmin[0] + (1 - mix0) * bmax[0];
						orig.m_CenterY = mix1 * bmin[1] + (1 - mix1) * bmax[1];
					}

					orig.m_PixelsPerUnit = orig.m_FinalRasW / (bmax[0] - bmin[0]);
				}

				os << tools.TruncateVariations(orig, 5);

				if (!didColor && rand.RandBit())
				{
					if (opt.Debug())
						cout << "improving colors..." << endl;

					tools.ImproveColors(orig, 100, false, 10);
					os << " improved colors";
				}

				orig.m_Edits = emberToXml.CreateNewEditdoc(aselp0, aselp1, os.str(), opt.Nick(), opt.Url(), opt.Id(), opt.Comment(), opt.SheepGen(), opt.SheepId());
				save = orig;
				SetDefaultTestValues(orig);
				renderer->SetEmber(orig);

				if (renderer->Run(finalImage) != RENDER_OK)
				{
					cout << "Error: test image rendering failed, aborting." << endl;
					return false;
				}

				tot = totb = totw = 0;
				n = orig.m_FinalRasW * orig.m_FinalRasH;

				for (i = 0; i < 3 * n; i += 3)
				{
					tot += (finalImage[i] + finalImage[i + 1] + finalImage[i + 2]);

					if (0   == finalImage[i] && 0   == finalImage[i + 1] && 0   == finalImage[i + 2]) totb++;
					if (255 == finalImage[i] && 255 == finalImage[i + 1] && 255 == finalImage[i + 2]) totw++;
				}

				avgPix = (tot / T(3 * n));
				fractionBlack = totb / T(n);
				fractionWhite = totw / T(n);

				if (opt.Debug())
					cout << "avgPix = " << avgPix << " fractionBlack = " << fractionBlack << " fractionWhite = " << fractionWhite << " n = " << n << endl;
				
				orig.Clear();
				count++;
			} while ((avgPix < opt.AvgThresh() ||
				fractionBlack < opt.BlackThresh() ||
				fractionWhite > opt.WhiteLimit()) &&
				count < opt.Tries());

			if (count == opt.Tries())
				cout << "Warning: reached maximum attempts, giving up." << endl;
		}

		if (pTemplate)
			tools.ApplyTemplate(save, *pTemplate);

		save.m_Time = T(rep);
		
		if (opt.MaxXforms() != UINT_MAX)
		{
			save.m_Symmetry = 0;

			while (save.TotalXformCount() > opt.MaxXforms())
				save.DeleteTotalXform(save.TotalXformCount() - 1);
		}

		cout << emberToXml.ToString(save, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), false, opt.HexPalette());
		VerbosePrint("\nDone. Action = " << os.str() << "\n");
		cout.flush();
		save.Clear();
	}

	if (opt.Enclosed())
		cout << "</pick>\n";

	return true;
}

/// <summary>
/// Main program entry point for EmberGenome.exe.
/// </summary>
/// <param name="argc">The number of command line arguments passed</param>
/// <param name="argv">The command line arguments passed</param>
/// <returns>0 if successful, else 1.</returns>
int _tmain(int argc, _TCHAR* argv[])
{
	bool b, d = true;
	EmberOptions opt;
	
	//Required for large allocs, else GPU memory usage will be severely limited to small sizes.
	//This must be done in the application and not in the EmberCL DLL.
	_putenv_s("GPU_MAX_ALLOC_PERCENT", "100");

	if (opt.Populate(argc, argv, OPT_USE_GENOME))
		return 0;

#ifdef DO_DOUBLE
	if (opt.Bits() == 64)
	{
		b = EmberGenome<double, double>(opt);
	}
	else
#endif		
	if (opt.Bits() == 33)
	{
		b = EmberGenome<float, float>(opt);
	}
	else if (opt.Bits() == 32)
	{
		cout << "Bits 32/int histogram no longer supported. Using bits == 33 (float)." << endl;
		b = EmberGenome<float, float>(opt);
	}

	return b ? 0 : 1;
}