#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Return whether the render timer is running.
/// </summary>
/// <returns>True if running, else false.</returns>
bool FractoriumEmberControllerBase::RenderTimerRunning()
{
	return m_RenderTimer && m_RenderTimer->isActive();
}

/// <summary>
/// Start the render timer.
/// If a renderer has not been created yet, it will be created from the options.
/// </summary>
void FractoriumEmberControllerBase::StartRenderTimer()
{
	if (m_RenderTimer)
	{
		UpdateRender();
		m_RenderTimer->start();
		m_RenderElapsedTimer.Tic();
	}
}

/// <summary>
/// Start the render timer after a short delay.
/// If the timer is already running, stop it first.
/// This is useful for stopping and restarting the render
/// process in response to things like a window resize.
/// </summary>
void FractoriumEmberControllerBase::DelayedStartRenderTimer()
{
	DeleteRenderer();

	if (m_RenderRestartTimer)
	{
		m_RenderRestartTimer->setSingleShot(true);
		m_RenderRestartTimer->start(300);//Will stop the timer if it's already running, and start again.
	}
}

/// <summary>
/// Stop the render timer and abort the rendering process.
/// Optionally block until stopping is complete.
/// </summary>
/// <param name="wait">True to block, else false.</param>
void FractoriumEmberControllerBase::StopRenderTimer(bool wait)
{
	if (m_RenderTimer)
		m_RenderTimer->stop();

	if (m_Renderer.get())
		m_Renderer->Abort();

	if (wait)
	{
		while (m_Rendering || RenderTimerRunning() || (Renderer() && (!m_Renderer->Aborted() || m_Renderer->InRender())))
			QApplication::processEvents();
	}
}

/// <summary>
/// Stop all timers, rendering and drawing and block until they are done.
/// </summary>
void FractoriumEmberControllerBase::Shutdown()
{
	StopRenderTimer(true);

	while(m_Fractorium->ui.GLDisplay->Drawing())
		QApplication::processEvents();
}

/// <summary>
/// Update the state of the renderer.
/// Upon changing values, some intelligence is used to avoid blindly restarting the
/// entire iteration proceess every time a value changes. This is because some values don't affect the
/// iteration, and only affect filtering and final accumulation. They are broken into three categories:
/// 1) Restart the entire process.
/// 2) Log/density filter, then final accum.
/// 3) Final accum only.
/// 4) Continue iterating.
/// </summary>
/// <param name="action">The action to take</param>
void FractoriumEmberControllerBase::UpdateRender(eProcessAction action)
{
	AddProcessAction(action);
	m_RenderElapsedTimer.Tic();
}

/// <summary>
/// Call Shutdown() then delete the renderer and clear the textures in the output window if there is one.
/// </summary>
void FractoriumEmberControllerBase::DeleteRenderer()
{
	Shutdown();
	m_Renderer.reset();

	if (GLController())
		GLController()->ClearWindow();
}

/// <summary>
/// Save the current render results to a file.
/// This could benefit from QImageWriter, however it's compression capabilities are
/// severely lacking. A Png file comes out larger than a bitmap, so instead use the
/// Png and Jpg wrapper functions from the command line programs.
/// This will embed the id, url and nick fields from the options in the image comments.
/// </summary>
/// <param name="filename">The full path and filename</param>
void FractoriumEmberControllerBase::SaveCurrentRender(const QString& filename, bool forcePull)
{
	if (filename != "")
	{
		bool b = false;
		uint i, j;
		uint width = m_Renderer->FinalRasW();
		uint height = m_Renderer->FinalRasH();
		byte* data = NULL;
		vector<byte> vecRgb;
		QFileInfo fileInfo(filename);
		QString suffix = fileInfo.suffix();
		FractoriumSettings* settings = m_Fractorium->m_Settings;
		RendererCLBase* rendererCL = dynamic_cast<RendererCLBase*>(m_Renderer.get());

		if (forcePull && rendererCL && m_Renderer->PrepFinalAccumVector(m_FinalImage))
		{
			if (!rendererCL->ReadFinal(m_FinalImage.data()))
			{
				m_Fractorium->ShowCritical("GPU Read Error", "Could not read image from the GPU, aborting image save.", true);
				return;
			}
		}

		//Ensure dimensions are valid.
		if (m_FinalImage.size() < (width * height * m_Renderer->NumChannels() * m_Renderer->BytesPerChannel()))
		{
			m_Fractorium->ShowCritical("Save Failed", "Dimensions didn't match, not saving.", true);
			return;
		}

		data = m_FinalImage.data();//Png and channels == 4.
		
		if ((suffix == "jpg" || suffix == "bmp") && m_Renderer->NumChannels() == 4)
		{
			RgbaToRgb(m_FinalImage, vecRgb, width, height);
			
			data = vecRgb.data();
		}
		
		string s = filename.toStdString();
		string id = settings->Id().toStdString();
		string url = settings->Url().toStdString();
		string nick = settings->Nick().toStdString();
		EmberImageComments comments = m_Renderer->ImageComments(m_Stats, 0, false, true);

		if (suffix == "png")
			b = WritePng(s.c_str(), data, width, height, 1, true, comments, id, url, nick);
		else if (suffix == "jpg")
			b = WriteJpeg(s.c_str(), data, width, height, 100, true, comments, id, url, nick);
		else if (suffix == "bmp")
			b = WriteBmp(s.c_str(), data, width, height);
		else
		{
			m_Fractorium->ShowCritical("Save Failed", "Unrecognized format " + suffix + ", not saving.", true);
			return;
		}

		if (b)
			settings->SaveFolder(fileInfo.canonicalPath());
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.", true);
	}
}

