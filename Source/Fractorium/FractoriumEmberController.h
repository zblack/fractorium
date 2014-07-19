#pragma once

#include "EmberFile.h"
#include "DoubleSpinBox.h"
#include "GLEmberController.h"

/// <summary>
/// FractoriumEmberControllerBase and FractoriumEmberController<T> classes.
/// </summary>

/// <summary>
/// An enum representing the type of edit being done.
/// </summary>
enum eEditUndoState : unsigned int { REGULAR_EDIT = 0, UNDO_REDO = 1, EDIT_UNDO = 2 };

/// <summary>
/// FractoriumEmberController and Fractorium need each other, but each can't include the other.
/// So Fractorium includes this file, and Fractorium is declared as a forward declaration here.
/// </summary>
class Fractorium;
#define PREVIEW_SIZE 256
#define UNDO_SIZE 128

/// <summary>
/// FractoriumEmberControllerBase serves as a non-templated base class with virtual
/// functions which will be overridden in a derived class that takes a template parameter.
/// The controller serves as a way to access both the Fractorium GUI and the underlying ember
/// objects through an interface that doesn't require template argument, but does allow
/// templated objects to be used underneath.
/// Note that there are a few functions which access a templated object, so for those both
/// versions for float and double must be provided, then overridden in the templated derived
/// class. It's definitely a design flaw, but C++ doesn't offer any alternative since
/// templated virtual functions are not supported.
/// The functions not implemented in this file can be found in the GUI files which use them.
/// </summary>
class FractoriumEmberControllerBase : public RenderCallback
{
public:
	FractoriumEmberControllerBase(Fractorium* fractorium);
	virtual ~FractoriumEmberControllerBase();

	//Embers.
	virtual void SetEmber(const Ember<float>& ember, bool verbatim = false) { }
	virtual void CopyEmber(Ember<float>& ember) { }
	virtual void SetEmberFile(const EmberFile<float>& emberFile) { }
	virtual void CopyEmberFile(EmberFile<float>& emberFile) { }
	virtual void SetTempPalette(const Palette<float>& palette) { }
	virtual void CopyTempPalette(Palette<float>& palette) { }
#ifdef DO_DOUBLE
	virtual void SetEmber(const Ember<double>& ember, bool verbatim = false) { }
	virtual void CopyEmber(Ember<double>& ember) { }
	virtual void SetEmberFile(const EmberFile<double>& emberFile) { }
	virtual void CopyEmberFile(EmberFile<double>& emberFile) { }
	virtual void SetTempPalette(const Palette<double>& palette) { }
	virtual void CopyTempPalette(Palette<double>& palette) { }
#endif
	virtual void SetEmber(size_t index) { }
	virtual void Clear() { }
	virtual void AddXform() { }
	virtual void DuplicateXform() { }
	virtual void ClearCurrentXform() { }
	virtual void DeleteCurrentXform() { }
	virtual void AddFinalXform() { }
	virtual bool UseFinalXform() { return false; }
	virtual size_t XformCount() { return 0; }
	virtual size_t TotalXformCount() { return 0; }
	virtual string Name() { return ""; }
	virtual void Name(string s) { }
	virtual unsigned int FinalRasW() { return 0; }
	virtual void FinalRasW(unsigned int w) { }
	virtual unsigned int FinalRasH() { return 0; }
	virtual void FinalRasH(unsigned int h) { }
	virtual void AddSymmetry(int sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) { }
	virtual void CalcNormalizedWeights() { }

	//Menu.
	virtual void NewFlock(unsigned int count) { }//File.
	virtual void NewEmptyFlameInCurrentFile() { }
	virtual void NewRandomFlameInCurrentFile() { }
	virtual void CopyFlameInCurrentFile() { }
	virtual void OpenAndPrepFiles(QStringList filenames, bool append) { }
	virtual void SaveCurrentAsXml() { }
	virtual void SaveEntireFileAsXml() { }
	virtual void SaveCurrentToOpenedFile() { }
	virtual void Undo() { }//Edit.
	virtual void Redo() { }
	virtual void CopyXml() { }
	virtual void CopyAllXml() { }
	virtual void PasteXmlAppend() { }
	virtual void PasteXmlOver() { }
	virtual void AddReflectiveSymmetry() { }//Tools.
	virtual void AddRotationalSymmetry() { }
	virtual void AddBothSymmetry() { }
	virtual void Flatten() { }
	virtual void Unflatten() { }
	virtual void ClearFlame() { }

