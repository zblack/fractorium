#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms xaos UI.
/// </summary>
void Fractorium::InitXformsXaosUI()
{
	connect(ui.ClearXaosButton, SIGNAL(clicked(bool)), this, SLOT(OnClearXaosButtonClicked(bool)), Qt::QueuedConnection);
	connect(ui.RandomXaosButton, SIGNAL(clicked(bool)), this, SLOT(OnRandomXaosButtonClicked(bool)), Qt::QueuedConnection);
}

/// <summary>
/// Fill the xaos table with the values from the ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillXaos()
{
	for (int i = 0, count = int(XformCount()); i < count; i++)
	{
		auto* xform = m_Ember.GetXform(i);

		for (int j = 0; j < count; j++)
			if (auto* spinBox = dynamic_cast<DoubleSpinBox*>(m_Fractorium->ui.XaosTable->cellWidget(i, j)))
				spinBox->SetValueStealth(xform->Xaos(j));
	}
}

/// <summary>
/// Create and return a xaos name string.
/// </summary>
/// <param name="i">The index of the xform whose xaos will be used</param>
/// <returns>The xaos name string</returns>
template <typename T>
QString FractoriumEmberController<T>::MakeXaosNameString(uint i)
{
	Xform<T>* xform = m_Ember.GetXform(i);
	QString name;

	//if (xform)
	//{
	//	int indexPlus1 = m_Ember.GetXformIndex(xform) + 1;//GUI is 1 indexed to avoid confusing the user.
	//	int curr = m_Fractorium->ui.CurrentXformCombo->currentIndex() + 1;
	//
	//	if (indexPlus1 != -1)
	//	{
	//		if (m_Fractorium->ui.XaosToRadio->isChecked())
	//			name = QString("From ") + ToString(curr) + QString(" To ") + ToString(indexPlus1);
	//		else
	//			name = QString("From ") + ToString(indexPlus1) + QString(" To ") + ToString(curr);
	//
	//		//if (xform->m_Name != "")
	//		//	name = name + " (" + QString::fromStdString(xform->m_Name) + ")";
	//	}
	//}

	return name;
}

/// <summary>
/// Set the xaos value.
/// Called when any xaos spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="sender">The DoubleSpinBox that triggered this event</param>
template <typename T>
void FractoriumEmberController<T>::XaosChanged(DoubleSpinBox* sender)
{
	auto p = sender->property("tableindex").toPoint();

	if (auto* xform = m_Ember.GetXform(p.x()))
		Update([&] { xform->SetXaos(p.y(), sender->value()); });
}

void Fractorium::OnXaosChanged(double d)
{
	if (auto* senderSpinBox = dynamic_cast<DoubleSpinBox*>(this->sender()))
		m_Controller->XaosChanged(senderSpinBox);
}

/// <summary>
/// Clear xaos table, recreate all spinners based on the xaos used in the current ember.
/// </summary>
void Fractorium::FillXaosTable()
{
	int spinHeight = 20;
	int count = int(m_Controller->XformCount());
	QWidget* w = nullptr;
	QString lbl("lbl");

	ui.XaosTable->setRowCount(count);//This will grow or shrink the number of rows and call the destructor for previous DoubleSpinBoxes.
	ui.XaosTable->setColumnCount(count);

	ui.XaosTable->verticalHeader()->setVisible(true);
	ui.XaosTable->horizontalHeader()->setVisible(true);
	ui.XaosTable->verticalHeader()->setSectionsClickable(false);
	ui.XaosTable->horizontalHeader()->setSectionsClickable(false);

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			QPoint p(i, j);
			DoubleSpinBox* spinBox = new DoubleSpinBox(ui.XaosTable, spinHeight, 0.1);

			spinBox->setFixedWidth(35);
			spinBox->DoubleClick(true);
			spinBox->DoubleClickZero(1);
			spinBox->DoubleClickNonZero(0);
			spinBox->setProperty("tableindex", p);
			ui.XaosTable->setCellWidget(i, j, spinBox);
			
			auto wp = ui.XaosTable->item(i, j);

			if (wp)
				wp->setTextAlignment(Qt::AlignCenter);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(OnXaosChanged(double)), Qt::QueuedConnection);

			if (i == 0 && j == 0)
				w = spinBox;
			else
				w = SetTabOrder(this, w, spinBox);
		}
	}
	
	for (int i = 0; i < count; i++)
	{
		ui.XaosTable->setHorizontalHeaderItem(i, new QTableWidgetItem("F" + QString::number(i + 1)));
		ui.XaosTable->setVerticalHeaderItem(i, new QTableWidgetItem("T" + QString::number(i + 1)));
		ui.XaosTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
		ui.XaosTable->verticalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	}

	ui.XaosTable->resizeRowsToContents();
	ui.XaosTable->resizeColumnsToContents();

	w = SetTabOrder(this, w, ui.ClearXaosButton);
	w = SetTabOrder(this, w, ui.RandomXaosButton);
}

/// <summary>
/// Clear all xaos from the current ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearXaos()
{
	Update([&] { m_Ember.ClearXaos(); });
	FillXaos();
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
	Update([&]
	{
		for (size_t i = 0; i < m_Ember.XformCount(); i++)
		{
			if (auto* xform = m_Ember.GetXform(i))
			{
				for (size_t j = 0; j < m_Ember.XformCount(); j++)
				{
					if (m_Rand.RandBit())
						xform->SetXaos(j, T(m_Rand.RandBit()));
					else
						xform->SetXaos(j, m_Rand.Frand<T>(0, 3));
				}
			}
		}
	});
	
	FillXaos();
}

void Fractorium::OnRandomXaosButtonClicked(bool checked) { m_Controller->RandomXaos(); }

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
