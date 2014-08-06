#include "FractoriumPch.h"
#include "FractoriumEmberController.h"
#include "FinalRenderEmberController.h"
#include "FinalRenderDialog.h"
#include "Fractorium.h"

/// <summary>
/// Constructor which accepts a pointer to the final render dialog.
/// It passes a pointer to the main window to the base and initializes members.
/// </summary>
/// <param name="finalRender">Pointer to the final render dialog</param>
FinalRenderEmberControllerBase::FinalRenderEmberControllerBase(FractoriumFinalRenderDialog* finalRender)
	: FractoriumEmberControllerBase(finalRender->m_Fractorium)
{
	m_Run = false;
	m_PreviewRun = false;
	m_ImageCount = 0;
	m_FinishedImageCount = 0;
	m_FinalRender = finalRender;
	m_Settings = m_Fractorium->m_Settings;
}

/// <summary>
/// Cancel the render by calling Abort().
/// This will block until the cancelling is actually finished.
/// It should never take longer than a few milliseconds because the
/// renderer checks the m_Abort flag in many places during the process.
/// </summary>
void FinalRenderEmberControllerBase::CancelRender()
{
	if (m_Result.isRunning())
	{
		tbb::task_group g;

		g.run([&]
		{
			m_Run = false;

			if (m_Renderer.get())
			{
				m_Renderer->Abort();

				while (m_Renderer->InRender())
					QApplication::processEvents();

				m_Renderer->EnterRender();
				m_Renderer->EnterFinalAccum();
				m_Renderer->LeaveFinalAccum();
				m_Renderer->LeaveRender();
			}
		});

		g.wait();

		while (m_Result.isRunning())
			QApplication::processEvents();

		m_FinalRender->ui.FinalRenderTextOutput->append("Render canceled.");
	}
}

/// <summary>
/// Create a new renderer based on the options selected on the GUI.
/// If a renderer matching the options has already been created, no action is taken.
/// </summary>
/// <returns>True if a valid renderer is created or if no action is taken, else false.</returns>
bool FinalRenderEmberControllerBase::CreateRendererFromGUI()
{
	bool useOpenCL = m_Wrapper.CheckOpenCL() && m_FinalRender->OpenCL();

	return CreateRenderer(useOpenCL ? OPENCL_RENDERER : CPU_RENDERER,
						  m_FinalRender->PlatformIndex(),
						  m_FinalRender->DeviceIndex(),
						  false);//Not shared.
}