	//Toolbar.

	//Params.
	virtual void SetCenter(double x, double y) { }
	virtual void FillParamTablesAndPalette() { }
	virtual void BrightnessChanged(double d) { }
	virtual void GammaChanged(double d) { }
	virtual void GammaThresholdChanged(double d) { }
	virtual void VibrancyChanged(double d) { }
	virtual void HighlightPowerChanged(double d) { }
	virtual void PaletteModeChanged(unsigned int i) { }
	virtual void CenterXChanged(double d) { }
	virtual void CenterYChanged(double d) { }
	virtual void ScaleChanged(double d) { }
	virtual void ZoomChanged(double d) { }
	virtual void RotateChanged(double d) { }
	virtual void ZPosChanged(double d) { }
	virtual void PerspectiveChanged(double d) { }
	virtual void PitchChanged(double d) { }
	virtual void YawChanged(double d) { }
	virtual void DepthBlurChanged(double d) { }
	virtual void SpatialFilterWidthChanged(double d) { }
	virtual void SpatialFilterTypeChanged(const QString& text) { }
	virtual void TemporalFilterWidthChanged(double d) { }
	virtual void TemporalFilterTypeChanged(const QString& text) { }
	virtual void DEFilterMinRadiusWidthChanged(double d) { }
	virtual void DEFilterMaxRadiusWidthChanged(double d) { }
	virtual void DEFilterCurveWidthChanged(double d) { }
	virtual void PassesChanged(int i) { }
	virtual void TemporalSamplesChanged(int d) { }
	virtual void QualityChanged(double d) { }
	virtual void SupersampleChanged(int d) { }
	virtual void AffineInterpTypeChanged(int i) { }
	virtual void InterpTypeChanged(int i) { }
	virtual void BackgroundChanged(const QColor& color) { }
	
	//Xforms.
	virtual void CurrentXformComboChanged(int index) { }
	virtual void XformWeightChanged(double d) { }
	virtual void EqualizeWeights() { }
	virtual void XformNameChanged(int row, int col) { }

	//Xforms Affine.
	virtual void AffineSetHelper(double d, int index, bool pre) { }
	virtual void FlipCurrentXform(bool horizontal, bool vertical, bool pre) { }
	virtual void RotateCurrentXformByAngle(double angle, bool pre) { }
	virtual void MoveCurrentXform(double x, double y, bool pre) { }
	virtual void ScaleCurrentXform(double scale, bool pre) { }
	virtual void ResetCurrentXformAffine(bool pre) { }

	//Xforms Color.
	virtual void XformColorIndexChanged(double d, bool updateRender) { }
	virtual void XformScrollColorIndexChanged(int d) { }
	virtual void XformColorSpeedChanged(double d) { }
	virtual void XformOpacityChanged(double d) { }
	virtual void XformDirectColorChanged(double d) { }
	void SetPaletteRefTable(QPixmap* pixmap);

	//Xforms Variations.
	virtual void SetupVariationTree() { }
	virtual void ClearVariationsTree() { }
	virtual void VariationSpinBoxValueChanged(double d) { }

	//Xforms Xaos.
	virtual void FillXaosWithCurrentXform() { }
	virtual QString MakeXaosNameString(unsigned int i) { return ""; }
	virtual void XaosChanged(DoubleSpinBox* sender) { }
	virtual void ClearXaos() { }

	//Palette.
	virtual bool InitPaletteTable(string s) { return false; }
	virtual void ApplyPaletteToEmber() { }
	virtual void PaletteAdjust() { }
	virtual QRgb GetQRgbFromPaletteIndex(unsigned int i) { return QRgb(); }
	virtual void PaletteCellClicked(int row, int col) { }

