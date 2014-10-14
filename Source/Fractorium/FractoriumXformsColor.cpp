#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms color UI.
/// </summary>
void Fractorium::InitXformsColorUI()
{
	int spinHeight = 20, row = 0;

	m_XformColorValueItem = new QTableWidgetItem();
	ui.XformColorIndexTable->setItem(0, 0, m_XformColorValueItem);
	
	m_PaletteRefItem = new QTableWidgetItem();
	ui.XformPaletteRefTable->setItem(0, 0, m_PaletteRefItem);
	ui.XformPaletteRefTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(ui.XformPaletteRefTable->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnXformRefPaletteResized(int, int, int)), Qt::QueuedConnection);

	SetupSpinner<DoubleSpinBox, double>(ui.XformColorIndexTable,  this, row, 1, m_XformColorIndexSpin,  spinHeight,  0, 1, 0.01, SIGNAL(valueChanged(double)), SLOT(OnXformColorIndexChanged(double)),  false,   0,   1,   0);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformColorSpeedSpin,  spinHeight, -1, 1,  0.1, SIGNAL(valueChanged(double)), SLOT(OnXformColorSpeedChanged(double)),  true,  0.5, 0.5, 0.5);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformOpacitySpin,	    spinHeight,  0, 1,  0.1, SIGNAL(valueChanged(double)), SLOT(OnXformOpacityChanged(double)),	    true,    1,   1,   0);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformDirectColorSpin, spinHeight,  0, 1,  0.1, SIGNAL(valueChanged(double)), SLOT(OnXformDirectColorChanged(double)),	true,	 1,   1,   0);

	m_XformColorIndexSpin->setDecimals(3);
	m_XformColorSpeedSpin->setDecimals(3);
	m_XformOpacitySpin->setDecimals(3);
	m_XformDirectColorSpin->setDecimals(3);
	connect(ui.XformColorScroll,  SIGNAL(valueChanged(int)), this, SLOT(OnXformScrollColorIndexChanged(int)),  Qt::QueuedConnection);
	connect(ui.SoloXformCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnSoloXformCheckBoxStateChanged(int)), Qt::QueuedConnection);
}

/// <summary>
/// Set the color index of the current xform.
/// Update the color index scrollbar to match.
/// Called when spinner in the color index cell in the palette ref table is changed.
/// Optionally resets the rendering process.
/// </summary>
/// <param name="d">The color index, 0-1/</param>
/// <param name="updateRender">True to reset the rendering process, else don't.</param>
template <typename T>
void FractoriumEmberController<T>::XformColorIndexChanged(double d, bool updateRender)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		QScrollBar* scroll = m_Fractorium->ui.XformColorScroll;
		int scrollVal = d * scroll->maximum();

		scroll->blockSignals(true);
		scroll->setValue(scrollVal);
		scroll->blockSignals(false);

		SetCurrentXformColorIndex(d);
	}, updateRender);
}

void Fractorium::OnXformColorIndexChanged(double d) { OnXformColorIndexChanged(d, true); }
void Fractorium::OnXformColorIndexChanged(double d, bool updateRender) { m_Controller->XformColorIndexChanged(d, updateRender); }

/// <summary>
/// Set the color index of the current xform.
/// Update the color index cell in the palette ref table to match.
/// Called when color index scrollbar is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The color index, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformScrollColorIndexChanged(int d)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		m_Fractorium->m_XformColorIndexSpin->setValue(d / (double)m_Fractorium->ui.XformColorScroll->maximum());//Will trigger an update.
	}, false);
}

void Fractorium::OnXformScrollColorIndexChanged(int d) { m_Controller->XformScrollColorIndexChanged(d); }

/// <summary>
/// Set the color speed of the current xform.
/// Called when xform color speed spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The color speed, -1-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformColorSpeedChanged(double d) { UpdateCurrentXform([&] (Xform<T>* xform) { xform->m_ColorSpeed = d; }); }
void Fractorium::OnXformColorSpeedChanged(double d) { m_Controller->XformColorSpeedChanged(d); }

/// <summary>
/// Set the opacity of the current xform.
/// Called when xform opacity spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The opacity, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformOpacityChanged(double d) { UpdateCurrentXform([&] (Xform<T>* xform) { xform->m_Opacity = d; }); }
void Fractorium::OnXformOpacityChanged(double d) { m_Controller->XformOpacityChanged(d); }

