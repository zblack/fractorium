#pragma once

#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// TwoButtonWidget and SpinnerButtonWidget classes.
/// </summary>

/// <summary>
/// Thin container that is both a widget and a container of two QPushButtons.
/// Used for when a layout expects a single widget, but two need to go in its place.
/// The buttons are public so the caller can easily use them individually.
/// </summary>
class TwoButtonWidget : public QWidget
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
	/// <param name="parent">The parent widget</param>
	TwoButtonWidget(QString caption1, QString caption2, int w1, int w2, int h, QWidget* parent)
		: QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		m_Button1 = new QPushButton(caption1, parent);
		m_Button2 = new QPushButton(caption2, parent);

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
 
		layout->addWidget(m_Button1);
		layout->addWidget(m_Button2);
		layout->setAlignment(Qt::AlignLeft);
		layout->setMargin(0);
		layout->setSpacing(2);

		setLayout(layout);
	}

	QPushButton* m_Button1;
	QPushButton* m_Button2;
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
	/// <param name="parent">The parent widget</param>
	SpinnerButtonWidget(DoubleSpinBox* spinBox, QString buttonCaption, int w, int h, QWidget* parent)
		: QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		m_Button = new QPushButton(buttonCaption, parent);
		m_SpinBox = spinBox;

		if (w != -1)
		{
			m_Button->setMinimumWidth(w);
			m_Button->setMaximumWidth(w);
		}

		m_Button->setMinimumHeight(h);
		m_Button->setMaximumHeight(h);
 
		layout->addWidget(spinBox);
		layout->addWidget(m_Button);
		layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		layout->setMargin(0);
		layout->setSpacing(0);

		setLayout(layout);
	}

	DoubleSpinBox* m_SpinBox;
	QPushButton* m_Button;
};
