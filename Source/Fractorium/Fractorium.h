#pragma once

#include "ui_Fractorium.h"
#include "GLWidget.h"
#include "EmberTreeWidgetItem.h"
#include "VariationTreeWidgetItem.h"
#include "StealthComboBox.h"
#include "TableWidget.h"
#include "FinalRenderDialog.h"
#include "OptionsDialog.h"
#include "AboutDialog.h"

/// <summary>
/// Fractorium class.
/// </summary>

/// <summary>
/// Fractorium is the main window for the interactive renderer. The main viewable area
/// is a derivation of QGLWidget named GLWidget. The design uses the concept of a controller
/// to allow for both polymorphism and templating to coexist. Most processing functionality
/// is contained within the controller, and the GUI events just call the appropriate controller
/// member functions.
/// The rendering takes place on a timer with
/// a period of 0 which means it will trigger an event whenever the event input queue is idle.
/// As it's rendering, if the user changes anything on the GUI, a re-render will trigger. Since
/// certain parameters don't require a full render, the minimum necessary processing will be ran.
/// When the user changes something on the GUI, the required processing action is added to a vector.
/// Upon the next execution of the idle timer function, the most significant action will be extracted
/// and applied to the renderer. The vector is then cleared.
/// On the left side of the window is a dock widget which contains all controls needed for
/// manipulating embers.
/// Qt takes very long to create file dialog windows, so they are kept as members and initialized
/// upon first use with lazy instantiation and then kept around for the remainder of the program.
/// Additional dialogs are for the about box, options, and final rendering out to a file.
/// While all class member variables and functions are declared in this .h file, the implementation
/// for them is far too lengthy to put in a single .cpp file. So general functionality is placed in 
/// Fractorium.cpp and the other functional areas are each broken out into their own files.
/// The order of the functions in each .cpp file should roughly match the order they appear in the .h file.
/// Future todo list:
///		Add all of the plugins/variations that work with Apophysis and are open source.
///		Allow specifying variations to include/exclude from random generation.
///		Allow the option to specify a different palette file rather than the default flam3-palettes.xml.
///		Implement more rendering types.
///		Add support for animation previewing.
///		Add support for full animation editing and rendering.
///		Possibly add some features from JWildFire.
/// </summary>
class Fractorium : public QMainWindow
{
	Q_OBJECT

	friend GLWidget;
	friend FractoriumOptionsDialog;
	friend FractoriumFinalRenderDialog;
	friend FractoriumAboutDialog;
	friend GLEmberControllerBase;
	friend GLEmberController<float>;
	friend GLEmberController<double>;
	friend FractoriumEmberControllerBase;
	friend FractoriumEmberController<float>;
	friend FractoriumEmberController<double>;
	friend FinalRenderEmberControllerBase;
	friend FinalRenderEmberController<float>;
	friend FinalRenderEmberController<double>;

public:
	Fractorium(QWidget* parent = 0);
	~Fractorium();

	//Geometry.
	void SetCenter(float x, float y);
	void SetRotation(double rot, bool stealth);
	void SetScale(double scale);
	void SetCoordinateStatus(int x, int y, float worldX, float worldY);

	//Xforms.
	void CurrentXform(unsigned int i);

	//Xforms Affine.
	bool DrawAllPre();
	bool DrawAllPost();
	bool LocalPivot();
	
public slots:
	//Dock.
	void OnDockTopLevelChanged(bool topLevel);
	void dockLocationChanged(Qt::DockWidgetArea area);

	//Menu.
	void OnActionNewFlock(bool checked);//File.
	void OnActionNewEmptyFlameInCurrentFile(bool checked);
	void OnActionNewRandomFlameInCurrentFile(bool checked);
	void OnActionCopyFlameInCurrentFile(bool checked);
	void OnActionOpen(bool checked);
	void OnActionSaveCurrentAsXml(bool checked);
	void OnActionSaveEntireFileAsXml(bool checked);
	void OnActionSaveCurrentScreen(bool checked);
	void OnActionSaveCurrentToOpenedFile(bool checked);
	void OnActionExit(bool checked);

	void OnActionUndo(bool checked);//Edit.
	void OnActionRedo(bool checked);
	void OnActionCopyXml(bool checked);
	void OnActionCopyAllXml(bool checked);
	void OnActionPasteXmlAppend(bool checked);
	void OnActionPasteXmlOver(bool checked);

	void OnActionAddReflectiveSymmetry(bool checked);//Tools.
	void OnActionAddRotationalSymmetry(bool checked);
	void OnActionAddBothSymmetry(bool checked);
	void OnActionFlatten(bool checked);
	void OnActionUnflatten(bool checked);
	void OnActionClearFlame(bool checked);
	void OnActionRenderPreviews(bool checked);
	void OnActionStopRenderingPreviews(bool checked);
	void OnActionFinalRender(bool checked);
	void OnFinalRenderClose(int result);
	void OnActionOptions(bool checked);

