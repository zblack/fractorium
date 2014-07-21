#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the parameters UI.
/// </summary>
void Fractorium::InitParamsUI()
{
	int row = 0;
	int spinHeight = 20;
	double dmax = numeric_limits<double>::max();
	vector<string> comboVals;
	QTableWidget* table = ui.ColorTable;

	//Because QTableWidget does not allow for a single title bar/header
	//at the top of a multi-column table, the workaround hack is to just
	//make another single column table with no rows, and use the single
	//column header as the title bar. Then positioning it right above the table
	//that holds the data. Disallow selecting and resizing of the title bar.
	SetFixedTableHeader(ui.ColorTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.GeometryTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.FilterTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.IterationTableHeader->horizontalHeader());

	//Color.
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_BrightnessSpin,	   spinHeight, 0.05,  100,    1,  SIGNAL(valueChanged(double)), SLOT(OnBrightnessChanged(double)),	   true,  4.0,  4.0,  4.0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_GammaSpin,		   spinHeight,    1, 9999,  0.5,  SIGNAL(valueChanged(double)), SLOT(OnGammaChanged(double)),          true,  4.0,  4.0,  4.0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_GammaThresholdSpin, spinHeight,    0,   10, 0.01, SIGNAL(valueChanged(double)), SLOT(OnGammaThresholdChanged(double)),  true,  0.1,  0.1,  0.0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_VibrancySpin,	   spinHeight,    0,   30, 0.01,  SIGNAL(valueChanged(double)), SLOT(OnVibrancyChanged(double)),       true,  1.0,  1.0,  0.0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_HighlightSpin,	   spinHeight, -1.0,  2.0,	0.1,  SIGNAL(valueChanged(double)), SLOT(OnHighlightPowerChanged(double)), true, -1.0, -1.0, -1.0);

	m_GammaThresholdSpin->setDecimals(4);
	m_BackgroundColorButton = new QPushButton("...", table);
	m_BackgroundColorButton->setMinimumWidth(21);
	m_BackgroundColorButton->setMaximumWidth(21);
	table->setCellWidget(row, 1, m_BackgroundColorButton);
	table->item(row, 1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	connect(m_BackgroundColorButton, SIGNAL(clicked(bool)), this, SLOT(OnBackgroundColorButtonClicked(bool)), Qt::QueuedConnection);
	row++;

	comboVals.push_back("Step");
	comboVals.push_back("Linear");
	SetupCombo(table, this, row, 1, m_PaletteModeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnPaletteModeComboCurrentIndexChanged(int)));

	//Geometry.
	row = 0;
	table = ui.GeometryTable;
	SetupSpinner<SpinBox, int>		   (table, this, row, 1, m_WidthSpin,       spinHeight,    10,  100000,     50, SIGNAL(valueChanged(int)),    SLOT(OnWidthChanged(int)));
	SetupSpinner<SpinBox, int>		   (table, this, row, 1, m_HeightSpin,      spinHeight,    10,  100000,     50, SIGNAL(valueChanged(int)),    SLOT(OnHeightChanged(int)));
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_CenterXSpin,     spinHeight,   -10,      10,   0.05, SIGNAL(valueChanged(double)), SLOT(OnCenterXChanged(double)),     true,	  0,   0,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_CenterYSpin,     spinHeight,   -10,      10,   0.05, SIGNAL(valueChanged(double)), SLOT(OnCenterYChanged(double)),     true,	  0,   0,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ScaleSpin,       spinHeight,    10,    5000,     20, SIGNAL(valueChanged(double)), SLOT(OnScaleChanged(double)),	      true, 240, 240, 240);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ZoomSpin,        spinHeight,     0,       5,    0.2, SIGNAL(valueChanged(double)), SLOT(OnZoomChanged(double)),	      true,	  0,   0,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_RotateSpin,      spinHeight,  -180,     180,     10, SIGNAL(valueChanged(double)), SLOT(OnRotateChanged(double)),      true,	  0,   0,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ZPosSpin,        spinHeight, -1000,    1000,      1, SIGNAL(valueChanged(double)), SLOT(OnZPosChanged(double)),        true,	  0,   1,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_PerspectiveSpin, spinHeight,  -500,     500,   0.01, SIGNAL(valueChanged(double)), SLOT(OnPerspectiveChanged(double)), true,	  0,   1,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_PitchSpin,       spinHeight,  -180,     180,      1, SIGNAL(valueChanged(double)), SLOT(OnPitchChanged(double)),       true,	  0,  45,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_YawSpin,         spinHeight,  -180,     180,      1, SIGNAL(valueChanged(double)), SLOT(OnYawChanged(double)),         true,	  0,  45,	0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DepthBlurSpin,   spinHeight,  -100,     100,   0.01, SIGNAL(valueChanged(double)), SLOT(OnDepthBlurChanged(double)),   true,	  0,   1,	0);
	m_WidthSpin->setEnabled(false);//Will programatically change these to match the window size, but the user should never be allowed to.
	m_HeightSpin->setEnabled(false);
	m_CenterXSpin->setDecimals(3);
	m_CenterYSpin->setDecimals(3);
	m_ZPosSpin->setDecimals(3);
	m_PerspectiveSpin->setDecimals(4);
	m_DepthBlurSpin->setDecimals(3);

	//Filter.
	row = 0;
	table = ui.FilterTable;
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_SpatialFilterWidthSpin, spinHeight, 0.1, 10, 0.1, SIGNAL(valueChanged(double)), SLOT(OnSpatialFilterWidthChanged(double)), true, 1.0, 1.0, 1.0);

	comboVals = SpatialFilterCreator<float>::FilterTypes();
	SetupCombo(table, this, row, 1, m_SpatialFilterTypeCombo, comboVals, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSpatialFilterTypeComboCurrentIndexChanged(const QString&)));

	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_TemporalFilterWidthSpin, spinHeight, 1, 10, 1, SIGNAL(valueChanged(double)), SLOT(OnTemporalFilterWidthChanged(double)), true, 1);

	comboVals = TemporalFilterCreator<float>::FilterTypes();
	SetupCombo(table, this, row, 1, m_TemporalFilterTypeCombo, comboVals, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnTemporalFilterTypeComboCurrentIndexChanged(const QString&)));

	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DEFilterMinRadiusSpin, spinHeight,    0, 25,   1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterMinRadiusWidthChanged(double)), true,   0,   0,   0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DEFilterMaxRadiusSpin, spinHeight,    0, 25,   1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterMaxRadiusWidthChanged(double)), true, 9.0, 9.0,   0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DECurveSpin,			  spinHeight, 0.15,  5, 0.1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterCurveWidthChanged(double)),     true, 0.4, 0.4, 0.4);

	//Iteration.
	row = 0;
	table = ui.IterationTable;
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_PassesSpin,          spinHeight, 1,    3,  1, SIGNAL(valueChanged(int)),	   SLOT(OnPassesChanged(int)),		    true,    1,  1,  1);
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_TemporalSamplesSpin, spinHeight, 1, 5000, 50, SIGNAL(valueChanged(int)),	   SLOT(OnTemporalSamplesChanged(int)), true, 1000);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_QualitySpin,			spinHeight, 1, dmax, 50, SIGNAL(valueChanged(double)), SLOT(OnQualityChanged(double)),	    true,   10, 10, 10);
	SetupSpinner<SpinBox, int>         (table, this, row, 1, m_SupersampleSpin,		spinHeight, 1,    4,  1, SIGNAL(valueChanged(int)),	   SLOT(OnSupersampleChanged(int)),	    true,    1,  1,  1);

	comboVals.clear();
	comboVals.push_back("Step");
	comboVals.push_back("Linear");
	SetupCombo(table, this, row, 1, m_AffineInterpTypeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnAffineInterpTypeComboCurrentIndexChanged(int)));

	comboVals.clear();
	comboVals.push_back("Linear");
	comboVals.push_back("Smooth");
	SetupCombo(table, this, row, 1, m_InterpTypeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnInterpTypeComboCurrentIndexChanged(int)));
}

