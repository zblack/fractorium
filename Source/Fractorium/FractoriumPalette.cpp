#include "FractoriumPch.h"
#include "Fractorium.h"

#define PALETTE_CELL_HEIGHT 16

/// <summary>
/// Initialize the palette UI.
/// </summary>
void Fractorium::InitPaletteUI()
{
	int spinHeight = 20, row = 0;
	QTableWidget* paletteTable = ui.PaletteListTable;
	QTableWidget* palettePreviewTable = ui.PalettePreviewTable;

	connect(ui.PaletteFilenameCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnPaletteFilenameComboChanged(const QString&)), Qt::QueuedConnection);

	connect(paletteTable, SIGNAL(cellClicked(int, int)),	   this, SLOT(OnPaletteCellClicked(int, int)),		 Qt::QueuedConnection);
	connect(paletteTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnPaletteCellDoubleClicked(int, int)), Qt::QueuedConnection);
	
	//Palette adjustment table.	
	QTableWidget* table = ui.PaletteAdjustTable;
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//Split width over all columns evenly.
	
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteHueSpin,		 spinHeight, -180, 180, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteSaturationSpin, spinHeight, -100, 100, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteBrightnessSpin, spinHeight, -255, 255, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	row = 0;

	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteContrastSpin,  spinHeight, -100, 100, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteBlurSpin,	    spinHeight,	   0, 127, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteFrequencySpin, spinHeight,	   1,  10, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 1, 1, 1);

	connect(ui.PaletteRandomSelect, SIGNAL(clicked(bool)), this, SLOT(OnPaletteRandomSelectButtonClicked(bool)), Qt::QueuedConnection);
	connect(ui.PaletteRandomAdjust, SIGNAL(clicked(bool)), this, SLOT(OnPaletteRandomAdjustButtonClicked(bool)), Qt::QueuedConnection);

	//Preview table.
	palettePreviewTable->setRowCount(1);
	palettePreviewTable->setColumnWidth(1, 260);//256 plus small margin on each side.
	QTableWidgetItem* previewNameCol = new QTableWidgetItem("");
	palettePreviewTable->setItem(0, 0, previewNameCol);
	QTableWidgetItem* previewPaletteItem = new QTableWidgetItem();
	palettePreviewTable->setItem(0, 1, previewPaletteItem);

	paletteTable->setColumnWidth(1, 260);//256 plus small margin on each side.
	paletteTable->horizontalHeader()->setSectionsClickable(false);
}

/// <summary>
/// Read all palette Xml files in the specified folder and populate the palette list with the contents.
/// This will clear any previous contents.
/// Called upon initialization, or controller type change.
/// </summary>
/// <param name="s">The full path to the palette files folder</param>
/// <returns>The number of palettes successfully added</returns>
template <typename T>
int FractoriumEmberController<T>::InitPaletteList(const string& s)
{
	QDirIterator it(s.c_str(), QStringList() << "*.xml", QDir::Files, QDirIterator::FollowSymlinks);

	m_PaletteList.Clear();
	m_Fractorium->ui.PaletteFilenameCombo->clear();
	m_Fractorium->ui.PaletteFilenameCombo->setProperty("path", QString::fromStdString(s));

	while (it.hasNext())
	{
		auto path = it.next().toStdString();
		auto qfilename = it.fileName();
	
		if (m_PaletteList.Add(path))
			m_Fractorium->ui.PaletteFilenameCombo->addItem(qfilename);
	}

	return m_PaletteList.Size();
}

/// <summary>
/// Read a palette Xml file and populate the palette table with the contents.
/// This will clear any previous contents.
/// Called upon initialization, palette combo index change, and controller type change.
/// </summary>
/// <param name="s">The name to the palette file without the path</param>
/// <returns>True if successful, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::FillPaletteTable(const string& s)
{
	QTableWidget* paletteTable = m_Fractorium->ui.PaletteListTable;
	QTableWidget* palettePreviewTable = m_Fractorium->ui.PalettePreviewTable;

	m_CurrentPaletteFilePath = m_Fractorium->ui.PaletteFilenameCombo->property("path").toString().toStdString() + s;
	size_t paletteSize = m_PaletteList.Size(m_CurrentPaletteFilePath);
	
	if (paletteSize)
	{
		paletteTable->clear();
		paletteTable->blockSignals(true);
		paletteTable->setRowCount(paletteSize);
		
		//Headers get removed when clearing, so must re-create here.
		QTableWidgetItem* nameHeader = new QTableWidgetItem("Name");
		QTableWidgetItem* paletteHeader = new QTableWidgetItem("Palette");

		nameHeader->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		paletteHeader->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		paletteTable->setHorizontalHeaderItem(0, nameHeader);
		paletteTable->setHorizontalHeaderItem(1, paletteHeader);

		//Palette list table.
		for (size_t i = 0; i < paletteSize; i++)
		{
			Palette<T>* p = m_PaletteList.GetPalette(m_CurrentPaletteFilePath, i);
			vector<byte> v = p->MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);
			QTableWidgetItem* nameCol = new QTableWidgetItem(p->m_Name.c_str());

			nameCol->setToolTip(p->m_Name.c_str());
			paletteTable->setItem(i, 0, nameCol);

			QImage image(v.data(), p->Size(), PALETTE_CELL_HEIGHT, QImage::Format_RGB888);
			QTableWidgetItem* paletteItem = new QTableWidgetItem();

			paletteItem->setData(Qt::DecorationRole, QPixmap::fromImage(image));
			paletteTable->setItem(i, 1, paletteItem);
		}

		paletteTable->blockSignals(false);
		m_Fractorium->OnPaletteRandomSelectButtonClicked(true);
		return true;
	}
	else
	{
		vector<string> errors = m_PaletteList.ErrorReport();

		m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit);
		m_Fractorium->ShowCritical("Palette Read Error", "Could not load palette file, all images will be black. See info tab for details.");
	}

	return false;
}

void Fractorium::OnPaletteFilenameComboChanged(const QString& text) { m_Controller->FillPaletteTable(text.toStdString()); }

/// <summary>
/// Apply adjustments to the current ember's palette.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ApplyPaletteToEmber()
{
	int i, rot = 0;
	uint blur = m_Fractorium->m_PaletteBlurSpin->value();
	uint freq = m_Fractorium->m_PaletteFrequencySpin->value();
	double sat = double(m_Fractorium->m_PaletteSaturationSpin->value() / 100.0);
	double brightness = double(m_Fractorium->m_PaletteBrightnessSpin->value() / 255.0);
	double contrast = double(m_Fractorium->m_PaletteContrastSpin->value() > 0 ? (m_Fractorium->m_PaletteContrastSpin->value() * 2) : m_Fractorium->m_PaletteContrastSpin->value()) / 100.0;

	m_Ember.m_Hue = double(m_Fractorium->m_PaletteHueSpin->value()) / 360.0;//This is the only palette adjustment value that gets saved with the ember, so just assign it here.
	
	//Use the temp palette as the base and apply the adjustments gotten from the GUI and save the result in the ember palette.
	m_TempPalette.MakeAdjustedPalette(m_Ember.m_Palette, 0, m_Ember.m_Hue, sat, brightness, contrast, blur, freq);
}

/// <summary>
/// Use adjusted palette to update all related GUI controls with new color values.
/// Resets the rendering process.
/// </summary>
/// <param name="palette">The palette to use</param>
/// <param name="paletteName">Name of the palette</param>
template <typename T>
void FractoriumEmberController<T>::UpdateAdjustedPaletteGUI(Palette<T>& palette)
{
	Xform<T>* xform = CurrentXform();
	QTableWidget* palettePreviewTable = m_Fractorium->ui.PalettePreviewTable;
	QTableWidgetItem* previewPaletteItem = palettePreviewTable->item(0, 1);
	QString paletteName = QString::fromStdString(m_Ember.m_Palette.m_Name);

	if (previewPaletteItem)//This can be null if the palette file was moved or corrupted.
	{
		//Use the adjusted palette to fill the preview palette control so the user can see the effects of applying the adjustements.
		vector<byte> v = palette.MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);//Make the palette repeat for PALETTE_CELL_HEIGHT rows.

		m_FinalPaletteImage = QImage(palette.Size(), PALETTE_CELL_HEIGHT, QImage::Format_RGB888);//Create a QImage out of it.
		memcpy(m_FinalPaletteImage.scanLine(0), v.data(), v.size() * sizeof(v[0]));//Memcpy the data in.
		QPixmap pixmap = QPixmap::fromImage(m_FinalPaletteImage);//Create a QPixmap out of the QImage.
		previewPaletteItem->setData(Qt::DecorationRole, pixmap.scaled(QSize(pixmap.width(), palettePreviewTable->rowHeight(0) + 2), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));//Set the pixmap on the palette tab.
		SetPaletteRefTable(&pixmap);//Set the palette ref table on the xforms | color tab.

		QTableWidgetItem* previewNameItem = palettePreviewTable->item(0, 0);
		previewNameItem->setText(paletteName);//Finally, set the name of the palette to be both the text and the tooltip.
		previewNameItem->setToolTip(paletteName);
	}

	//Update the current xform's color and reset the rendering process.
	if (xform)
		XformColorIndexChanged(xform->m_ColorX, true);
}

