#include "FractoriumPch.h"
#include "FractoriumEmberController.h"
#include "Fractorium.h"
#include "GLEmberController.h"

/// <summary>
/// Constructor which initializes the non-templated members contained in this class.
/// The renderer, other templated members and GUI setup will be done in the templated derived controller class.
/// </summary>
/// <param name="fractorium">Pointer to the main window.</param>
FractoriumEmberControllerBase::FractoriumEmberControllerBase(Fractorium* fractorium)
{
	Timing t;

	m_Rendering = false;
	m_Shared = true;
	m_Platform = 0;
	m_Device = 0;
	m_FailedRenders = 0;
	m_UndoIndex = 0;
	m_RenderType = CPU_RENDERER;
	m_OutputTexID = 0;
	m_SubBatchCount = 1;//Will be ovewritten by the options on first render.
	m_Fractorium = fractorium;
	m_RenderTimer = NULL;
	m_RenderRestartTimer = NULL;
	m_Rand = QTIsaac<ISAAC_SIZE, ISAAC_INT>(ISAAC_INT(t.Tic()), ISAAC_INT(t.Tic() * 2), ISAAC_INT(t.Tic() * 3));//Ensure a different rand seed on each instance.

	m_RenderTimer = new QTimer(m_Fractorium);
	m_RenderTimer->setInterval(0);
	m_Fractorium->connect(m_RenderTimer, SIGNAL(timeout()), SLOT(IdleTimer()));

	m_RenderRestartTimer = new QTimer(m_Fractorium);
	m_Fractorium->connect(m_RenderRestartTimer, SIGNAL(timeout()), SLOT(StartRenderTimer()));
}

/// <summary>
/// Destructor which stops rendering and deletes the timers.
/// All other memory is cleared automatically through the use of STL.
/// </summary>
FractoriumEmberControllerBase::~FractoriumEmberControllerBase()
{
	StopRenderTimer(true);

	if (m_RenderTimer)
	{
		m_RenderTimer->stop();
		delete m_RenderTimer;
		m_RenderTimer = NULL;
	}

	if (m_RenderRestartTimer)
	{
		m_RenderRestartTimer->stop();
		delete m_RenderRestartTimer;
		m_RenderRestartTimer = NULL;
	}
}

