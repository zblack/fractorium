#include "EmberCommonPch.h"
#include "EmberAnimate.h"
#include "JpegUtils.h"

/// <summary>
/// The core of the EmberAnimate.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T, typename bucketT>
bool EmberAnimate(EmberOptions& opt)
{
	OpenCLWrapper wrapper;

	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cout << opt.GetValues(OPT_USE_ANIMATE) << endl;

	if (opt.OpenCLInfo())
	{
		cout << "\nOpenCL Info: " << endl;
		cout << wrapper.DumpInfo();
		return true;
	}

	//Regular variables.
	Timing t;
	bool unsorted = false;
	bool startXml = false;
	bool finishXml = false;
	bool appendXml = false;
	uint finalImageIndex = 0;
	uint i, channels, ftime;
	string s, flameName, filename, inputPath = GetPath(opt.Input());
	ostringstream os;
	vector<Ember<T>> embers;
	EmberStats stats;
	EmberReport emberReport;
	EmberImageComments comments;
	Ember<T> centerEmber;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	vector<byte> finalImages[2];
	std::thread writeThread;
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

		renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);
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
		renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);

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

	if (opt.Dtime() < 1)
	{
		cout << "Warning: dtime must be positive, not " << opt.Dtime() << ". Setting to 1." << endl;
		opt.Dtime(1);
	}

	if (opt.Frame())
	{
		if (opt.Time())
		{
			cout << "Cannot specify both time and frame." << endl;
			return false;
		}

		if (opt.FirstFrame() || opt.LastFrame())
		{
			cout << "Cannot specify both frame and begin or end." << endl;
			return false;
		}

		opt.FirstFrame(opt.Frame());
		opt.LastFrame(opt.Frame());
	}

	if (opt.Time())
	{
		if (opt.FirstFrame() || opt.LastFrame())
		{
			cout << "Cannot specify both time and begin or end." << endl;
			return false;
		}

		opt.FirstFrame(opt.Time());
		opt.LastFrame(opt.Time());
	}

	//Prep all embers, by ensuring they:
	//-Are sorted by time.
	//-Do not have a dimension of 0.
	//-Do not have a memory requirement greater than max uint.
	//-Have quality and size scales applied, if present.
	//-Have equal dimensions.
	for (i = 0; i < embers.size(); i++)
	{
		if (i > 0 && embers[i].m_Time <= embers[i - 1].m_Time)
			unsorted = true;

		if (opt.Supersample() > 0)
			embers[i].m_Supersample = opt.Supersample();

		if (opt.SubBatchSize() != DEFAULT_SBS)
			embers[i].m_SubBatchSize = opt.SubBatchSize();

		embers[i].m_Quality *= T(opt.QualityScale());
		embers[i].m_FinalRasW = uint(T(embers[i].m_FinalRasW) * opt.SizeScale());
		embers[i].m_FinalRasH = uint(T(embers[i].m_FinalRasH) * opt.SizeScale());
		embers[i].m_PixelsPerUnit *= T(opt.SizeScale());

		//Cast to double in case the value exceeds 2^32.
		double imageMem = double(channels) * double(embers[i].m_FinalRasW)
			   * double(embers[i].m_FinalRasH) * double(renderer->BytesPerChannel());
		double maxMem = pow(2.0, double((sizeof(void*) * 8) - 1));

		if (imageMem > maxMem)//Ensure the max amount of memory for a process isn't exceeded.
		{
			cout << "Image " << i << " size > " << maxMem << ". Setting to 1920 x 1080." << endl;
			embers[i].m_FinalRasW = 1920;
			embers[i].m_FinalRasH = 1080;
		}

		if (embers[i].m_FinalRasW == 0 || embers[i].m_FinalRasH == 0)
		{
			cout << "Warning: Output image " << i << " has dimension 0: " << embers[i].m_FinalRasW  << ", " << embers[i].m_FinalRasH << ". Setting to 1920 x 1080." << endl;
			embers[i].m_FinalRasW = 1920;
			embers[i].m_FinalRasH = 1080;
		}

		if ((embers[i].m_FinalRasW != embers[0].m_FinalRasW) ||
			(embers[i].m_FinalRasH != embers[0].m_FinalRasH))
		{
			cout << "Warning: flame " << i << " at time " << embers[i].m_Time << " size mismatch. (" << embers[i].m_FinalRasW << ", " << embers[i].m_FinalRasH <<
				") should be (" << embers[0].m_FinalRasW << ", " << embers[0].m_FinalRasH << "). Setting to " << embers[0].m_FinalRasW << ", " << embers[0].m_FinalRasH << "." << endl;

			embers[i].m_FinalRasW = embers[0].m_FinalRasW;
			embers[i].m_FinalRasH = embers[0].m_FinalRasH;
		}
	}

	if (unsorted)
	{
		cout << "Embers were unsorted by time. First out of order index was " << i << ". Sorting." << endl;
		std::sort(embers.begin(), embers.end(), &CompareEmbers<T>);
	}

	if (!opt.Time() && !opt.Frame())
	{
		if (opt.FirstFrame() == UINT_MAX)
			opt.FirstFrame(int(embers[0].m_Time));

		if (opt.LastFrame() == UINT_MAX)
			opt.LastFrame(ClampGte<uint>(uint(embers.back().m_Time - 1), opt.FirstFrame()));
	}

	if (!opt.Out().empty())
	{
		appendXml = true;
		filename = opt.Out();
		cout << "Single output file " << opt.Out() << " specified for multiple images. They will be all overwritten and only the last image will remain." << endl;
	}

	//Final setup steps before running.
	os.imbue(std::locale(""));
	renderer->SetEmber(embers);
	renderer->EarlyClip(opt.EarlyClip());
	renderer->YAxisUp(opt.YAxisUp());
	renderer->LockAccum(opt.LockAccum());
	renderer->InsertPalette(opt.InsertPalette());
	renderer->PixelAspectRatio(T(opt.AspectRatio()));
	renderer->Transparency(opt.Transparency());
	renderer->NumChannels(channels);
	renderer->BytesPerChannel(opt.BitsPerChannel() / 8);
	renderer->Callback(opt.DoProgress() ? progress.get() : nullptr);

	std::function<void(uint)> saveFunc = [&](uint threadVecIndex)
	{
		bool writeSuccess = false;
		byte* finalImagep = finalImages[threadVecIndex].data();

		if ((opt.Format() == "jpg" || opt.Format() == "bmp") && renderer->NumChannels() == 4)
			RgbaToRgb(finalImages[threadVecIndex], finalImages[threadVecIndex], renderer->FinalRasW(), renderer->FinalRasH());

		if (opt.Format() == "png")
			writeSuccess = WritePng(filename.c_str(), finalImagep, renderer->FinalRasW(), renderer->FinalRasH(), opt.BitsPerChannel() / 8, opt.PngComments(), comments, opt.Id(), opt.Url(), opt.Nick());
		else if (opt.Format() == "jpg")
			writeSuccess = WriteJpeg(filename.c_str(), finalImagep, renderer->FinalRasW(), renderer->FinalRasH(), opt.JpegQuality(), opt.JpegComments(), comments, opt.Id(), opt.Url(), opt.Nick());
		else if (opt.Format() == "ppm")
			writeSuccess = WritePpm(filename.c_str(), finalImagep, renderer->FinalRasW(), renderer->FinalRasH());
		else if (opt.Format() == "bmp")
			writeSuccess = WriteBmp(filename.c_str(), finalImagep, renderer->FinalRasW(), renderer->FinalRasH());

		if (!writeSuccess)
			cout << "Error writing " << filename << endl;/**/
	};

	//Begin run.
	for (ftime = opt.FirstFrame(); ftime <= opt.LastFrame(); ftime += opt.Dtime())
	{
		T localTime = T(ftime);

		if ((opt.LastFrame() - opt.FirstFrame()) / opt.Dtime() >= 1)
			VerbosePrint("Time = " << ftime << " / " << opt.LastFrame() << " / " << opt.Dtime());

		renderer->Reset();

		if ((renderer->Run(finalImages[finalImageIndex], localTime) != RENDER_OK) || renderer->Aborted() || finalImages[finalImageIndex].empty())
		{
			cout << "Error: image rendering failed, skipping to next image." << endl;
			renderer->DumpErrorReport();//Something went wrong, print errors.
			continue;
		}

		if (opt.Out().empty())
		{
			ostringstream fnstream;
			fnstream << inputPath << opt.Prefix() << setfill('0') << setw(opt.FilenamePadding()) << ftime << opt.Suffix() << "." << opt.Format();
			filename = fnstream.str();
		}

		if (opt.WriteGenome())
		{
			flameName = filename.substr(0, filename.find_last_of('.')) + ".flam3";
			VerbosePrint("Writing " + flameName);
			Interpolater<T>::Interpolate(embers, localTime, 0, centerEmber);//Get center flame.

			if (appendXml)
			{
				startXml = ftime == opt.FirstFrame();
				finishXml = ftime == opt.LastFrame();
			}

			emberToXml.Save(flameName, centerEmber, opt.PrintEditDepth(), true, opt.IntPalette(), opt.HexPalette(), true, startXml, finishXml);
		}

		stats = renderer->Stats();
		comments = renderer->ImageComments(stats, opt.PrintEditDepth(), opt.IntPalette(), opt.HexPalette());
		os.str("");
		size_t iterCount = renderer->TotalIterCount(1);
		os << comments.m_NumIters << " / " << iterCount << " (" << std::fixed << std::setprecision(2) << ((double(stats.m_Iters) / double(iterCount)) * 100) << "%)";

		VerbosePrint("\nIters ran/requested: " + os.str());
		VerbosePrint("Bad values: " << stats.m_Badvals);
		VerbosePrint("Render time: " + t.Format(stats.m_RenderMs));
		VerbosePrint("Pure iter time: " + t.Format(stats.m_IterMs));
		VerbosePrint("Iters/sec: " << size_t(stats.m_Iters / (stats.m_IterMs / 1000.0)) << endl);
		VerbosePrint("Writing " + filename);

		//Run image writing in a thread. Although doing it this way duplicates the final output memory, it saves a lot of time
		//when running with OpenCL. Call join() to ensure the previous thread call has completed.
		if (writeThread.joinable())
			writeThread.join();

		uint threadVecIndex = finalImageIndex;//Cache before launching thread.

		if (opt.ThreadedWrite())
			writeThread = std::thread(saveFunc, threadVecIndex);
		else
			saveFunc(threadVecIndex);

		centerEmber.Clear();
		finalImageIndex ^= 1;//Toggle the index.
	}

	if (writeThread.joinable())
		writeThread.join();

	VerbosePrint("Done.\n");

	if (opt.Verbose())
		t.Toc("\nTotal time: ", true);

	return true;
}