/// <summary>
/// Color.
/// </summary>

/// <summary>
/// Set the brightness to be used for calculating K1 and K2 for filtering and final accum.
/// Called when brightness spinner is changed.
/// Resets the rendering process to the filtering stage.
/// </summary>
/// <param name="d">The brightness</param>
template <typename T>
void FractoriumEmberController<T>::BrightnessChanged(double d) { Update([&] { m_Ember.m_Brightness = d; }, true, FILTER_AND_ACCUM); }
void Fractorium::OnBrightnessChanged(double d) { m_Controller->BrightnessChanged(d); }

/// <summary>
/// Set the gamma to be used for final accum.
/// Called when gamma spinner is changed.
/// Resets the rendering process if temporal samples is greater than 1,
/// else if early clip is true, filter and accum, else final accum only.
/// </summary>
/// <param name="d">The gamma value</param>
template <typename T> void FractoriumEmberController<T>::GammaChanged(double d) { Update([&] { m_Ember.m_Gamma = d; }, true, m_Ember.m_TemporalSamples > 1 ? FULL_RENDER : (m_Renderer->EarlyClip() ? FILTER_AND_ACCUM : ACCUM_ONLY)); }
void Fractorium::OnGammaChanged(double d) { m_Controller->GammaChanged(d); }