/// <summary>
/// Constructor which passes the main window parameter to the base, initializes the templated members contained in this class.
/// Then sets up the parts of the GUI that require templated Widgets, such as the variations tree and the palette table.
/// Note the renderer is not setup here automatically. Instead, it must be manually created by the caller later.
/// </summary>
/// <param name="fractorium">Pointer to the main window.</param>
template <typename T>
FractoriumEmberController<T>::FractoriumEmberController(Fractorium* fractorium)
	: FractoriumEmberControllerBase(fractorium)
{
	m_PreviewRun = false;
	m_PreviewRunning = false;
	m_SheepTools = unique_ptr<SheepTools<T, T>>(new SheepTools<T, T>("flam3-palettes.xml", new EmberNs::Renderer<T, T>()));
	m_GLController = unique_ptr<GLEmberController<T>>(new GLEmberController<T>(fractorium, fractorium->ui.GLDisplay, this));
	m_PreviewRenderer = unique_ptr<EmberNs::Renderer<T, T>>(new EmberNs::Renderer<T, T>());
	SetupVariationTree();
	InitPaletteTable("flam3-palettes.xml");
	BackgroundChanged(QColor(0, 0, 0));//Default to black.
	ClearUndo();

	m_PreviewRenderer->Callback(NULL);
	m_PreviewRenderer->NumChannels(4);
	m_PreviewRenderer->EarlyClip(m_Fractorium->m_Settings->EarlyClip());
	m_PreviewRenderer->YAxisUp(m_Fractorium->m_Settings->YAxisUp());
	m_PreviewRenderer->SetEmber(m_Ember);//Give it an initial ember, will be updated many times later.
	//m_PreviewRenderer->ThreadCount(1);//For debugging.

	m_PreviewRenderFunc = [&](uint start, uint end)
	{
		while(m_PreviewRun || m_PreviewRunning)
		{
		}

		m_PreviewRun = true;
		m_PreviewRunning = true;
		m_PreviewRenderer->ThreadCount(max(1u, Timing::ProcessorCount() - 1));//Leave one processor free so the GUI can breathe.
		QTreeWidget* tree = m_Fractorium->ui.LibraryTree;

		if (QTreeWidgetItem* top = tree->topLevelItem(0))
		{
			for (size_t i = start; m_PreviewRun && i < end && i < m_EmberFile.Size(); i++)
			{
				Ember<T> ember = m_EmberFile.m_Embers[i];

				ember.SyncSize();
				ember.SetSizeAndAdjustScale(PREVIEW_SIZE, PREVIEW_SIZE, false, SCALE_WIDTH);
				ember.m_TemporalSamples = 1;
				ember.m_Quality = 25;
				ember.m_Supersample = 1;
				m_PreviewRenderer->SetEmber(ember);

				if (m_PreviewRenderer->Run(m_PreviewFinalImage) == RENDER_OK)
				{
					if (EmberTreeWidgetItem<T>* treeItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i)))
					{
						//It is critical that Qt::BlockingQueuedConnection is passed because this is running on a different thread than the UI.
						//This ensures the events are processed in order as each preview is updated, and that control does not return here
						//until the update is complete.
						QMetaObject::invokeMethod(m_Fractorium, "SetLibraryTreeItemData", Qt::BlockingQueuedConnection,
							Q_ARG(EmberTreeWidgetItemBase*, dynamic_cast<EmberTreeWidgetItemBase*>(treeItem)),
							Q_ARG(vector<byte>&, m_PreviewFinalImage),
							Q_ARG(uint, PREVIEW_SIZE),
							Q_ARG(uint, PREVIEW_SIZE));
						
						//treeItem->SetImage(m_PreviewFinalImage, PREVIEW_SIZE, PREVIEW_SIZE);
					}
				}
			}
		}

		m_PreviewRun = false;
		m_PreviewRunning = false;
	};
}

/// <summary>
/// Empty destructor that does nothing.
/// </summary>
template <typename T>
FractoriumEmberController<T>::~FractoriumEmberController() { }

/// <summary>
/// Setters for embers, ember files and palettes which convert between float and double types.
/// These are used to preserve the current ember/file when switching between renderers.
/// Note that some precision will be lost when going from double to float.
/// </summary>
template <typename T> void FractoriumEmberController<T>::SetEmber(const Ember<float>& ember, bool verbatim) { SetEmberPrivate<float>(ember, verbatim); }
template <typename T> void FractoriumEmberController<T>::CopyEmber(Ember<float>& ember, std::function<void(Ember<float>& ember)> perEmberOperation) { ember = m_Ember; perEmberOperation(ember); }
template <typename T> void FractoriumEmberController<T>::SetEmberFile(const EmberFile<float>& emberFile) { m_EmberFile = emberFile; }
template <typename T> void FractoriumEmberController<T>::CopyEmberFile(EmberFile<float>& emberFile, std::function<void(Ember<float>& ember)> perEmberOperation)
{
	emberFile.m_Filename = m_EmberFile.m_Filename;
	CopyVec(emberFile.m_Embers, m_EmberFile.m_Embers, perEmberOperation);
}

template <typename T> void FractoriumEmberController<T>::SetTempPalette(const Palette<float>& palette) { m_TempPalette = palette; }
template <typename T> void FractoriumEmberController<T>::CopyTempPalette(Palette<float>& palette) { palette = m_TempPalette; }
#ifdef DO_DOUBLE
template <typename T> void FractoriumEmberController<T>::SetEmber(const Ember<double>& ember, bool verbatim) { SetEmberPrivate<double>(ember, verbatim); }
template <typename T> void FractoriumEmberController<T>::CopyEmber(Ember<double>& ember, std::function<void(Ember<double>& ember)> perEmberOperation) { ember = m_Ember; perEmberOperation(ember); }
template <typename T> void FractoriumEmberController<T>::SetEmberFile(const EmberFile<double>& emberFile) { m_EmberFile = emberFile; }
template <typename T> void FractoriumEmberController<T>::CopyEmberFile(EmberFile<double>& emberFile, std::function<void(Ember<double>& ember)> perEmberOperation)
{
	emberFile.m_Filename = m_EmberFile.m_Filename;
	CopyVec(emberFile.m_Embers, m_EmberFile.m_Embers, perEmberOperation);
}

