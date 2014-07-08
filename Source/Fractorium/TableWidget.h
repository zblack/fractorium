#pragma once

#include "FractoriumPch.h"

/// <summary>
/// TableWidget class.
/// </summary>

/// <summary>
/// The entire purpose for this subclass is to overcome a glaring flaw
/// in the way Qt handles table drawing.
/// For most of the tables Fractorium uses, it draw the grid lines. Qt draws them
/// in a very naive manner, whereby it draws lines above the first row and below
/// the last row. It also draws to the left of the first column and to the right
/// of the last column. This has the effect of putting an additional border inside
/// of the specified border. This extra border becomes very noticeable when changing
/// the background color of a cell.
/// The workaround is to scrunch the size of the table up by one pixel. However,
/// since the viewable area is then smaller than the size of the table, it will scroll
/// by one pixel if the mouse is hovered over the table and the user moves the mouse wheel.
/// This subclass is done solely to filter out the mouse wheel event.
/// Note that this filtering only applies to the table as a whole, which means
/// mouse wheel events still get properly routed to spinners.
/// </summary>
class TableWidget : public QTableWidget
{
	 Q_OBJECT
public:
	/// <summary>
	/// Constructor that passes the parent to the base and installs
	/// the event filter.
	/// </summary>
	/// <param name="parent">The parent widget</param>
	explicit TableWidget(QWidget* parent = 0)
		: QTableWidget(parent)
	{
		viewport()->installEventFilter(this);
	}

protected:
	/// <summary>
	/// Event filter to ignore mouse wheel events.
	/// </summary>
	/// <param name="obj">The object sending the event</param>
	/// <param name="e">The event</param>
	/// <returns>True if mouse wheel, else return the result of calling the base fucntion.</returns>
	bool eventFilter(QObject* obj, QEvent* e)
	{
		if(e->type() == QEvent::Wheel)
		{
			e->ignore();
			return true;
		}

		return QTableWidget::eventFilter(obj, e);
	}
};