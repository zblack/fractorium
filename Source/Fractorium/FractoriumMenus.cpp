#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the menus UI.
/// </summary>
void Fractorium::InitMenusUI()
{
	//File menu.
	connect(ui.ActionNewFlock,					  SIGNAL(triggered(bool)), this, SLOT(OnActionNewFlock(bool)),					  Qt::QueuedConnection);
	connect(ui.ActionNewEmptyFlameInCurrentFile,  SIGNAL(triggered(bool)), this, SLOT(OnActionNewEmptyFlameInCurrentFile(bool)),  Qt::QueuedConnection);
	connect(ui.ActionNewRandomFlameInCurrentFile, SIGNAL(triggered(bool)), this, SLOT(OnActionNewRandomFlameInCurrentFile(bool)), Qt::QueuedConnection);
	connect(ui.ActionCopyFlameInCurrentFile,	  SIGNAL(triggered(bool)), this, SLOT(OnActionCopyFlameInCurrentFile(bool)),	  Qt::QueuedConnection);
	connect(ui.ActionOpen,						  SIGNAL(triggered(bool)), this, SLOT(OnActionOpen(bool)),						  Qt::QueuedConnection);
	connect(ui.ActionSaveCurrentAsXml,			  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveCurrentAsXml(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionSaveEntireFileAsXml,		  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveEntireFileAsXml(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionSaveCurrentToOpenedFile,	  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveCurrentToOpenedFile(bool)),	  Qt::QueuedConnection);
	connect(ui.ActionSaveCurrentScreen,			  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveCurrentScreen(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionExit,						  SIGNAL(triggered(bool)), this, SLOT(OnActionExit(bool)),						  Qt::QueuedConnection);

	//Edit menu.
	connect(ui.ActionUndo,			 SIGNAL(triggered(bool)), this, SLOT(OnActionUndo(bool)),			Qt::QueuedConnection);
	connect(ui.ActionRedo,			 SIGNAL(triggered(bool)), this, SLOT(OnActionRedo(bool)),			Qt::QueuedConnection);
	connect(ui.ActionCopyXml,		 SIGNAL(triggered(bool)), this, SLOT(OnActionCopyXml(bool)),		Qt::QueuedConnection);
	connect(ui.ActionCopyAllXml,	 SIGNAL(triggered(bool)), this, SLOT(OnActionCopyAllXml(bool)),		Qt::QueuedConnection);
	connect(ui.ActionPasteXmlAppend, SIGNAL(triggered(bool)), this, SLOT(OnActionPasteXmlAppend(bool)),	Qt::QueuedConnection);
	connect(ui.ActionPasteXmlOver,	 SIGNAL(triggered(bool)), this, SLOT(OnActionPasteXmlOver(bool)),	Qt::QueuedConnection);

	//Tools menu.
	connect(ui.ActionAddReflectiveSymmetry, SIGNAL(triggered(bool)), this, SLOT(OnActionAddReflectiveSymmetry(bool)), Qt::QueuedConnection);
	connect(ui.ActionAddRotationalSymmetry, SIGNAL(triggered(bool)), this, SLOT(OnActionAddRotationalSymmetry(bool)), Qt::QueuedConnection);
	connect(ui.ActionAddBothSymmetry,		SIGNAL(triggered(bool)), this, SLOT(OnActionAddBothSymmetry(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionClearFlame,			SIGNAL(triggered(bool)), this, SLOT(OnActionClearFlame(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionFlatten,			    SIGNAL(triggered(bool)), this, SLOT(OnActionFlatten(bool)),			      Qt::QueuedConnection);
	connect(ui.ActionUnflatten,			    SIGNAL(triggered(bool)), this, SLOT(OnActionUnflatten(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionStopRenderingPreviews,	SIGNAL(triggered(bool)), this, SLOT(OnActionStopRenderingPreviews(bool)), Qt::QueuedConnection);
	connect(ui.ActionRenderPreviews,		SIGNAL(triggered(bool)), this, SLOT(OnActionRenderPreviews(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionFinalRender,			SIGNAL(triggered(bool)), this, SLOT(OnActionFinalRender(bool)),			  Qt::QueuedConnection);
	connect(m_FinalRenderDialog,			SIGNAL(finished(int)),   this, SLOT(OnFinalRenderClose(int)),			  Qt::QueuedConnection);
	connect(ui.ActionOptions,				SIGNAL(triggered(bool)), this, SLOT(OnActionOptions(bool)),				  Qt::QueuedConnection);

	//Help menu.
	connect(ui.ActionAbout, SIGNAL(triggered(bool)), this, SLOT(OnActionAbout(bool)), Qt::QueuedConnection);
}

/// <summary>
/// Create a new flock of random embers, with the specified length.
/// </summary>
/// <param name="count">The number of embers to include in the flock</param>
template <typename T>
void FractoriumEmberController<T>::NewFlock(uint count)
{
	Ember<T> ember;

	StopPreviewRender();
	m_EmberFile.Clear();
	m_EmberFile.m_Embers.reserve(count);
	m_EmberFile.m_Filename = EmberFile<T>::DefaultFilename();

	for (uint i = 0; i < count; i++)
	{
		m_SheepTools->Random(ember);
		ParamsToEmber(ember);
		ember.m_Index = i;
		ember.m_Name = m_EmberFile.m_Filename.toStdString() + "-" + ToString(i + 1).toStdString();
		m_EmberFile.m_Embers.push_back(ember);
	}

	m_LastSaveAll = "";
	FillLibraryTree();
}

/// <summary>
/// Create a new flock and assign the first ember as the current one.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionNewFlock(bool checked)
{
	m_Controller->NewFlock(10);
	m_Controller->SetEmber(0);
}

/// <summary>
/// Create and add a new empty ember in the currently opened file
/// and set it as the current one.
/// It will have one empty xform in it.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::NewEmptyFlameInCurrentFile()
{
	Ember<T> ember;
	Xform<T> xform;
	QDateTime local(QDateTime::currentDateTime());

	StopPreviewRender();
	ParamsToEmber(ember);
	xform.m_Weight = T(0.25);
	xform.m_ColorX = m_Rand.Frand01<T>();
	ember.AddXform(xform);
	ember.m_Palette = *m_PaletteList.GetPalette(-1);
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = m_EmberFile.Size();
	m_EmberFile.m_Embers.push_back(ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	m_EmberFile.MakeNamesUnique();
	UpdateLibraryTree();
	SetEmber(m_EmberFile.Size() - 1);
}

void Fractorium::OnActionNewEmptyFlameInCurrentFile(bool checked) { m_Controller->NewEmptyFlameInCurrentFile(); }

/// <summary>
/// Create and add a new random ember in the currently opened file
/// and set it as the current one.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::NewRandomFlameInCurrentFile()
{
	Ember<T> ember;

	StopPreviewRender();
	m_SheepTools->Random(ember);
	ParamsToEmber(ember);
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = m_EmberFile.Size();
	m_EmberFile.m_Embers.push_back(ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	m_EmberFile.MakeNamesUnique();
	UpdateLibraryTree();
	SetEmber(m_EmberFile.Size() - 1);
}

void Fractorium::OnActionNewRandomFlameInCurrentFile(bool checked) { m_Controller->NewRandomFlameInCurrentFile(); }

/// <summary>
/// Create and add a a copy of the current ember in the currently opened file
/// and set it as the current one.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyFlameInCurrentFile()
{
	Ember<T> ember = m_Ember;

	StopPreviewRender();
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = m_EmberFile.Size();
	m_EmberFile.m_Embers.push_back(ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	m_EmberFile.MakeNamesUnique();
	UpdateLibraryTree();
	SetEmber(m_EmberFile.Size() - 1);
}

void Fractorium::OnActionCopyFlameInCurrentFile(bool checked) { m_Controller->CopyFlameInCurrentFile(); }

/// <summary>
/// Open a list of ember Xml files, apply various values from the GUI widgets.
/// Either append these newly read embers to the existing open embers,
/// or clear the current ember file first.
/// When appending, add the new embers the the end of library tree.
/// When not appending, clear and populate the library tree with the new embers.
/// Set the current ember to the first one in the newly opened list.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
/// <param name="filenames">A list of full paths and filenames</param>
/// <param name="append">True to append the embers in the new files to the end of the currently open embers, false to clear and replace them</param>
template <typename T>
void FractoriumEmberController<T>::OpenAndPrepFiles(const QStringList& filenames, bool append)
{
	if (!filenames.empty())
	{
		size_t i;
		EmberFile<T> emberFile;
		XmlToEmber<T> parser;
		vector<Ember<T>> embers;
		uint previousSize = append ? m_EmberFile.Size() : 0;

		StopPreviewRender();
		emberFile.m_Filename = filenames[0];

		foreach(const QString& filename, filenames)
		{
			embers.clear();

			if (parser.Parse(filename.toStdString().c_str(), embers) && !embers.empty())
			{
				for (i = 0; i < embers.size(); i++)
				{
					ConstrainDimensions(embers[i]);//Do not exceed the max texture size.

					//Also ensure it has a name.
					if (embers[i].m_Name == "" || embers[i].m_Name == "No name")
						embers[i].m_Name = ToString<qulonglong>(i).toStdString();

					embers[i].m_Quality = m_Fractorium->m_QualitySpin->value();
					embers[i].m_Supersample = m_Fractorium->m_SupersampleSpin->value();
				}

				m_LastSaveAll = "";
				emberFile.m_Embers.insert(emberFile.m_Embers.end(), embers.begin(), embers.end());
			}
			else
			{
				vector<string> errors = parser.ErrorReport();

				m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit);
				m_Fractorium->ShowCritical("Open Failed", "Could not open file, see info tab for details.");
			}
		}
		
		if (append)
		{
			if (m_EmberFile.m_Filename == "")
				m_EmberFile.m_Filename = filenames[0];

			m_EmberFile.m_Embers.insert(m_EmberFile.m_Embers.end(), emberFile.m_Embers.begin(), emberFile.m_Embers.end());
		}
		else
			m_EmberFile = emberFile;

		//Resync indices and names.
		for (i = 0; i < m_EmberFile.Size(); i++)
			m_EmberFile.m_Embers[i].m_Index = i;

		m_EmberFile.MakeNamesUnique();

		if (append)
			UpdateLibraryTree();
		else
			FillLibraryTree(append ? previousSize - 1 : 0);

		ClearUndo();
		SetEmber(previousSize);
	}
}

/// <summary>
/// Show a file open dialog to open ember Xml files.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionOpen(bool checked) { m_Controller->OpenAndPrepFiles(SetupOpenXmlDialog(), false); }

/// <summary>
/// Save current ember as Xml, using the Xml saving template values from the options.
/// This will first save the current ember back to the opened file in memory before
/// saving it to disk.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SaveCurrentAsXml()
{
	QString filename;
	FractoriumSettings* s = m_Fractorium->m_Settings;

	if (s->SaveAutoUnique() && m_LastSaveCurrent != "")
	{
		filename = EmberFile<T>::UniqueFilename(m_LastSaveCurrent);
	}
	else if (QFile::exists(m_LastSaveCurrent))
	{
		filename = m_LastSaveCurrent;
	}
	else
	{
		if (m_EmberFile.Size() == 1)
			filename = m_Fractorium->SetupSaveXmlDialog(m_EmberFile.m_Filename);//If only one ember present, just use parent filename.
		else
			filename = m_Fractorium->SetupSaveXmlDialog(QString::fromStdString(m_Ember.m_Name));//More than one ember present, use individual ember name.
	}
	
	if (filename != "")
	{
		Ember<T> ember = m_Ember;
		EmberToXml<T> writer;
		QFileInfo fileInfo(filename);
		xmlDocPtr tempEdit = ember.m_Edits;

		SaveCurrentToOpenedFile();//Save the current ember back to the opened file before writing to disk.
		ApplyXmlSavingTemplate(ember);
		ember.m_Edits = writer.CreateNewEditdoc(&ember, nullptr, "edit", s->Nick().toStdString(), s->Url().toStdString(), s->Id().toStdString(), "", 0, 0);

		if (tempEdit != nullptr)
			xmlFreeDoc(tempEdit);

		if (writer.Save(filename.toStdString().c_str(), ember, 0, true, false, true))
		{
			s->SaveFolder(fileInfo.canonicalPath());

			if (!s->SaveAutoUnique() || m_LastSaveCurrent == "")//Only save filename on first time through when doing auto unique names.
				m_LastSaveCurrent = filename;
		}
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.");
	}
}

void Fractorium::OnActionSaveCurrentAsXml(bool checked) { m_Controller->SaveCurrentAsXml(); }

/// <summary>
/// Save entire opened file Xml, using the Xml saving template values from the options on each ember.
/// This will first save the current ember back to the opened file in memory before
/// saving all to disk.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SaveEntireFileAsXml()
{
	QString filename;
	FractoriumSettings* s = m_Fractorium->m_Settings;

	if (s->SaveAutoUnique() && m_LastSaveAll != "")
		filename = EmberFile<T>::UniqueFilename(m_LastSaveAll);
	else if (QFile::exists(m_LastSaveAll))
		filename = m_LastSaveAll;
	else
		filename = m_Fractorium->SetupSaveXmlDialog(m_EmberFile.m_Filename);
	
	if (filename != "")
	{
		EmberFile<T> emberFile;
		EmberToXml<T> writer;
		QFileInfo fileInfo(filename);

		SaveCurrentToOpenedFile();//Save the current ember back to the opened file before writing to disk.
		emberFile = m_EmberFile;

		for (size_t i = 0; i < emberFile.Size(); i++)
			ApplyXmlSavingTemplate(emberFile.m_Embers[i]);

		if (writer.Save(filename.toStdString().c_str(), emberFile.m_Embers, 0, true, false, true))
		{
			if (!s->SaveAutoUnique() || m_LastSaveAll == "")//Only save filename on first time through when doing auto unique names.
				m_LastSaveAll = filename;

			s->SaveFolder(fileInfo.canonicalPath());
		}
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.");
	}
}

void Fractorium::OnActionSaveEntireFileAsXml(bool checked) { m_Controller->SaveEntireFileAsXml(); }

/// <summary>
/// Show a file save dialog and save what is currently shown in the render window to disk as an image.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionSaveCurrentScreen(bool checked)
{
	QString filename = SetupSaveImageDialog(m_Controller->Name());

	m_Controller->SaveCurrentRender(filename, true);
}

/// <summary>
/// Save the current ember back to its position in the opened file.
/// This does not save to disk.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SaveCurrentToOpenedFile()
{
	size_t i;
	bool fileFound = false;

	for (i = 0; i < m_EmberFile.Size(); i++)
	{
		if ((m_Ember.m_Name == m_EmberFile.m_Embers[i].m_Name) &&//Check both to be extra sure.
			(m_Ember.m_Index == m_EmberFile.m_Embers[i].m_Index))
		{
			m_EmberFile.m_Embers[i] = m_Ember;
			fileFound = true;
			break;
		}
	}

	if (!fileFound)
	{
		StopPreviewRender();
		m_EmberFile.m_Embers.push_back(m_Ember);
		m_EmberFile.MakeNamesUnique();
		UpdateLibraryTree();
	}
	else
	{
		RenderPreviews(i, i + 1);
	}
}

void Fractorium::OnActionSaveCurrentToOpenedFile(bool checked) { m_Controller->SaveCurrentToOpenedFile(); }

/// <summary>
/// Exit the application.
/// </summary>
/// <param name="checked">Ignore.</param>
void Fractorium::OnActionExit(bool checked)
{
	closeEvent(nullptr);
	QApplication::exit();
}

/// <summary>
/// Undoes this instance.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Undo()
{
	if (m_UndoList.size() > 1 && m_UndoIndex > 0)
	{
		int index = m_Ember.GetTotalXformIndex(CurrentXform());

		m_LastEditWasUndoRedo = true;
		m_UndoIndex = std::max(0u, m_UndoIndex - 1u);
		SetEmber(m_UndoList[m_UndoIndex], true);
		m_EditState = UNDO_REDO;
		
		if (index >= 0)
			m_Fractorium->CurrentXform(index);

		m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1 && (m_UndoIndex > 0));
		m_Fractorium->ui.ActionRedo->setEnabled(m_UndoList.size() > 1 && !(m_UndoIndex == m_UndoList.size() - 1));
	}
}

void Fractorium::OnActionUndo(bool checked) { m_Controller->Undo(); }

/// <summary>
/// Redoes this instance.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Redo()
{
	if (m_UndoList.size() > 1 && m_UndoIndex < m_UndoList.size() - 1)
	{
		int index = m_Ember.GetTotalXformIndex(CurrentXform());

		m_LastEditWasUndoRedo = true;
		m_UndoIndex = std::min<uint>(m_UndoIndex + 1, m_UndoList.size() - 1);
		SetEmber(m_UndoList[m_UndoIndex], true);
		m_EditState = UNDO_REDO;
		
		if (index >= 0)
			m_Fractorium->CurrentXform(index);

		m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1 && (m_UndoIndex > 0));
		m_Fractorium->ui.ActionRedo->setEnabled(m_UndoList.size() > 1 && !(m_UndoIndex == m_UndoList.size() - 1));
	}
}

void Fractorium::OnActionRedo(bool checked) { m_Controller->Redo(); }

/// <summary>
/// Copy the current ember Xml to the clipboard.
/// Apply Xml saving settings from the options first.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyXml()
{
	Ember<T> ember = m_Ember;
	EmberToXml<T> emberToXml;
	FractoriumSettings* settings = m_Fractorium->m_Settings;

	ember.m_Quality         = settings->XmlQuality();
	ember.m_Supersample     = settings->XmlSupersample();
	ember.m_TemporalSamples = settings->XmlTemporalSamples();
	QApplication::clipboard()->setText(QString::fromStdString(emberToXml.ToString(ember, "", 0, false, false, true)));
}

void Fractorium::OnActionCopyXml(bool checked) { m_Controller->CopyXml(); }

/// <summary>
/// Copy the Xmls for all open embers as a single string to the clipboard, enclosed with the <flames> tag.
/// Apply Xml saving settings from the options first for each ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyAllXml()
{
	ostringstream os;
	EmberToXml<T> emberToXml;
	FractoriumSettings* settings = m_Fractorium->m_Settings;

	os << "<flames>\n";

	for (size_t i = 0; i < m_EmberFile.Size(); i++)
	{
		Ember<T> ember = m_EmberFile.m_Embers[i];

		ApplyXmlSavingTemplate(ember);
		os << emberToXml.ToString(ember, "", 0, false, false, true);
	}

	os << "</flames>\n";
	QApplication::clipboard()->setText(QString::fromStdString(os.str()));
}

void Fractorium::OnActionCopyAllXml(bool checked) { m_Controller->CopyAllXml(); }

/// <summary>
/// Convert the Xml text from the clipboard to an ember, add it to the end
/// of the current file and set it as the current ember. If multiple Xmls were
/// copied to the clipboard and were enclosed in <flames> tags, then all of them will be added.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PasteXmlAppend()
{
	uint i, previousSize = m_EmberFile.Size();
	string s, errors;
	XmlToEmber<T> parser;
	vector<Ember<T>> embers;
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QByteArray b = codec->fromUnicode(QApplication::clipboard()->text());

	s.reserve(b.size());

	for (i = 0; i < b.size(); i++)
	{
		if (uint(b[i]) < 128u)
			s.push_back(b[i]);
	}

	b.clear();
	StopPreviewRender();
	parser.Parse(reinterpret_cast<byte*>(const_cast<char*>(s.c_str())), "", embers);
	errors = parser.ErrorReportString();

	if (errors != "")
	{
		m_Fractorium->ShowCritical("Paste Error", QString::fromStdString(errors));
	}

	if (!embers.empty())
	{
		for (i = 0; i < embers.size(); i++)
		{
			embers[i].m_Index = m_EmberFile.Size();
			ConstrainDimensions(embers[i]);//Do not exceed the max texture size.
		
			//Also ensure it has a name.
			if (embers[i].m_Name == "" || embers[i].m_Name == "No name")
				embers[i].m_Name = ToString<qulonglong>(embers[i].m_Index).toStdString();

			m_EmberFile.m_Embers.push_back(embers[i]);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
		}

		m_EmberFile.MakeNamesUnique();
		UpdateLibraryTree();
		SetEmber(previousSize);
	}
}

void Fractorium::OnActionPasteXmlAppend(bool checked) { m_Controller->PasteXmlAppend(); }

/// <summary>
/// Convert the Xml text from the clipboard to an ember, overwrite the
/// current file and set the first as the current ember. If multiple Xmls were
/// copied to the clipboard and were enclosed in <flames> tags, then the current file will contain all of them.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PasteXmlOver()
{
	uint i;
	string s, errors;
	XmlToEmber<T> parser;
	Ember<T> backupEmber = m_EmberFile.m_Embers[0];
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QByteArray b = codec->fromUnicode(QApplication::clipboard()->text());
	
	s.reserve(b.size());
	
	for (i = 0; i < b.size(); i++)
	{
		if (uint(b[i]) < 128u)
			s.push_back(b[i]);
	}

	b.clear();
	StopPreviewRender();
	m_EmberFile.m_Embers.clear();//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	parser.Parse(reinterpret_cast<byte*>(const_cast<char*>(s.c_str())), "", m_EmberFile.m_Embers);
	errors = parser.ErrorReportString();

	if (errors != "")
	{
		m_Fractorium->ShowCritical("Paste Error", QString::fromStdString(errors));
	}

	if (m_EmberFile.Size())
	{
		for (i = 0; i < m_EmberFile.Size(); i++)
		{
			m_EmberFile.m_Embers[i].m_Index = i;
			ConstrainDimensions(m_EmberFile.m_Embers[i]);//Do not exceed the max texture size.
		
			//Also ensure it has a name.
			if (m_EmberFile.m_Embers[i].m_Name == "" || m_EmberFile.m_Embers[i].m_Name == "No name")
				m_EmberFile.m_Embers[i].m_Name = ToString<qulonglong>(m_EmberFile.m_Embers[i].m_Index).toStdString();
		}
	}
	else
	{
		backupEmber.m_Index = 0;
		m_EmberFile.m_Embers.push_back(backupEmber);
	}

	m_EmberFile.MakeNamesUnique();
	FillLibraryTree();
	SetEmber(0);
}

void Fractorium::OnActionPasteXmlOver(bool checked) { m_Controller->PasteXmlOver(); }

/// <summary>
/// Add reflective symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddReflectiveSymmetry()
{
	QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

	m_Ember.AddSymmetry(-1, m_Rand);
	m_Fractorium->FillXforms();
	combo->setCurrentIndex(combo->count() - (m_Fractorium->HaveFinal() ? 2 : 1));//Set index to the last item before final.
	UpdateRender();
}

void Fractorium::OnActionAddReflectiveSymmetry(bool checked) { m_Controller->AddReflectiveSymmetry(); }

/// <summary>
/// Add rotational symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddRotationalSymmetry()
{
	QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

	m_Ember.AddSymmetry(2, m_Rand);
	m_Fractorium->FillXforms();
	combo->setCurrentIndex(combo->count() - (m_Fractorium->HaveFinal() ? 2 : 1));//Set index to the last item before final.
	UpdateRender();
}

void Fractorium::OnActionAddRotationalSymmetry(bool checked) { m_Controller->AddRotationalSymmetry(); }

/// <summary>
/// Add both reflective and rotational symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddBothSymmetry()
{
	QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

	m_Ember.AddSymmetry(-2, m_Rand);
	m_Fractorium->FillXforms();
	combo->setCurrentIndex(combo->count() - (m_Fractorium->HaveFinal() ? 2 : 1));//Set index to the last item before final.
	UpdateRender();
}

void Fractorium::OnActionAddBothSymmetry(bool checked) { m_Controller->AddBothSymmetry(); }

/// <summary>
/// Adds a FlattenVariation to every xform in the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Flatten() { UpdateCurrentXform([&] (Xform<T>* xform) { m_Ember.Flatten(XmlToEmber<T>::m_FlattenNames); FillVariationTreeWithXform(xform); }); }
void Fractorium::OnActionFlatten(bool checked) { m_Controller->Flatten(); }
	
/// <summary>
/// Removes pre/reg/post FlattenVariation from every xform in the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Unflatten() { UpdateCurrentXform([&] (Xform<T>* xform) { m_Ember.Unflatten(); FillVariationTreeWithXform(xform); }); }
void Fractorium::OnActionUnflatten(bool checked) { m_Controller->Unflatten(); }

/// <summary>
/// Delete all but one xform in the current ember.
/// Clear that xform's variations.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearFlame()
{
	while (m_Ember.TotalXformCount() > 1)
		m_Ember.DeleteTotalXform(m_Ember.TotalXformCount() - 1);

	if (m_Ember.XformCount() == 1)
	{
		if (Xform<T>* xform = m_Ember.GetXform(0))
		{
			xform->Clear();
			xform->ParentEmber(&m_Ember);
		}
	}

	m_Fractorium->FillXforms();
	m_Fractorium->ui.CurrentXformCombo->setCurrentIndex(0);
	UpdateRender();
}

void Fractorium::OnActionClearFlame(bool checked) { m_Controller->ClearFlame(); }

/// <summary>
/// Re-render all previews.
/// </summary>
void Fractorium::OnActionRenderPreviews(bool checked)
{
	m_Controller->RenderPreviews();
}

/// <summary>
/// Stop all previews from being rendered. This is handy if the user
/// opens a large file with many embers in it, such as an animation sequence.
/// </summary>
void Fractorium::OnActionStopRenderingPreviews(bool checked) { m_Controller->StopPreviewRender(); }

/// <summary>
/// Show the final render dialog as a modeless dialog to allow
/// the user to minimize the main window while doing a lengthy final render.
/// Note: The user probably should not be otherwise interacting with the main GUI
/// while the final render is taking place.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionFinalRender(bool checked)
{
	//First completely stop what the current rendering process is doing.
	m_Controller->DeleteRenderer();//Delete the renderer, but not the controller.
	OnActionSaveCurrentToOpenedFile(true);//Save whatever was edited back to the current open file.
	m_RenderStatusLabel->setText("Renderer stopped.");
	m_FinalRenderDialog->show();
}

/// <summary>
/// Called when the final render dialog has been closed.
/// </summary>
/// <param name="result">Ignored</param>
void Fractorium::OnFinalRenderClose(int result)
{
	m_RenderStatusLabel->setText("Renderer starting...");
	StartRenderTimer();//Re-create the renderer and start rendering again.
}

/// <summary>
/// Show the final options dialog.
/// Restart rendering and sync options after the options dialog is dismissed with Ok.
/// Called when the options dialog is finished with ok.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionOptions(bool checked)
{
	if (m_OptionsDialog->exec())
	{
		//First completely stop what the current rendering process is doing.
		m_Controller->Shutdown();
		StartRenderTimer();//This will recreate the controller and/or the renderer from the options if necessary, then start the render timer.
		m_Settings->sync();
	}
}

/// <summary>
/// Show the about dialog.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionAbout(bool checked)
{
	m_AboutDialog->exec();
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
