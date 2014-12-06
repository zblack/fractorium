#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the library tree UI.
/// </summary>
void Fractorium::InitLibraryUI()
{
	connect(ui.LibraryTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),		  this, SLOT(OnEmberTreeItemChanged(QTreeWidgetItem*, int)),	   Qt::QueuedConnection);
	connect(ui.LibraryTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEmberTreeItemDoubleClicked(QTreeWidgetItem*, int)), Qt::QueuedConnection);
}

/// <summary>
/// Slot function to be called via QMetaObject::invokeMethod() to update preview images in the preview thread.
/// </summary>
/// <param name="item">The item double clicked on</param>
/// <param name="v">The vector holding the RGBA bitmap</param>
/// <param name="width">The width of the bitmap</param>
/// <param name="height">The height of the bitmap</param>
void Fractorium::SetLibraryTreeItemData(EmberTreeWidgetItemBase* item, vector<byte>& v, uint width, uint height)
{
	item->SetImage(v, width, height);
}

/// <summary>
/// Set all libary tree entries to the name of the corresponding ember they represent.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SyncNames()
{
	EmberTreeWidgetItem<T>* item;
	QTreeWidget* tree = m_Fractorium->ui.LibraryTree;
	QTreeWidgetItem* top = tree->topLevelItem(0);
	
	tree->blockSignals(true);

	if (top)
	{
		for (int i = 0; i < top->childCount(); i++)//Iterate through all of the children, which will represent the open embers.
		{
			if ((item = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i))) && i < m_EmberFile.Size())//Cast the child widget to the EmberTreeWidgetItem type.
				item->setText(0, QString::fromStdString(m_EmberFile.m_Embers[i].m_Name));
		}
	}

	tree->blockSignals(false);
}

/// <summary>
/// Fill the library tree with the names of the embers in the
/// currently opened file.
/// Start preview render thread.
/// </summary>
/// <param name="selectIndex">After the tree is filled, select this index. Pass -1 to omit selecting an index.</param>
template <typename T>
void FractoriumEmberController<T>::FillLibraryTree(int selectIndex)
{
	uint i, j, size = 64;
	QTreeWidget* tree = m_Fractorium->ui.LibraryTree;
	vector<byte> v(size * size * 4);

	StopPreviewRender();
	tree->clear();
	QCoreApplication::flush();

	tree->blockSignals(true);

	QTreeWidgetItem* fileItem = new QTreeWidgetItem(tree);
	QFileInfo info(m_EmberFile.m_Filename);

	fileItem->setText(0, info.fileName());
	fileItem->setToolTip(0, m_EmberFile.m_Filename);
	fileItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);

	for (j = 0; j < m_EmberFile.Size(); j++)
	{
		Ember<T>* ember = &m_EmberFile.m_Embers[j];
		EmberTreeWidgetItem<T>* emberItem = new EmberTreeWidgetItem<T>(ember, fileItem);

		emberItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);

		if (ember->m_Name.empty())
			emberItem->setText(0, ToString(j));
		else
			emberItem->setText(0, ember->m_Name.c_str());

		emberItem->setToolTip(0, emberItem->text(0));
		emberItem->SetImage(v, size, size);
	}

	tree->blockSignals(false);

	if (selectIndex != -1)
		if (QTreeWidgetItem* top = tree->topLevelItem(0))
			if (EmberTreeWidgetItem<T>* emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(selectIndex)))
				emberItem->setSelected(true);

	QCoreApplication::flush();
	RenderPreviews(0, m_EmberFile.Size());
	tree->expandAll();	
}

/// <summary>
/// Update the library tree with the newly added embers (most likely from pasting) and
/// only render previews for the new ones, without clearing the entire tree.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::UpdateLibraryTree()
{
	uint i, size = 64;
	QTreeWidget* tree = m_Fractorium->ui.LibraryTree;
	vector<byte> v(size * size * 4);

	if (QTreeWidgetItem* top = tree->topLevelItem(0))
	{
		int childCount = top->childCount();
		
		tree->blockSignals(true);

		for (i = childCount; i < m_EmberFile.Size(); i++)
		{
			Ember<T>* ember = &m_EmberFile.m_Embers[i];
			EmberTreeWidgetItem<T>* emberItem = new EmberTreeWidgetItem<T>(ember, top);

			emberItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);

			if (ember->m_Name.empty())
				emberItem->setText(0, ToString(i));
			else
				emberItem->setText(0, ember->m_Name.c_str());

			emberItem->setToolTip(0, emberItem->text(0));
			emberItem->SetImage(v, size, size);
		}

		//When adding elements to the vector, they may have been reshuffled which will have invalidated
		//the pointers contained in the EmberTreeWidgetItems. So reassign all pointers here.
		for (i = 0; i < m_EmberFile.Size(); i++)
		{
			if (EmberTreeWidgetItem<T>* emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i)))
				emberItem->SetEmberPointer(&m_EmberFile.m_Embers[i]);
		}

		tree->blockSignals(false);
		RenderPreviews(childCount, m_EmberFile.Size());
	}
}