/// <summary>
/// Set the gamma threshold to be used for final accum.
/// Called when gamma threshold spinner is changed.
/// Resets the rendering process to the final accumulation stage.
/// </summary>
/// <param name="d">The gamma threshold</param>
template <typename T> void FractoriumEmberController<T>::GammaThresholdChanged(double d) { Update([&] { m_Ember.m_GammaThresh = d; }, true, m_Ember.m_TemporalSamples > 1 ? FULL_RENDER : (m_Renderer->EarlyClip() ? FILTER_AND_ACCUM : ACCUM_ONLY)); }
void Fractorium::OnGammaThresholdChanged(double d) { m_Controller->GammaThresholdChanged(d); }

/// <summary>
/// Set the vibrancy to be used for final accum.
/// Called when vibrancy spinner is changed.
/// Resets the rendering process to the final accumulation stage if temporal samples is 1, else full reset.
/// </summary>
/// <param name="d">The vibrancy</param>
template <typename T> void FractoriumEmberController<T>::VibrancyChanged(double d) { Update([&] { m_Ember.m_Vibrancy = d; }, true, m_Ember.m_TemporalSamples > 1 ? FULL_RENDER : (m_Renderer->EarlyClip() ? FILTER_AND_ACCUM : ACCUM_ONLY)); }
void Fractorium::OnVibrancyChanged(double d) { m_Controller->VibrancyChanged(d); }

/// <summary>
/// Set the highlight power to be used for final accum.
/// Called when highlight power spinner is changed.
/// Resets the rendering process to the final accumulation stage.
/// </summary>
/// <param name="d">The highlight power</param>
template <typename T> void FractoriumEmberController<T>::HighlightPowerChanged(double d) { Update([&] { m_Ember.m_HighlightPower = d; }, true, m_Ember.m_TemporalSamples > 1 ? FULL_RENDER : (m_Renderer->EarlyClip() ? FILTER_AND_ACCUM : ACCUM_ONLY)); }
void Fractorium::OnHighlightPowerChanged(double d) { m_Controller->HighlightPowerChanged(d); }

/// <summary>
/// Show the color selection dialog.
/// Called when background color button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnBackgroundColorButtonClicked(bool checked)
{
	m_ColorDialog->show();
}

/// <summary>
/// Set a new ember background color when the user accepts the color dialog.
/// Also change the background and foreground colors of the color cell in the
/// color params table.
/// Resets the rendering process.
/// </summary>
/// <param name="color">The color to set, RGB in the 0-255 range</param>
template <typename T>
void FractoriumEmberController<T>::BackgroundChanged(const QColor& color)
{
	Update([&]
	{
		int itemRow = 5;
		QTableWidget* colorTable = m_Fractorium->ui.ColorTable;
		colorTable->item(itemRow, 1)->setBackgroundColor(color);

		QString r = QString::number(color.red());
		QString g = QString::number(color.green());
		QString b = QString::number(color.blue());

		int threshold = 105;
		int delta = (color.red()   * 0.299) + //Magic numbers gotten from a Stack Overflow post.
					(color.green() * 0.587) +
					(color.blue()  * 0.114);

		QColor textColor = (255 - delta < threshold) ? QColor(0, 0, 0) : QColor(255, 255, 255);
		colorTable->item(itemRow, 1)->setTextColor(textColor);
		colorTable->item(itemRow, 1)->setText("rgb(" + r + ", " + g + ", " + b + ")");

		//Color is 0-255, normalize to 0-1.
		m_Ember.m_Background.r = color.red()   / 255.0;
		m_Ember.m_Background.g = color.green() / 255.0;
		m_Ember.m_Background.b = color.blue()  / 255.0;
	});
}

void Fractorium::OnColorSelected(const QColor& color) { m_Controller->BackgroundChanged(color); }

