#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// Constructor that passes parent to the base and sets up height and step.
/// Specific focus policy is used to allow the user to hover over the control
/// and change its value using the mouse wheel without explicitly having to click
/// inside of it.
/// </summary>
/// <param name="p">The parent widget. Default: NULL.</param>
/// <param name="height">The height of the spin box. Default: 16.</param>
/// <param name="step">The step used to increment/decrement the spin box when using the mouse wheel. Default: 0.05.</param>
DoubleSpinBox::DoubleSpinBox(QWidget* p, int h, double step)
	: QDoubleSpinBox(p)
{
	m_Select = false;
	m_DoubleClick = false;
	m_DoubleClickNonZero = 0;
	m_DoubleClickZero = 1;
	m_Step = step;
	m_SmallStep = step / 10.0;
	setSingleStep(step);
	setFrame(false);
	setButtonSymbols(QAbstractSpinBox::NoButtons);
	setFocusPolicy(Qt::StrongFocus);
	setMinimumHeight(h);//setGeometry() has no effect, so must set both of these instead.
	setMaximumHeight(h);
	lineEdit()->installEventFilter(this);
	lineEdit()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	connect(this, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged(double)), Qt::QueuedConnection);

}

/// <summary>
/// Set the value of the control without triggering signals.
/// </summary>
/// <param name="d">The value to set it to</param>
void DoubleSpinBox::SetValueStealth(double d)
{
	blockSignals(true);
	setValue(d);
	blockSignals(false);
}

/// <summary>
/// Set whether to respond to double click events.
/// </summary>
/// <param name="b">True if this should respond to double click events, else false.</param>
void DoubleSpinBox::DoubleClick(bool b)
{
	m_DoubleClick = b;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains zero.
/// </summary>
/// <param name="val">The value to be used</param>
void DoubleSpinBox::DoubleClickZero(double val)
{
	m_DoubleClickZero = val;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains a non-zero value.
/// </summary>
/// <param name="val">The value to be used</param>
void DoubleSpinBox::DoubleClickNonZero(double val)
{
	m_DoubleClickNonZero = val;
}

/// <summary>
/// Set the default step to be used when the user scrolls.
/// </summary>
/// <param name="step">The step to use for scrolling</param>
void DoubleSpinBox::Step(double step)
{
	m_Step = step;
}

/// <summary>
/// Set the small step to be used when the user holds down shift while scrolling.
/// The default is step / 10, so use this if something else is needed.
/// </summary>
/// <param name="step">The small step to use for scrolling while the shift key is down</param>
void DoubleSpinBox::SmallStep(double step)
{
	m_SmallStep = step;
}

/// <summary>
/// Expose the underlying QLineEdit control to the caller.
/// </summary>
/// <returns>A pointer to the QLineEdit</returns>
QLineEdit* DoubleSpinBox::lineEdit()
{
	return QDoubleSpinBox::lineEdit();
}

/// <summary>
/// Another workaround for the persistent text selection bug in Qt.
/// </summary>
void DoubleSpinBox::onSpinBoxValueChanged(double d)
{
	lineEdit()->deselect();//Gets rid of nasty "feature" that always has text selected.
}

/// <summary>
/// Event filter for taking special action on double click events.
/// </summary>
/// <param name="o">The object</param>
/// <param name="e">The eevent</param>
/// <returns>false</returns>
bool DoubleSpinBox::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QMouseEvent::MouseButtonPress && isEnabled())
	{
	//	QPoint pt;
	//
	//	if (QMouseEvent* me = (QMouseEvent*)e)
	//		pt = me->localPos().toPoint();
	//
	//	int pos = lineEdit()->cursorPositionAt(pt);
	//
	//	if (lineEdit()->selectedText() != "")
	//	{
	//		lineEdit()->deselect();
	//		lineEdit()->setCursorPosition(pos);
	//		return true;
	//	}
	//	else if (m_Select)
	//	{
	//		lineEdit()->setCursorPosition(pos);
	//		selectAll();
	//		m_Select = false;
	//		return true;
	//	}
	}
	else if (m_DoubleClick && e->type() == QMouseEvent::MouseButtonDblClick && isEnabled())
	{
		if (IsNearZero(value()))
			setValue(m_DoubleClickZero);
		else
			setValue(m_DoubleClickNonZero);
	}
	else
	{
		if (e->type() == QEvent::Wheel)
		{
			//Take special action for shift to reduce the scroll amount. Control already
			//increases it automatically.
			if (QWheelEvent* we = dynamic_cast<QWheelEvent*>(e))
			{
				Qt::KeyboardModifiers mod = we->modifiers();

				if (mod.testFlag(Qt::ShiftModifier))
					setSingleStep(m_SmallStep);
				else
					setSingleStep(m_Step);
			}
		}
	}

	return QDoubleSpinBox::eventFilter(o, e);
}

/// <summary>
/// Called when focus enters the spinner.
/// </summary>
/// <param name="e">The event</param>
void DoubleSpinBox::focusInEvent(QFocusEvent* e)
{
	//lineEdit()->setReadOnly(false);
	QDoubleSpinBox::focusInEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Qt has a nasty "feature" that leaves the text in a spinner selected
/// and the cursor visible, regardless of whether it has the focus.
/// Manually clear both here.
/// </summary>
/// <param name="e">The event</param>
void DoubleSpinBox::focusOutEvent(QFocusEvent* e)
{
	 //lineEdit()->deselect();//Clear selection when leaving.
	 //lineEdit()->setReadOnly(true);//Clever hack to clear the cursor when leaving.
	 QDoubleSpinBox::focusOutEvent(e);
}

/// <summary>
/// Called when focus enters the spinner.
/// Must set the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void DoubleSpinBox::enterEvent(QEvent* e)
{
	//m_Select = true;
	//setFocus();
	QDoubleSpinBox::enterEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Must clear the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void DoubleSpinBox::leaveEvent(QEvent* e)
{
	//m_Select = false;
	//clearFocus();
	QDoubleSpinBox::leaveEvent(e);
}
