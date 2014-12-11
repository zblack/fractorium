#pragma once

#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// VariationTreeWidgetItem class.
/// </summary>

/// <summary>
/// A derivation of QTreeWidgetItem which helps us with sorting.
/// This is used when the user chooses to sort the variations tree
/// by index or by weight. It supports weights less than, equal to, or
/// greater than zero.
/// </summary>
template <typename T>
class VariationTreeWidgetItem : public QTreeWidgetItem
{
public:
	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidget as the parent
	/// and passes it to the base.
	/// </summary>
	/// <param name="id">The ID of the variation this widget will represent</param>
	/// <param name="p">The parent widget</param>
	VariationTreeWidgetItem(eVariationId id, QTreeWidget* p = 0)
		: QTreeWidgetItem(p)
	{
		m_Id = id;
	}

	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidgetItem as the parent
	/// and passes it to the base.
	/// This is used for making sub items for parametric variation parameters.
	/// </summary>
	/// <param name="id">The ID of the variation this widget will represent</param>
	/// <param name="p">The parent widget</param>
	VariationTreeWidgetItem(eVariationId id, QTreeWidgetItem* p = 0)
		: QTreeWidgetItem(p)
	{
		m_Id = id;
	}

	virtual ~VariationTreeWidgetItem() { }
	eVariationId Id() { return m_Id; }

private:
	/// <summary>
	/// Less than operator used for sorting.
	/// </summary>
	/// <param name="other">The QTreeWidgetItem to compare against for sorting</param>
	/// <returns>True if this is less than other, else false.</returns>
	bool operator < (const QTreeWidgetItem& other) const
	{
		int column = treeWidget()->sortColumn();
		eVariationId index1, index2;
		double weight1 = 0, weight2 = 0;
		VariationTreeWidgetItem<T>* varItemWidget;
		VariationTreeDoubleSpinBox<T>* spinBox1, *spinBox2;

		QWidget* itemWidget1 = treeWidget()->itemWidget(const_cast<VariationTreeWidgetItem<T>*>(this), 1);//Get the widget for the second column.
		
		if ((spinBox1 = dynamic_cast<VariationTreeDoubleSpinBox<T>*>(itemWidget1)))//Cast the widget to the VariationTreeDoubleSpinBox type.
		{
			QWidget* itemWidget2 = treeWidget()->itemWidget(const_cast<QTreeWidgetItem*>(&other), 1);//Get the widget for the second column of the widget item passed in.
			
			if ((spinBox2 = dynamic_cast<VariationTreeDoubleSpinBox<T>*>(itemWidget2)))//Cast the widget to the VariationTreeDoubleSpinBox type.
			{
				if (spinBox1->IsParam() || spinBox2->IsParam())//Do not sort params, their order will always remain the same.
					return false;

				weight1 = spinBox1->value();
				weight2 = spinBox2->value();
				index1 = spinBox1->GetVariation()->VariationId();
				index2 = spinBox2->GetVariation()->VariationId();

				if (column == 0)//First column clicked, sort by variation index.
				{
					return index1 < index2;
				}
				else if (column == 1)//Second column clicked, sort by weight.
				{
					if (IsNearZero(weight1) && IsNearZero(weight2))
						return index1 > index2;
					else
						return fabs(weight1) < fabs(weight2);
				}
			}
		}
		
		return false;
	}

	eVariationId m_Id;
};