template <typename T> void FractoriumEmberController<T>::SetTempPalette(const Palette<double>& palette) { m_TempPalette = palette; }
template <typename T> void FractoriumEmberController<T>::CopyTempPalette(Palette<double>& palette) { palette = m_TempPalette; }
#endif
template <typename T> Ember<T>* FractoriumEmberController<T>::CurrentEmber() { return &m_Ember; }

template <typename T>
void FractoriumEmberController<T>::ConstrainDimensions(Ember<T>& ember)
{
	ember.m_FinalRasW = std::min<int>(m_Fractorium->ui.GLDisplay->MaxTexSize(), ember.m_FinalRasW);
	ember.m_FinalRasH = std::min<int>(m_Fractorium->ui.GLDisplay->MaxTexSize(), ember.m_FinalRasH);
}

/// <summary>
/// Set the ember at the specified index from the currently opened file as the current Ember.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
/// <param name="index">The index in the file from which to retrieve the ember</param>
template <typename T>
void FractoriumEmberController<T>::SetEmber(size_t index)
{
	if (index < m_EmberFile.Size())
	{
		if (QTreeWidgetItem* top = m_Fractorium->ui.LibraryTree->topLevelItem(0))
		{
			for (uint i = 0; i < top->childCount(); i++)
			{
				if (EmberTreeWidgetItem<T>* emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i)))
					emberItem->setSelected(i == index);
			}
		}

		ClearUndo();
		SetEmber(m_EmberFile.m_Embers[index]);
	}
}

/// <summary>
/// Wrapper to call a function, then optionally add the requested action to the rendering queue.
/// </summary>
/// <param name="func">The function to call</param>
/// <param name="updateRender">True to update renderer, else false. Default: false.</param>
/// <param name="action">The action to add to the rendering queue. Default: FULL_RENDER.</param>
template <typename T>
void FractoriumEmberController<T>::Update(std::function<void (void)> func, bool updateRender, eProcessAction action)
{
	func();

	if (updateRender)
		UpdateRender(action);
}

/// <summary>
/// Wrapper to call a function on the current xform, then optionally add the requested action to the rendering queue.
/// </summary>
/// <param name="func">The function to call</param>
/// <param name="updateRender">True to update renderer, else false. Default: true.</param>
/// <param name="action">The action to add to the rendering queue. Default: FULL_RENDER.</param>
template <typename T>
void FractoriumEmberController<T>::UpdateCurrentXform(std::function<void (Xform<T>*)> func, bool updateRender, eProcessAction action)
{
	if (Xform<T>* xform = CurrentXform())
	{
		func(xform);

		if (updateRender)
			UpdateRender(action);
	}
}

/// <summary>
/// Set the current ember, but use GUI values for the fields which make sense to
/// keep the same between ember selection changes.
/// Note the extra template parameter U allows for assigning ember of different types.
/// Resets the rendering process.
/// </summary>
/// <param name="ember">The ember to set as the current</param>
template <typename T>
template <typename U>
void FractoriumEmberController<T>::SetEmberPrivate(const Ember<U>& ember, bool verbatim)
{
	if (ember.m_Name != m_Ember.m_Name)
		m_LastSaveCurrent = "";
	
	m_Ember = ember;

	if (!verbatim)
	{
		//m_Ember.SetSizeAndAdjustScale(m_Fractorium->ui.GLDisplay->width(), m_Fractorium->ui.GLDisplay->height(), true, SCALE_WIDTH);
		m_Ember.m_TemporalSamples = 1;//Change once animation is supported.
		m_Ember.m_Quality = m_Fractorium->m_QualitySpin->value();
		m_Ember.m_Supersample = m_Fractorium->m_SupersampleSpin->value();
	}

	m_Fractorium->FillXforms();//Must do this first because the palette setup in FillParamTablesAndPalette() uses the xforms combo.
	FillParamTablesAndPalette();
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