/// <summary>
/// Add a process action to the list of actions to take.
/// Called in response to the user changing something on the GUI.
/// </summary>
/// <param name="action">The action for the renderer to take</param>
void FractoriumEmberControllerBase::AddProcessAction(eProcessAction action)
{
	m_Cs.Enter();
	m_ProcessActions.push_back(action);

	if (m_Renderer.get())
		m_Renderer->Abort();

	m_Cs.Leave();
}

/// <summary>
/// Condense and clear the process actions into a single action and return.
/// Many actions may be specified, but only the one requiring the greatest amount
/// of processing matters. Extract and return the greatest and clear the vector.
/// </summary>
/// <returns>The most significant processing action desired</returns>
eProcessAction FractoriumEmberControllerBase::CondenseAndClearProcessActions()
{
	m_Cs.Enter();
	eProcessAction action = NOTHING;

	for (size_t i = 0; i < m_ProcessActions.size(); i++)
		if (m_ProcessActions[i] > action)
			action = m_ProcessActions[i];
	
	m_ProcessActions.clear();
	m_Cs.Leave();
	return action;
}

/// <summary>
/// Render progress callback function to update progress bar.
/// </summary>
/// <param name="ember">The ember currently being rendered</param>
/// <param name="foo">An extra dummy parameter</param>
/// <param name="fraction">The progress fraction from 0-100</param>
/// <param name="stage">The stage of iteration. 1 is iterating, 2 is density filtering, 2 is final accumulation.</param>
/// <param name="etaMs">The estimated milliseconds to completion of the current stage</param>
/// <returns>0 if the user has changed anything on the GUI, else 1 to continue rendering.</returns>
template <typename T>
int FractoriumEmberController<T>::ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs)
{
	QString status;

	m_Fractorium->m_ProgressBar->setValue((int)fraction);//Only really applies to iter and filter, because final accum only gives progress 0 and 100.

	if (stage == 0)
		status = "Iterating";
	else if (stage == 1)
		status = "Density Filtering";
	else if (stage == 2)
		status = "Spatial Filtering + Final Accumulation";

	m_Fractorium->m_RenderStatusLabel->setText(status);
	return m_ProcessActions.empty() ? 1 : 0;//If they've done anything, abort.
}

/// <summary>
/// Clear the undo list as well as the undo/redo index and state.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearUndo()
{
	m_UndoIndex = 0;
	m_UndoList.clear();
	m_EditState = REGULAR_EDIT;
	m_LastEditWasUndoRedo = false;
	m_Fractorium->ui.ActionUndo->setEnabled(false);
	m_Fractorium->ui.ActionRedo->setEnabled(false);
}

