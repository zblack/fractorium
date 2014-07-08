#include "FractoriumPch.h"
#include "SpinBox.h"

/// <summary>
/// Constructor that passes parent to the base and sets up height and step.
/// Specific focus policy is used to allow the user to hover over the control
/// and change its value using the mouse wheel without explicitly having to click
/// inside of it.
/// </summary>
/// <param name="parent">The parent widget. Default: NULL.</param>
/// <param name="height">The height of the spin box. Default: 16.</param>
/// <param name="step">The step used to increment/decrement the spin box when using the mouse wheel. Default: 1.</param>
SpinBox::SpinBox(QWidget* parent, int height, int step)
	: QSpinBox(parent)
{
	m_Select = false;
	m_DoubleClick = false;
	m_DoubleClickNonZero = 0;
	m_DoubleClickZero = 1;
	m_Step = step;
	m_SmallStep = 1;
	setSingleStep(step);
	setFrame(false);
	setButtonSymbols(QAbstractSpinBox::NoButtons);
	setFocusPolicy(Qt::StrongFocus);
	setMinimumHeight(height);//setGeometry() has no effect, so set both of these instead.
	setMaximumHeight(height);
	lineEdit()->installEventFilter(this);
	lineEdit()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)), Qt::QueuedConnection);
}

/// <summary>
/// Set the value of the control without triggering signals.
/// </summary>
/// <param name="d">The value to set it to</param>
void SpinBox::SetValueStealth(int d)
{
	blockSignals(true);
	setValue(d);
	blockSignals(false);
}

/// <summary>
/// Set whether to respond to double click events.
/// </summary>
/// <param name="b">True if this should respond to double click events, else false.</param>
void SpinBox::DoubleClick(bool b)
{
	m_DoubleClick = b;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains zero.
/// </summary>
/// <param name="val">The value to be used</param>
void SpinBox::DoubleClickZero(int val)
{
	m_DoubleClickZero = val;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains a non-zero value.
/// </summary>
/// <param name="val">The value to be used</param>
void SpinBox::DoubleClickNonZero(int val)
{
	m_DoubleClickNonZero = val;
}

/// <summary>
/// Set the small step to be used when the user holds down shift while scrolling.
/// The default is step / 10, so use this if something else is needed.
/// </summary>
/// <param name="step">The small step to use for scrolling while the shift key is down</param>
void SpinBox::SmallStep(int step)
{
	m_SmallStep = min(1, step);
}

/// <summary>
/// Expose the underlying QLineEdit control to the caller.
/// </summary>
/// <returns>A pointer to the QLineEdit</returns>
QLineEdit* SpinBox::lineEdit()
{
	return QSpinBox::lineEdit();
}

/// <summary>
/// Another workaround for the persistent text selection bug in Qt.
/// </summary>
void SpinBox::onSpinBoxValueChanged(int i)
{
	lineEdit()->deselect();//Gets rid of nasty "feature" that always has text selected.
}

/// <summary>
/// Event filter for taking special action on double click events.
/// </summary>
/// <param name="o">The object</param>
/// <param name="e">The eevent</param>
/// <returns>false</returns>
bool SpinBox::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QMouseEvent::MouseButtonPress && isEnabled())
	{
		QPoint pt;

		if (QMouseEvent* me = (QMouseEvent*)e)
			pt = me->localPos().toPoint();

		int pos = lineEdit()->cursorPositionAt(pt);

		if (lineEdit()->selectedText() != "")
		{
			lineEdit()->deselect();
			lineEdit()->setCursorPosition(pos);
			return true;
		}
		else if (m_Select)
		{
			lineEdit()->setCursorPosition(pos);
			selectAll();
			m_Select = false;
			return true;
		}
	}
	else if (m_DoubleClick && e->type() == QMouseEvent::MouseButtonDblClick && isEnabled())
	{
		if (value() == 0)
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
			if (QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(e))
			{
				Qt::KeyboardModifiers mod = wheelEvent->modifiers();

				if (mod.testFlag(Qt::ShiftModifier))
					setSingleStep(m_SmallStep);
				else
					setSingleStep(m_Step);
			}
		}
	}

	return false;
}

/// <summary>
/// Called when focus enters the spinner.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::focusInEvent(QFocusEvent* e)
{
	lineEdit()->setReadOnly(false);
	QSpinBox::focusInEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Qt has a nasty "feature" that leaves the text in a spinner selected
/// and the cursor visible, regardless of whether it has the focus.
/// Manually clear both here.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::focusOutEvent(QFocusEvent* e)
{
	 lineEdit()->deselect();//Clear selection when leaving.
	 lineEdit()->setReadOnly(true);//Clever hack to clear the cursor when leaving.
	 QSpinBox::focusOutEvent(e);
}

/// <summary>
/// Called when focus enters the spinner.
/// Must set the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::enterEvent(QEvent* e)
{
	m_Select = true;
	setFocus();
	QSpinBox::enterEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Must clear the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::leaveEvent(QEvent* e)
{
	m_Select = false;
	clearFocus();
	QSpinBox::leaveEvent(e);
}
