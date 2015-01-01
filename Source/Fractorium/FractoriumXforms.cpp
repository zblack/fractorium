#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms UI.
/// </summary>
void Fractorium::InitXformsUI()
{
	int spinHeight = 20, row = 0;

	connect(ui.AddXformButton,		 SIGNAL(clicked(bool)),			   this, SLOT(OnAddXformButtonClicked(bool)),	    Qt::QueuedConnection);
	connect(ui.DuplicateXformButton, SIGNAL(clicked(bool)),			   this, SLOT(OnDuplicateXformButtonClicked(bool)),	Qt::QueuedConnection);
	connect(ui.ClearXformButton,	 SIGNAL(clicked(bool)),			   this, SLOT(OnClearXformButtonClicked(bool)),	    Qt::QueuedConnection);
	connect(ui.DeleteXformButton,	 SIGNAL(clicked(bool)),			   this, SLOT(OnDeleteXformButtonClicked(bool)),    Qt::QueuedConnection);
	connect(ui.AddFinalXformButton,  SIGNAL(clicked(bool)),			   this, SLOT(OnAddFinalXformButtonClicked(bool)),  Qt::QueuedConnection);
	connect(ui.CurrentXformCombo,	 SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentXformComboChanged(int)),	    Qt::QueuedConnection);

	SetFixedTableHeader(ui.XformWeightNameTable->horizontalHeader());
	//Use SetupSpinner() just to create the spinner, but use col of -1 to prevent it from being added to the table.
	SetupSpinner<DoubleSpinBox, double>(ui.XformWeightNameTable, this, row, -1, m_XformWeightSpin, spinHeight, 0, 1000, 0.05, SIGNAL(valueChanged(double)), SLOT(OnXformWeightChanged(double)), false, 0, 1, 0);
	m_XformWeightSpin->setDecimals(3);
	m_XformWeightSpin->SmallStep(0.001);
	m_XformWeightSpinnerButtonWidget = new SpinnerButtonWidget(m_XformWeightSpin, "=", 20, 19, ui.XformWeightNameTable);
	m_XformWeightSpinnerButtonWidget->m_Button->setToolTip("Equalize weights");
	m_XformWeightSpinnerButtonWidget->m_Button->setStyleSheet("text-align: center center");
	connect(m_XformWeightSpinnerButtonWidget->m_Button, SIGNAL(clicked(bool)), this, SLOT(OnEqualWeightButtonClicked(bool)), Qt::QueuedConnection);

	ui.XformWeightNameTable->setCellWidget(0, 0, m_XformWeightSpinnerButtonWidget);
	ui.XformWeightNameTable->setItem(0, 1, new QTableWidgetItem());
	connect(ui.XformWeightNameTable, SIGNAL(cellChanged(int, int)), this, SLOT(OnXformNameChanged(int, int)), Qt::QueuedConnection);

	ui.CurrentXformCombo->setProperty("soloxform", -1);
	
#ifndef WIN32    
	//For some reason linux makes these 24x24, even though the designer explicitly says 16x16.
	ui.AddXformButton->setIconSize(QSize(16, 16));
	ui.DuplicateXformButton->setIconSize(QSize(16, 16));
	ui.ClearXformButton->setIconSize(QSize(16, 16));
	ui.DeleteXformButton->setIconSize(QSize(16, 16));
	ui.AddFinalXformButton->setIconSize(QSize(16, 16));
	ui.CurrentXformCombo->setIconSize(QSize(16, 16));
#endif
}

/// <summary>
/// Get the current xform.
/// </summary>
/// <returns>The current xform as specified by the current xform combo box index. NULL if out of range (should never happen).</returns>
template <typename T>
Xform<T>* FractoriumEmberController<T>::CurrentXform()
{
	return m_Ember.GetTotalXform(m_Fractorium->ui.CurrentXformCombo->currentIndex());
}

/// <summary>
/// Set the current xform to the index passed in.
/// </summary>
/// <param name="i">The index to set the current xform to</param>
void Fractorium::CurrentXform(uint i)
{
	if (i < uint(ui.CurrentXformCombo->count()))
		ui.CurrentXformCombo->setCurrentIndex(i);
}