	//Library.
	virtual void SyncNames() { }
	virtual void FillLibraryTree(int selectIndex = -1) { }
	virtual void UpdateLibraryTree() { }
	virtual void EmberTreeItemChanged(QTreeWidgetItem* item, int col) { }
	virtual void EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col) { }
	virtual void RenderPreviews(unsigned int start = UINT_MAX, unsigned int end = UINT_MAX) { }
	virtual void StopPreviewRender() { }

	//Info.

	//Rendering/progress.
	virtual bool Render() { return false; }
	virtual bool CreateRenderer(eRendererType renderType, unsigned int platform, unsigned int device, bool shared = true) { return false; }
	virtual unsigned int SizeOfT() { return 0; }
	virtual void ClearUndo() { }
	virtual GLEmberControllerBase* GLController() { return NULL; }
	bool RenderTimerRunning();
	void StartRenderTimer();
	void DelayedStartRenderTimer();
	void StopRenderTimer(bool wait);
	void Shutdown();
	void UpdateRender(eProcessAction action = FULL_RENDER);
	void DeleteRenderer();
	void SaveCurrentRender(QString filename);
	RendererBase* Renderer() { return m_Renderer.get(); }
	vector<unsigned char>* FinalImage() { return &m_FinalImage; }
	vector<unsigned char>* PreviewFinalImage() { return &m_PreviewFinalImage; }

protected:
	//Rendering/progress.
	void AddProcessAction(eProcessAction action);
	eProcessAction CondenseAndClearProcessActions();
	eProcessState ProcessState() { return m_Renderer.get() ? m_Renderer->ProcessState() : NONE; }

	//Non-templated members.
	bool m_Rendering;
	bool m_Shared;
	bool m_LastEditWasUndoRedo;
	unsigned int m_Platform;
	unsigned int m_Device;
	unsigned int m_SubBatchCount;
	unsigned int m_FailedRenders;
	unsigned int m_UndoIndex;
	eRendererType m_RenderType;
	eEditUndoState m_EditState;
	GLuint m_OutputTexID;
	Timing m_RenderElapsedTimer;
	QImage m_FinalPaletteImage;
	QString m_LastSaveAll;
	QString m_LastSaveCurrent;
	CriticalSection m_Cs;
	vector<unsigned char> m_FinalImage;
	vector<unsigned char> m_PreviewFinalImage;
	vector<eProcessAction> m_ProcessActions;
	auto_ptr<EmberNs::RendererBase> m_Renderer;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> m_Rand;
	Fractorium* m_Fractorium;
	QTimer* m_RenderTimer;
	QTimer* m_RenderRestartTimer;
};

/// <summary>
/// Templated derived class which implements all interaction functionality between the embers
/// of a specific template type and the GUI.
/// Switching between template arguments requires complete re-creation of the controller and the
/// underlying renderer. Switching between CPU and OpenCL only requires re-creation of the renderer.
/// </summary>
template<typename T>
class FractoriumEmberController : public FractoriumEmberControllerBase
{
public:
	FractoriumEmberController(Fractorium* fractorium);
	virtual ~FractoriumEmberController();

