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
	unsigned int i, channels, strip, strips, realHeight, origHeight;
	size_t stripOffset;
	T centerY, centerBase, zoomScale, floatStripH;
	string filename;
	ostringstream os;
	vector<Ember<T>> embers;
	vector<unsigned char> finalImage, vecRgb;
	EmberStats stats;
	EmberReport emberReport;
	EmberImageComments comments;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> randVec;
	auto_ptr<RenderProgress<T>> progress(new RenderProgress<T>());
	auto_ptr<Renderer<T, bucketT>> renderer(CreateRenderer<T, bucketT>(opt.EmberCL() ? OPENCL_RENDERER : CPU_RENDERER, opt.Platform(), opt.Device(), false, 0, emberReport));
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
	renderer->LockAccum(opt.LockAccum());
	renderer->InsertPalette(opt.InsertPalette());
	renderer->SubBatchSize(opt.SubBatchSize());
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
			cout << endl;

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

		renderer->SetEmber(embers[i]);
		renderer->PrepFinalAccumVector(finalImage);//Must manually call this first because it could be erroneously made smaller due to strips if called inside Renderer::Run().

		if (opt.Strips() > 1)
		{
			strips = opt.Strips();
		}
		else
		{
			strips = CalcStrips((double)renderer->MemoryRequired(false), (double)renderer->MemoryAvailable(), opt.UseMem());

			if (strips > 1)
				VerbosePrint("Setting strips to " << strips << " with specified memory usage of " << opt.UseMem());
		}

		if (strips > embers[i].m_FinalRasH)
		{
			cout << "Cannot have more strips than rows: " << opt.Strips() << " > " << embers[i].m_FinalRasH << ". Setting strips = rows." << endl;
			opt.Strips(strips = embers[i].m_FinalRasH);
		}

		if (embers[i].m_FinalRasH % strips != 0)
		{
			cout << "A strips value of " << strips << " does not divide evenly into a height of " << embers[i].m_FinalRasH;
			
			strips = NextHighestEvenDiv(embers[i].m_FinalRasH, strips);

			if (strips == 1)//No higher divisor, check for a lower one.
				strips = NextLowestEvenDiv(embers[i].m_FinalRasH, strips);

			cout << ". Setting strips to " << strips << "." << endl;
		}

		embers[i].m_Quality *= strips;
		realHeight = embers[i].m_FinalRasH;
		floatStripH = T(embers[i].m_FinalRasH) / T(strips);
		embers[i].m_FinalRasH = (unsigned int)ceil(floatStripH);
		centerY = embers[i].m_CenterY;
		zoomScale = pow(T(2), embers[i].m_Zoom);
		centerBase = centerY - ((strips - 1) * floatStripH) / (2 * embers[i].m_PixelsPerUnit * zoomScale);
		
		if (strips > 1)
			randVec = renderer->RandVec();
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

		for (strip = 0; strip < strips; strip++)
		{
			stripOffset = (size_t)embers[i].m_FinalRasH * strip * renderer->FinalRowSize();
			embers[i].m_CenterY = centerBase + embers[i].m_FinalRasH * T(strip) / (embers[i].m_PixelsPerUnit * zoomScale);

			if ((embers[i].m_FinalRasH * (strip + 1)) > realHeight)
			{
				origHeight = embers[i].m_FinalRasH;
				embers[i].m_FinalRasH = realHeight - origHeight * strip;
				embers[i].m_CenterY -= (origHeight - embers[i].m_FinalRasH) * T(0.5) / (embers[i].m_PixelsPerUnit * zoomScale);
			}

			if (strips > 1)
			{
				renderer->RandVec(randVec);//Use the same vector of ISAAC rands for each strip.
				renderer->SetEmber(embers[i]);//Set one final time after modifications for strips.

				if (opt.Verbose() && (strips > 1) && strip > 0)
					cout << endl;

				VerbosePrint("Strip = " << (strip + 1) << "/" << strips);
			}

			if ((renderer->Run(finalImage, 0, 0, false, stripOffset) != RENDER_OK) || renderer->Aborted() || finalImage.empty())
			{
				cout << "Error: image rendering failed, skipping to next image." << endl;
				renderer->DumpErrorReport();//Something went wrong, print errors.
				break;//Exit strips loop, resume next iter in embers loop.
			}

			progress->Clear();

			//Original wrote every strip as a full image which could be very slow with many large images.
			//Only write once all strips for this image are finished.
			if (strip == strips - 1)
			{
				if (!opt.Out().empty())
				{
					filename = opt.Out();
				}
				else if (opt.NameEnable() && !embers[i].m_Name.empty())
				{
					filename = opt.Prefix() + embers[i].m_Name + opt.Suffix() + "." + opt.Format();
				}
				else
				{
					ostringstream ssLocal;

					ssLocal << opt.Prefix() << setfill('0') << setw(5) << i << opt.Suffix() << "." << opt.Format();
					filename = ssLocal.str();
				}

				writeSuccess = false;
				comments = renderer->ImageComments(opt.PrintEditDepth(), opt.IntPalette(), opt.HexPalette());
				stats = renderer->Stats();
				os.str("");
				os << comments.m_NumIters << "/" << renderer->TotalIterCount() << " (" << std::fixed << std::setprecision(2) << ((double)stats.m_Iters/(double)renderer->TotalIterCount() * 100) << "%)";

				VerbosePrint("\nIters ran/requested: " + os.str());
				VerbosePrint("Bad values: " << stats.m_Badvals);
				VerbosePrint("Render time: " + t.Format(stats.m_RenderSeconds * 1000));
				VerbosePrint("Writing " + filename);

				if ((opt.Format() == "jpg" || opt.Format() == "bmp") && renderer->NumChannels() == 4)
				{
					EmberNs::RgbaToRgb(finalImage, vecRgb, renderer->FinalRasW(), realHeight);

					finalImagep = vecRgb.data();
				}
				else
				{
					finalImagep = finalImage.data();
				}

				if (opt.Format() == "png")
					writeSuccess = WritePng(filename.c_str(), finalImagep, renderer->FinalRasW(), realHeight, opt.BitsPerChannel() / 8, opt.PngComments(), comments, opt.Id(), opt.Url(), opt.Nick());
				else if (opt.Format() == "jpg")
					writeSuccess = WriteJpeg(filename.c_str(), finalImagep, renderer->FinalRasW(), realHeight, opt.JpegQuality(), opt.JpegComments(), comments, opt.Id(), opt.Url(), opt.Nick());
				else if (opt.Format() == "ppm")
					writeSuccess = WritePpm(filename.c_str(), finalImagep, renderer->FinalRasW(), realHeight);
				else if (opt.Format() == "bmp")
					writeSuccess = WriteBmp(filename.c_str(), finalImagep, renderer->FinalRasW(), realHeight);

				if (!writeSuccess)
					cout << "Error writing " << filename << endl;
			}
		}
		
		//Restore the ember values to their original values.
		if (strips > 1)
		{
			embers[i].m_Quality /= strips;
			embers[i].m_FinalRasH = realHeight;
			embers[i].m_CenterY = centerY;
			memset(finalImage.data(), 0, finalImage.size());
		}

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