/// <summary>
/// Set the current xform and populate all GUI widgets.
/// Called when the current xform combo box index changes.
/// </summary>
/// <param name="index">The selected combo box index</param>
template <typename T>
void FractoriumEmberController<T>::CurrentXformComboChanged(int index)
{
	if (Xform<T>* xform = m_Ember.GetTotalXform(index))
	{
		FillWithXform(xform);
		m_GLController->SetSelectedXform(xform);

		int solo = m_Fractorium->ui.CurrentXformCombo->property("soloxform").toInt();

		m_Fractorium->ui.SoloXformCheckBox->blockSignals(true);
		m_Fractorium->ui.SoloXformCheckBox->setChecked(solo == index);
		m_Fractorium->ui.SoloXformCheckBox->blockSignals(false);

		bool enable = !IsFinal(CurrentXform());

		m_Fractorium->ui.DuplicateXformButton->setEnabled(enable);
		m_Fractorium->m_XformWeightSpin->setEnabled(enable);
		m_Fractorium->ui.SoloXformCheckBox->setEnabled(enable);
		m_Fractorium->ui.AddFinalXformButton->setEnabled(enable);
	}
}

void Fractorium::OnCurrentXformComboChanged(int index) { m_Controller->CurrentXformComboChanged(index); }

/// <summary>
/// Add an empty xform in the current ember and set it as the current xform.
/// Called when the add xform button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::AddXform()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Xform<T> newXform;
		QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

		newXform.m_Weight = 0.25;
		newXform.m_ColorX = m_Rand.Frand01<T>();
		m_Ember.AddXform(newXform);
		m_Fractorium->FillXforms();
		combo->setCurrentIndex(combo->count() - (m_Fractorium->HaveFinal() ? 2 : 1));//Set index to the last item before final.
	});
}

void Fractorium::OnAddXformButtonClicked(bool checked) { m_Controller->AddXform(); }

/// <summary>
/// Duplicate the current xform in the current ember, and set it as the current xform.
/// Called when the duplicate xform button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::DuplicateXform()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

		if (xform && !IsFinal(xform))
		{
			m_Ember.AddXform(*xform);
			m_Fractorium->FillXforms();
			combo->setCurrentIndex(combo->count() - (m_Fractorium->HaveFinal() ? 2 : 1));//Set index to the last item before final.
		}
	});
}

void Fractorium::OnDuplicateXformButtonClicked(bool checked) { m_Controller->DuplicateXform(); }

/// <summary>
/// Clear all variations from the current xform, affine, palette and xaos are left untouched.
/// Called when the clear xform button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::ClearCurrentXform()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		xform->ClearAndDeleteVariations();
		//Note xaos is left alone.
		FillVariationTreeWithXform(xform);
	});
}

void Fractorium::OnClearXformButtonClicked(bool checked) { m_Controller->ClearCurrentXform(); }

/// <summary>
/// Delete the current xform.
/// Will not delete the last remaining non-final xform.
/// Called when the delete xform button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::DeleteCurrentXform()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		bool haveFinal = m_Fractorium->HaveFinal();
		QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;
		int count = combo->count();
		int index = combo->currentIndex();

		//Do not allow deleting the only remaining non-final xform.
		if (haveFinal && count <= 2 && index == 0)
			return;

		if (!haveFinal && count == 1)
			return;

		m_Ember.DeleteTotalXform(index);
		m_Fractorium->FillXforms();
		combo->setCurrentIndex(combo->count() - (haveFinal ? 2 : 1));//Set index to the last item before final.
	});
}

void Fractorium::OnDeleteXformButtonClicked(bool checked) { m_Controller->DeleteCurrentXform(); }

/// <summary>
/// Add a final xform to the ember and set it as the current xform.
/// Will only take action if a final xform is not already present.
/// Called when the add final xform button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::AddFinalXform()
{
	QComboBox* combo = m_Fractorium->ui.CurrentXformCombo;

	//Check to see if a final xform is already present.
	if (!m_Fractorium->HaveFinal())
	{
		Xform<T> xform;

		xform.AddVariation(new LinearVariation<T>());//Just a placeholder so other parts of the code don't see it as being empty.
		m_Ember.SetFinalXform(xform);
		combo->addItem("Final");
		combo->setItemIcon(combo->count() - 1, m_Fractorium->m_FinalXformComboIcon);
		combo->setCurrentIndex(combo->count() - 1);//Set index to the last item.
		UpdateRender();
	}
}

void Fractorium::OnAddFinalXformButtonClicked(bool checked) { m_Controller->AddFinalXform(); }

/// <summary>
/// Set the weight of the current xform.
/// Called when weight spinner changes.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The weight</param>
template <typename T>
void FractoriumEmberController<T>::XformWeightChanged(double d)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		xform->m_Weight = d;
		SetNormalizedWeightText(xform);
	});
}