	//Embers.
	virtual void SetEmber(const Ember<float>& ember, bool verbatim = false);
	virtual void CopyEmber(Ember<float>& ember);
	virtual void SetEmberFile(const EmberFile<float>& emberFile);
	virtual void CopyEmberFile(EmberFile<float>& emberFile);
	virtual void SetTempPalette(const Palette<float>& palette);
	virtual void CopyTempPalette(Palette<float>& palette);
#ifdef DO_DOUBLE
	virtual void SetEmber(const Ember<double>& ember, bool verbatim = false);
	virtual void CopyEmber(Ember<double>& ember);
	virtual void SetEmberFile(const EmberFile<double>& emberFile);
	virtual void CopyEmberFile(EmberFile<double>& emberFile);
	virtual void SetTempPalette(const Palette<double>& palette);
	virtual void CopyTempPalette(Palette<double>& palette);
#endif
	virtual void SetEmber(size_t index);
	virtual void Clear() { }
	virtual void AddXform();
	virtual void DuplicateXform();
	virtual void ClearCurrentXform();
	virtual void DeleteCurrentXform();
	virtual void AddFinalXform();
	virtual bool UseFinalXform() { return m_Ember.UseFinalXform(); }
	//virtual bool IsFinal(unsigned int i) { return false; }
	virtual size_t XformCount() { return m_Ember.XformCount(); }
	virtual size_t TotalXformCount() { return m_Ember.TotalXformCount(); }
	virtual string Name() { return m_Ember.m_Name; }
	virtual void Name(string s) { m_Ember.m_Name = s; }
	virtual unsigned int FinalRasW() { return m_Ember.m_FinalRasW; }
	virtual void FinalRasW(unsigned int w) { m_Ember.m_FinalRasW = w; }
	virtual unsigned int FinalRasH() { return m_Ember.m_FinalRasH; }
	virtual void FinalRasH(unsigned int h) { m_Ember.m_FinalRasH = h; }
	virtual void AddSymmetry(int sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) { m_Ember.AddSymmetry(sym, rand); }
	virtual void CalcNormalizedWeights() { m_Ember.CalcNormalizedWeights(m_NormalizedWeights); }
	Ember<T>* CurrentEmber();

	//Menu.
	virtual void NewFlock(unsigned int count);
	virtual void NewEmptyFlameInCurrentFile();
	virtual void NewRandomFlameInCurrentFile();
	virtual void CopyFlameInCurrentFile();
	virtual void OpenAndPrepFiles(QStringList filenames, bool append);
	virtual void SaveCurrentAsXml();
	virtual void SaveEntireFileAsXml();
	virtual void SaveCurrentToOpenedFile();
	virtual void Undo();
	virtual void Redo();
	virtual void CopyXml();
	virtual void CopyAllXml();
	virtual void PasteXmlAppend();
	virtual void PasteXmlOver();
	virtual void AddReflectiveSymmetry();
	virtual void AddRotationalSymmetry();
	virtual void AddBothSymmetry();
	virtual void Flatten();
	virtual void Unflatten();
	virtual void ClearFlame();

	//Toolbar.

	//Params.
	virtual void SetCenter(double x, double y);
	virtual void FillParamTablesAndPalette();
	virtual void BrightnessChanged(double d);
	virtual void GammaChanged(double d);
	virtual void GammaThresholdChanged(double d);
	virtual void VibrancyChanged(double d);
	virtual void HighlightPowerChanged(double d);
	virtual void PaletteModeChanged(unsigned int i);
	virtual void CenterXChanged(double d);
	virtual void CenterYChanged(double d);
	virtual void ScaleChanged(double d);
	virtual void ZoomChanged(double d);
	virtual void RotateChanged(double d);
	virtual void ZPosChanged(double d);
	virtual void PerspectiveChanged(double d);
	virtual void PitchChanged(double d);
	virtual void YawChanged(double d);
	virtual void DepthBlurChanged(double d);
	virtual void SpatialFilterWidthChanged(double d);
	virtual void SpatialFilterTypeChanged(const QString& text);
	virtual void TemporalFilterWidthChanged(double d);
	virtual void TemporalFilterTypeChanged(const QString& text);
	virtual void DEFilterMinRadiusWidthChanged(double d);
	virtual void DEFilterMaxRadiusWidthChanged(double d);
	virtual void DEFilterCurveWidthChanged(double d);
	virtual void PassesChanged(int d);
	virtual void TemporalSamplesChanged(int d);
	virtual void QualityChanged(double d);
	virtual void SupersampleChanged(int d);
	virtual void AffineInterpTypeChanged(int index);
	virtual void InterpTypeChanged(int index);
	virtual void BackgroundChanged(const QColor& col);

	//Xforms.
	virtual void CurrentXformComboChanged(int index);
	virtual void XformWeightChanged(double d);
	virtual void EqualizeWeights();
	virtual void XformNameChanged(int row, int col);
	void FillWithXform(Xform<T>* xform);
	Xform<T>* CurrentXform();

