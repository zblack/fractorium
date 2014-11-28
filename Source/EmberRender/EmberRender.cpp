#include "EmberCommonPch.h"
#include "EmberRender.h"
#include "JpegUtils.h"

/// <summary>
/// The core of the EmberRender.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
bool EmberRender(EmberOptions& opt)	
{
	OpenCLWrapper wrapper;

	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cout << opt.GetValues(OPT_USE_RENDER) << endl;

	if (opt.OpenCLInfo())
	{
		cout << "\nOpenCL Info: " << endl;
		cout << wrapper.DumpInfo();
		return true;
	}

	Timing t;
	bool writeSuccess = false;
	unsigned char* finalImagep;
	size_t i, channels;
	size_t strips;
	size_t iterCount;
	string filename;
	string inputPath = GetPath(opt.Input());
	ostringstream os;
	vector<Ember<T>> embers;
	vector<unsigned char> finalImage, vecRgb;
	EmberStats stats;
	EmberReport emberReport;
	EmberImageComments comments;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> randVec;
	unique_ptr<RenderProgress<T>> progress(new RenderProgress<T>());
	unique_ptr<Renderer<T, bucketT>> renderer(CreateRenderer<T, bucketT>(opt.EmberCL() ? OPENCL_RENDERER : CPU_RENDERER, opt.Platform(), opt.Device(), false, 0, emberReport));
	vector<string> errorReport = emberReport.ErrorReport();

	if (!errorReport.empty())
		emberReport.DumpErrorReport();

	if (!renderer.get())
	{
		cout << "Renderer creation failed, exiting." << endl;
		return false;
	}

	if (opt.EmberCL() && renderer->RendererType() != OPENCL_RENDERER)//OpenCL init failed, so fall back to CPU.
		opt.EmberCL(false);

	if (!InitPaletteList<T>(opt.PalettePath()))
		return false;

	if (!ParseEmberFile(parser, opt.Input(), embers))
		return false;

	if (!opt.EmberCL())
	{
		if (opt.ThreadCount() == 0)
		{
			cout << "Using " << Timing::ProcessorCount() << " automatically detected threads." << endl;
			opt.ThreadCount(Timing::ProcessorCount());
		}
		else
		{
			cout << "Using " << opt.ThreadCount() << " manually specified threads." << endl;
		}

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

		if (opt.ThreadCount() > 1)
			cout << "Cannot specify threads with OpenCL, using 1 thread." << endl;

		opt.ThreadCount(1);
		renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : NULL);

		if (opt.BitsPerChannel() != 8)
		{
			cout << "Bits per channel cannot be anything other than 8 with OpenCL, setting to 8." << endl;
			opt.BitsPerChannel(8);
		}
	}

	if (opt.Format() != "jpg" &&
		opt.Format() != "png" &&
		opt.Format() != "ppm" &&
		opt.Format() != "bmp")
	{
		cout << "Format must be jpg, png, ppm, or bmp not " << opt.Format() << ". Setting to jpg." << endl;
	}

	channels = opt.Format() == "png" ? 4 : 3;

	if (opt.BitsPerChannel() == 16 && opt.Format() != "png")
	{
		cout << "Support for 16 bits per channel images is only present for the png format. Setting to 8." << endl;
		opt.BitsPerChannel(8);
	}
	else if (opt.BitsPerChannel() != 8 && opt.BitsPerChannel() != 16)
	{
		cout << "Unexpected bits per channel specified " << opt.BitsPerChannel() << ". Setting to 8." << endl;
		opt.BitsPerChannel(8);
	}

	if (opt.InsertPalette() && opt.BitsPerChannel() != 8)
	{
		cout << "Inserting palette only supported with 8 bits per channel, insertion will not take place." << endl;
		opt.InsertPalette(false);
	}

	if (opt.AspectRatio() < 0)
	{
		cout << "Invalid pixel aspect ratio " << opt.AspectRatio() << endl << ". Must be positive, setting to 1." << endl;
		opt.AspectRatio(1);
	}

	if (!opt.Out().empty() && (embers.size() > 1))
	{
		cout << "Single output file " << opt.Out() << " specified for multiple images. Changing to use prefix of badname-changethis instead. Always specify prefixes when reading a file with multiple embers." << endl;
		opt.Out("");
		opt.Prefix("badname-changethis");
	}

	//Final setup steps before running.
	os.imbue(std::locale(""));
	renderer->EarlyClip(opt.EarlyClip());
	renderer->YAxisUp(opt.YAxisUp());
	renderer->LockAccum(opt.LockAccum());
	renderer->InsertPalette(opt.InsertPalette());
	renderer->PixelAspectRatio(T(opt.AspectRatio()));
	renderer->Transparency(opt.Transparency());
	renderer->NumChannels(channels);
	renderer->BytesPerChannel(opt.BitsPerChannel() / 8);
	renderer->Callback(opt.DoProgress() ? progress.get() : NULL);

	for (i = 0; i < embers.size(); i++)
	{
		if (opt.Verbose() && embers.size() > 1)
			cout << "\nFlame = " << i + 1 << "/" << embers.size() << endl;
		else
			VerbosePrint(endl);

		if (opt.Supersample() > 0)
			embers[i].m_Supersample = opt.Supersample();

		if (opt.SubBatchSize() != DEFAULT_SBS)
			embers[i].m_SubBatchSize = opt.SubBatchSize();

		embers[i].m_TemporalSamples = 1;//Force temporal samples to 1 for render.
		embers[i].m_Quality *= T(opt.QualityScale());
		embers[i].m_FinalRasW = (unsigned int)((T)embers[i].m_FinalRasW * opt.SizeScale());
		embers[i].m_FinalRasH = (unsigned int)((T)embers[i].m_FinalRasH * opt.SizeScale());
		embers[i].m_PixelsPerUnit *= T(opt.SizeScale());

		if (embers[i].m_FinalRasW == 0 || embers[i].m_FinalRasH == 0)
		{
			cout << "Output image " << i << " has dimension 0: " << embers[i].m_FinalRasW  << ", " << embers[i].m_FinalRasH << ". Setting to 1920 x 1080." << endl;
			embers[i].m_FinalRasW = 1920;
			embers[i].m_FinalRasH = 1080;
		}

		//Cast to double in case the value exceeds 2^32.
		double imageMem = (double)renderer->NumChannels() * (double)embers[i].m_FinalRasW 
			   * (double)embers[i].m_FinalRasH * (double)renderer->BytesPerChannel();
		double maxMem = pow(2.0, double((sizeof(void*) * 8) - 1));

		if (imageMem > maxMem)//Ensure the max amount of memory for a process is not exceeded.
		{
			cout << "Image " << i << " size > " << maxMem << ". Setting to 1920 x 1080." << endl;
			embers[i].m_FinalRasW = 1920;
			embers[i].m_FinalRasH = 1080;
		}

		stats.Clear();
		renderer->SetEmber(embers[i]);
		renderer->PrepFinalAccumVector(finalImage);//Must manually call this first because it could be erroneously made smaller due to strips if called inside Renderer::Run().

		if (opt.Strips() > 1)
		{
			strips = opt.Strips();
		}
		else
		{
			strips = CalcStrips((double)renderer->MemoryRequired(1, true), (double)renderer->MemoryAvailable(), opt.UseMem());

			if (strips > 1)
				VerbosePrint("Setting strips to " << strips << " with specified memory usage of " << opt.UseMem());
		}

		strips = VerifyStrips(embers[i].m_FinalRasH, strips,
			[&](const string& s) { cout << s << endl; },//Greater than height.
			[&](const string& s) { cout << s << endl; },//Mod height != 0.
			[&](const string& s) { cout << s << endl; });//Final strips value to be set.

		//For testing incremental renderer.
		//int sb = 1;
		//bool resume = false, success = false;
		//do
		//{
		//	success = renderer->Run(finalImage, 0, sb, false/*resume == false*/) == RENDER_OK;
		//	sb++;
		//	resume = true;
		//}
		//while (success && renderer->ProcessState() != ACCUM_DONE);

		StripsRender<T>(renderer.get(), embers[i], finalImage, 0, strips, opt.YAxisUp(),
		[&](size_t strip)//Pre strip.
		{
			if (opt.Verbose() && (strips > 1) && strip > 0)
				cout << endl;

			if (strips > 1)
				VerbosePrint("Strip = " << (strip + 1) << "/" << strips);
		},
		[&](size_t strip)//Post strip.
		{
			progress->Clear();
			stats += renderer->Stats();
		},
		[&](size_t strip)//Error.
		{
			cout << "Error: image rendering failed, skipping to next image." << endl;
			renderer->DumpErrorReport();//Something went wrong, print errors.
		},
		//Final strip.
		//Original wrote every strip as a full image which could be very slow with many large images.
		//Only write once all strips for this image are finished.
		[&](Ember<T>& finalEmber)
		{
			if (!opt.Out().empty())
			{
				filename = opt.Out();
			}
			else if (opt.NameEnable() && !finalEmber.m_Name.empty())
			{
				filename = inputPath + opt.Prefix() + finalEmber.m_Name + opt.Suffix() + "." + opt.Format();
			}
			else
			{
				ostringstream ssLocal;

				ssLocal << inputPath << opt.Prefix() << setfill('0') << setw(5) << i << opt.Suffix() << "." << opt.Format();
				filename = ssLocal.str();
			}

			//TotalIterCount() is actually using ScaledQuality() which does not get reset upon ember assignment,
			//so it ends up using the correct value for quality * strips.
			iterCount = renderer->TotalIterCount(1);
			comments = renderer->ImageComments(stats, opt.PrintEditDepth(), opt.IntPalette(), opt.HexPalette());
			os.str("");
			os << comments.m_NumIters << " / " << iterCount << " (" << std::fixed << std::setprecision(2) << (((double)stats.m_Iters / (double)iterCount) * 100) << "%)";

			VerbosePrint("\nIters ran/requested: " + os.str());
			VerbosePrint("Bad values: " << stats.m_Badvals);
			VerbosePrint("Render time: " + t.Format(stats.m_RenderMs));
			VerbosePrint("Pure iter time: " + t.Format(stats.m_IterMs));
			VerbosePrint("Iters/sec: " << unsigned __int64(stats.m_Iters / (stats.m_IterMs / 1000.0)) << endl);
			VerbosePrint("Writing " + filename);

			if ((opt.Format() == "jpg" || opt.Format() == "bmp") && renderer->NumChannels() == 4)
			{
				RgbaToRgb(finalImage, vecRgb, finalEmber.m_FinalRasW, finalEmber.m_FinalRasH);

				finalImagep = vecRgb.data();
			}
			else
			{
				finalImagep = finalImage.data();
			}

			writeSuccess = false;

			if (opt.Format() == "png")
				writeSuccess = WritePng(filename.c_str(), finalImagep, finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.BitsPerChannel() / 8, opt.PngComments(), comments, opt.Id(), opt.Url(), opt.Nick());
			else if (opt.Format() == "jpg")
				writeSuccess = WriteJpeg(filename.c_str(), finalImagep, finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.JpegQuality(), opt.JpegComments(), comments, opt.Id(), opt.Url(), opt.Nick());
			else if (opt.Format() == "ppm")
				writeSuccess = WritePpm(filename.c_str(), finalImagep, finalEmber.m_FinalRasW, finalEmber.m_FinalRasH);
			else if (opt.Format() == "bmp")
				writeSuccess = WriteBmp(filename.c_str(), finalImagep, finalEmber.m_FinalRasW, finalEmber.m_FinalRasH);

			if (!writeSuccess)
				cout << "Error writing " << filename << endl;
		});

		if (opt.EmberCL() && opt.DumpKernel())
			cout << "Iteration kernel: \n" << ((RendererCL<T>*)renderer.get())->IterKernel() << endl;

		VerbosePrint("Done.");
	}

	if (opt.Verbose())
		t.Toc("\nTotal time: ", true);

	return true;
}

/// <summary>
/// Main program entry point for EmberRender.exe.
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

	if (opt.Populate(argc, argv, OPT_USE_RENDER))
		return 0;

#ifdef DO_DOUBLE
	if (opt.Bits() == 64)
	{
		b = EmberRender<double, double>(opt);
	}
	else
#endif
	if (opt.Bits() == 33)
	{
		b = EmberRender<float, float>(opt);
	}
	else if (opt.Bits() == 32)
	{
		cout << "Bits 32/int histogram no longer supported. Using bits == 33 (float)." << endl;
		b = EmberRender<float, float>(opt);
	}

	return b ? 0 : 1;
}