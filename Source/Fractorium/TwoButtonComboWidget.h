#pragma once

#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// TwoButtonComboWidget and SpinnerButtonWidget classes.
/// </summary>

/// <summary>
/// Thin container that is both a widget and a container of two QPushButtons.
/// Used for when a layout expects a single widget, but two need to go in its place.
/// The buttons are public so the caller can easily use them individually.
/// </summary>
class TwoButtonComboWidget : public QWidget
{
	Q_OBJECT

public:
	/// <summary>
	/// Constructor that passes the parent to the base, then creates two QPushButtons,
	/// and sets up their captions and dimensions.
	/// </summary>
	/// <param name="caption1">The caption of the first button</param>
	/// <param name="caption2">The caption of the second button</param>
	/// <param name="w1">The width of the first button</param>
	/// <param name="w2">The width of the second button</param>
	/// <param name="h">The height of both buttons</param>
	/// <param name="p">The parent widget</param>
	TwoButtonComboWidget(const QString& caption1, const QString& caption2, QStringList comboStrings, int w1, int w2, int h, QWidget* p)
		: QWidget(p)
	{
		QHBoxLayout* l = new QHBoxLayout(this);
		m_Button1 = new QPushButton(caption1, p);
		m_Button2 = new QPushButton(caption2, p);
		m_Combo = new QComboBox(p);

		m_Combo->addItems(comboStrings);

		if (w1 != -1)
		{
			m_Button1->setMinimumWidth(w1);
			m_Button1->setMaximumWidth(w1);
		}

		if (w2 != -1)
		{
			m_Button2->setMinimumWidth(w2);
			m_Button2->setMaximumWidth(w2);
		}

		m_Button1->setMinimumHeight(h);
		m_Button1->setMaximumHeight(h);
		m_Button2->setMinimumHeight(h);
		m_Button2->setMaximumHeight(h);
		m_Combo->setMinimumHeight(h - 3);
		m_Combo->setMaximumHeight(h - 3);
 
		l->addWidget(m_Combo);
		l->addWidget(m_Button1);
		l->addWidget(m_Button2);
		l->setAlignment(Qt::AlignLeft);
		l->setMargin(0);
		l->setSpacing(2);

		setLayout(l);
	}

	QPushButton* m_Button1;
	QPushButton* m_Button2;
	QComboBox* m_Combo;
};

/// <summary>
/// Thin container that is both a widget and a container of one DoubleSpinBox and one QPushButton.
/// Used for when a layout expects a single widget, but two need to go in its place.
/// The widgets are public so the caller can easily use them individually.
/// </summary>
class SpinnerButtonWidget : public QWidget
{
	Q_OBJECT

public:
	/// <summary>
	/// Constructor that passes the parent to the base, then creates a QPushButton and
	/// sets up its caption and dimensions, then assigns the DoubleSpinBox.
	/// </summary>
	/// <param name="spinBox">The pre-created DoubleSpinBox</param>
	/// <param name="buttonCaption">The caption of the button</param>
	/// <param name="w">The width of the button</param>
	/// <param name="h">The height of the button</param>
	/// <param name="p">The parent widget</param>
	SpinnerButtonWidget(DoubleSpinBox* spinBox, QString buttonCaption, int w, int h, QWidget* p)
		: QWidget(p)
	{
		QHBoxLayout* l = new QHBoxLayout(this);
		m_Button = new QPushButton(buttonCaption, p);
		m_SpinBox = spinBox;

		if (w != -1)
		{
			m_Button->setMinimumWidth(w);
			m_Button->setMaximumWidth(w);
		}

		m_Button->setMinimumHeight(h);
		m_Button->setMaximumHeight(h);
 
		l->addWidget(spinBox);
		l->addWidget(m_Button);
		l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		l->setMargin(0);
		l->setSpacing(0);

		setLayout(l);
	}

	DoubleSpinBox* m_SpinBox;
	QPushButton* m_Button;
};