	void OnActionAbout(bool checked);//Help.

	//Toolbar.
	void OnSaveCurrentAsXmlButtonClicked(bool checked);
	void OnSaveEntireFileAsXmlButtonClicked(bool checked);
	void OnSaveCurrentToOpenedFileButtonClicked(bool checked);

	//Params.
	void OnBrightnessChanged(double d);//Color.
	void OnGammaChanged(double d);
	void OnGammaThresholdChanged(double d);
	void OnVibrancyChanged(double d);
	void OnHighlightPowerChanged(double d);
	void OnBackgroundColorButtonClicked(bool checked);
	void OnColorSelected(const QColor& color);
	void OnPaletteModeComboCurrentIndexChanged(int index);
	void OnWidthChanged(int d);//Geometry.
	void OnHeightChanged(int d);
	void OnCenterXChanged(double d);
	void OnCenterYChanged(double d);
	void OnScaleChanged(double d);
	void OnZoomChanged(double d);
	void OnRotateChanged(double d);
	void OnZPosChanged(double d);
	void OnPerspectiveChanged(double d);
	void OnPitchChanged(double d);
	void OnYawChanged(double d);
	void OnDepthBlurChanged(double d);
	void OnSpatialFilterWidthChanged(double d);//Filter.
	void OnSpatialFilterTypeComboCurrentIndexChanged(const QString& text);
	void OnTemporalFilterWidthChanged(double d);
	void OnTemporalFilterTypeComboCurrentIndexChanged(const QString& text);
	void OnDEFilterMinRadiusWidthChanged(double d);
	void OnDEFilterMaxRadiusWidthChanged(double d);
	void OnDEFilterCurveWidthChanged(double d);
	void OnPassesChanged(int d);//Iteration.
	void OnTemporalSamplesChanged(int d);
	void OnQualityChanged(double d);
	void OnSupersampleChanged(int d);
	void OnAffineInterpTypeComboCurrentIndexChanged(int index);
	void OnInterpTypeComboCurrentIndexChanged(int index);

	//Xforms.
	void OnCurrentXformComboChanged(int index);
	void OnAddXformButtonClicked(bool checked);
	void OnDuplicateXformButtonClicked(bool checked);
	void OnClearXformButtonClicked(bool checked);
	void OnDeleteXformButtonClicked(bool checked);
	void OnAddFinalXformButtonClicked(bool checked);
	void OnXformWeightChanged(double d);
	void OnEqualWeightButtonClicked(bool checked);
	void OnXformNameChanged(int row, int col);

	//Xforms Affine.
	void OnX1Changed(double d);
	void OnX2Changed(double d);
	void OnY1Changed(double d);
	void OnY2Changed(double d);
	void OnO1Changed(double d);
	void OnO2Changed(double d);

	void OnFlipHorizontalButtonClicked(bool checked);
	void OnFlipVerticalButtonClicked(bool checked);
	void OnRotate90CButtonClicked(bool checked);
	void OnRotate90CcButtonClicked(bool checked);
	void OnRotateCButtonClicked(bool checked);
	void OnRotateCcButtonClicked(bool checked);
	void OnMoveUpButtonClicked(bool checked);
	void OnMoveDownButtonClicked(bool checked);
	void OnMoveLeftButtonClicked(bool checked);
	void OnMoveRightButtonClicked(bool checked);
	void OnScaleDownButtonClicked(bool checked);
	void OnScaleUpButtonClicked(bool checked);
	void OnResetAffineButtonClicked(bool checked);

	void OnAffineGroupBoxToggled(bool on);
	void OnAffineDrawAllCurrentRadioButtonToggled(bool checked);

	//Xforms Color.
	void OnXformColorIndexChanged(double d);
	void OnXformColorIndexChanged(double d, bool updateRender);
	void OnXformScrollColorIndexChanged(int d);
	void OnXformColorSpeedChanged(double d);
	void OnXformOpacityChanged(double d);
	void OnXformDirectColorChanged(double d);
	void OnSoloXformCheckBoxStateChanged(int state);
	void OnXformRefPaletteResized(int logicalIndex, int oldSize, int newSize);
	
	//Xforms Variations.
	void OnVariationSpinBoxValueChanged(double d);
	void OnTreeHeaderSectionClicked(int);
	void OnVariationsFilterLineEditTextChanged(const QString& text);
	void OnVariationsFilterClearButtonClicked(bool checked);

	//Xforms Xaos.
	void OnXaosChanged(double d);
	void OnXaosFromToToggled(bool checked);
	void OnClearXaosButtonClicked(bool checked);