/// <summary>
/// Constructor which accepts a pointer to the final render dialog and passes it to the base.
/// The main final rendering lambda function is constructed here.
/// </summary>
/// <param name="finalRender">Pointer to the final render dialog</param>
template<typename T>
FinalRenderEmberController<T>::FinalRenderEmberController(FractoriumFinalRenderDialog* finalRender)
	: FinalRenderEmberControllerBase(finalRender)
{
	m_FinalPreviewRenderer = auto_ptr<EmberNs::Renderer<T, T>>(new EmberNs::Renderer<T, T>());
	m_FinalPreviewRenderer->Callback(NULL);
	m_FinalPreviewRenderer->NumChannels(4);
	m_FinalPreviewRenderer->ReclaimOnResize(true);

	m_FinalPreviewRenderFunc = [&]()
	{
		m_PreviewCs.Enter();//Thread prep.
		m_PreviewRun = true;
		m_FinalPreviewRenderer->Abort();

		QLabel* widget = m_FinalRender->ui.FinalRenderPreviewLabel;
		unsigned int maxDim = 100u;
		T scalePercentage;

		//Determine how to scale the scaled ember to fit in the label with a max of 100x100.
		if (m_Ember.m_FinalRasW >= m_Ember.m_FinalRasH)
			scalePercentage = T(maxDim) / m_Ember.m_FinalRasW;
		else
			scalePercentage = T(maxDim) / m_Ember.m_FinalRasH;

		m_PreviewEmber = m_Ember;		
		m_PreviewEmber.m_Quality = 100;
		m_PreviewEmber.m_TemporalSamples = 1;
		m_PreviewEmber.m_FinalRasW = min(maxDim, unsigned int(scalePercentage * m_Ember.m_FinalRasW));
		m_PreviewEmber.m_FinalRasH = min(maxDim, unsigned int(scalePercentage * m_Ember.m_FinalRasH));
		m_PreviewEmber.m_PixelsPerUnit = scalePercentage * m_Ember.m_PixelsPerUnit;

		while (!m_FinalPreviewRenderer->Aborted() || m_FinalPreviewRenderer->InRender())
			QApplication::processEvents();

		m_FinalPreviewRenderer->EarlyClip(m_FinalRender->EarlyClip());
		m_FinalPreviewRenderer->YAxisUp(m_FinalRender->YAxisUp());
		m_FinalPreviewRenderer->Transparency(m_FinalRender->Transparency());
		m_FinalPreviewRenderer->SetEmber(m_PreviewEmber);

		if (m_FinalPreviewRenderer->Run(m_PreviewFinalImage) == RENDER_OK)
		{
			QImage image(m_PreviewEmber.m_FinalRasW, m_PreviewEmber.m_FinalRasH, QImage::Format_RGBA8888);//The label wants RGBA.
			memcpy(image.scanLine(0), m_PreviewFinalImage.data(), m_PreviewFinalImage.size() * sizeof(m_PreviewFinalImage[0]));//Memcpy the data in.
			QPixmap pixmap = QPixmap::fromImage(image);
			QMetaObject::invokeMethod(widget, "setPixmap", Qt::QueuedConnection, Q_ARG(QPixmap, pixmap));
		}

		m_PreviewRun = false;
		m_PreviewCs.Leave();
	};

	//The main rendering function which will be called in a Qt thread.
	//A backup Xml is made before the rendering process starts just in case it crashes before finishing.
	//If it finishes successfully, delete the backup file.
	m_FinalRenderFunc = [&]()
	{
		size_t i;

		m_Run = true;
		m_TotalTimer.Tic();//Begin timing for progress.
		m_GuiState = m_FinalRender->State();//Cache render settings from the GUI before running.
		m_FinishedImageCount = 0;

		QFileInfo original(m_GuiState.m_Path);
		QString backup = original.absolutePath() + QDir::separator() + m_GuiState.m_Prefix + original.completeBaseName() + m_GuiState.m_Suffix + "_backup.flame";

		QMetaObject::invokeMethod(m_Fractorium, "OnActionSaveCurrentToOpenedFile", Qt::QueuedConnection, Q_ARG(bool, true));//First, save the current ember back to its opened file.
		m_Fractorium->m_Controller->CopyEmber(m_Ember);
		m_Fractorium->m_Controller->CopyEmberFile(m_EmberFile);//Copy the whole file, will take about 0.2ms per ember in the file.

		//Save backup Xml.
		if (m_GuiState.m_DoAll && m_EmberFile.m_Embers.size() > 1)
			m_XmlWriter.Save(backup.toStdString().c_str(), m_EmberFile.m_Embers, 0, true, false, true);
		else
			m_XmlWriter.Save(backup.toStdString().c_str(), m_Ember, 0, true, false, true);

		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "setText", Qt::QueuedConnection, Q_ARG(QString, "Begin rendering..."));
		m_Renderer->EarlyClip(m_GuiState.m_EarlyClip);
		m_Renderer->YAxisUp(m_GuiState.m_YAxisUp);
		m_Renderer->ThreadCount(m_GuiState.m_ThreadCount);
		m_Renderer->Transparency(m_GuiState.m_Transparency);

		if (m_GuiState.m_Path.endsWith(".png", Qt::CaseInsensitive) || m_Renderer->RendererType() == OPENCL_RENDERER)
			m_Renderer->NumChannels(4);
		else
			m_Renderer->NumChannels(3);

		//The rendering process is different between doing a single image, and doing multiple.
		if (m_GuiState.m_DoAll && m_EmberFile.m_Embers.size() > 1)
		{
			m_ImageCount = m_EmberFile.m_Embers.size();
			ResetProgress();

			//Different action required for rendering as animation or not.
			if (m_GuiState.m_DoSequence)
			{
				//Need to loop through and set all w, h, q, ts, ss and t vals.
				for (i = 0; i < m_EmberFile.m_Embers.size() && m_Run; i++)
				{
					Sync(m_EmberFile.m_Embers[i]);
					
					if (i > 0)
					{
						if (m_EmberFile.m_Embers[i].m_Time <= m_EmberFile.m_Embers[i - 1].m_Time)
							m_EmberFile.m_Embers[i].m_Time  = m_EmberFile.m_Embers[i - 1].m_Time + 1;
					}
					else if (i == 0)
					{
						m_EmberFile.m_Embers[i].m_Time = 0;
					}

					m_EmberFile.m_Embers[i].m_TemporalSamples = m_GuiState.m_TemporalSamples;
				}

				m_Renderer->SetEmber(m_EmberFile.m_Embers);//Copy all embers to the local storage inside the renderer.

				//Render each image, cancelling if m_Run ever gets set to false.
				for (i = 0; i < m_EmberFile.m_Embers.size() && m_Run; i++)
				{
					m_Renderer->Reset();//Have to manually set this since the ember is not set each time through.
					m_RenderTimer.Tic();//Toc() is called in the progress function.

					if (m_Renderer->Run(m_FinalImage, i) != RENDER_OK)
					{
						QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "append", Qt::QueuedConnection, Q_ARG(QString, "Renderering failed.\n"));
						m_Fractorium->ErrorReportToQTextEdit(m_Renderer->ErrorReport(), m_FinalRender->ui.FinalRenderTextOutput, false);
					}
				}
			}
			else//Render all images, but not as an animation sequence (without temporal samples motion blur).
			{
				//Copy widget values to all embers
				for (i = 0; i < m_EmberFile.m_Embers.size() && m_Run; i++)
				{
					Sync(m_EmberFile.m_Embers[i]);
					m_EmberFile.m_Embers[i].m_TemporalSamples = 1;//No temporal sampling.
				}

				//Render each image, cancelling if m_Run ever gets set to false.
				for (i = 0; i < m_EmberFile.m_Embers.size() && m_Run; i++)
				{
					m_Renderer->SetEmber(m_EmberFile.m_Embers[i]);
					m_RenderTimer.Tic();//Toc() is called in the progress function.

					if (m_Renderer->Run(m_FinalImage) != RENDER_OK)
					{
						QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "append", Qt::QueuedConnection, Q_ARG(QString, "Renderering failed.\n"));
						m_Fractorium->ErrorReportToQTextEdit(m_Renderer->ErrorReport(), m_FinalRender->ui.FinalRenderTextOutput, false);
					}
				}
			}
		}
		else//Render a single image.
		{
			m_ImageCount = 1;
			Sync(m_Ember);
			ResetProgress();
			m_Ember.m_TemporalSamples = 1;
			m_Renderer->SetEmber(m_Ember);
			memset(m_FinalImage.data(), 0, m_FinalImage.size() * sizeof(m_FinalImage[0]));
			m_RenderTimer.Tic();//Toc() is called in the progress function.

			if (m_Renderer->Run(m_FinalImage) != RENDER_OK)
			{
				QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "append", Qt::QueuedConnection, Q_ARG(QString, "Renderering failed.\n"));
				m_Fractorium->ErrorReportToQTextEdit(m_Renderer->ErrorReport(), m_FinalRender->ui.FinalRenderTextOutput, false);
			}
		}

		QFile::remove(backup);
		m_Run = false;
	};
}