void Fractorium::OnXformWeightChanged(double d) { m_Controller->XformWeightChanged(d); }

/// <summary>
/// Equalize the weights of all xforms in the ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::EqualizeWeights()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		m_Ember.EqualizeWeights();
		m_Fractorium->m_XformWeightSpin->setValue(xform->m_Weight);//Will trigger an update, so pass false to updateRender below.
	}, false);
}

void Fractorium::OnEqualWeightButtonClicked(bool checked) { m_Controller->EqualizeWeights(); }

/// <summary>
/// Set the name of the current xform.
/// Called when the user types in the name cell of the table.
/// </summary>
/// <param name="row">The row of the cell</param>
/// <param name="col">The col of the cell</param>
template <typename T>
void FractoriumEmberController<T>::XformNameChanged(int row, int col)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		int index = m_Ember.GetXformIndex(xform);

		xform->m_Name = m_Fractorium->ui.XformWeightNameTable->item(row, col)->text().toStdString();

		if (index != -1)
		{
			if (QTableWidgetItem* xformNameItem = m_Fractorium->ui.XaosTable->item(index, 0))
				xformNameItem->setText(MakeXaosNameString(index));
		}
	}, false);
}

void Fractorium::OnXformNameChanged(int row, int col) { m_Controller->XformNameChanged(row, col); }

/// <summary>
/// Fill all GUI widgets with values from the passed in xform.
/// </summary>
/// <param name="xform">The xform whose values will be used to populate the widgets</param>
template <typename T>
void FractoriumEmberController<T>::FillWithXform(Xform<T>* xform)
{
	m_Fractorium->m_XformWeightSpin->SetValueStealth(xform->m_Weight);
	SetNormalizedWeightText(xform);

	if (QTableWidgetItem* item = m_Fractorium->ui.XformWeightNameTable->item(0, 1))
	{
		m_Fractorium->ui.XformWeightNameTable->blockSignals(true);
		item->setText(QString::fromStdString(xform->m_Name));
		m_Fractorium->ui.XformWeightNameTable->blockSignals(false);
	}

	FillVariationTreeWithXform(xform);
	FillColorWithXform(xform);
	FillAffineWithXform(xform, true);
	FillAffineWithXform(xform, false);
	FillXaosWithCurrentXform();
}

/// <summary>
/// Set the normalized weight of the current xform as the suffix text of the weight spinner.
/// </summary>
/// <param name="xform">The current xform whose normalized weight will be shown</param>
template <typename T>
void FractoriumEmberController<T>::SetNormalizedWeightText(Xform<T>* xform)
{
	if (xform)
	{
		int index = m_Ember.GetXformIndex(xform);

		m_Ember.CalcNormalizedWeights(m_NormalizedWeights);

		if (index != -1 && index < m_NormalizedWeights.size())
			m_Fractorium->m_XformWeightSpin->setSuffix(QString(" (") + QLocale::system().toString(double(m_NormalizedWeights[index]), 'g', 3) + ")");
	}
}

/// <summary>
/// Determine whether the specified xform is the final xform in the ember.
/// </summary>
/// <param name="xform">The xform to examine</param>
/// <returns>True if final, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::IsFinal(Xform<T>* xform)
{
	return (m_Fractorium->HaveFinal() && (xform == m_Ember.FinalXform()));
}

/// <summary>
/// Fill the xforms combo box with the xforms in the current ember.
/// Select the first one and fill all widgets with its values.
/// </summary>
void Fractorium::FillXforms()
{
	int i = 0, spinHeight = 20;
	QComboBox* combo = ui.CurrentXformCombo;

	combo->blockSignals(true);
	combo->clear();
	
	for (i = 0; i < int(m_Controller->XformCount()); i++)
	{
		combo->addItem(ToString(i + 1));
		combo->setItemIcon(i, m_XformComboIcons[i % XFORM_COLOR_COUNT]);
	}
	
	if (m_Controller->UseFinalXform())
	{
		combo->addItem("Final");
		combo->setItemIcon(i, m_FinalXformComboIcon);
	}

	combo->blockSignals(false);
	combo->setCurrentIndex(0);
	FillXaosTable();
	OnSoloXformCheckBoxStateChanged(Qt::Unchecked);
	OnCurrentXformComboChanged(0);//Make sure the event gets called, because it won't if the zero index is already selected.
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
