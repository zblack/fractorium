#pragma once

#include "FractoriumPch.h"

/// <summary>
/// Fractorium global utility functions.
/// </summary>

/// <summary>
/// Setup a spinner to be placed in a table cell.
/// Due to a serious compiler bug in MSVC, this must be declared as an outside function instead of a static member of Fractorium.
/// The reason is that the default arguments of type valType will not be interpreted correctly by the compiler.
/// If the bug is ever fixed, put it back as a static member function.
/// </summary>
/// <param name="table">The table the spinner belongs to</param>
/// <param name="receiver">The receiver object</param>
/// <param name="row">The row in the table where this spinner resides</param>
/// <param name="col">The col in the table where this spinner resides</param>
/// <param name="spinBox">Double pointer to spin box which will hold the spinner upon exit</param>
/// <param name="height">The height of the spinner</param>
/// <param name="min">The minimum value of the spinner</param>
/// <param name="max">The maximum value of the spinner</param>
/// <param name="step">The step of the spinner</param>
/// <param name="signal">The signal the spinner emits</param>
/// <param name="slot">The slot to receive the signal</param>
/// <param name="incRow">Whether to increment the row value</param>
/// <param name="val">The default value for the spinner</param>
/// <param name="doubleClickZero">When the spinner has a value of zero and is double clicked, assign this value</param>
/// <param name="doubleClickNonZero">When the spinner has a value of non-zero and is double clicked, assign this value</param>
template<typename spinType, typename valType>
static void SetupSpinner(QTableWidget* table, const QObject* receiver, int& row, int col, spinType*& spinBox, int height, valType min, valType max, valType step, const char* signal, const char* slot, bool incRow = true, valType val = 0, valType doubleClickZero = -999, valType doubleClickNonZero = -999)
{
	spinBox = new spinType(table, height, step);
	spinBox->setRange(min, max);
	spinBox->setValue(val);

	if (col >= 0)
		table->setCellWidget(row, col, spinBox);

	if (string(signal) != "" && string(slot) != "")
		receiver->connect(spinBox, signal, receiver, slot, Qt::QueuedConnection);

	if (doubleClickNonZero != -999 && doubleClickZero != -999)
	{
		spinBox->DoubleClick(true);
		spinBox->DoubleClickZero(valType(doubleClickZero));
		spinBox->DoubleClickNonZero(valType(doubleClickNonZero));
	}

	if (incRow)
		row++;
}

/// <summary>
/// Wrapper around QWidget::setTabOrder() to return the second widget.
/// This makes it easy to chain multiple calls without having to retype
/// all of them if the order changes or if a new widget is inserted.
/// </summary>
/// <param name="p">The parent widget that w1 and w2 belong to</param>
/// <param name="w1">The widget to come first in the tab order</param>
/// <param name="w2">The widget to come second in the tab order</param>
static QWidget* SetTabOrder(QWidget* p, QWidget* w1, QWidget* w2)
{
	p->setTabOrder(w1, w2);
	return w2;
}

/// <summary>
/// Wrapper around QLocale::system().toDouble().
/// </summary>
/// <param name="s">The string to convert</param>
/// <param name="ok">Pointer to boolean which stores the success value of the conversion</param>
/// <returns>The converted value if successful, else 0.</returns>
static double ToDouble(const QString &s, bool *ok)
{
	return QLocale::system().toDouble(s, ok);
}

/// <summary>
/// Wrapper around QLocale::system().toString().
/// </summary>
/// <param name="s">The value to convert</param>
/// <returns>The string value if successful, else "".</returns>
template <typename T>
static QString ToString(T val)
{
	return QLocale::system().toString(val);
}

/// <summary>
/// Force a QString to end with the specified value.
/// </summary>
/// <param name="s">The string to append a suffix to</param>
/// <param name="e">The suffix to append</param>
/// <returns>The original string value if it already ended in e, else the original value appended with e.</returns>
template <typename T>
static QString MakeEnd(const QString& s, T e)
{
	if (!s.endsWith(e))
		return s + e;
	else
		return s;
}

/// <summary>
/// Check if a path is not empty and exists on the file system.
/// </summary>
/// <param name="s">The path to check</param>
/// <returns>True if s was not empty and existed, else false.</returns>
static bool Exists(const QString& s)
{
	 return s != "" && QDir(s).exists();
}
