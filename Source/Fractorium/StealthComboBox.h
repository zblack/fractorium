#pragma once

#include "FractoriumPch.h"

/// <summary>
/// StealthComboBox class.
/// </summary>

/// <summary>
/// A thin derivation of QComboBox which allows the user
/// to set the index without triggering signals.
/// </summary>
class StealthComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit StealthComboBox(QWidget* parent = 0) : QComboBox(parent) { }
	
	/// <summary>
	/// Set the current index of the combo box without triggering signals.
	/// </summary>
	/// <param name="index">The current index to set</param>
	void StealthComboBox::SetCurrentIndexStealth(int index)
	{
		blockSignals(true);
		setCurrentIndex(index);
		blockSignals(false);
	}
};