/// <summary>
/// The hierarchy/order of sizes is like so:
/// Ember
///		GL Widget
///			Texture (passed to RendererCL)
///				Viewport
/// Since this uses m_GL->SizesMatch(), which uses the renderer's dimensions, this
/// must be called after the renderer has set the current ember.
/// </summary>
/// <returns>True if dimensions had to be resized due to a mismatch, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::SyncSizes()
{
	bool changed = false;
	GLWidget* gl = m_Fractorium->ui.GLDisplay;
	RendererCL<T>* rendererCL;

	if (!m_GLController->SizesMatch())
	{
		m_GLController->ClearWindow();
		gl->SetDimensions(m_Ember.m_FinalRasW, m_Ember.m_FinalRasH);
		gl->Allocate();
		gl->SetViewport();

		if (m_Renderer->RendererType() == OPENCL_RENDERER && (rendererCL = (RendererCL<T>*)m_Renderer.get()))
			rendererCL->SetOutputTexture(gl->OutputTexID());

		changed = true;
	}

	return changed;
}

/// <summary>
/// The main rendering function.
/// Called whenever the event loop is idle.
/// </summary>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::Render()
{
	m_Rendering = true;

	bool success = true;
	GLWidget* gl = m_Fractorium->ui.GLDisplay;
	RendererCL<T>* rendererCL = nullptr;
	eProcessAction action = CondenseAndClearProcessActions();

	if (m_Renderer->RendererType() == OPENCL_RENDERER)
		rendererCL = (RendererCL<T>*)m_Renderer.get();

	//Force temporal samples to always be 1. Perhaps change later when animation is implemented.
	m_Ember.m_TemporalSamples = 1;

	//Take care of solo xforms and set the current ember and action.
	if (action != NOTHING)
	{
		int i, solo = m_Fractorium->ui.CurrentXformCombo->property("soloxform").toInt();

		if (solo != -1)
		{
			m_TempOpacities.resize(m_Ember.TotalXformCount());

			for (i = 0; i < m_Ember.TotalXformCount(); i++)
			{
				m_TempOpacities[i] = m_Ember.GetTotalXform(i)->m_Opacity;
				m_Ember.GetTotalXform(i)->m_Opacity = i == solo ? 1 : 0;
			}
		}

		m_Renderer->SetEmber(m_Ember, action);

		if (solo != -1)
		{
			for (i = 0; i < m_Ember.TotalXformCount(); i++)
			{
				m_Ember.GetTotalXform(i)->m_Opacity = m_TempOpacities[i];
			}
		}
	}

	//Ensure sizes are equal and if not, update dimensions.
	if (SyncSizes())
	{
		action = FULL_RENDER;
		return true;
	}

	//Determining if a completely new rendering process is being started.
	bool iterBegin = ProcessState() == NONE;

	if (iterBegin)
	{
		if (m_Renderer->RendererType() == CPU_RENDERER)
			m_SubBatchCount = m_Fractorium->m_Settings->CpuSubBatch();
		else if (m_Renderer->RendererType() == OPENCL_RENDERER)
			m_SubBatchCount = m_Fractorium->m_Settings->OpenCLSubBatch();

		m_Fractorium->m_ProgressBar->setValue(0);
		m_Fractorium->m_RenderStatusLabel->setText("Starting");
	}

	//If the rendering process hasn't finished, render with the current specified action.
	if (ProcessState() != ACCUM_DONE)
	{
		//if (m_Renderer->Run(m_FinalImage, 0) == RENDER_OK)//Full, non-incremental render for debugging.
		if (m_Renderer->Run(m_FinalImage, 0, m_SubBatchCount, iterBegin) == RENDER_OK)//Force output on iterBegin.
		{
			//The amount to increment sub batch while rendering proceeds is purely empirical.
			//Change later if better values can be derived/observed.
			if (m_Renderer->RendererType() == OPENCL_RENDERER)
			{
				if (m_SubBatchCount < 3)//More than 3 with OpenCL gives a sluggish UI.
					m_SubBatchCount++;
			}
			else
			{
				if (m_SubBatchCount < 5)
					m_SubBatchCount++;
				else if (m_SubBatchCount < 105)//More than 105 with CPU gives a sluggish UI.
					m_SubBatchCount += 25;
			}

			//Rendering has finished, update final stats.
			if (ProcessState() == ACCUM_DONE)
			{
				EmberStats stats = m_Renderer->Stats();
				QString iters = ToString(stats.m_Iters);
				QString scaledQuality = ToString((uint)m_Renderer->ScaledQuality());
				string renderTime = m_RenderElapsedTimer.Format(m_RenderElapsedTimer.Toc());

				m_Fractorium->m_ProgressBar->setValue(100);

				//Only certain status can be reported with OpenCL.
				if (m_Renderer->RendererType() == OPENCL_RENDERER)
				{
					m_Fractorium->m_RenderStatusLabel->setText("Iters: " + iters + ". Scaled quality: " + scaledQuality + ". Total time: " + QString::fromStdString(renderTime));
				}
				else
				{
					double percent = (double)stats.m_Badvals / (double)stats.m_Iters;
					QString badVals = ToString(stats.m_Badvals);
					QString badPercent = QLocale::system().toString(percent * 100, 'f', 2);

					m_Fractorium->m_RenderStatusLabel->setText("Iters: " + iters + ". Scaled quality: " + scaledQuality + ". Bad values: " + badVals + " (" + badPercent + "%). Total time: " + QString::fromStdString(renderTime));
				}
				
				if (m_LastEditWasUndoRedo && (m_UndoIndex == m_UndoList.size() - 1))//Traversing through undo list, reached the end, so put back in regular edit mode.
				{
					m_EditState = REGULAR_EDIT;
				}
				else if (m_EditState == REGULAR_EDIT)//Regular edit, just add to the end of the undo list.
				{
					m_UndoList.push_back(m_Ember);
					m_UndoIndex = m_UndoList.size() - 1;
					m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1);
					m_Fractorium->ui.ActionRedo->setEnabled(false);

					if (m_UndoList.size() >= UNDO_SIZE)
						m_UndoList.pop_front();
				}
				else if (!m_LastEditWasUndoRedo && m_UndoIndex < m_UndoList.size() - 1)//They were anywhere but the end of the undo list, then did a manual edit, so clear the undo list.
				{
					Ember<T> ember(m_UndoList[m_UndoIndex]);

					ClearUndo();
					m_UndoList.push_back(ember);
					m_UndoList.push_back(m_Ember);
					m_UndoIndex = m_UndoList.size() - 1;
					m_Fractorium->ui.ActionUndo->setEnabled(true);
					m_Fractorium->ui.ActionRedo->setEnabled(false);
				}

				m_LastEditWasUndoRedo = false;
				m_Fractorium->UpdateHistogramBounds();//Mostly of engineering interest.
			}
			
			//Update the GL window on start because the output will be forced.
			//Update it on finish because the rendering process is completely done.
			if (iterBegin || ProcessState() == ACCUM_DONE)
			{
				if (m_FinalImage.size() == m_Renderer->FinalBufferSize())//Make absolutely sure the correct amount of data is passed.
					gl->repaint();
				
				//Uncomment for debugging kernel build and execution errors.
				//m_Fractorium->ui.InfoRenderingTextEdit->setText(QString::fromStdString(m_Fractorium->m_Wrapper.DumpInfo()));
				//if (rendererCL)
				//	m_Fractorium->ui.InfoRenderingTextEdit->setText(QString::fromStdString(rendererCL->IterKernel()));
			}
		}
		else//Something went very wrong, show error report.
		{
			vector<string> errors = m_Renderer->ErrorReport();
			
			success = false;
			m_FailedRenders++;
			m_Fractorium->m_RenderStatusLabel->setText("Rendering failed, see info tab. Try changing parameters.");
			m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoRenderingTextEdit);
			m_Renderer->ClearErrorReport();
		
			if (m_FailedRenders >= 3)
			{
				m_Rendering = false;
				StopRenderTimer(true);
				m_Fractorium->m_RenderStatusLabel->setText("Rendering failed 3 or more times, stopping all rendering, see info tab. Try changing renderer types.");
				Memset(m_FinalImage);
		
				if (rendererCL)
					rendererCL->ClearFinal();

				m_GLController->ClearWindow();
			}
		}
	}
	
	//Upon finishing, or having nothing to do, rest.
	if (ProcessState() == ACCUM_DONE)
		QThread::msleep(1);

	m_Rendering = false;
	return success;
}