/// <summary>
/// Set the palette index interpolation mode.
/// Called when palette mode combo box index is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="index">The index of the palette mode combo box</param>
template <typename T> void FractoriumEmberController<T>::PaletteModeChanged(unsigned int i) { Update([&] { m_Ember.m_PaletteMode = i == 0 ? PALETTE_STEP : PALETTE_LINEAR; }); }
void Fractorium::OnPaletteModeComboCurrentIndexChanged(int index) { m_Controller->PaletteModeChanged(index); }

/// <summary>
/// Geometry.
/// </summary>

/// <summary>
/// Placeholder, do nothing.
/// Dimensions are set automatically to match the dimensions of GLWidget.
/// </summary>
/// <param name="d">Ignored</param>
void Fractorium::OnWidthChanged(int d) { }

/// <summary>
/// Placeholder, do nothing.
/// Dimensions are set automatically to match the dimensions of GLWidget.
/// </summary>
/// <param name="d">Ignored</param>
void Fractorium::OnHeightChanged(int d) { }

/// <summary>
/// Set the x offset applied to the center of the image.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The x offset value</param>
template <typename T> void FractoriumEmberController<T>::CenterXChanged(double d) { Update([&] { m_Ember.m_CenterX = d; }); }
void Fractorium::OnCenterXChanged(double d) { m_Controller->CenterXChanged(d); }

/// <summary>
/// Set the y offset applied to the center of the image.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The y offset value</param>
template <typename T> void FractoriumEmberController<T>::CenterYChanged(double d) { Update([&] { m_Ember.m_CenterY = d; }); }
void Fractorium::OnCenterYChanged(double d) { m_Controller->CenterYChanged(d); }

/// <summary>
/// Set the scale (pixels per unit) value of the image.
/// Note this will not increase the number of iters ran, but will degrade quality.
/// To preserve quality, but exponentially increase iters, use zoom.
/// Called when scale spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The scale value</param>
template <typename T> void FractoriumEmberController<T>::ScaleChanged(double d) { Update([&] { m_Ember.m_PixelsPerUnit = d; }); }
void Fractorium::OnScaleChanged(double d) { m_Controller->ScaleChanged(d); }

/// <summary>
/// Set the zoom value of the image.
/// Note this will increase the number of iters ran exponentially.
/// To zoom in without increasing iters, but sacrifice quality, use scale.
/// Called when zoom spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The zoom value</param>
template <typename T> void FractoriumEmberController<T>::ZoomChanged(double d) { Update([&] { m_Ember.m_Zoom = d; }); }
void Fractorium::OnZoomChanged(double d) { m_Controller->ZoomChanged(d); }

/// <summary>
/// Set the angular rotation of the image.
/// Called when rotate spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The rotation in angles</param>
template <typename T> void FractoriumEmberController<T>::RotateChanged(double d) { Update([&] { m_Ember.m_Rotate = d; }); }
void Fractorium::OnRotateChanged(double d) { m_Controller->RotateChanged(d); }

template <typename T> void FractoriumEmberController<T>::ZPosChanged(double d) { Update([&] { m_Ember.m_CamZPos = d; }); }
void Fractorium::OnZPosChanged(double d) {  m_Controller->ZPosChanged(d); }

template <typename T> void FractoriumEmberController<T>::PerspectiveChanged(double d) { Update([&] { m_Ember.m_CamPerspective = d; }); }
void Fractorium::OnPerspectiveChanged(double d) { m_Controller->PerspectiveChanged(d); }

template <typename T> void FractoriumEmberController<T>::PitchChanged(double d) { Update([&] { m_Ember.m_CamPitch = d * DEG_2_RAD; }); }
void Fractorium::OnPitchChanged(double d) { m_Controller->PitchChanged(d); }

template <typename T> void FractoriumEmberController<T>::YawChanged(double d) { Update([&] { m_Ember.m_CamYaw = d * DEG_2_RAD; }); }
void Fractorium::OnYawChanged(double d) { m_Controller->YawChanged(d); }

template <typename T> void FractoriumEmberController<T>::DepthBlurChanged(double d) { Update([&] { m_Ember.m_CamDepthBlur = d; }); }
void Fractorium::OnDepthBlurChanged(double d) { m_Controller->DepthBlurChanged(d); }

/// <summary>
/// Filter.
/// </summary>

/// <summary>
/// Set the spatial filter width.
/// Called when the spatial filter width spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The spatial filter width</param>
template <typename T> void FractoriumEmberController<T>::SpatialFilterWidthChanged(double d) { Update([&] { m_Ember.m_SpatialFilterRadius = d; }); }//Must fully reset because it's used to create bounds.
void Fractorium::OnSpatialFilterWidthChanged(double d) { m_Controller->SpatialFilterWidthChanged(d); }

