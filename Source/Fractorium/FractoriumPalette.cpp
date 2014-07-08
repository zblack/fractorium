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
}

/// <summary>
/// Read a palette Xml file and populate the palette table with the contents.
/// This will clear any previous contents.
/// Called upon initialization, or controller type change.
/// </summary>
/// <param name="s">The full path to the palette file</param>
/// <returns>True if successful, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::InitPaletteTable(string s)
{
	QTableWidget* paletteTable = m_Fractorium->ui.PaletteListTable;
	QTableWidget* palettePreviewTable = m_Fractorium->ui.PalettePreviewTable;

	paletteTable->clear();

	if (m_PaletteList.Init(s))//Default to this, but add an option later.//TODO
	{
		//Preview table.
		palettePreviewTable->setRowCount(1);
		palettePreviewTable->setColumnWidth(1, 260);//256 plus small margin on each side.
		QTableWidgetItem* previewNameCol = new QTableWidgetItem("");
		palettePreviewTable->setItem(0, 0, previewNameCol);
		QTableWidgetItem* previewPaletteItem = new QTableWidgetItem();
		palettePreviewTable->setItem(0, 1, previewPaletteItem);

		//Palette list table.
		paletteTable->setRowCount(m_PaletteList.Count());
		paletteTable->setColumnWidth(1, 260);//256 plus small margin on each side.
		paletteTable->horizontalHeader()->setSectionsClickable(false);

		//Headers get removed when clearing, so must re-create here.
		QTableWidgetItem* nameHeader = new QTableWidgetItem("Name");
		QTableWidgetItem* paletteHeader = new QTableWidgetItem("Palette");

		nameHeader->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
		paletteHeader->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

		paletteTable->setHorizontalHeaderItem(0, nameHeader);
		paletteTable->setHorizontalHeaderItem(1, paletteHeader);

		//Palette list table.
		for (size_t i = 0; i < m_PaletteList.Count(); i++)
		{
			Palette<T>* p = m_PaletteList.GetPalette(i);
			vector<unsigned char> v = p->MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);
			QTableWidgetItem* nameCol = new QTableWidgetItem(p->m_Name.c_str());

			nameCol->setToolTip(p->m_Name.c_str());
			paletteTable->setItem(i, 0, nameCol);

			QImage image(v.data(), p->Size(), PALETTE_CELL_HEIGHT, QImage::Format_RGB888);
			QTableWidgetItem* paletteItem = new QTableWidgetItem();

			paletteItem->setData(Qt::DecorationRole, QPixmap::fromImage(image));
			paletteTable->setItem(i, 1, paletteItem);
		}

		return true;
	}
	else
	{
		vector<string> errors = m_PaletteList.ErrorReport();

		m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit);
		QMessageBox::critical(m_Fractorium, "Palette Read Error", "Could not load palette file, all images will be black. See info tab for details.");
	}

	return false;
}

/// <summary>
/// Apply adjustments to the current ember's palette.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ApplyPaletteToEmber()
{
	int i, rot = 0;
	unsigned int blur = m_Fractorium->m_PaletteBlurSpin->value();
	unsigned int freq = m_Fractorium->m_PaletteFrequencySpin->value();
	double sat = (double)m_Fractorium->m_PaletteSaturationSpin->value() / 100.0;
	double brightness = (double)m_Fractorium->m_PaletteBrightnessSpin->value() / 255.0;
	double contrast = (double)(m_Fractorium->m_PaletteContrastSpin->value() > 0 ? (m_Fractorium->m_PaletteContrastSpin->value() * 2) : m_Fractorium->m_PaletteContrastSpin->value()) / 100.0;

	m_Ember.m_Hue = (double)(m_Fractorium->m_PaletteHueSpin->value()) / 360.0;//This is the only palette adjustment value that gets saved with the ember, so just assign it here.
	
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
		vector<unsigned char> v = palette.MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);//Make the palette repeat for PALETTE_CELL_HEIGHT rows.

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
	Palette<T>* palette = m_PaletteList.GetPalette(row);
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