/// <summary>
/// Set the direct color percentage of the current xform.
/// Called when xform direct color spinner is changed.
/// Note this only affects xforms that include a dc_ variation.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The direct color percentage, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformDirectColorChanged(double d) { UpdateCurrentXform([&] (Xform<T>* xform) { xform->m_DirectColor = d; }); }
void Fractorium::OnXformDirectColorChanged(double d) { m_Controller->XformDirectColorChanged(d); }

/// <summary>
/// Set whether the current xform should be rendered solo.
/// If checked, current is solo, if unchecked, none are solo.
/// Solo means that all other xforms will have their opacity temporarily
/// set to zero while rendering so that only the effect of current xform is visible.
/// This will not permanently alter the ember, as the temporary opacity values will be applied
/// right before rendering and reset right after.
/// Called when solo xform check box is checked.
/// Resets the rendering process.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void Fractorium::OnSoloXformCheckBoxStateChanged(int state)
{
	if (state == Qt::Checked)
	{
		ui.CurrentXformCombo->setProperty("soloxform", ui.CurrentXformCombo->currentIndex());
		ui.SoloXformCheckBox->setText("Solo (" + ToString(ui.CurrentXformCombo->currentIndex() + 1) + ")");
	}
	else if (state == Qt::Unchecked)
	{
		ui.CurrentXformCombo->setProperty("soloxform", -1);
		ui.SoloXformCheckBox->setText("Solo");
	}

	m_Controller->UpdateRender();
}

/// <summary>
/// Redraw the palette ref table.
/// Called on resize.
/// </summary>
/// <param name="logicalIndex">Ignored</param>
/// <param name="oldSize">Ignored</param>
/// <param name="newSize">Ignored</param>
void Fractorium::OnXformRefPaletteResized(int logicalIndex, int oldSize, int newSize)
{
	m_Controller->SetPaletteRefTable(NULL);
}

/// <summary>
/// Set the current xform color index spinner to the current xform's color index.
/// Set the color cell in the palette ref table.
/// </summary>
/// <param name="d">The index value to set, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::SetCurrentXformColorIndex(double d)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		xform->m_ColorX = Clamp<T>(d, 0, 1);

		//Grab the current color from the index and assign it to the first cell of the first table.
		v4T entry = m_Ember.m_Palette[Clamp<int>(d * COLORMAP_LENGTH_MINUS_1, 0, m_Ember.m_Palette.Size())];
		
		entry.r *= 255;
		entry.g *= 255;
		entry.b *= 255;

		QRgb rgb = (unsigned int)entry.r << 16 | (unsigned int)entry.g << 8 | (unsigned int)entry.b;
		m_Fractorium->ui.XformColorIndexTable->item(0, 0)->setBackgroundColor(QColor::fromRgb(rgb));
	}, false);
}

/// <summary>
/// Set the color index, speed and opacity spinners with the values of the current xform.
/// Set the cells of the palette ref table as well.
/// </summary>
/// <param name="xform">The xform whose values will be copied to the GUI</param>
template <typename T>
void FractoriumEmberController<T>::FillColorWithXform(Xform<T>* xform)
{
	m_Fractorium->m_XformColorIndexSpin->SetValueStealth(xform->m_ColorX);
	m_Fractorium->m_XformColorSpeedSpin->SetValueStealth(xform->m_ColorSpeed);
	m_Fractorium->m_XformOpacitySpin->SetValueStealth(xform->m_Opacity);
	m_Fractorium->m_XformDirectColorSpin->SetValueStealth(xform->m_DirectColor);
	m_Fractorium->OnXformColorIndexChanged(xform->m_ColorX, false);//Had to call stealth before to avoid doing an update, now manually update related controls, still without doing an update.
}

/// <summary>
/// Set the palette reference table to the passed in pixmap
/// </summary>
/// <param name="pixmap">The pixmap</param>
void FractoriumEmberControllerBase::SetPaletteRefTable(QPixmap* pixmap)
{
	QSize size(m_Fractorium->ui.XformPaletteRefTable->columnWidth(0), m_Fractorium->ui.XformPaletteRefTable->rowHeight(0) + 1);

	if (pixmap)
	{
		m_Fractorium->m_PaletteRefItem->setData(Qt::DecorationRole, pixmap->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	}
	else if (!m_FinalPaletteImage.isNull())
	{
		QPixmap pixTemp = QPixmap::fromImage(m_FinalPaletteImage);

		m_Fractorium->m_PaletteRefItem->setData(Qt::DecorationRole, pixTemp.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	}
}