/// <summary>
/// Setters for embers and ember files which convert between float and double types.
/// These are used to preserve the current ember/file when switching between renderers.
/// Note that some precision will be lost when going from double to float.
/// </summary>
template <typename T> void FinalRenderEmberController<T>::SetEmber(const Ember<float>& ember, bool verbatim) { m_Ember = ember; }
template <typename T> void FinalRenderEmberController<T>::CopyEmber(Ember<float>& ember) { ember = m_Ember; }
template <typename T> void FinalRenderEmberController<T>::SetEmberFile(const EmberFile<float>& emberFile) { m_EmberFile = emberFile; }
template <typename T> void FinalRenderEmberController<T>::CopyEmberFile(EmberFile<float>& emberFile) { emberFile = m_EmberFile; }
template <typename T> void FinalRenderEmberController<T>::SetOriginalEmber(Ember<float>& ember) { m_OriginalEmber = ember; }
template <typename T> double FinalRenderEmberController<T>::OriginalAspect() { return double(m_OriginalEmber.m_OrigFinalRasW) / m_OriginalEmber.m_OrigFinalRasH; }
#ifdef DO_DOUBLE
template <typename T> void FinalRenderEmberController<T>::SetEmber(const Ember<double>& ember, bool verbatim) { m_Ember = ember; }
template <typename T> void FinalRenderEmberController<T>::CopyEmber(Ember<double>& ember) { ember = m_Ember; }
template <typename T> void FinalRenderEmberController<T>::SetEmberFile(const EmberFile<double>& emberFile) { m_EmberFile = emberFile; }
template <typename T> void FinalRenderEmberController<T>::CopyEmberFile(EmberFile<double>& emberFile) { emberFile = m_EmberFile; }
template <typename T> void FinalRenderEmberController<T>::SetOriginalEmber(Ember<double>& ember) { m_OriginalEmber = ember; }
#endif

