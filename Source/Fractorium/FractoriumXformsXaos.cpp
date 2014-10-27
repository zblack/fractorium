#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms xaos UI.
/// </summary>
void Fractorium::InitXformsXaosUI()
{
	connect(ui.XaosToRadio, SIGNAL(toggled(bool)), this, SLOT(OnXaosFromToToggled(bool)), Qt::QueuedConnection);
	connect(ui.ClearXaosButton, SIGNAL(clicked(bool)), this, SLOT(OnClearXaosButtonClicked(bool)), Qt::QueuedConnection);
	connect(ui.RandomXaosButton, SIGNAL(clicked(bool)), this, SLOT(OnRandomXaosButtonClicked(bool)), Qt::QueuedConnection);
}

/// <summary>
/// Fill the xaos table with the values from the current xform.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillXaosWithCurrentXform()
{
	Xform<T>* currXform = CurrentXform();

	if (!IsFinal(currXform))
	{
		for (int i = 0; i < m_Ember.XformCount(); i++)
		{
			DoubleSpinBox* spinBox = dynamic_cast<DoubleSpinBox*>(m_Fractorium->ui.XaosTable->cellWidget(i, 1));

			//Fill in values column.
			if (m_Fractorium->ui.XaosToRadio->isChecked())//"To": Single xform, advance index.
				spinBox->SetValueStealth(currXform->Xaos(i));
			else//"From": Advance xforms, single index.
				if (currXform = m_Ember.GetXform(i))
					spinBox->SetValueStealth(currXform->Xaos(m_Fractorium->ui.CurrentXformCombo->currentIndex()));

			//Fill in name column.
			Xform<T>* xform = m_Ember.GetXform(i);
			QTableWidgetItem* xformNameItem = m_Fractorium->ui.XaosTable->item(i, 0);

			if (xform && xformNameItem)
				xformNameItem->setText(MakeXaosNameString(i));
		}
	}

	m_Fractorium->ui.XaosTable->setEnabled(!IsFinal(currXform));//Disable if final, else enable.
}

/// <summary>
/// Create and return a xaos name string.
/// </summary>
/// <param name="i">The index of the xform whose xaos will be used</param>
/// <returns>The xaos name string</returns>
template <typename T>
QString FractoriumEmberController<T>::MakeXaosNameString(unsigned int i)
{
	Xform<T>* xform = m_Ember.GetXform(i);
	QString name;

	if (xform)
	{
		int i = m_Ember.GetXformIndex(xform) + 1;//GUI is 1 indexed to avoid confusing the user.
		int curr = m_Fractorium->ui.CurrentXformCombo->currentIndex() + 1;

		if (i != -1)
		{
			if (m_Fractorium->ui.XaosToRadio->isChecked())
				name = QString("From ") + ToString(curr) + QString(" To ") + ToString(i);
			else
				name = QString("From ") + ToString(i) + QString(" To ") + ToString(curr);

			//if (xform->m_Name != "")
			//	name = name + " (" + QString::fromStdString(xform->m_Name) + ")";
		}
	}

	return name;
}

/// <summary>
/// Set the xaos value.
/// Called when any xaos spinner is changed.
/// Different action taken based on the state of to/from radio button.
/// Resets the rendering process.
/// </summary>
/// <param name="sender">The DoubleSpinBox that triggered this event</param>
template <typename T>
void FractoriumEmberController<T>::XaosChanged(DoubleSpinBox* sender)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		QTableWidget* xaosTable = m_Fractorium->ui.XaosTable;

		if (!IsFinal(xform))//This should never get called for the final xform because the table will be disabled, but check just to be safe.
		{
			for (int i = 0; i < xaosTable->rowCount(); i++)//Find the spin box that triggered the event.
			{
				DoubleSpinBox* spinBox = dynamic_cast<DoubleSpinBox*>(xaosTable->cellWidget(i, 1));

				if (spinBox == sender)
				{
					if (m_Fractorium->ui.XaosToRadio->isChecked())//"To": Single xform, advance index.
					{
						xform->SetXaos(i, spinBox->value());
					}
					else//"From": Advance xforms, single index.
					{
						if (xform = m_Ember.GetXform(i))//Single = is intentional.
							xform->SetXaos(m_Fractorium->ui.CurrentXformCombo->currentIndex(), spinBox->value());
					}

					break;
				}
			}
		}
	});
}