/// <summary>
/// Main program entry point for EmberAnimate.exe.
/// </summary>
/// <param name="argc">The number of command line arguments passed</param>
/// <param name="argv">The command line arguments passed</param>
/// <returns>0 if successful, else 1.</returns>
int _tmain(int argc, _TCHAR* argv[])
{
	bool b = false;
	EmberOptions opt;

	//Required for large allocs, else GPU memory usage will be severely limited to small sizes.
	//This must be done in the application and not in the EmberCL DLL.
#ifdef WIN32
	_putenv_s("GPU_MAX_ALLOC_PERCENT", "100");
#else
	putenv(const_cast<char*>("GPU_MAX_ALLOC_PERCENT=100"));
#endif

	if (!opt.Populate(argc, argv, OPT_USE_ANIMATE))
	{

#ifdef DO_DOUBLE
		if (opt.Bits() == 64)
		{
			b = EmberAnimate<double, double>(opt);
		}
		else
#endif
		if (opt.Bits() == 33)
		{
			b = EmberAnimate<float, float>(opt);
		}
		else if (opt.Bits() == 32)
		{
			cout << "Bits 32/int histogram no longer supported. Using bits == 33 (float)." << endl;
			b = EmberAnimate<float, float>(opt);
		}
	}

	return b ? 0 : 1;
}