/// <summary>
/// Progress function.
/// Take special action to sync options upon finishing.
/// </summary>
/// <param name="ember">The ember currently being rendered</param>
/// <param name="foo">An extra dummy parameter</param>
/// <param name="fraction">The progress fraction from 0-100</param>
/// <param name="stage">The stage of iteration. 1 is iterating, 2 is density filtering, 2 is final accumulation.</param>
/// <param name="etaMs">The estimated milliseconds to completion of the current stage</param>
/// <returns>0 if the user has clicked cancel, else 1 to continue rendering.</returns>
template <typename T>
int FinalRenderEmberController<T>::ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs)
{
	static int count = 0;
	int intFract = (int)fraction;

	if (stage == 0)
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderIterationProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, intFract));
	else if (stage == 1)
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderFilteringProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, intFract));
	else if (stage == 2)
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderAccumProgress,     "setValue", Qt::QueuedConnection, Q_ARG(int, intFract));

	//Finished, so take special action.
	if (stage == 2 && intFract == 100)
	{
		string renderTimeString = m_RenderTimer.Format(m_RenderTimer.Toc()), totalTimeString;
		QString status, filename = m_GuiState.m_Path;
		QFileInfo original(filename);
		EmberStats stats = m_Renderer->Stats();
		QString iters = QLocale(QLocale::English).toString(stats.m_Iters);
		QString itersPerSec = QLocale(QLocale::English).toString(unsigned __int64(stats.m_Iters / (stats.m_IterMs / 1000.0)));

		if (m_GuiState.m_DoAll && m_EmberFile.m_Embers.size() > 1)
			filename = original.absolutePath() + QDir::separator() + m_GuiState.m_Prefix + QString::fromStdString(m_EmberFile.m_Embers[m_FinishedImageCount].m_Name) + m_GuiState.m_Suffix + "." + m_GuiState.m_DoAllExt;
		else
			filename = original.absolutePath() + QDir::separator() + m_GuiState.m_Prefix + original.completeBaseName() + m_GuiState.m_Suffix + "." + original.suffix();

		filename = EmberFile<T>::UniqueFilename(filename);

		//Save whatever options were specified on the GUI to the settings.
		m_Settings->FinalEarlyClip(m_GuiState.m_EarlyClip);
		m_Settings->FinalYAxisUp(m_GuiState.m_YAxisUp);
		m_Settings->FinalTransparency(m_GuiState.m_Transparency);
		m_Settings->FinalOpenCL(m_GuiState.m_OpenCL);
		m_Settings->FinalDouble(m_GuiState.m_Double);
		m_Settings->FinalPlatformIndex(m_GuiState.m_PlatformIndex);
		m_Settings->FinalDeviceIndex(m_GuiState.m_DeviceIndex);
		m_Settings->FinalSaveXml(m_GuiState.m_SaveXml);
		m_Settings->FinalDoAll(m_GuiState.m_DoAll);
		m_Settings->FinalDoSequence(m_GuiState.m_DoSequence);
		m_Settings->FinalKeepAspect(m_GuiState.m_KeepAspect);
		m_Settings->FinalScale(m_GuiState.m_Scale);
		m_Settings->FinalDoAllExt(m_GuiState.m_DoAllExt);
		m_Settings->FinalThreadCount(m_GuiState.m_ThreadCount);
		m_Settings->FinalWidth(m_GuiState.m_Width);
		m_Settings->FinalHeight(m_GuiState.m_Height);
		m_Settings->FinalQuality(m_GuiState.m_Quality);
		m_Settings->FinalTemporalSamples(m_GuiState.m_TemporalSamples);
		m_Settings->FinalSupersample(m_GuiState.m_Supersample);
		SaveCurrentRender(filename);
		
		if (m_GuiState.m_SaveXml)
		{
			QFileInfo xmlFileInfo(filename);//Create another one in case it was modified for batch rendering.
			QString newPath = xmlFileInfo.absolutePath() + QDir::separator() + xmlFileInfo.completeBaseName() + ".flame";
			xmlDocPtr tempEdit = ember.m_Edits;

			ember.m_Edits = m_XmlWriter.CreateNewEditdoc(&ember, NULL, "edit", m_Settings->Nick().toStdString(), m_Settings->Url().toStdString(), m_Settings->Id().toStdString(), "", 0, 0);
			m_XmlWriter.Save(newPath.toStdString().c_str(), ember, 0, true, false, true);//Note that the ember passed is used, rather than m_Ember because it's what was actually rendered.

			if (tempEdit != NULL)
				xmlFreeDoc(tempEdit);
		}

		m_FinishedImageCount++;
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderIterationProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, 100));//Just to be safe.
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderFilteringProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, 100));
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderAccumProgress,     "setValue", Qt::QueuedConnection, Q_ARG(int, 100));
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTotalProgress,   "setValue", Qt::QueuedConnection, Q_ARG(int, int(((float)m_FinishedImageCount / (float)m_ImageCount) * 100)));
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderImageCountLabel, "setText",  Qt::QueuedConnection, Q_ARG(QString, QString::number(m_FinishedImageCount) + " / " + QString::number(m_ImageCount)));

		status = "Image " + QString::number(m_FinishedImageCount) + ":\nPure render time: " + QString::fromStdString(renderTimeString);
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "append", Qt::QueuedConnection, Q_ARG(QString, status));

		totalTimeString = m_TotalTimer.Format(m_TotalTimer.Toc());
		status = "Total render time: " + QString::fromStdString(totalTimeString) + "\nTotal iters: " + iters + "\nIters/second: " + itersPerSec + "\n";
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "append", Qt::QueuedConnection, Q_ARG(QString, status));
		QMetaObject::invokeMethod(m_FinalRender, "MoveCursorToEnd", Qt::QueuedConnection);

		if (m_FinishedImageCount != m_ImageCount)
		{
			ResetProgress(false);
		}
	}

	QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTextOutput, "update", Qt::QueuedConnection);
	QApplication::processEvents();
	return m_Run ? 1 : 0;
}