void Fractorium::OnXaosChanged(double d)
{
	if (DoubleSpinBox* senderSpinBox = dynamic_cast<DoubleSpinBox*>(this->sender()))
		m_Controller->XaosChanged(senderSpinBox);
}

/// <summary>
/// Update xaos display to use either "to" or "from" logic.
/// Called when xaos to/from radio buttons checked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnXaosFromToToggled(bool checked)
{
	m_Controller->FillXaosWithCurrentXform();
}

/// <summary>
/// Clear xaos table, recreate all spinners based on the xaos used by the current xform in the current ember.
/// Called every time the current xform changes.
/// </summary>
void Fractorium::FillXaosTable()
{
	int spinHeight = 20;
	QWidget* w;
	ui.XaosTable->setRowCount(m_Controller->XformCount());//This will grow or shrink the number of rows and call the destructor for previous DoubleSpinBoxes.

	for (int i = 0; i < m_Controller->XformCount(); i++)
	{
		DoubleSpinBox* spinBox = new DoubleSpinBox(ui.XaosTable, spinHeight, 0.1);
		QTableWidgetItem* xformNameItem = new QTableWidgetItem(m_Controller->MakeXaosNameString(i));

		spinBox->DoubleClick(true);
		spinBox->DoubleClickZero(1);
		spinBox->DoubleClickNonZero(0);
		ui.XaosTable->setItem(i, 0, xformNameItem);
		ui.XaosTable->setCellWidget(i, 1, spinBox);
		connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(OnXaosChanged(double)), Qt::QueuedConnection);

		if (i > 0)
			w = SetTabOrder(this, w, spinBox);
		else
			w = spinBox;
	}

	w = SetTabOrder(this, w, ui.XaosToRadio);
	w = SetTabOrder(this, w, ui.XaosFromRadio);
	w = SetTabOrder(this, w, ui.ClearXaosButton);
	w = SetTabOrder(this, w, ui.RandomXaosButton);
}

/// <summary>
/// Clear all xaos from the current ember.
/// Called when xaos to/from radio buttons checked.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::ClearXaos()
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		m_Ember.ClearXaos();
	});
	
	//Can't just call FillXaosWithCurrentXform() because the current xform might the final.
	for (int i = 0; i < m_Ember.XformCount(); i++)
		if (DoubleSpinBox* spinBox = dynamic_cast<DoubleSpinBox*>(m_Fractorium->ui.XaosTable->cellWidget(i, 1)))
			spinBox->SetValueStealth(1.0);
}

void Fractorium::OnClearXaosButtonClicked(bool checked) { m_Controller->ClearXaos(); }

/// <summary>
/// Set all xaos values to random numbers.
/// There is a 50% chance they're set to 0 or 1, and
/// 50% that they're 0-3.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
template <typename T>
void FractoriumEmberController<T>::RandomXaos()
{
	UpdateCurrentXform([&](Xform<T>* xform)
	{
		for (size_t i = 0; i < m_Ember.XformCount(); i++)
		{
			if (Xform<T>* xform = m_Ember.GetXform(i))
			{
				for (size_t j = 0; j < m_Ember.XformCount(); j++)
				{
					if (m_Rand.RandBit())
						xform->SetXaos(j, (T)m_Rand.RandBit());
					else
						xform->SetXaos(j, m_Rand.Frand<T>(0, 3));
				}
			}
		}
	});

	//If current is final, it will update when they change to a non-final xform.
	FillXaosWithCurrentXform();
}

void Fractorium::OnRandomXaosButtonClicked(bool checked) { m_Controller->RandomXaos(); }