/// <summary>
/// Set the spatial filter type.
/// Called when the spatial filter type combo box index is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="text">The spatial filter type</param>
template <typename T> void FractoriumEmberController<T>::SpatialFilterTypeChanged(const QString& text) { Update([&] { m_Ember.m_SpatialFilterType = SpatialFilterCreator<T>::FromString(text.toStdString()); }); }//Must fully reset because it's used to create bounds.
void Fractorium::OnSpatialFilterTypeComboCurrentIndexChanged(const QString& text) { m_Controller->SpatialFilterTypeChanged(text); }

/// <summary>
/// Set the temporal filter width to be used with animation.
/// Called when the temporal filter width spinner is changed.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// </summary>
/// <param name="d">The temporal filter width</param>
template <typename T> void FractoriumEmberController<T>::TemporalFilterWidthChanged(double d) { Update([&] { m_Ember.m_TemporalFilterWidth = d; }, true, NOTHING); }//Don't do anything until animation is implemented.
void Fractorium::OnTemporalFilterWidthChanged(double d) { m_Controller->TemporalFilterWidthChanged(d); }

/// <summary>
/// Set the temporal filter type to be used with animation.
/// Called when the temporal filter combo box index is changed.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// </summary>
/// <param name="text">The name of the temporal filter</param>
template <typename T> void FractoriumEmberController<T>::TemporalFilterTypeChanged(const QString& text) { Update([&] { m_Ember.m_TemporalFilterType = TemporalFilterCreator<T>::FromString(text.toStdString()); }, true, NOTHING); }//Don't do anything until animation is implemented.
void Fractorium::OnTemporalFilterTypeComboCurrentIndexChanged(const QString& text) { m_Controller->TemporalFilterTypeChanged(text); }

/// <summary>
/// Set the density estimation filter min radius value.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The min radius value</param>
template <typename T>
void FractoriumEmberController<T>::DEFilterMinRadiusWidthChanged(double d)
{
	Update([&]
	{
		if (m_Fractorium->m_DEFilterMinRadiusSpin->value() > m_Fractorium->m_DEFilterMaxRadiusSpin->value())
		{
			m_Fractorium->m_DEFilterMinRadiusSpin->setValue(m_Fractorium->m_DEFilterMaxRadiusSpin->value() - 1);
			return;
		}

		m_Ember.m_MinRadDE = d;
	});
}

void Fractorium::OnDEFilterMinRadiusWidthChanged(double d) { m_Controller->DEFilterMinRadiusWidthChanged(d); }

/// <summary>
/// Set the density estimation filter max radius value.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The max radius value</param>
template <typename T>
void FractoriumEmberController<T>::DEFilterMaxRadiusWidthChanged(double d)
{
	Update([&]
	{
		if (m_Fractorium->m_DEFilterMaxRadiusSpin->value() < m_Fractorium->m_DEFilterMinRadiusSpin->value())
		{
			m_Fractorium->m_DEFilterMaxRadiusSpin->setValue(m_Fractorium->m_DEFilterMinRadiusSpin->value() + 1);
			return;
		}

		m_Ember.m_MaxRadDE = d;
	});
}

void Fractorium::OnDEFilterMaxRadiusWidthChanged(double d) { m_Controller->DEFilterMaxRadiusWidthChanged(d); }

/// <summary>
/// Set the density estimation filter curve value.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The curve value</param>
template <typename T> void FractoriumEmberController<T>::DEFilterCurveWidthChanged(double d) { Update([&] { m_Ember.m_CurveDE = d; }); }
void Fractorium::OnDEFilterCurveWidthChanged(double d) { m_Controller->DEFilterCurveWidthChanged(d); }

/// <summary>
/// Iteration.
/// </summary>

/// <summary>
/// Set the number of passes.
/// This is a feature that is mostly useless and unused, and may even be removed soon.
/// It should never be set to a value greater than 1.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The passes value</param>
template <typename T> void FractoriumEmberController<T>::PassesChanged(int i) { Update([&] { m_Ember.m_Passes = i; }); }
void Fractorium::OnPassesChanged(int d) { m_Controller->PassesChanged(d); }