	//Palette.
	void OnPaletteAdjust(int d);
	void OnPaletteCellClicked(int row, int col);
	void OnPaletteCellDoubleClicked(int row, int col);

	//Library.
	void OnEmberTreeItemChanged(QTreeWidgetItem* item, int col);
	void OnEmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col);

	//Rendering/progress.
	void StartRenderTimer();
	void IdleTimer();
	bool ControllersOk();
	void ShowCritical(const QString& title, const QString& text, bool invokeRequired = false);

	//Can't have a template function be a slot.
	void SetLibraryTreeItemData(EmberTreeWidgetItemBase* item, vector<unsigned char>& v, unsigned int width, unsigned int height);

public:
	//template<typename spinType, typename valType>//See below.
	//static void SetupSpinner(QTableWidget* table, const QObject* receiver, int& row, int col, spinType*& spinBox, int height, valType min, valType max, valType step, const char* signal, const char* slot, bool incRow = true, valType val = 0, valType doubleClickZero = -999, valType doubleClickNonZero = -999);
	static void SetupAffineSpinner(QTableWidget* table, const QObject* receiver, int row, int col, DoubleSpinBox*& spinBox, int height, double min, double max, double step, double prec, const char* signal, const char* slot);
	static void SetupCombo(QTableWidget* table, const QObject* receiver, int& row, int col, StealthComboBox*& comboBox, vector<string>& vals, const char* signal, const char* slot, Qt::ConnectionType connectionType = Qt::QueuedConnection);
	static void SetFixedTableHeader(QHeaderView* header, QHeaderView::ResizeMode mode = QHeaderView::Fixed);
	static int FlipDet(Affine2D<float>& affine);

protected:
	virtual void resizeEvent(QResizeEvent* e) override;
	virtual void closeEvent(QCloseEvent* e) override;
	virtual void dragEnterEvent(QDragEnterEvent* e) override;
	virtual void dragMoveEvent(QDragMoveEvent* e) override;
	virtual void dropEvent(QDropEvent* e) override;

