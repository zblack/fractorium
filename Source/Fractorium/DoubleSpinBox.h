#pragma once

#include "FractoriumPch.h"

/// <summary>
/// DoubleSpinBox and VariationTreeDoubleSpinBox classes.
/// </summary>

/// <summary>
/// A derivation to prevent the spin box from selecting its own text
/// when editing. Also to prevent multiple spin boxes from all having
/// selected text at once.
/// </summary>
class DoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	explicit DoubleSpinBox(QWidget* parent = 0, int height = 16, double step = 0.05);
	virtual ~DoubleSpinBox() { }
	void SetValueStealth(double d);
	void DoubleClick(bool b);
	void DoubleClickZero(double val);
	void DoubleClickNonZero(double val);
	void Step(double step);
	void SmallStep(double step);
	QLineEdit* lineEdit();

public slots:
	void onSpinBoxValueChanged(double d);

protected:
	bool eventFilter(QObject* o, QEvent* e);
	virtual void focusInEvent(QFocusEvent* e);
	virtual void focusOutEvent(QFocusEvent* e);
	virtual void enterEvent(QEvent* e);
	virtual void leaveEvent(QEvent* e);

private:
	bool m_Select;
	bool m_DoubleClick;
	double m_DoubleClickNonZero;
	double m_DoubleClickZero;
	double m_Step;
	double m_SmallStep;
};

/// <summary>
/// Derivation for the double spin boxes that are in the
/// variations tree.
/// </summary>
template <typename T>
class VariationTreeDoubleSpinBox : public DoubleSpinBox
{
public:
	/// <summary>
	/// Constructor that passes agruments to the base and assigns the m_Param and m_Variation members.
	/// </summary>
	/// <param name="parent">The parent widget</param>
	/// <param name="var">The variation this spinner is for</param>
	/// <param name="param">The name of the parameter this is for</param>
	/// <param name="height">The height of the spin box. Default: 16.</param>
	/// <param name="step">The step used to increment/decrement the spin box when using the mouse wheel. Default: 0.05.</param>
	explicit VariationTreeDoubleSpinBox(QWidget* parent, Variation<T>* var, string param, int height = 16, double step = 0.05)
		: DoubleSpinBox(parent, height, step)
	{
		m_Param = param;
		m_Variation = var;
		setDecimals(3);
	}

	virtual ~VariationTreeDoubleSpinBox() { }
	bool IsParam() { return !m_Param.empty(); }
	string ParamName() { return m_Param; }
	Variation<T>* GetVariation() { return m_Variation; }

private:
	string m_Param;
	Variation<T>* m_Variation;
};