/// <summary>
/// Start the final rendering process.
/// Create the needed renderer from the GUI if it has not been created yet.
/// </summary>
/// <returns></returns>
template<typename T>
bool FinalRenderEmberController<T>::Render()
{
	QString filename = m_FinalRender->Path();

	if (filename == "")
	{
		QMessageBox::critical(m_FinalRender, "File Error", "Please enter a valid path and filename for the output.");
		return false;
	}

	if (CreateRendererFromGUI())
	{
		m_FinalRender->ui.FinalRenderTextOutput->clear();

		//Note that a Qt thread must be used, rather than a tbb task.
		//This is because tbb does a very poor job of allocating thread resources
		//and dedicates an entire core just to this thread which does nothing waiting for the
		//parallel iteration loops inside of the CPU renderer to finish. The result is that
		//the renderer ends up using ThreadCount - 1 to iterate, instead of ThreadCount.
		//By using a Qt thread here, and tbb inside the renderer, all cores can be maxed out.
		m_Result = QtConcurrent::run(m_FinalRenderFunc);
		m_Settings->sync();
		return true;
	}
	else
		return false;
}

/// <summary>
/// Stop rendering and initialize a new renderer, using the specified type and the options on the final render dialog.
/// </summary>
/// <param name="renderType">The type of render to create</param>
/// <param name="platform">The index platform of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="outputTexID">The texture ID of the shared OpenGL texture if shared</param>
/// <param name="shared">True if shared with OpenGL, else false. Default: true.</param>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T>
bool FinalRenderEmberController<T>::CreateRenderer(eRendererType renderType, unsigned int platform, unsigned int device, bool shared)
{
	bool ok = true;
	vector<string> errorReport;
	QString filename = m_FinalRender->Path();
	unsigned int channels = filename.endsWith(".png", Qt::CaseInsensitive) ? 4 : 3;

	CancelRender();

	if (!m_Renderer.get() ||
		!m_Renderer->Ok() ||
		m_Renderer->RendererType() != renderType ||
		m_Platform != platform ||
		m_Device != device ||
		m_Shared != shared)
	{
		EmberReport emberReport;
		vector<string> errorReport;

		m_Platform = platform;//Store values for re-creation later on.
		m_Device = device;
		m_OutputTexID = 0;//Don't care about tex ID when doing final render.
		m_Shared = shared;
		
		m_Renderer = auto_ptr<EmberNs::RendererBase>(::CreateRenderer<T, T>(renderType, platform, device, shared, m_OutputTexID, emberReport));
		errorReport = emberReport.ErrorReport();

		if (!errorReport.empty())
		{
			ok = false;
			QMessageBox::critical(m_Fractorium, "Renderer Creation Error", "Could not create requested renderer, fallback CPU renderer created. See info tab for details.");
			m_Fractorium->ErrorReportToQTextEdit(errorReport, m_Fractorium->ui.InfoRenderingTextEdit);
		}
	}

	if (m_Renderer.get())
	{
		if (m_Renderer->RendererType() == OPENCL_RENDERER)
			channels = 4;//Always using 4 since the GL texture is RGBA.

		m_Renderer->Callback(this);
		m_Renderer->NumChannels(channels);
		m_Renderer->ReclaimOnResize(true);
		m_Renderer->EarlyClip(m_FinalRender->EarlyClip());
		m_Renderer->YAxisUp(m_FinalRender->YAxisUp());
		m_Renderer->ThreadCount(m_FinalRender->ThreadCount());
		m_Renderer->Transparency(m_FinalRender->Transparency());
	}
	else
	{
		ok = false;
		QMessageBox::critical(m_FinalRender, "Renderer Creation Error", "Could not create renderer, aborting. See info tab for details.");
	}

	return ok;
}