	//Xforms Affine.
	virtual void AffineSetHelper(double d, int index, bool pre);
	virtual void FlipCurrentXform(bool horizontal, bool vertical, bool pre);
	virtual void RotateCurrentXformByAngle(double angle, bool pre);
	virtual void MoveCurrentXform(double x, double y, bool pre);
	virtual void ScaleCurrentXform(double scale, bool pre);
	virtual void ResetCurrentXformAffine(bool pre);
	void FillAffineWithXform(Xform<T>* xform, bool pre);

	//Xforms Color.
	virtual void XformColorIndexChanged(double d, bool updateRender);
	virtual void XformScrollColorIndexChanged(int d);
	virtual void XformColorSpeedChanged(double d);
	virtual void XformOpacityChanged(double d);
	virtual void XformDirectColorChanged(double d);
	void FillColorWithXform(Xform<T>* xform);

	//Xforms Variations.
	virtual void SetupVariationTree();
	virtual void ClearVariationsTree();
	virtual void VariationSpinBoxValueChanged(double d);
	void FillVariationTreeWithXform(Xform<T>* xform);

	//Xforms Xaos.
	virtual void FillXaosWithCurrentXform();
	virtual QString MakeXaosNameString(unsigned int i);
	virtual void XaosChanged(DoubleSpinBox* sender);
	virtual void ClearXaos();

	//Palette.
	virtual bool InitPaletteTable(string s);
	virtual void ApplyPaletteToEmber();
	virtual void PaletteAdjust();
	virtual QRgb GetQRgbFromPaletteIndex(unsigned int i) { return QRgb(); }
	virtual void PaletteCellClicked(int row, int col);

	//Library.
	virtual void SyncNames();
	virtual void FillLibraryTree(int selectIndex = -1);
	virtual void UpdateLibraryTree();
	virtual void EmberTreeItemChanged(QTreeWidgetItem* item, int col);
	virtual void EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col);
	virtual void RenderPreviews(unsigned int start = UINT_MAX, unsigned int end = UINT_MAX);
	virtual void StopPreviewRender();

	//Info.

	//Rendering/progress.
	virtual bool Render();
	virtual bool CreateRenderer(eRendererType renderType, unsigned int platform, unsigned int device, bool shared = true);
	virtual unsigned int SizeOfT() { return sizeof(T); }
	virtual int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs);
	virtual void ClearUndo();
	virtual GLEmberControllerBase* GLController() { return m_GLController.get(); }

private:
	//Embers.
	void ApplyXmlSavingTemplate(Ember<T>& ember);
	template <typename U> void SetEmberPrivate(const Ember<U>& ember, bool verbatim);

	//Params.
	void ParamsToEmber(Ember<T>& ember);

	//Xforms.
	void SetNormalizedWeightText(Xform<T>* xform);
	bool IsFinal(Xform<T>* xform);

	//Xforms Color.
	void SetCurrentXformColorIndex(double d);

	//Palette.
	void UpdateAdjustedPaletteGUI(Palette<T>& palette);

	//Rendering/progress.
	void Update(std::function<void (void)> func, bool updateRender = true, eProcessAction action = FULL_RENDER);
	void UpdateCurrentXform(std::function<void (Xform<T>*)> func, bool updateRender = true, eProcessAction action = FULL_RENDER);

	//Templated members.
	bool m_PreviewRun;
	bool m_PreviewRunning;
	vector<T> m_TempOpacities;
	vector<T> m_NormalizedWeights;
	Ember<T> m_Ember;
	EmberFile<T> m_EmberFile;
	deque<Ember<T>> m_UndoList;
	Palette<T> m_TempPalette;
	PaletteList<T> m_PaletteList;
	VariationList<T> m_VariationList;
	auto_ptr<SheepTools<T, T>> m_SheepTools;
	auto_ptr<GLEmberController<T>> m_GLController;
	auto_ptr<EmberNs::Renderer<T, T>> m_PreviewRenderer;
	QFuture<void> m_PreviewResult;
	std::function<void (unsigned int, unsigned int)> m_PreviewRenderFunc;
};

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif