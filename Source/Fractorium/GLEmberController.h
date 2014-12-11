#pragma once

#include "FractoriumPch.h"

/// <summary>
/// GLEmberControllerBase and GLEmberController<T> classes.
/// </summary>

/// <summary>
/// Use/draw pre or post affine transform.
/// </summary>
enum eAffineType   { AffinePre, AffinePost };

/// <summary>
/// Hovering over nothing, the x axis, the y axis or the center.
/// </summary>
enum eHoverType	   { HoverNone, HoverXAxis, HoverYAxis, HoverTranslation };

/// <summary>
/// Dragging an affine transform or panning, rotating or scaling the image.
/// </summary>
enum eDragState    { DragNone, DragPanning, DragDragging, DragRotateScale };

/// <summary>
/// Dragging with no keys pressed, shift, control or alt.
/// </summary>
enum eDragModifier { DragModNone = 0x00, DragModShift = 0x01, DragModControl = 0x02, DragModAlt = 0x04 };

/// <summary>
/// GLController, FractoriumEmberController, GLWidget and Fractorium need each other, but each can't all include the other.
/// So GLWidget includes this file, and GLWidget, FractoriumEmberController and Fractorium are declared as forward declarations here.
/// </summary>
class GLWidget;
class Fractorium;
template <typename T> class FractoriumEmberController;

/// <summary>
/// GLEmberControllerBase serves as a non-templated base class with virtual
/// functions which will be overridden in a derived class that takes a template parameter.
/// The controller serves as a way to access both the GLWidget and the underlying ember
/// objects through an interface that doesn't require template argument, but does allow
/// templated objects to be used underneath.
/// The functions not implemented in this file can be found in GLWidget.cpp near the area of code which uses them.
/// </summary>
class GLEmberControllerBase
{
public:
	GLEmberControllerBase(Fractorium* fractorium, GLWidget* glWidget);
	virtual ~GLEmberControllerBase();

	void ClearDrag();
	bool Allocate(bool force = false);

	virtual void DrawImage() { }
	virtual void DrawAffines(bool pre, bool post) { }
	virtual void ClearWindow() { }
	virtual bool KeyPress_(QKeyEvent* e);
	virtual bool KeyRelease_(QKeyEvent* e);
	virtual void MousePress(QMouseEvent* e) { }
	virtual void MouseRelease(QMouseEvent* e) { }
	virtual void MouseMove(QMouseEvent* e) { }
	virtual void Wheel(QWheelEvent* e) { }
	virtual bool SizesMatch() { return false; }
	virtual bool CheckForSizeMismatch(int w, int h) { return true; }
	virtual void QueryMatrices(bool print) { }

protected:
	uint m_DragModifier;
	glm::ivec2 m_MousePos;
	glm::ivec2 m_MouseDownPos;
	glm::ivec4 m_Viewport;
	eDragState m_DragState;
	eHoverType m_HoverType;
	eAffineType m_AffineType;
	GLWidget* m_GL;
	Fractorium* m_Fractorium;
};

/// <summary>
/// Templated derived class which implements all interaction functionality between the embers
/// of a specific template type and the GLWidget;
/// </summary>
template<typename T>
class GLEmberController : public GLEmberControllerBase
{
public:
	GLEmberController(Fractorium* fractorium, GLWidget* glWidget, FractoriumEmberController<T>* controller);
	virtual ~GLEmberController();
	virtual void DrawImage() override;
	virtual void DrawAffines(bool pre, bool post) override;
	virtual void ClearWindow() override;
	virtual void MousePress(QMouseEvent* e) override;
	virtual void MouseRelease(QMouseEvent* e) override;
	virtual void MouseMove(QMouseEvent* e) override;
	virtual void Wheel(QWheelEvent* e) override;
	virtual void QueryMatrices(bool print) override;
	virtual bool SizesMatch() override;
	virtual bool CheckForSizeMismatch(int w, int h) override;

	T CalcScale();
	T CalcRotation();
	Affine2D<T> CalcDragXAxis();
	Affine2D<T> CalcDragYAxis();
	Affine2D<T> CalcDragTranslation();

	void SetEmber(Ember<T>* ember);
	void SetSelectedXform(Xform<T>* xform);
	void DrawAffine(Xform<T>* xform, bool pre, bool selected);
	int UpdateHover(v3T& glCoords);
	bool CheckXformHover(Xform<T>* xform, v3T& glCoords, T& bestDist, bool pre, bool post);

private:
	v3T SnapToGrid(v3T& vec);
	v3T SnapToNormalizedAngle(v3T& vec, uint divisions);
	v3T WindowToWorld(v3T& v, bool flip);
	void QueryVMP();
	void MultMatrix(m4T& mat);

	T m_CenterDownX;
	T m_CenterDownY;
	T m_RotationDown;
	T m_ScaleDown;
	v4T m_BoundsDown;

	v3T m_MouseWorldPos;
	v3T m_MouseDownWorldPos;
	v3T m_DragHandlePos;
	v3T m_DragHandleOffset;
	v3T m_HoverHandlePos;
	
	m4T m_Modelview;
	m4T m_Projection;

	Affine2D<T> m_DragSrcTransform;

	Xform<T>* m_HoverXform;
	Xform<T>* m_SelectedXform;
	FractoriumEmberController<T>* m_FractoriumEmberController;
	T GridStep;
};