/// <summary>
/// Set various parameters in the renderer and current ember with the values
/// specified in the widgets and compute the amount of memory required to render.
/// This includes the memory needed for the final output image.
/// </summary>
/// <returns>If successful, memory required in bytes, else zero.</returns>
template <typename T>
unsigned __int64 FinalRenderEmberController<T>::SyncAndComputeMemory()
{
	if (m_Renderer.get())
	{
		bool b = false;
		QString filename = m_FinalRender->Path();
		unsigned int channels = filename.endsWith(".png", Qt::CaseInsensitive) ? 4 : 3;//4 channels for Png, else 3.

		Sync(m_Ember);
		m_Renderer->SetEmber(m_Ember);
		m_Renderer->CreateSpatialFilter(b);
		m_Renderer->CreateTemporalFilter(b);
		m_Renderer->NumChannels(channels);
		m_Renderer->ComputeBounds();
		CancelPreviewRender();
		//m_FinalPreviewResult = QtConcurrent::run(m_PreviewRenderFunc);
		//while (!m_FinalPreviewResult.isRunning()) { QApplication::processEvents(); }//Wait for it to start up.
		m_FinalPreviewRenderFunc();
		return m_Renderer->MemoryRequired(true);
	}

	return 0;
}

/// <summary>
/// Reset the progress bars.
/// </summary>
/// <param name="total">True to reset render image and total progress bars, else false to only do iter, filter and accum bars.</param>
template <typename T>
void FinalRenderEmberController<T>::ResetProgress(bool total)
{
	if (total)
	{
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderImageCountLabel, "setText",  Qt::QueuedConnection, Q_ARG(QString, "0 / " + QString::number(m_ImageCount)));
		QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderTotalProgress,   "setValue", Qt::QueuedConnection, Q_ARG(int, 0));
	}

	QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderIterationProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, 0));
	QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderFilteringProgress, "setValue", Qt::QueuedConnection, Q_ARG(int, 0));
	QMetaObject::invokeMethod(m_FinalRender->ui.FinalRenderAccumProgress,     "setValue", Qt::QueuedConnection, Q_ARG(int, 0));
}

template <typename T>
void FinalRenderEmberController<T>::CancelPreviewRender()
{
	m_FinalPreviewRenderer->Abort();

	while (m_FinalPreviewRenderer->InRender()) { QApplication::processEvents(); }
	while (m_PreviewRun) { QApplication::processEvents(); }
	while (m_FinalPreviewResult.isRunning()) { QApplication::processEvents(); }
}

/// <summary>
/// Copy widget values to the ember passed in.
/// </summary>
/// <param name="ember">The ember whose values will be modified</param>
template <typename T>
void FinalRenderEmberController<T>::Sync(Ember<T>& ember)
{
	int w = m_FinalRender->m_WidthSpin->value();
	int h = m_FinalRender->m_HeightSpin->value();

	ember.m_FinalRasW = m_OriginalEmber.m_OrigFinalRasW;//Scale is always in terms of the original dimensions of the ember in the editor.
	ember.m_FinalRasH = m_OriginalEmber.m_OrigFinalRasH;
	ember.m_PixelsPerUnit = m_OriginalEmber.m_PixelsPerUnit;
	ember.SetSizeAndAdjustScale(w, h, false, m_FinalRender->Scale());
	ember.m_Quality = m_FinalRender->m_QualitySpin->value();
	ember.m_Supersample = m_FinalRender->m_SupersampleSpin->value();

	if (m_FinalRender->ui.FinalRenderDoSequenceCheckBox->isChecked())
		ember.m_TemporalSamples = m_FinalRender->m_TemporalSamplesSpin->value();
}