/// <summary>
/// Stop rendering and initialize a new renderer, using the specified type.
/// Rendering will be left in a stopped state. The caller is responsible for restarting the render loop again.
/// </summary>
/// <param name="renderType">The type of render to create</param>
/// <param name="platform">The index platform of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="outputTexID">The texture ID of the shared OpenGL texture if shared</param>
/// <param name="shared">True if shared with OpenGL, else false. Default: true.</param>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::CreateRenderer(eRendererType renderType, uint platform, uint device, bool shared)
{
	bool ok = true;
	FractoriumSettings* s = m_Fractorium->m_Settings;
	GLWidget* gl = m_Fractorium->ui.GLDisplay;

	if (!m_Renderer.get() || (m_Renderer->RendererType() != renderType) || (m_Platform != platform) || (m_Device != device))
	{
		EmberReport emberReport;
		vector<string> errorReport;

		DeleteRenderer();//Delete the renderer and refresh the textures.
		//Before starting, must take care of allocations.
		gl->Allocate(true);//Forcing a realloc of the texture is necessary on AMD, but not on nVidia.
		m_Renderer = unique_ptr<EmberNs::RendererBase>(::CreateRenderer<T, T>(renderType, platform, device, shared, gl->OutputTexID(), emberReport));
		errorReport = emberReport.ErrorReport();

		if (errorReport.empty())
		{
			m_Platform = platform;//Store values for re-creation later on.
			m_Device = device;
			m_OutputTexID = gl->OutputTexID();
			m_Shared = shared;
		}
		else
		{
			ok = false;
			m_Fractorium->ShowCritical("Renderer Creation Error", "Could not create requested renderer, fallback CPU renderer created. See info tab for details.");
			m_Fractorium->ErrorReportToQTextEdit(errorReport, m_Fractorium->ui.InfoRenderingTextEdit);
		}
	}

	if (m_Renderer.get())
	{
		m_RenderType = m_Renderer->RendererType();

		if (m_RenderType == OPENCL_RENDERER)
		{
			m_Fractorium->m_QualitySpin->DoubleClickZero(30);
			m_Fractorium->m_QualitySpin->DoubleClickNonZero(30);

			if (m_Fractorium->m_QualitySpin->value() < 30)
				m_Fractorium->m_QualitySpin->setValue(30);
		}
		else
		{
			m_Fractorium->m_QualitySpin->DoubleClickZero(10);
			m_Fractorium->m_QualitySpin->DoubleClickNonZero(10);

			if (m_Fractorium->m_QualitySpin->value() > 10)
				m_Fractorium->m_QualitySpin->setValue(10);
		}

		m_Renderer->Callback(this);
		m_Renderer->NumChannels(4);//Always using 4 since the GL texture is RGBA.
		m_Renderer->ReclaimOnResize(true);
		m_Renderer->SetEmber(m_Ember);//Give it an initial ember, will be updated many times later.
		m_Renderer->EarlyClip(s->EarlyClip());
		m_Renderer->YAxisUp(s->YAxisUp());
		m_Renderer->ThreadCount(s->ThreadCount());
		m_Renderer->Transparency(s->Transparency());
		
		if (m_Renderer->RendererType() == CPU_RENDERER)
			m_Renderer->InteractiveFilter(s->CpuDEFilter() ? FILTER_DE : FILTER_LOG);
		else
			m_Renderer->InteractiveFilter(s->OpenCLDEFilter() ? FILTER_DE : FILTER_LOG);

		if ((m_Renderer->EarlyClip() != m_PreviewRenderer->EarlyClip()) ||
			(m_Renderer->YAxisUp() != m_PreviewRenderer->YAxisUp()))
		{
			StopPreviewRender();
			m_PreviewRenderer->EarlyClip(m_Renderer->EarlyClip());
			m_PreviewRenderer->YAxisUp(m_Renderer->YAxisUp());
			RenderPreviews();
		}

		m_FailedRenders = 0;
		m_RenderElapsedTimer.Tic();
		//Leave rendering in a stopped state. The caller is responsible for restarting the render loop again.
	}
	else
	{
		ok = false;
		m_Fractorium->ShowCritical("Renderer Creation Error", "Creating a basic CPU renderer failed, something is catastrophically wrong. Exiting program.");
		QApplication::quit();
	}

	return ok;
}