/// <summary>
/// Set the temporal samples to be used with animation.
/// Called when the temporal samples spinner is changed.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// </summary>
/// <param name="d">The temporal samples value</param>
template <typename T> void FractoriumEmberController<T>::TemporalSamplesChanged(int i) { Update([&] { m_Ember.m_TemporalSamples = i; }, true, NOTHING); }//Don't do anything until animation is implemented.
void Fractorium::OnTemporalSamplesChanged(int d) { m_Controller->TemporalSamplesChanged(d); }

/// <summary>
/// Set the quality.
/// 10 is good for interactive rendering on the CPU.
/// 20-50 is good for OpenCL.
/// Above 500 seems to offer little additional value for final renders.
/// Called when the quality spinner is changed.
/// If rendering is done, and the value is greater than the last value, 
/// the rendering process is continued, else it's reset.
/// </summary>
/// <param name="d">The quality in terms of iterations per pixel</param>
template <typename T> void FractoriumEmberController<T>::QualityChanged(double d) { Update([&] { m_Ember.m_Quality = d; }, true, d > m_Ember.m_Quality ? KEEP_ITERATING : FULL_RENDER); }
void Fractorium::OnQualityChanged(double d) { m_Controller->QualityChanged(d); }

/// <summary>
/// Set the supersample.
/// Note this will dramatically degrade performance, especially in
/// OpenCL, while only giving a minor improvement in visual quality.
/// Values above 2 add no noticeable difference.
/// The user should only use this for a final render, or a quick preview, and then
/// reset it back to 1 when done.
/// Called when the supersample spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The supersample value to set</param>
template <typename T> void FractoriumEmberController<T>::SupersampleChanged(int d) { Update([&] { m_Ember.m_Supersample = d; }); }
void Fractorium::OnSupersampleChanged(int d) { m_Controller->SupersampleChanged(d); }

/// <summary>
/// Set the affine interpolation type.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// Called when the affine interp type combo box index is changed.
/// </summary>
/// <param name="index">The index</param>
template <typename T>
void FractoriumEmberController<T>::AffineInterpTypeChanged(int i)
{
	Update([&]
	{
		if (i == 0)
			m_Ember.m_AffineInterp = INTERP_LINEAR;
		else if (i == 1)
			m_Ember.m_AffineInterp = INTERP_LOG;
		else
			m_Ember.m_AffineInterp = INTERP_LINEAR;
	}, true, NOTHING);
}

void Fractorium::OnAffineInterpTypeComboCurrentIndexChanged(int index) { m_Controller->AffineInterpTypeChanged(index); }

/// <summary>
/// Set the interpolation type.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// Called when the interp type combo box index is changed.
/// </summary>
/// <param name="i">The index</param>
template <typename T>
void FractoriumEmberController<T>::InterpTypeChanged(int i)
{
	Update([&]
	{
		if (i == 0)
			m_Ember.m_Interp = EMBER_INTERP_LINEAR;
		else if (i == 1)
			m_Ember.m_Interp = EMBER_INTERP_SMOOTH;
		else
			m_Ember.m_Interp = EMBER_INTERP_LINEAR;
	}, true, NOTHING);
}

void Fractorium::OnInterpTypeComboCurrentIndexChanged(int index) { m_Controller->InterpTypeChanged(index); }

/// <summary>
/// Set the center.
/// This updates the spinners as well as the current ember center.
/// Resets the renering process.
/// </summary>
/// <param name="x">The x offset</param>
/// <param name="y">The y offset</param>
template <typename T>
void FractoriumEmberController<T>::SetCenter(double x, double y)
{
	m_Ember.m_CenterX = x;
	m_Ember.m_CenterY = y;
	m_Fractorium->m_CenterXSpin->SetValueStealth(x);//Don't trigger a redraw twice.
	m_Fractorium->m_CenterYSpin->SetValueStealth(y);

	if (m_Renderer.get())//On startup, this will be null at first because a resize takes place before the rendering thread is started.
		CenterXChanged(m_Ember.m_CenterX);//Trigger update using both new values.
}