private:
	void InitMenusUI();
	void InitToolbarUI();
	void InitParamsUI();
	void InitXformsUI();
	void InitXformsColorUI();
	void InitXformsAffineUI();
	void InitXformsVariationsUI();
	void InitXformsXaosUI();
	void InitPaletteUI();
	void InitLibraryUI();
	void SetTabOrders();

	//Embers.
	bool HaveFinal();

	//Params.

	//Xforms.
	void FillXforms();

	//Xforms Color.

	//Xforms Affine.

	//Xforms Variations.

	//Xforms Xaos.
	void FillXaosTable();
	
	//Palette.
	void ResetPaletteControls();

	//Library.
	
	//Info.
	void UpdateHistogramBounds();
	void ErrorReportToQTextEdit(vector<string>& errors, QTextEdit* textEdit, bool clear = true);

	//Rendering/progress.
	bool CreateRendererFromOptions();
	bool CreateControllerFromOptions();
	
	//Dialogs.
	QStringList SetupOpenXmlDialog();
	QString SetupSaveXmlDialog(const QString& defaultFilename);
	QString SetupSaveImageDialog(const QString& defaultFilename);
	QString SetupSaveFolderDialog();
	QColorDialog* m_ColorDialog;
	FractoriumFinalRenderDialog* m_FinalRenderDialog;
	FractoriumOptionsDialog* m_OptionsDialog;
	FractoriumAboutDialog* m_AboutDialog;

	//Params.
	DoubleSpinBox* m_BrightnessSpin;//Color.
	DoubleSpinBox* m_GammaSpin;
	DoubleSpinBox* m_GammaThresholdSpin;
	DoubleSpinBox* m_VibrancySpin;
	DoubleSpinBox* m_HighlightSpin;
	QPushButton* m_BackgroundColorButton;
	StealthComboBox* m_PaletteModeCombo;
	SpinBox* m_WidthSpin;//Geometry.
	SpinBox* m_HeightSpin;
	DoubleSpinBox* m_CenterXSpin;
	DoubleSpinBox* m_CenterYSpin;
	DoubleSpinBox* m_ScaleSpin;
	DoubleSpinBox* m_ZoomSpin;
	DoubleSpinBox* m_RotateSpin;
	DoubleSpinBox* m_ZPosSpin;
	DoubleSpinBox* m_PerspectiveSpin;
	DoubleSpinBox* m_PitchSpin;
	DoubleSpinBox* m_YawSpin;
	DoubleSpinBox* m_DepthBlurSpin;
	DoubleSpinBox* m_SpatialFilterWidthSpin;//Filter.
	StealthComboBox* m_SpatialFilterTypeCombo;
	DoubleSpinBox* m_TemporalFilterWidthSpin;
	StealthComboBox* m_TemporalFilterTypeCombo;
	DoubleSpinBox* m_DEFilterMinRadiusSpin;
	DoubleSpinBox* m_DEFilterMaxRadiusSpin;
	DoubleSpinBox* m_DECurveSpin;
	SpinBox* m_PassesSpin;//Iteration.
	SpinBox* m_TemporalSamplesSpin;
	DoubleSpinBox* m_QualitySpin;
	SpinBox* m_SupersampleSpin;
	StealthComboBox* m_AffineInterpTypeCombo;
	StealthComboBox* m_InterpTypeCombo;
	
	//Xforms.
	DoubleSpinBox* m_XformWeightSpin;
	SpinnerButtonWidget* m_XformWeightSpinnerButtonWidget;

	//Xforms Color.
	QTableWidgetItem* m_XformColorValueItem;
	QTableWidgetItem* m_PaletteRefItem;
	DoubleSpinBox* m_XformColorIndexSpin;
	DoubleSpinBox* m_XformColorSpeedSpin;
	DoubleSpinBox* m_XformOpacitySpin;
	DoubleSpinBox* m_XformDirectColorSpin;

	//Xforms Affine.
	DoubleSpinBox* m_PreX1Spin;//Pre.
	DoubleSpinBox* m_PreX2Spin;
	DoubleSpinBox* m_PreY1Spin;
	DoubleSpinBox* m_PreY2Spin;
	DoubleSpinBox* m_PreO1Spin;
	DoubleSpinBox* m_PreO2Spin;

	DoubleSpinBox* m_PostX1Spin;//Post.
	DoubleSpinBox* m_PostX2Spin;
	DoubleSpinBox* m_PostY1Spin;
	DoubleSpinBox* m_PostY2Spin;
	DoubleSpinBox* m_PostO1Spin;
	DoubleSpinBox* m_PostO2Spin;

	//Palette.
	SpinBox* m_PaletteHueSpin;
	SpinBox* m_PaletteSaturationSpin;
	SpinBox* m_PaletteBrightnessSpin;
	SpinBox* m_PaletteContrastSpin;
	SpinBox* m_PaletteBlurSpin;
	SpinBox* m_PaletteFrequencySpin;

	//Files.
	QFileDialog* m_FileDialog;
	QFileDialog* m_FolderDialog;
	QString m_LastSaveAll;
	QString m_LastSaveCurrent;
	//QMenu* m_FileTreeMenu;

	QProgressBar* m_ProgressBar;
	QLabel* m_RenderStatusLabel;
	QLabel* m_CoordinateStatusLabel;
	FractoriumSettings* m_Settings;
	char m_ULString[32];
	char m_URString[32];
	char m_LRString[32];
	char m_LLString[32];
	char m_WString[16];
	char m_HString[16];
	char m_DEString[16];
	char m_CoordinateString[128];
	QColor m_XformComboColors[XFORM_COLOR_COUNT], m_FinalXformComboColor;
	QIcon m_XformComboIcons[XFORM_COLOR_COUNT], m_FinalXformComboIcon;

	int m_FontSize;
	int m_VarSortMode;
	int m_PreviousPaletteRow;
	OpenCLWrapper m_Wrapper;
	auto_ptr<FractoriumEmberControllerBase> m_Controller;
	Ui::FractoriumClass ui;
};


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
		spinBox->DoubleClickZero((valType)doubleClickZero);
		spinBox->DoubleClickNonZero((valType)doubleClickNonZero);
	}

	if (incRow)
		row++;
}

/// <summary>
/// Wrapper around QWidget::setTabOrder() to return the second widget.
/// This makes it easy to chain multiple calls without having to retype
/// all of them if the order changes or if a new widget is inserted.
/// </summary>
/// <param name="parent">The parent widget that w1 and w2 belong to</param>
/// <param name="w1">The widget to come first in the tab order</param>
/// <param name="w2">The widget to come second in the tab order</param>
static QWidget* SetTabOrder(QWidget* parent, QWidget* w1, QWidget* w2)
{
	parent->setTabOrder(w1, w2);
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

//template void Fractorium::SetupSpinner<SpinBox, int>         (QTableWidget* table, const QObject* receiver, int& row, int col, SpinBox*& spinBox, int height, int min, int max, int step, const char* signal, const char* slot, bool incRow, int val, int doubleClickZero, int doubleClickNonZero);
//template void Fractorium::SetupSpinner<DoubleSpinBox, double>(QTableWidget* table, const QObject* receiver, int& row, int col, DoubleSpinBox*& spinBox, int height, double min, double max, double step, const char* signal, const char* slot, bool incRow, double val, double doubleClickZero, double doubleClickNonZero);