/// <summary>
/// Create a new renderer from the options.
/// </summary>
/// <returns>True if nothing went wrong, else false.</returns>
bool Fractorium::CreateRendererFromOptions()
{
	bool ok = true;
	bool useOpenCL = m_Wrapper.CheckOpenCL() && m_Settings->OpenCL();

	//The most important option to process is what kind of renderer is desired, so do it first.
	if (!m_Controller->CreateRenderer(useOpenCL ? OPENCL_RENDERER : CPU_RENDERER,
						 m_Settings->PlatformIndex(),
						 m_Settings->DeviceIndex()))
	{
		//If using OpenCL, will only get here if creating RendererCL failed, but creating a backup CPU Renderer succeeded.
		ShowCritical("Renderer Creation Error", "Error creating renderer, most likely a GPU problem. Using CPU instead.");
		m_Settings->OpenCL(false);
		m_OptionsDialog->ui.OpenCLCheckBox->setChecked(false);
		m_FinalRenderDialog->ui.FinalRenderOpenCLCheckBox->setChecked(false);
		ok = false;
	}

	return ok;
}

/// <summary>
/// Create a new controller from the options.
/// This does not create the internal renderer or start the timers.
/// </summary>
/// <returns>True if successful, else false.</returns>
bool Fractorium::CreateControllerFromOptions()
{
	bool ok = true;

	size_t size =
#ifdef DO_DOUBLE
		m_Settings->Double() ? sizeof(double) :
#endif
		sizeof(float);
	
	if (!m_Controller.get() || (m_Controller->SizeOfT() != size))
	{
		double hue = m_PaletteHueSpin->value();
		double sat = m_PaletteSaturationSpin->value();
		double bright = m_PaletteBrightnessSpin->value();
		double con = m_PaletteContrastSpin->value();
		double blur = m_PaletteBlurSpin->value();
		double freq = m_PaletteFrequencySpin->value();
#ifdef DO_DOUBLE
		Ember<double> ed;
		EmberFile<double> efd;
		Palette<double> tempPalette;
#else
		Ember<float> ed;
		EmberFile<float> efd;
		Palette<float> tempPalette;
#endif
		QModelIndex index = ui.LibraryTree->currentIndex();

		//First check if a controller has already been created, and if so, save its embers and gracefully shut it down.
		if (m_Controller.get())
		{
			m_Controller->CopyTempPalette(tempPalette);//Convert float to double or save double verbatim;
			m_Controller->CopyEmber(ed);
			m_Controller->CopyEmberFile(efd);
			m_Controller->Shutdown();
		}

#ifdef DO_DOUBLE
		if (m_Settings->Double())
			m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<double>(this));
		else
#endif
			m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<float>(this));

		//Restore the ember and ember file.
		if (m_Controller.get())
		{
			m_Controller->SetEmber(ed);//Convert float to double or set double verbatim;
			m_Controller->SetEmberFile(efd);

			//Template specific palette table and variations tree setup in controller constructor, but
			//must manually setup the library tree here because it's after the embers were assigned.
			m_Controller->FillLibraryTree(index.row());//Passing row re-selects the item that was previously selected.
			m_Controller->SetTempPalette(tempPalette);//Restore palette.
			m_PaletteHueSpin->SetValueStealth(hue);
			m_PaletteSaturationSpin->SetValueStealth(sat);
			m_PaletteBrightnessSpin->SetValueStealth(bright);
			m_PaletteContrastSpin->SetValueStealth(con);
			m_PaletteBlurSpin->SetValueStealth(blur);
			m_PaletteFrequencySpin->SetValueStealth(freq);
			m_Controller->PaletteAdjust();//Fills in the palette.
		}
	}

	return m_Controller.get() != NULL;
}

/// <summary>
/// Start the render timer.
/// If a renderer has not been created yet, or differs form the options, it will first be created from the options.
/// </summary>
void Fractorium::StartRenderTimer()
{
	//Starting the render timer, either for the first time
	//or from a paused state, such as resizing or applying new options.
	CreateControllerFromOptions();

	if (m_Controller.get())
	{
		//On program startup, the renderer does not get initialized until now.
		CreateRendererFromOptions();

		if (m_Controller->Renderer())
			m_Controller->StartRenderTimer();
	}
}

/// <summary>
/// Idle timer event which calls the controller's Render() function.
/// </summary>
void Fractorium::IdleTimer() { m_Controller->Render(); }

/// <summary>
/// Thin wrapper to determine if the controllers have been properly initialized.
/// </summary>
/// <returns>True if the ember controller and GL controllers are both not NULL, else false.</returns>
bool Fractorium::ControllersOk() { return m_Controller.get() && m_Controller->GLController(); }