/// <summary>
/// Fill the parameter tables and palette widgets with values from the current ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillParamTablesAndPalette()
{
	m_Fractorium->m_BrightnessSpin->SetValueStealth(m_Ember.m_Brightness);//Color.
	m_Fractorium->m_GammaSpin->SetValueStealth(m_Ember.m_Gamma);
	m_Fractorium->m_GammaThresholdSpin->SetValueStealth(m_Ember.m_GammaThresh);
	m_Fractorium->m_VibrancySpin->SetValueStealth(m_Ember.m_Vibrancy);
	m_Fractorium->m_HighlightSpin->SetValueStealth(m_Ember.m_HighlightPower);
	m_Fractorium->m_ColorDialog->setCurrentColor(QColor(m_Ember.m_Background.r * 255, m_Ember.m_Background.g * 255, m_Ember.m_Background.b * 255));
	m_Fractorium->ui.ColorTable->item(5, 1)->setBackgroundColor(m_Fractorium->m_ColorDialog->currentColor());
	m_Fractorium->m_PaletteModeCombo->SetCurrentIndexStealth((int)m_Ember.m_PaletteMode);
	m_Fractorium->m_WidthSpin->SetValueStealth(m_Ember.m_FinalRasW);//Geometry.
	m_Fractorium->m_HeightSpin->SetValueStealth(m_Ember.m_FinalRasH);
	m_Fractorium->m_CenterXSpin->SetValueStealth(m_Ember.m_CenterX);
	m_Fractorium->m_CenterYSpin->SetValueStealth(m_Ember.m_CenterY);
	m_Fractorium->m_ScaleSpin->SetValueStealth(m_Ember.m_PixelsPerUnit);
	m_Fractorium->m_RotateSpin->SetValueStealth(m_Ember.m_Rotate);
	m_Fractorium->m_ZPosSpin->SetValueStealth(m_Ember.m_CamZPos);
	m_Fractorium->m_PerspectiveSpin->SetValueStealth(m_Ember.m_CamPerspective);
	m_Fractorium->m_PitchSpin->SetValueStealth(m_Ember.m_CamPitch * RAD_2_DEG_T);
	m_Fractorium->m_YawSpin->SetValueStealth(m_Ember.m_CamYaw * RAD_2_DEG_T);
	m_Fractorium->m_DepthBlurSpin->SetValueStealth(m_Ember.m_CamDepthBlur);
	m_Fractorium->m_SpatialFilterWidthSpin->SetValueStealth(m_Ember.m_SpatialFilterRadius);//Filter.
	m_Fractorium->m_SpatialFilterTypeCombo->SetCurrentIndexStealth((int)m_Ember.m_SpatialFilterType);
	m_Fractorium->m_TemporalFilterWidthSpin->SetValueStealth(m_Ember.m_TemporalFilterWidth);
	m_Fractorium->m_TemporalFilterTypeCombo->SetCurrentIndexStealth((int)m_Ember.m_TemporalFilterType);
	m_Fractorium->m_DEFilterMinRadiusSpin->SetValueStealth(m_Ember.m_MinRadDE);
	m_Fractorium->m_DEFilterMaxRadiusSpin->SetValueStealth(m_Ember.m_MaxRadDE);
	m_Fractorium->m_DECurveSpin->SetValueStealth(m_Ember.m_CurveDE);
	m_Fractorium->m_PassesSpin->SetValueStealth(m_Ember.m_Passes);//Iteration.
	m_Fractorium->m_TemporalSamplesSpin->SetValueStealth(m_Ember.m_TemporalSamples);
	m_Fractorium->m_QualitySpin->SetValueStealth(m_Ember.m_Quality);
	m_Fractorium->m_SupersampleSpin->SetValueStealth(m_Ember.m_Supersample);
	m_Fractorium->m_AffineInterpTypeCombo->SetCurrentIndexStealth(m_Ember.m_AffineInterp);
	m_Fractorium->m_InterpTypeCombo->SetCurrentIndexStealth(m_Ember.m_Interp);

	//Palette.
	m_Fractorium->ResetPaletteControls();
	m_Fractorium->m_PaletteHueSpin->SetValueStealth(NormalizeDeg180<double>(m_Ember.m_Hue * 360.0));//Convert -0.5 to 0.5 range to -180 - 180.

	//Use -1 as a placeholder to mean either generate a random palette from the list or
	//to just use the values "as-is" without looking them up in the list.
	if (m_Ember.m_Palette.m_Index >= 0)
	{
		m_Fractorium->OnPaletteCellClicked(Clamp<int>(m_Ember.m_Palette.m_Index, 0, m_Fractorium->ui.PaletteListTable->rowCount() - 1), 1);
	}
	else
	{
		//An ember with an embedded palette was loaded, rather than one from the list, so assign it directly to the controls without applying adjustments.
		//Normally, temp palette is assigned whenever the user clicks on a palette cell. But since that is skipped here just make a copy of the ember's palette.
		m_TempPalette = m_Ember.m_Palette;
		UpdateAdjustedPaletteGUI(m_Ember.m_Palette);//Will clear name string since embedded palettes have no name. This will trigger a full render.
	}
}

/// <summary>
/// Copy all GUI widget values on the parameters tab to the passed in ember.
/// </summary>
/// <param name="ember">The ember to copy values to.</param>
template <typename T>
void FractoriumEmberController<T>::ParamsToEmber(Ember<T>& ember)
{
	QColor color = m_Fractorium->ui.ColorTable->item(5, 1)->backgroundColor();

	ember.m_Brightness = m_Fractorium->m_BrightnessSpin->value();//Color.
	ember.m_Gamma = m_Fractorium->m_GammaSpin->value();
	ember.m_GammaThresh = m_Fractorium->m_GammaThresholdSpin->value();
	ember.m_Vibrancy = m_Fractorium->m_VibrancySpin->value();
	ember.m_HighlightPower = m_Fractorium->m_HighlightSpin->value();
	ember.m_Background.r = color.red() / 255.0;
	ember.m_Background.g = color.green() / 255.0;
	ember.m_Background.b = color.blue() / 255.0;
	ember.m_PaletteMode = (ePaletteMode)m_Fractorium->m_PaletteModeCombo->currentIndex();
	ember.m_FinalRasW = m_Fractorium->m_WidthSpin->value();//Geometry.
	ember.m_FinalRasH = m_Fractorium->m_HeightSpin->value();
	ember.m_CenterX = m_Fractorium->m_CenterXSpin->value();
	ember.m_CenterY = m_Fractorium->m_CenterYSpin->value();
	ember.m_PixelsPerUnit = m_Fractorium->m_ScaleSpin->value();
	ember.m_Rotate = m_Fractorium->m_RotateSpin->value();
	ember.m_CamZPos = m_Fractorium->m_ZPosSpin->value();
	ember.m_CamPerspective = m_Fractorium->m_PerspectiveSpin->value();
	ember.m_CamPitch = m_Fractorium->m_PitchSpin->value() * DEG_2_RAD_T;
	ember.m_CamYaw = m_Fractorium->m_YawSpin->value() * DEG_2_RAD_T;
	ember.m_CamDepthBlur = m_Fractorium->m_DepthBlurSpin->value();
	ember.m_SpatialFilterRadius = m_Fractorium->m_SpatialFilterWidthSpin->value();//Filter.
	ember.m_SpatialFilterType = (eSpatialFilterType)m_Fractorium->m_SpatialFilterTypeCombo->currentIndex();
	ember.m_TemporalFilterWidth = m_Fractorium->m_TemporalFilterWidthSpin->value();
	ember.m_TemporalFilterType = (eTemporalFilterType)m_Fractorium->m_TemporalFilterTypeCombo->currentIndex();
	ember.m_MinRadDE = m_Fractorium->m_DEFilterMinRadiusSpin->value();
	ember.m_MaxRadDE = m_Fractorium->m_DEFilterMaxRadiusSpin->value();
	ember.m_CurveDE = m_Fractorium->m_DECurveSpin->value();
	ember.m_Passes = m_Fractorium->m_PassesSpin->value();
	ember.m_TemporalSamples = m_Fractorium->m_TemporalSamplesSpin->value();
	ember.m_Quality = m_Fractorium->m_QualitySpin->value();
	ember.m_Supersample = m_Fractorium->m_SupersampleSpin->value();
	ember.m_AffineInterp = (eAffineInterp)m_Fractorium->m_AffineInterpTypeCombo->currentIndex();
	ember.m_Interp = (eInterp)m_Fractorium->m_InterpTypeCombo->currentIndex();
}

/// <summary>
/// Set the rotation.
/// This updates the spinner, optionally stealth.
/// </summary>
/// <param name="rot">The rotation value in angles to set</param>
/// <param name="stealth">True if stealth to skip re-rendering, else false to trigger a new render</param>
void Fractorium::SetRotation(double rot, bool stealth)
{
	if (stealth)
		m_RotateSpin->SetValueStealth(rot);
	else
		m_RotateSpin->setValue(rot);
}

/// <summary>
/// Set the scale.
/// This is the number of raster pixels that correspond to the distance
/// between 0-1 in the cartesian plane. The higher the number, the more
/// zoomed in the image is.
/// Resets the rendering process.
/// </summary>
/// <param name="scale">The scale value</param>
void Fractorium::SetScale(double scale)
{
	m_ScaleSpin->setValue(scale);
}
