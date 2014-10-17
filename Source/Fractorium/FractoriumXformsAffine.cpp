#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms affine UI.
/// </summary>
void Fractorium::InitXformsAffineUI()
{
	int row = 0, affinePrec = 6, spinHeight = 20;
	double affineStep = 0.01, affineMin = std::numeric_limits<double>::lowest(), affineMax = std::numeric_limits<double>::max();
	QTableWidget* table = ui.PreAffineTable;

	SetFixedTableHeader(table->horizontalHeader(), QHeaderView::Stretch);//The designer continually clobbers these values, so must manually set them here.
	SetFixedTableHeader(table->verticalHeader());
	
	//Pre affine spinners.
	SetupAffineSpinner(table, this, 0, 0, m_PreX1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX1Changed(double)));
	SetupAffineSpinner(table, this, 0, 1, m_PreX2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX2Changed(double)));
	SetupAffineSpinner(table, this, 1, 0, m_PreY1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY1Changed(double)));
	SetupAffineSpinner(table, this, 1, 1, m_PreY2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY2Changed(double)));
	SetupAffineSpinner(table, this, 2, 0, m_PreO1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO1Changed(double)));
	SetupAffineSpinner(table, this, 2, 1, m_PreO2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO2Changed(double)));

	table = ui.PostAffineTable;
	SetFixedTableHeader(table->horizontalHeader(), QHeaderView::Stretch);//The designer continually clobbers these values, so must manually set them here.
	SetFixedTableHeader(table->verticalHeader());
	
	//Post affine spinners.
	SetupAffineSpinner(table, this, 0, 0, m_PostX1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX1Changed(double)));
	SetupAffineSpinner(table, this, 0, 1, m_PostX2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX2Changed(double)));
	SetupAffineSpinner(table, this, 1, 0, m_PostY1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY1Changed(double)));
	SetupAffineSpinner(table, this, 1, 1, m_PostY2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY2Changed(double)));
	SetupAffineSpinner(table, this, 2, 0, m_PostO1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO1Changed(double)));
	SetupAffineSpinner(table, this, 2, 1, m_PostO2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO2Changed(double)));

	QDoubleValidator* preRotateVal = new QDoubleValidator(ui.PreRotateCombo); preRotateVal->setLocale(QLocale::system());
	QDoubleValidator* preMoveVal   = new QDoubleValidator(ui.PreMoveCombo);   preMoveVal->setLocale(QLocale::system());
	QDoubleValidator* preScaleVal  = new QDoubleValidator(ui.PreScaleCombo);  preScaleVal->setLocale(QLocale::system());

	QDoubleValidator* postRotateVal = new QDoubleValidator(ui.PostRotateCombo); postRotateVal->setLocale(QLocale::system());
	QDoubleValidator* postMoveVal   = new QDoubleValidator(ui.PostMoveCombo);   postMoveVal->setLocale(QLocale::system());
	QDoubleValidator* postScaleVal  = new QDoubleValidator(ui.PostScaleCombo);  postScaleVal->setLocale(QLocale::system());

	ui.PreRotateCombo->setValidator(preRotateVal);
	ui.PreMoveCombo->setValidator(preMoveVal);
	ui.PreScaleCombo->setValidator(preScaleVal);

	ui.PostRotateCombo->setValidator(postRotateVal);
	ui.PostMoveCombo->setValidator(postMoveVal);
	ui.PostScaleCombo->setValidator(postScaleVal);

	connect(ui.PreFlipHorizontalButton,    SIGNAL(clicked(bool)), this, SLOT(OnFlipHorizontalButtonClicked(bool)),			  Qt::QueuedConnection);
	connect(ui.PreFlipVerticalButton,      SIGNAL(clicked(bool)), this, SLOT(OnFlipVerticalButtonClicked(bool)),			  Qt::QueuedConnection);
	connect(ui.PreRotate90CButton,         SIGNAL(clicked(bool)), this, SLOT(OnRotate90CButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreRotate90CcButton,        SIGNAL(clicked(bool)), this, SLOT(OnRotate90CcButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreRotateCButton,           SIGNAL(clicked(bool)), this, SLOT(OnRotateCButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PreRotateCcButton,          SIGNAL(clicked(bool)), this, SLOT(OnRotateCcButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreMoveUpButton,            SIGNAL(clicked(bool)), this, SLOT(OnMoveUpButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PreMoveDownButton,          SIGNAL(clicked(bool)), this, SLOT(OnMoveDownButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreMoveLeftButton,          SIGNAL(clicked(bool)), this, SLOT(OnMoveLeftButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreMoveRightButton,         SIGNAL(clicked(bool)), this, SLOT(OnMoveRightButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreScaleDownButton,         SIGNAL(clicked(bool)), this, SLOT(OnScaleDownButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PreScaleUpButton,           SIGNAL(clicked(bool)), this, SLOT(OnScaleUpButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PreResetButton,             SIGNAL(clicked(bool)), this, SLOT(OnResetAffineButtonClicked(bool)),				  Qt::QueuedConnection);

	connect(ui.PostFlipHorizontalButton,   SIGNAL(clicked(bool)), this, SLOT(OnFlipHorizontalButtonClicked(bool)),			  Qt::QueuedConnection);
	connect(ui.PostFlipVerticalButton,     SIGNAL(clicked(bool)), this, SLOT(OnFlipVerticalButtonClicked(bool)),			  Qt::QueuedConnection);
	connect(ui.PostRotate90CcButton,       SIGNAL(clicked(bool)), this, SLOT(OnRotate90CcButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostRotateCcButton,         SIGNAL(clicked(bool)), this, SLOT(OnRotateCcButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostRotateCButton,          SIGNAL(clicked(bool)), this, SLOT(OnRotateCButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PostRotate90CButton,        SIGNAL(clicked(bool)), this, SLOT(OnRotate90CButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostMoveUpButton,           SIGNAL(clicked(bool)), this, SLOT(OnMoveUpButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PostMoveDownButton,         SIGNAL(clicked(bool)), this, SLOT(OnMoveDownButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostMoveLeftButton,         SIGNAL(clicked(bool)), this, SLOT(OnMoveLeftButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostMoveRightButton,        SIGNAL(clicked(bool)), this, SLOT(OnMoveRightButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostScaleDownButton,        SIGNAL(clicked(bool)), this, SLOT(OnScaleDownButtonClicked(bool)),				  Qt::QueuedConnection);
	connect(ui.PostScaleUpButton,          SIGNAL(clicked(bool)), this, SLOT(OnScaleUpButtonClicked(bool)),					  Qt::QueuedConnection);
	connect(ui.PostResetButton,            SIGNAL(clicked(bool)), this, SLOT(OnResetAffineButtonClicked(bool)),				  Qt::QueuedConnection);

	connect(ui.PreAffineGroupBox,		   SIGNAL(toggled(bool)), this, SLOT(OnAffineGroupBoxToggled(bool)),				  Qt::QueuedConnection);
	connect(ui.PostAffineGroupBox,		   SIGNAL(toggled(bool)), this, SLOT(OnAffineGroupBoxToggled(bool)),				  Qt::QueuedConnection);
	
	connect(ui.ShowPreAffineAllRadio,      SIGNAL(toggled(bool)), this, SLOT(OnAffineDrawAllCurrentRadioButtonToggled(bool)), Qt::QueuedConnection);
	connect(ui.ShowPreAffineCurrentRadio,  SIGNAL(toggled(bool)), this, SLOT(OnAffineDrawAllCurrentRadioButtonToggled(bool)), Qt::QueuedConnection);
	connect(ui.ShowPostAffineAllRadio,     SIGNAL(toggled(bool)), this, SLOT(OnAffineDrawAllCurrentRadioButtonToggled(bool)), Qt::QueuedConnection);
	connect(ui.ShowPostAffineCurrentRadio, SIGNAL(toggled(bool)), this, SLOT(OnAffineDrawAllCurrentRadioButtonToggled(bool)), Qt::QueuedConnection);

	ui.PostAffineGroupBox->setChecked(true);//Flip it once to force the disabling of the group box.
	ui.PostAffineGroupBox->setChecked(false);
}

/// <summary>
/// Helper for setting the value of a single pre/post affine coefficient.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The value to set</param>
/// <param name="index">The index to set, 0-5, corresponding to a, b, c, d, e, f</param>
/// <param name="pre">True if pre affine, false if post affine.</param>
template <typename T>
void FractoriumEmberController<T>::AffineSetHelper(double d, int index, bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		QObject* objSender = m_Fractorium->sender();
		DoubleSpinBox* spinBox = dynamic_cast<DoubleSpinBox*>(objSender);

		if (spinBox)
		{
			Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;

			switch (index)
			{
				case 0:
					affine->A(spinBox->value());
					break;
				case 1:
					affine->B(spinBox->value());
					break;
				case 2:
					affine->C(spinBox->value());
					break;
				case 3:
					affine->D(spinBox->value());
					break;
				case 4:
					affine->E(spinBox->value());
					break;
				case 5:
					affine->F(spinBox->value());
					break;
			}
		}
	});
}

/// <summary>
/// Pre and post affine spinner changed events.
/// Resets the rendering process.
/// </summary>
void Fractorium::OnX1Changed(double d) { m_Controller->AffineSetHelper(d, 0, sender() == m_PreX1Spin); }
void Fractorium::OnX2Changed(double d) { m_Controller->AffineSetHelper(d, 3, sender() == m_PreX2Spin); }
void Fractorium::OnY1Changed(double d) { m_Controller->AffineSetHelper(d, 1, sender() == m_PreY1Spin); }
void Fractorium::OnY2Changed(double d) { m_Controller->AffineSetHelper(d, 4, sender() == m_PreY2Spin); }
void Fractorium::OnO1Changed(double d) { m_Controller->AffineSetHelper(d, 2, sender() == m_PreO1Spin); }
void Fractorium::OnO2Changed(double d) { m_Controller->AffineSetHelper(d, 5, sender() == m_PreO2Spin); }

/// <summary>
/// Flip the current pre/post affine vertically and/or horizontally.
/// Resets the rendering process.
/// </summary>
/// <param name="horizontal">True to flip horizontally</param>
/// <param name="vertical">True to flip vertically</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::FlipCurrentXform(bool horizontal, bool vertical, bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;

		if (horizontal)
		{
			affine->A(-affine->A());
			affine->B(-affine->B());

			if (!m_Fractorium->LocalPivot())
				affine->C(-affine->C());
		}

		if (vertical)
		{
			affine->D(-affine->D());
			affine->E(-affine->E());

			if (!m_Fractorium->LocalPivot())
				affine->F(-affine->F());
		}

		FillAffineWithXform(xform, pre);
	});
}

void Fractorium::OnFlipHorizontalButtonClicked(bool checked) { m_Controller->FlipCurrentXform(true, false, sender() == ui.PreFlipHorizontalButton); }
void Fractorium::OnFlipVerticalButtonClicked(bool checked) { m_Controller->FlipCurrentXform(false, true, sender() == ui.PreFlipVerticalButton); }

/// <summary>
/// Rotate the current pre/post affine transform x degrees.
/// Resets the rendering process.
/// </summary>
/// <param name="angle">The angle to rotate by</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::RotateCurrentXformByAngle(double angle, bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;

		affine->Rotate(angle);
		FillAffineWithXform(xform, pre);
	});
}

/// <summary>
/// Rotate the selected pre/post affine transform 90 degrees clockwise/counter clockwise.
/// Called when the rotate 90 c/cc pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotate90CButtonClicked(bool checked) { m_Controller->RotateCurrentXformByAngle(90, sender() == ui.PreRotate90CButton); }
void Fractorium::OnRotate90CcButtonClicked(bool checked) { m_Controller->RotateCurrentXformByAngle(-90, sender() == ui.PreRotate90CcButton); }

/// <summary>
/// Rotate the selected pre/post affine transform x degrees clockwise.
/// Called when the rotate x c pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotateCButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreRotateCButton;
	QComboBox* combo = pre ? ui.PreRotateCombo : ui.PostRotateCombo;
	double d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->RotateCurrentXformByAngle(d, pre);
}

/// <summary>
/// Rotate the selected pre/post affine transform x degrees counter clockwise.
/// Called when the rotate x cc pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotateCcButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreRotateCcButton;
	QComboBox* combo = pre ? ui.PreRotateCombo : ui.PostRotateCombo;
	double d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->RotateCurrentXformByAngle(-d, pre);
}

/// <summary>
/// Move the current pre/post affine in the x and y directions by the specified amount.
/// Resets the rendering process.
/// </summary>
/// <param name="x">The x direction to move</param>
/// <param name="y">The y direction to move</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::MoveCurrentXform(double x, double y, bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;
		
		affine->C(affine->C() + x);
		affine->F(affine->F() + y);
		FillAffineWithXform(xform, pre);
	});
}

/// <summary>
/// Move the selected pre/post affine transform x units up.
/// Called when the move pre/post up buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveUpButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreMoveUpButton;
	QComboBox* combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->MoveCurrentXform(0, d, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units down.
/// Called when the move pre/post down buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveDownButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreMoveDownButton;
	QComboBox* combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->MoveCurrentXform(0, -d, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units left.
/// Called when the move pre/post left buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveLeftButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreMoveLeftButton;
	QComboBox* combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->MoveCurrentXform(-d, 0, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units right.
/// Called when the move pre/post right buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveRightButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreMoveRightButton;
	QComboBox* combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->MoveCurrentXform(d, 0, pre);
}

/// <summary>
/// Scale the current pre/post affine by the specified amount.
/// Resets the rendering process.
/// </summary>
/// <param name="scale">The scale value</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::ScaleCurrentXform(double scale, bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;
		
		affine->A(affine->A() * scale);
		affine->B(affine->B() * scale);
		affine->D(affine->D() * scale);
		affine->E(affine->E() * scale);
		FillAffineWithXform(xform, pre);
	});
}

/// <summary>
/// Scale the selected pre/post affine transform x units down.
/// Called when the scale pre/post down buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnScaleDownButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreScaleDownButton;
	QComboBox* combo = pre ? ui.PreScaleCombo : ui.PostScaleCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->ScaleCurrentXform(1.0 / (d / 100.0), pre);
}

/// <summary>
/// Scale the selected pre/post affine transform x units up.
/// Called when the scale pre/post up buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnScaleUpButtonClicked(bool checked)
{
	bool ok;
	bool pre = sender() == ui.PreScaleUpButton;
	QComboBox* combo = pre ? ui.PreScaleCombo : ui.PostScaleCombo;
	double d = ToDouble(combo->currentText(), &ok);
	
	if (ok)
		m_Controller->ScaleCurrentXform(d / 100.0, pre);
}

/// <summary>
/// Reset pre/post affine to the identity matrix.
/// Called when reset pre/post affine buttons are clicked.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ResetCurrentXformAffine(bool pre)
{
	UpdateCurrentXform([&] (Xform<T>* xform)
	{
		Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;

		affine->MakeID();
		FillAffineWithXform(xform, pre);
	});
}

/// <summary>
/// Reset pre/post affine to the identity matrix.
/// Called when reset pre/post affine buttons are clicked.
/// Resets the rendering process.
/// </summary>
void Fractorium::OnResetAffineButtonClicked(bool checked) { m_Controller->ResetCurrentXformAffine(sender() == ui.PreResetButton); }

/// <summary>
/// Fill the GUI with the pre/post affine xform values.
/// </summary>
/// <param name="xform">The xform to fill with</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::FillAffineWithXform(Xform<T>* xform, bool pre)
{
	if (pre)
	{
		m_Fractorium->m_PreX1Spin->SetValueStealth(xform->m_Affine.A());
		m_Fractorium->m_PreY1Spin->SetValueStealth(xform->m_Affine.B());
		m_Fractorium->m_PreO1Spin->SetValueStealth(xform->m_Affine.C());
		m_Fractorium->m_PreX2Spin->SetValueStealth(xform->m_Affine.D());
		m_Fractorium->m_PreY2Spin->SetValueStealth(xform->m_Affine.E());
		m_Fractorium->m_PreO2Spin->SetValueStealth(xform->m_Affine.F());
	}
	else
	{
		m_Fractorium->m_PostX1Spin->SetValueStealth(xform->m_Post.A());
		m_Fractorium->m_PostY1Spin->SetValueStealth(xform->m_Post.B());
		m_Fractorium->m_PostO1Spin->SetValueStealth(xform->m_Post.C());
		m_Fractorium->m_PostX2Spin->SetValueStealth(xform->m_Post.D());
		m_Fractorium->m_PostY2Spin->SetValueStealth(xform->m_Post.E());
		m_Fractorium->m_PostO2Spin->SetValueStealth(xform->m_Post.F());
	}
}

/// <summary>
/// Trigger a redraw which will show or hide the circle affine transforms
/// based on whether each group box is checked or not.
/// Called when the group box check box for pre or post affine is checked.
/// </summary>
/// <param name="on">Ignored</param>
void Fractorium::OnAffineGroupBoxToggled(bool on)
{
	ui.GLDisplay->update();
}

/// <summary>
/// Trigger a redraw which will show all, or only the current, circle affine transforms
/// based on which radio buttons are selected.
/// Called when and pre/post show all/current radio buttons are checked.
/// </summary>
/// <param name="on">Ignored</param>
void Fractorium::OnAffineDrawAllCurrentRadioButtonToggled(bool checked)
{
	OnCurrentXformComboChanged(ui.CurrentXformCombo->currentIndex());
	ui.GLDisplay->update();
}

/// <summary>
/// Setup a spinner to be placed in a table cell.
/// Special setup function for affine spinners which differs slightly from the regular
/// SetupSpinner() function.
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
/// <param name="prec">The precision of the spinner</param>
/// <param name="signal">The signal the spinner emits</param>
/// <param name="slot">The slot to receive the signal</param>
void Fractorium::SetupAffineSpinner(QTableWidget* table, const QObject* receiver, int row, int col, DoubleSpinBox*& spinBox, int height, double min, double max, double step, double prec, const char* signal, const char* slot)
{
	spinBox = new DoubleSpinBox(table, height, step);
	spinBox->setRange(min, max);
	spinBox->setDecimals(prec);
	table->setCellWidget(row, col, spinBox);
	connect(spinBox, signal, receiver, slot, Qt::QueuedConnection);
	spinBox->DoubleClick(true);
	spinBox->DoubleClickNonZero(0);
	spinBox->DoubleClickZero(1);
}

/// <summary>
/// GUI wrapper functions, getters only.
/// </summary>

bool Fractorium::DrawAllPre()  { return ui.ShowPreAffineAllRadio->isChecked();  }
bool Fractorium::DrawAllPost() { return ui.ShowPostAffineAllRadio->isChecked(); }
bool Fractorium::LocalPivot()  { return ui.LocalPivotRadio->isChecked();        }