/// <summary>
/// Copy the text of the item which was changed to the name of the current ember.
/// Ensure all names are unique in the opened file.
/// This seems to be called spuriously, so we do a check inside to make sure
/// the text was actually changed.
/// We also have to wrap the dynamic_cast call in a try/catch block  because this can
/// be called on a widget that has already been deleted.
/// </summary>
/// <param name="item">The libary tree item changed</param>
/// <param name="col">The column clicked, ignored.</param>
template <typename T>
void FractoriumEmberController<T>::EmberTreeItemChanged(QTreeWidgetItem* item, int col)
{
	try
	{
		QTreeWidget* tree = m_Fractorium->ui.LibraryTree;
		EmberTreeWidgetItem<T>* emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(item);
	
		if (emberItem)
		{
			string oldName = emberItem->GetEmber()->m_Name;//First preserve the previous name.

			tree->blockSignals(true);
			emberItem->UpdateEmberName();//Copy edit text to the ember's name variable.
			m_EmberFile.MakeNamesUnique();//Ensure all names remain unique.
			SyncNames();//Copy all ember names to the tree items since some might have changed to be made unique.
			string newName = emberItem->GetEmber()->m_Name;//Get the new, final, unique name.

			if (m_Ember.m_Name == oldName && oldName != newName)//If the ember edited was the current one, and the name was indeed changed, update the name of the current one.
			{
				m_Ember.m_Name = newName;
				m_LastSaveCurrent = "";//Reset will force the dialog to show on the next save current since the user probably wants a different name.
			}
	
			tree->blockSignals(false);
		}
		else if (QTreeWidgetItem* parentItem = dynamic_cast<QTreeWidgetItem*>(item))
		{
			QString text = parentItem->text(0);

			if (text != "")
			{
				m_EmberFile.m_Filename = text;
				m_LastSaveAll = "";//Reset will force the dialog to show on the next save all since the user probably wants a different name.
			}
		}
	}
	catch(std::exception& e)
	{
		qDebug() << "FractoriumEmberController<T>::EmberTreeItemChanged() : Exception thrown: " << e.what();
	}
}

void Fractorium::OnEmberTreeItemChanged(QTreeWidgetItem* item, int col) { m_Controller->EmberTreeItemChanged(item, col); }

/// <summary>
/// Set the current ember to the selected item.
/// Clears the undo state.
/// Resets the rendering process.
/// Called when the user double clicks on a library tree item.
/// </summary>
/// <param name="item">The item double clicked on</param>
/// <param name="col">The column clicked, ignored.</param>
template <typename T>
void FractoriumEmberController<T>::EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col)
{
	if (EmberTreeWidgetItem<T>* emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(item))
	{
		ClearUndo();
		SetEmber(*emberItem->GetEmber());
	}
}

void Fractorium::OnEmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col) { m_Controller->EmberTreeItemDoubleClicked(item, col); }

/// <summary>
/// Stop the preview renderer if it's already running.
/// Clear all of the existing preview images, then start the preview rendering thread.
/// Optionally only render previews for a subset of all open embers.
/// </summary>
/// <param name="start">The 0-based index to start rendering previews for</param>
/// <param name="end">The 0-based index which is one beyond the last ember to render a preview for</param>
template <typename T>
void FractoriumEmberController<T>::RenderPreviews(uint start, uint end)
{
	StopPreviewRender();

	if (start == UINT_MAX && end == UINT_MAX)
	{
		QTreeWidget* tree = m_Fractorium->ui.LibraryTree;

		tree->blockSignals(true);

		if (QTreeWidgetItem* top = tree->topLevelItem(0))
		{
			int childCount = top->childCount();
			vector<byte> emptyPreview(PREVIEW_SIZE * PREVIEW_SIZE * 3);

			for (int i = 0; i < childCount; i++)
				if (EmberTreeWidgetItem<T>* treeItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i)))
					treeItem->SetImage(emptyPreview, PREVIEW_SIZE, PREVIEW_SIZE);
		}

		tree->blockSignals(false);
		m_PreviewResult = QtConcurrent::run(m_PreviewRenderFunc, 0, m_EmberFile.Size());
	}
	else
		m_PreviewResult = QtConcurrent::run(m_PreviewRenderFunc, start, end);
}

/// <summary>
/// Stop the preview rendering thread.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::StopPreviewRender()
{
	m_PreviewRun = false;

	while (m_PreviewRunning)
		QApplication::processEvents();
	
	m_PreviewResult.cancel();

	while (m_PreviewResult.isRunning())
		QApplication::processEvents();

	QCoreApplication::sendPostedEvents(m_Fractorium->ui.LibraryTree);
	QCoreApplication::flush();
}