/// <summary>
/// Apply all adjustments to the selected palette, show it
/// and assign it to the current ember.
/// Called when any adjustment spinner is modified.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PaletteAdjust()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		ApplyPaletteToEmber();
		UpdateAdjustedPaletteGUI(m_Ember.m_Palette);
	});
}

void Fractorium::OnPaletteAdjust(int d) { m_Controller->PaletteAdjust(); }

/// <summary>
/// Set the selected palette as the current one,
/// applying any adjustments previously specified.
/// Called when a palette cell is clicked. Unfortunately,
/// this will get called twice on a double click when moving
/// from one palette to another. It happens quickly so it shouldn't
/// be too much of a problem.
/// Resets the rendering process.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table col clicked</param>
template <typename T>
void FractoriumEmberController<T>::PaletteCellClicked(int row, int col)
{
	Palette<T>* palette = m_PaletteList.GetPalette(m_CurrentPaletteFilePath, row);
	QTableWidgetItem* nameItem = m_Fractorium->ui.PaletteListTable->item(row, 0);

	if (palette)
	{
		m_TempPalette = *palette;//Deep copy.
		ApplyPaletteToEmber();//Copy temp palette to ember palette and apply adjustments.
		UpdateAdjustedPaletteGUI(m_Ember.m_Palette);//Show the adjusted palette.
	}
}

void Fractorium::OnPaletteCellClicked(int row, int col)
{
	if (m_PreviousPaletteRow != row)
	{
		m_Controller->PaletteCellClicked(row, col);
		m_PreviousPaletteRow = row;//Save for comparison on next click.
	}
}

/// <summary>
/// Set the selected palette as the current one,
/// resetting any adjustments previously specified.
/// Called when a palette cell is double clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table col clicked</param>
void Fractorium::OnPaletteCellDoubleClicked(int row, int col)
{
	ResetPaletteControls();
	m_PreviousPaletteRow = -1;
	OnPaletteCellClicked(row, col);
}

/// <summary>
/// Set the selected palette to a randomly selected one,
/// applying any adjustments previously specified if the checked parameter is true.
/// Called when the Random Palette button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">True to clear the current adjustments, else leave current adjustments.</param>
void Fractorium::OnPaletteRandomSelectButtonClicked(bool checked)
{
	uint i = 0;
	int rowCount = ui.PaletteListTable->rowCount() - 1;

	while ((i = QTIsaac<ISAAC_SIZE, ISAAC_INT>::GlobalRand->Rand(rowCount)) == uint(m_PreviousPaletteRow));

	if (checked)
		OnPaletteCellDoubleClicked(i, 1);//Will clear the adjustments.
	else
		OnPaletteCellClicked(i, 1);
}

/// <summary>
/// Apply random adjustments to the selected palette.
/// Called when the Random Adjustment button is clicked.
/// Resets the rendering process.
/// </summary>
void Fractorium::OnPaletteRandomAdjustButtonClicked(bool checked)
{
	QTIsaac<ISAAC_SIZE, ISAAC_INT>* gRand = QTIsaac<ISAAC_SIZE, ISAAC_INT>::GlobalRand.get();

	m_PaletteHueSpin->setValue(-180 + gRand->Rand(361));
	m_PaletteSaturationSpin->setValue(-50 + gRand->Rand(101));//Full range of these leads to bad palettes, so clamp range.
	m_PaletteBrightnessSpin->setValue(-50 + gRand->Rand(101));
	m_PaletteContrastSpin->setValue(-50 + gRand->Rand(101));

	//Doing frequency and blur together gives bad palettes that are just a solid color.
	if (gRand->RandBit())
	{
		m_PaletteBlurSpin->setValue(gRand->Rand(21));
		m_PaletteFrequencySpin->setValue(1);
	}
	else
	{
		m_PaletteBlurSpin->setValue(0);
		m_PaletteFrequencySpin->setValue(1 + gRand->Rand(10));
	}

	OnPaletteAdjust(0);
}

/// <summary>
/// Reset the palette controls.
/// Usually in response to a palette cell double click.
/// </summary>
void Fractorium::ResetPaletteControls()
{
	m_PaletteHueSpin->SetValueStealth(0);
	m_PaletteSaturationSpin->SetValueStealth(0);
	m_PaletteBrightnessSpin->SetValueStealth(0);
	m_PaletteContrastSpin->SetValueStealth(0);
	m_PaletteBlurSpin->SetValueStealth(0);
	m_PaletteFrequencySpin->SetValueStealth(1);
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
