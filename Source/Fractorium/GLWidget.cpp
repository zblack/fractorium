#include "FractoriumPch.h"
#include "GLWidget.h"
#include "Fractorium.h"

//#define OLDDRAG 1

/// <summary>
/// Constructor which passes parent widget to the base and initializes OpenGL profile.
/// This will need to change in the future to implement all drawing as shader programs.
/// </summary>
/// <param name="parent">The parent widget</param>
GLWidget::GLWidget(QWidget* parent)
	: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	QGLFormat qglFormat;

	m_Init = false;
	m_Drawing = false;
	m_TexWidth = 0;
	m_TexHeight = 0;
	m_OutputTexID = 0;
	m_Fractorium = NULL;
	qglFormat.setSwapInterval(1);//Vsync.
	qglFormat.setDoubleBuffer(true); 
	qglFormat.setVersion(2, 0);
	//qglFormat.setVersion(3, 2);
	qglFormat.setProfile(QGLFormat::CompatibilityProfile);
	//qglFormat.setProfile(QGLFormat::CoreProfile);
	
	setFormat(qglFormat);
}

/// <summary>
/// Empty destructor.
/// </summary>
GLWidget::~GLWidget()
{
}

/// <summary>
/// Draw the final rendered image as a texture on a quad that is the same size as the window.
/// Different action is taken based on whether a CPU or OpenCL renderer is used.
/// For CPU, the output image buffer must be copied to OpenGL every time it's drawn.
/// For OpenCL, the output image and the texture are the same thing, so no copying is necessary
/// and all image memory remains on the card.
/// </summary>
void GLWidget::DrawQuad()
{
	GLint texWidth = 0, texHeight = 0;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RendererBase* renderer = m_Fractorium->m_Controller->Renderer();
	vector<unsigned char>* finalImage = m_Fractorium->m_Controller->FinalImage();

	//Ensure all allocation has taken place first.
	if (m_OutputTexID != 0 && finalImage && !finalImage->empty())
	{
		glBindTexture(GL_TEXTURE_2D, m_OutputTexID);//The texture to draw to.
		
		//Only draw if the dimensions match exactly.
		if (m_TexWidth == width() && m_TexHeight == height() && ((m_TexWidth * m_TexHeight * 4) == finalImage->size()))
		{
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, 1, 1, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			//Copy data from CPU to OpenGL if using a CPU renderer. This is not needed when using OpenCL.
			if (renderer->RendererType() == CPU_RENDERER)
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_TexWidth, m_TexHeight, GL_RGBA, GL_UNSIGNED_BYTE, finalImage->data());

			glBegin(GL_QUADS);//This will need to be converted to a shader at some point in the future.

			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
			
			glEnd();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}

		glBindTexture(GL_TEXTURE_2D, 0);//Stop using this texture.
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

/// <summary>
/// Set drag and drag modifier states to nothing.
/// </summary>
void GLEmberControllerBase::ClearDrag()
{
	m_DragModifier = 0;
	m_DragState = DragNone;
}

/// <summary>
/// Wrapper around Allocate() call on the GL widget.
/// </summary>
bool GLEmberControllerBase::Allocate(bool force) { return m_GL->Allocate(force); }

/// <summary>
/// Clear the OpenGL output window to be the background color of the current ember.
/// Both buffers must be cleared, else artifacts will show up.
/// </summary>
template <typename T>
void GLEmberController<T>::ClearWindow()
{
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	m_GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_GL->glClearColor(ember->m_Background.r, ember->m_Background.g, ember->m_Background.b, 1.0);

	m_GL->swapBuffers();

	m_GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_GL->glClearColor(ember->m_Background.r, ember->m_Background.g, ember->m_Background.b, 1.0);
}

/// <summary>
/// Set the currently selected xform.
/// The currently selected xform is drawn with a circle around it, with all others only showing their axes.
/// </summary>
/// <param name="xform">The xform.</param>
template <typename T>
void GLEmberController<T>::SetSelectedXform(Xform<T>* xform)
{
	//By doing this check, it prevents triggering unnecessary events when selecting an xform on this window with the mouse,
	//which will set the combo box on the main window, which will trigger this call. However, if the xform has been selected
	//here with the mouse, the window has already repainted, so there's no need to do it again.
	if (m_SelectedXform != xform || m_HoverXform != xform)
	{
		m_HoverXform = xform;
		m_SelectedXform = xform;

		if (m_GL->m_Init)
			m_GL->repaint();//Force immediate redraw with repaint() instead of update().
	}
}

/// <summary>
/// Setters for main window pointers.
/// </summary>

void GLWidget::SetMainWindow(Fractorium* f) { m_Fractorium = f; }

/// <summary>
/// Getters for OpenGL state.
/// </summary>

bool GLWidget::Init() { return m_Init; }
bool GLWidget::Drawing() { return m_Drawing; }
GLuint GLWidget::OutputTexID() { return m_OutputTexID; }

/// <summary>
/// Initialize OpenGL, called once at startup after the main window constructor finishes.
/// Once this is done, the render timer is started after a short delay.
/// Rendering is then clear to begin.
/// </summary>
void GLWidget::initializeGL()
{
	if (!m_Init && initializeOpenGLFunctions() && m_Fractorium)
	{
		glClearColor(0.0, 0.0, 0.0, 1.0);

		//Start with a flock of 10 random embers. Can't do this until now because the window wasn't maximized yet, so the sizes would have been off.
		m_Fractorium->OnActionNewFlock(false);
		m_Fractorium->m_Controller->DelayedStartRenderTimer();
		m_Init = true;
	}
}

/// <summary>
/// The main drawing/update function.
/// First the quad will be drawn, then the remaining affine circles.
/// </summary>
void GLWidget::paintGL()
{
	FractoriumEmberControllerBase* controller = m_Fractorium->m_Controller.get();

	//Ensure there is a renderer and that it's supposed to be drawing, signified by the running timer.
	if (controller && controller->Renderer() && controller->RenderTimerRunning())
	{
		RendererBase* renderer = controller->Renderer();

		m_Drawing = true;
		//renderer->EnterResize();
		controller->GLController()->DrawImage();
		//renderer->LeaveResize();//Unlock, may not be necessary.
		
		//Affine drawing.
		bool pre = m_Fractorium->ui.PreAffineGroupBox->isChecked();
		bool post = m_Fractorium->ui.PostAffineGroupBox->isChecked();
		float unitX = fabs(renderer->UpperRightX(false) - renderer->LowerLeftX(false)) / 2.0f;
		float unitY = fabs(renderer->UpperRightY(false) - renderer->LowerLeftY(false)) / 2.0f;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-unitX, unitX, -unitY, unitY, -1, 1);//Projection matrix: OpenGL camera is always centered, just move the ember internally inside the renderer.
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);

		controller->GLController()->DrawAffines(pre, post);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);

		glFinish();
		m_Drawing = false;
	}
}

/// <summary>
/// Draw the image on the quad.
/// </summary>
template <typename T>
void GLEmberController<T>::DrawImage()
{
	RendererBase* renderer = m_FractoriumEmberController->Renderer();
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	m_GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_GL->glClearColor(ember->m_Background.r, ember->m_Background.g, ember->m_Background.b, 1.0);
	m_GL->glDisable(GL_DEPTH_TEST);

	renderer->EnterFinalAccum();//Lock, may not be necessary, but just in case.
	renderer->EnterResize();
		
	if (SyncSizes())//Ensure all sizes are correct. If not, do nothing.
	{
		vector<unsigned char>* finalImage = m_FractoriumEmberController->FinalImage();

		if (renderer->RendererType() == OPENCL_RENDERER || finalImage)//Final image only matters for CPU renderer.
			if (renderer->RendererType() == OPENCL_RENDERER || finalImage->size() == renderer->FinalBufferSize())
				m_GL->DrawQuad();//Output image is drawn here.
	}

	renderer->LeaveResize();//Unlock, may not be necessary.
	renderer->LeaveFinalAccum();
}

/// <summary>
/// Draw the affine circles.
/// </summary>
/// <param name="pre">True to draw pre affines, else don't.</param>
/// <param name="post">True to draw post affines, else don't.</param>
template <typename T>
void GLEmberController<T>::DrawAffines(bool pre, bool post)
{
	QueryVMP();//Resolves to float or double specialization function depending on T.
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	//Draw grid if control key is pressed.
	if ((m_DragModifier & DragModControl) == DragModControl)
	{
		m_GL->glLineWidth(1.0f);
		m_GL->DrawGrid();
	}
	//When dragging, only draw the selected xform's affine and hide all others.
	if (m_DragState == DragDragging)
	{
		if (m_SelectedXform)
			DrawAffine(m_SelectedXform, m_AffineType == AffinePre, true);

		m_GL->glPointSize(6.0f);//Draw large yellow dot on select or drag.
		m_GL->glBegin(GL_POINTS);
		m_GL->glColor4f(1.0f, 1.0f, 0.5f, 1.0f);
		m_GL->glVertex2f(m_DragHandlePos.x, m_DragHandlePos.y);
		m_GL->glEnd();
		m_GL->glPointSize(1.0f);//Restore point size.
	}
	else//Not dragging, just hovering/mouse move.
	{
		if (pre && m_Fractorium->DrawAllPre())//Draw all pre affine if specified.
		{
			for (unsigned int i = 0; i < ember->TotalXformCount(); i++)
			{
				Xform<T>* xform = ember->GetTotalXform(i);
				bool selected = m_HoverXform == xform;

				DrawAffine(xform, true, selected);
			}
		}
		else if (pre && m_HoverXform)//Only draw current pre affine.
		{
			DrawAffine(m_HoverXform, true, true);
		}

		if (post && m_Fractorium->DrawAllPost())//Draw all post affine if specified.
		{
			for (unsigned int i = 0; i < ember->TotalXformCount(); i++)
			{
				Xform<T>* xform = ember->GetTotalXform(i);
				bool selected = m_HoverXform == xform;

				DrawAffine(xform, false, selected);
			}
		}
		else if (post && m_HoverXform)//Only draw current post affine.
		{
			DrawAffine(m_HoverXform, false, true);
		}

		//Draw large turquoise dot on hover if they are hovering over the selected xform.
		if (m_HoverType != HoverNone && m_HoverXform == m_SelectedXform)
		{
			m_GL->glPointSize(6.0f);
			m_GL->glBegin(GL_POINTS);
			m_GL->glColor4f(0.5f, 1.0f, 1.0f, 1.0f);
			m_GL->glVertex2f(m_HoverHandlePos.x, m_HoverHandlePos.y);
			m_GL->glEnd();
			m_GL->glPointSize(1.0f);
		}
	}
}

/// <summary>
/// Set drag modifiers based on key press.
/// </summary>
/// <param name="e">The event</param>
bool GLEmberControllerBase::KeyPress(QKeyEvent* e)
{
#ifdef OLDDRAG
	if (e->key() == Qt::Key_Shift)
		m_DragModifier |= DragModShift;
	else if (e->key() == Qt::Key_Control || e->key() == Qt::Key_C)
		m_DragModifier |= DragModControl;
	else if (e->key() == Qt::Key_Alt || e->key() == Qt::Key_A)
		m_DragModifier |= DragModAlt;
	else
		return false;

	return true;
#else
	if (e->key() == Qt::Key_Control)
	{
		m_DragModifier |= DragModControl;
		return true;
	}
#endif

	return false;
}

/// <summary>
/// Call controller KeyPress().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::keyPressEvent(QKeyEvent* e)
{
	if (!GLController() || !GLController()->KeyPress(e))
		QGLWidget::keyPressEvent(e);

	update();
}

/// <summary>
/// Set drag modifiers based on key release.
/// </summary>
/// <param name="e">The event</param>
bool GLEmberControllerBase::KeyRelease(QKeyEvent* e)
{
#ifdef OLDDRAG
	if (e->key() == Qt::Key_Shift)
		m_DragModifier &= ~DragModShift;
	else if (e->key() == Qt::Key_Control || e->key() == Qt::Key_C)
		m_DragModifier &= ~DragModControl;
	else if (e->key() == Qt::Key_Alt || e->key() == Qt::Key_A)
		m_DragModifier &= ~DragModAlt;
	else
		return false;

	return true;
#else
	if (e->key() == Qt::Key_Control)
	{
		m_DragModifier &= ~DragModControl;
		return true;
	}
#endif

	return false;
}

/// <summary>
/// Call controller KeyRelease().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::keyReleaseEvent(QKeyEvent* e)
{	
	if (!GLController() || !GLController()->KeyRelease(e))
		QGLWidget::keyReleaseEvent(e);

	update();
}

/// <summary>
/// Determine if the mouse click was over an affine circle
/// and set the appropriate selection information to be used
/// on subsequent mouse move events.
/// If nothing was selected, then reset the selection and drag states.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MousePress(QMouseEvent* e)
{
	v3T mouseFlipped(e->x(), m_Viewport[3] - e->y(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();
	RendererBase* renderer = m_FractoriumEmberController->Renderer();

	//Ensure everything has been initialized.
	if (!renderer)
		return;

	m_MouseDownPos = glm::ivec2(e->x(), e->y());//Capture the raster coordinates of where the mouse was clicked.
	m_MouseWorldPos = WindowToWorld(mouseFlipped, false);//Capture the world cartesian coordinates of where the mouse is.
	m_BoundsDown.w = renderer->LowerLeftX(false);//Need to capture these because they'll be changing if scaling.
	m_BoundsDown.x = renderer->LowerLeftY(false);
	m_BoundsDown.y = renderer->UpperRightX(false);
	m_BoundsDown.z = renderer->UpperRightY(false);

#ifndef OLDDRAG
	Qt::KeyboardModifiers mod = e->modifiers();
	
	if (mod.testFlag(Qt::ShiftModifier))
		m_DragModifier |= DragModShift;
	//if (mod.testFlag(Qt::ControlModifier))// || mod.testFlag(Qt::Key_C))
	//	m_DragModifier |= DragModControl;
	if (mod.testFlag(Qt::AltModifier))// || mod.testFlag(Qt::Key_A))
		m_DragModifier |= DragModAlt;
#endif
	if (m_DragState == DragNone)//Only take action if the user wasn't already dragging.
	{
		m_MouseDownWorldPos = m_MouseWorldPos;//Set the mouse down position to the current position.

		if (e->button() & Qt::LeftButton)
		{
			int xformIndex = UpdateHover(mouseFlipped);//Determine if an affine circle was clicked.

			if (m_HoverXform && xformIndex != -1)
			{
				m_SelectedXform = m_HoverXform;
				m_DragSrcTransform = Affine2D<T>(m_AffineType == AffinePre ? m_SelectedXform->m_Affine : m_SelectedXform->m_Post);//Copy the affine of the xform that was selected.
				m_DragHandlePos = m_HoverHandlePos;
				m_DragHandleOffset = m_DragHandlePos - m_MouseWorldPos;
				m_DragState = DragDragging;
				
				//The user has selected an xform by clicking on it, so update the main GUI by selecting this xform in the combo box.
				m_Fractorium->CurrentXform(xformIndex);
			}
			else//Nothing was selected.
			{
				//m_SelectedXform = NULL;
				m_DragState = DragNone;
			}
		}
		else if (e->button() == Qt::MiddleButton)//Middle button does whole image translation.
		{
			m_CenterDownX = ember->m_CenterX;//Capture where the center of the image is because this value will change when panning.
			m_CenterDownY = ember->m_CenterY;
			m_DragState = DragPanning;
		}
		else if (e->button() == Qt::RightButton)//Right button does whole image rotation and scaling.
		{
			UpdateHover(mouseFlipped);
			m_SelectedXform = m_HoverXform;
			m_CenterDownX = ember->m_CenterX;//Capture these because they will change when rotating and scaling.
			m_CenterDownY = ember->m_CenterY;
			m_RotationDown = ember->m_Rotate;
			m_ScaleDown = ember->m_PixelsPerUnit;
			m_DragState = DragRotateScale;
		}
	}
}

/// <summary>
/// Call controller MousePress().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mousePressEvent(QMouseEvent* e)
{
	setFocus();//Must do this so that this window gets keyboard events.

	if (GLEmberControllerBase* controller = GLController())
		controller->MousePress(e);

	QGLWidget::mousePressEvent(e);
}

/// <summary>
/// Reset the selection and dragging state, but re-calculate the
/// hovering state because the mouse might still be over an affine circle.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MouseRelease(QMouseEvent* e)
{
	v3T mouseFlipped(e->x(), m_Viewport[3] - e->y(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.

	m_MouseWorldPos = WindowToWorld(mouseFlipped, false);
	
	if (m_DragState == DragDragging && (e->button() & Qt::LeftButton))
		UpdateHover(mouseFlipped);

	m_DragState = DragNone;

#ifndef OLDDRAG
	m_DragModifier = 0;
#endif

	m_GL->repaint();//Force immediate redraw.
}

/// <summary>
/// Call controller MouseRelease().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setFocus();//Must do this so that this window gets keyboard events.

	if (GLEmberControllerBase* controller = GLController())
		controller->MouseRelease(e);

	QGLWidget::mouseReleaseEvent(e);
}

/// <summary>
/// If dragging, update relevant values and reset entire rendering process.
/// If hovering, update display.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MouseMove(QMouseEvent* e)
{
	bool draw = true;
	glm::ivec2 mouse(e->x(), e->y());
	v3T mouseFlipped(e->x(), m_Viewport[3] - e->y(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	//First check to see if the mouse actually moved.
	if (mouse == m_MousePos)
		return;

	m_MousePos = mouse;
	m_MouseWorldPos = WindowToWorld(mouseFlipped, false);
	v3T mouseDelta = m_MouseWorldPos - m_MouseDownWorldPos;//Determine how far the mouse has moved in world cartesian coordinates.

	//Update status bar on main window, regardless of whether anything is being dragged.
	if (m_Fractorium->m_Controller->RenderTimerRunning())
		m_Fractorium->SetCoordinateStatus(e->x(), e->y(), m_MouseWorldPos.x, m_MouseWorldPos.y);

	if (m_SelectedXform && m_DragState == DragDragging)//Dragging and affine.
	{
		bool pre = m_AffineType == AffinePre;
		Affine2D<T>* affine = pre ? &m_SelectedXform->m_Affine : &m_SelectedXform->m_Post;//Determine pre or post affine.

		if (m_HoverType == HoverTranslation)
			*affine = CalcDragTranslation();
		else if (m_HoverType == HoverXAxis)
			*affine = CalcDragXAxis();
		else if (m_HoverType == HoverYAxis)
			*affine = CalcDragYAxis();

		m_FractoriumEmberController->FillAffineWithXform(m_SelectedXform, pre);//Update the spinners in the affine tab of the main window.
		m_FractoriumEmberController->UpdateRender();//Restart the rendering process.
	}
	else if (m_DragState == DragPanning)//Translating the whole image.
	{
		T x = -(m_MouseWorldPos.x - m_MouseDownWorldPos.x);
		T y = (m_MouseWorldPos.y - m_MouseDownWorldPos.y);
		Affine2D<T> rotMat;

		rotMat.C(m_CenterDownX);
		rotMat.F(m_CenterDownY);
		rotMat.Rotate(ember->m_Rotate);

		v2T v1(x, y);
		v2T v2 = rotMat.TransformVector(v1);

		ember->m_CenterX = v2.x;
		ember->m_CenterY = v2.y;
		m_FractoriumEmberController->SetCenter(ember->m_CenterX, ember->m_CenterY);//Will restart the rendering process.
	}
	else if (m_DragState == DragRotateScale)//Rotating and scaling the whole image.
	{
		T rot = CalcRotation();
		T scale = CalcScale();

		ember->m_Rotate = NormalizeDeg180<T>(m_RotationDown + rot);
		m_Fractorium->SetRotation(ember->m_Rotate, true);
		ember->m_PixelsPerUnit = m_ScaleDown + scale;
		m_Fractorium->SetScale(ember->m_PixelsPerUnit);//Will restart the rendering process.
	}
	else
	{
		//If the user doesn't already have a key down, and they aren't dragging, clear the keys to be safe.
		//This is done because if they do an alt+tab between windows, it thinks the alt key is down.
		if (e->modifiers() == Qt::NoModifier)
			ClearDrag();

		//Check if they weren't dragging and weren't hovering over any affine.
		//In that case, nothing needs to be done.
		if (UpdateHover(mouseFlipped) == -1)
			draw = false;
		
		//Xform<T>* previousHover = m_HoverXform;
		//
		//if (UpdateHover(mouseFlipped) == -1)
		//	m_HoverXform = m_SelectedXform;
		//
		//if (m_HoverXform == previousHover)
		//	draw = false;
	}

	//Only update if the user was dragging or hovered over a point.
	//Use repaint() to update immediately for a more responsive feel.
	if ((m_DragState != DragNone) || draw)
		m_GL->update();
		//m_GL->repaint();
}

/// <summary>
/// Call controller MouseMove().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mouseMoveEvent(QMouseEvent* e)
{
	setFocus();//Must do this so that this window gets keyboard events.

	if (GLEmberControllerBase* controller = GLController())
		controller->MouseMove(e);

	QGLWidget::mouseMoveEvent(e);
}

/// <summary>
/// Mouse wheel changes the scale (pixels per unit) which
/// will zoom in the image in our out, while sacrificing quality.
/// If the user needs to preserve quality, they can use the zoom spinner
/// on the main window.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::Wheel(QWheelEvent* e)
{
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	if (m_Fractorium && !(e->buttons() & Qt::MiddleButton))//Middle button does whole image translation, so ignore the mouse wheel while panning to avoid inadvertent zooming.
		m_Fractorium->SetScale(ember->m_PixelsPerUnit + (e->angleDelta().y() >= 0 ? 50 : -50));
}

/// <summary>
/// Call controller Wheel().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::wheelEvent(QWheelEvent* e)
{
	if (GLEmberControllerBase* controller = GLController())
		controller->Wheel(e);
	
	QGLWidget::wheelEvent(e);
}

/// <summary>
/// Respond to a resize event which will set the read only
/// main window width and height spinners.
/// The main window will take care of stopping and restarting the
/// render timer.
/// </summary>
/// <param name="e">The event</param>
void GLWidget::resizeEvent(QResizeEvent* e)
{
	if (m_Fractorium)
	{
		m_Fractorium->m_WidthSpin->setValue(width());
		m_Fractorium->m_HeightSpin->setValue(height());

		if (GLEmberControllerBase* controller = GLController())
			controller->SyncSizes();//For some reason the base resize can't be called here or else it causes a crash.
	}
}

/// <summary>
/// Set up texture memory to match the size of the window.
/// If first allocation, generate, bind and set parameters.
/// If subsequent call, only take action if dimensions don't match the window. In such case,
/// first deallocate, then reallocate.
/// </summary>
/// <returns>True if success, else false.</returns>
bool GLWidget::Allocate(bool force)
{
	bool alloc = false;

	if (m_OutputTexID == 0)
	{
		m_TexWidth = width();
		m_TexHeight = height();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glGenTextures(1, &m_OutputTexID);
		glBindTexture(GL_TEXTURE_2D, m_OutputTexID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Fractron had this as GL_LINEAR_MIPMAP_LINEAR for OpenCL and Cuda.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_TexWidth, m_TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		alloc = true;
	}
	else
	{
		if (force || (m_TexWidth != width() || m_TexHeight != height()))
		{
			m_TexWidth = width();
			m_TexHeight = height();
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture(GL_TEXTURE_2D, m_OutputTexID);
			Deallocate();
			glGenTextures(1, &m_OutputTexID);
			glBindTexture(GL_TEXTURE_2D, m_OutputTexID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Fractron had this as GL_LINEAR_MIPMAP_LINEAR for OpenCL and Cuda.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_TexWidth, m_TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			alloc = true;
		}
	}

	if (alloc)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	return m_OutputTexID != 0;
}

/// <summary>
/// Deallocate texture memory.
/// </summary>
/// <returns>True if anything deleted, else false.</returns>
bool GLWidget::Deallocate()
{
	bool deleted = false;

	if (m_OutputTexID != 0)
	{
		glDeleteTextures(1, &m_OutputTexID);
		m_OutputTexID = 0;
		deleted = true;
	}

	return deleted;
}

/// <summary>
/// Set the viewport to match the window dimensions.
/// If the dimensions already match, no action is taken.
/// </summary>
void GLWidget::SetViewport()
{
	if (m_Init && (m_ViewWidth != width() || m_ViewHeight != height()))
	{
		glViewport(0, 0, (GLint)width(), (GLint)height());
		m_ViewWidth = width();
		m_ViewHeight = height();
	}
}

/// <summary>
/// Synchronize the size of the renderer output image, the texture, and this window.
/// They must all match. If the renderer output size doesn't match, then the render
/// timer is stopped and restarted with a delay.
/// This will get called once or twice when a resize occurs.
/// </summary>
/// <returns>True if all sizes match, else false.</returns>
template <typename T>
bool GLEmberController<T>::SyncSizes()
{
	if (m_GL->Init())
	{
		m_GL->SetViewport();

		//First make sure the dimensions of the ember match the window size.
		if (CheckForSizeMismatch(m_GL->width(), m_GL->height()))
		{
			if (m_Fractorium)//This will stop the rendering process and start the reset timer.
				m_Fractorium->Resize();

			return false;
		}

		//The renderer and window dimensions match, now make sure the dims of the texture match.
		if (!m_GL->Allocate())
			return false;

		return true;
	}

	return false;
}

/// <summary>
/// Draw the grid in response to the control key being pressed.
/// The frequency of the grid lines will change depending on the zoom.
/// Calculated with the frame always centered, the renderer just moves the camera.
/// </summary>
void GLWidget::DrawGrid()
{
	RendererBase* renderer = m_Fractorium->m_Controller->Renderer();
	float unitX = fabs(renderer->UpperRightX(false) - renderer->LowerLeftX(false)) / 2.0f;
	float unitY = fabs(renderer->UpperRightY(false) - renderer->LowerLeftY(false)) / 2.0f;
	float rad = max(unitX, unitY);
	float xLow =  floor(-unitX);
	float xHigh = ceil(unitX);
	float yLow =  floor(-unitY);
	float yHigh = ceil(unitY);
			
	glBegin(GL_LINES);

	if (rad <= 8.0f)
	{
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);

		for (float x = xLow; x <= xHigh; x += GridStep)
		{
			glVertex2f(x, yLow);
			glVertex2f(x, yHigh);
		}

		for (float y = yLow; y < yHigh; y += GridStep)
		{
			glVertex2f(xLow,  y);
			glVertex2f(xHigh, y);
		}
	}

	if (unitX <= 64.0f)
	{
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);

		for (float x = xLow; x <= xHigh; x += 1.0f)
		{
			glVertex2f(x, yLow);
			glVertex2f(x, yHigh);
		}

		for (float y = yLow; y < yHigh; y += 1.0f)
		{
			glVertex2f(xLow,  y);
			glVertex2f(xHigh, y);
		}
	}

	glColor4f(1.0f,   0.0f, 0.0f, 1.0f);
	glVertex2f(0.0f,  0.0f);
	glVertex2f(xHigh, 0.0f);
	glColor4f(0.5f,   0.0f, 0.0f, 1.0f);
	glVertex2f(0.0f,  0.0f);
	glVertex2f(xLow,  0.0f);
	glColor4f(0.0f,   1.0f, 0.0f, 1.0f);
	glVertex2f(0.0f,  0.0f);
	glVertex2f(0.0f,  yHigh);
	glColor4f(0.0f,   0.5f, 0.0f, 1.0f);
	glVertex2f(0.0f,  0.0f);
	glVertex2f(0.0f,  yLow);
	glEnd();
}

/// <summary>
/// Draw the unit square.
/// </summary>
void GLWidget::DrawUnitSquare()
{
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
	glVertex2f(-1,-1);
	glVertex2f( 1,-1);
	glVertex2f(-1, 1);
	glVertex2f( 1, 1);

	glVertex2f(-1,-1);
	glVertex2f(-1, 1);
	glVertex2f( 1,-1);
	glVertex2f( 1, 1);
			
	glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
	glVertex2f(-1, 0);
	glVertex2f( 1, 0);
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	glVertex2f( 0,-1);
	glVertex2f( 0, 1);
	glEnd();
}

/// <summary>
/// Draw the pre or post affine circle for the passed in xform.
/// For drawing affine transforms, multiply the identity model view matrix by the
/// affine for each xform, so that all points are considered to be "1".
/// </summary>
/// <param name="xform">A pointer to the xform whose affine will be drawn</param>
/// <param name="pre">True for pre affine, else false for post.</param>
/// <param name="selected">True if selected (draw enclosing circle), else false (only draw axes).</param>
template <typename T>
void GLEmberController<T>::DrawAffine(Xform<T>* xform, bool pre, bool selected)
{
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();
	bool final = ember->IsFinalXform(xform);
	size_t size = ember->m_Palette.m_Entries.size();
	v4T color = ember->m_Palette.m_Entries[Clamp<T>(xform->m_ColorX * size, 0, size - 1)];
	Affine2D<T>* affine = pre ? &xform->m_Affine : &xform->m_Post;
	
	//For some incredibly strange reason, even though glm and OpenGL use matrices with a column-major
	//data layout, nothing will work here unless they are flipped to row major order. This is how it was
	//done in Fractron.
	m4T mat = affine->ToMat4RowMajor();

	m_GL->glPushMatrix();
	m_GL->glLoadIdentity();
	MultMatrix(mat);
	m_GL->glLineWidth(3.0f);//One 3px wide, colored black, except green on x axis for post affine.
	m_GL->DrawAffineHelper(selected, pre, final, true);

	m_GL->glLineWidth(1.0f);//Again 1px wide, colored white, to give a white middle with black outline effect.
	m_GL->DrawAffineHelper(selected, pre, final, false);

	m_GL->glPointSize(5.0f);//Three black points, one in the center and two on the circle. Drawn big 5px first to give a black outline.
	m_GL->glBegin(GL_POINTS);
	m_GL->glColor4f(0.0f, 0.0f, 0.0f, selected ? 1.0f : 0.5f);
	m_GL->glVertex2f(0.0f, 0.0f);
	m_GL->glVertex2f(1.0f, 0.0f);
	m_GL->glVertex2f(0.0f, 1.0f);
	m_GL->glEnd();

	m_GL->glLineWidth(2.0f);//Draw lines again for y axis only, without drawing the circle, using the color of the selected xform.
	m_GL->glBegin(GL_LINES);
	m_GL->glColor4f(color.r, color.g, color.b, 1.0f);
	m_GL->glVertex2f(0.0f, 0.0f);
	m_GL->glVertex2f(0.0f, 1.0f);
	m_GL->glEnd();

	m_GL->glPointSize(3.0f);//Draw smaller white points, to give a black outline effect.
	m_GL->glBegin(GL_POINTS);
	m_GL->glColor4f(1.0f, 1.0f, 1.0f, selected ? 1.0f : 0.5f);
	m_GL->glVertex2f(0.0f, 0.0f);
	m_GL->glVertex2f(1.0f, 0.0f);
	m_GL->glVertex2f(0.0f, 1.0f);
	m_GL->glEnd();
	m_GL->glPopMatrix();
}

/// <summary>
/// Draw the axes, and optionally the surrounding circle
/// of an affine transform.
/// </summary>
/// <param name="selected">True if selected (draw enclosing circle), else false (only draw axes).</param>
void GLWidget::DrawAffineHelper(bool selected, bool pre, bool final, bool background)
{
	float px = 1.0f;
	float py = 0.0f;

	glBegin(GL_LINES);

	//Circle part.
	if (!background)
	{
		if (pre)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);//Draw pre affine transform with white.
		else
			glColor4f(0.0f, 0.75f, 0.0f, 1.0f);//Draw post affine transform with green.
	}
	else
	{
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	if (selected)
	{
		for (int i = 1; i <= 64; i++)//The circle.
		{
			float theta = (float)M_PI * 2.0f * (float)(i % 64) / 64.0f;
			float x = (float)cos(theta);
			float y = (float)sin(theta);

			glVertex2f(px, py);
			glVertex2f(x, y);
			px = x;
			py = y;
		}
	}

	//Lines from center to circle.
	if (!background)
	{
		if (final)
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);//Draw final xforms with red.
		else
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);//Draw normal xforms with white.
	}
	else
	{
		if (!pre)
			glColor4f(0.0f, 0.75f, 0.0f, 1.0f);//Draw post affine transform with green outline.
	}

	//The lines from the center to the circle.

	glVertex2f(0.0f, 0.0f);//X axis.
	glVertex2f(1.0f, 0.0f);
	
	if (background)
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	glVertex2f(0.0f, 0.0f);//Y axis.
	glVertex2f(0.0f, 1.0f);

	glEnd();
}

/// <summary>
/// Determine the index of the xform being hovered over if any.
/// Give precedence to the currently selected xform, if any.
/// </summary>
/// <param name="glCoords">The mouse raster coordinates to check</param>
/// <returns>The index of the xform being hovered over, else -1 if no hover.</returns>
template <typename T>
int GLEmberController<T>::UpdateHover(v3T& glCoords)
{
	bool pre = m_Fractorium->ui.PreAffineGroupBox->isChecked();
	bool post = m_Fractorium->ui.PostAffineGroupBox->isChecked();
	bool preAll = pre && m_Fractorium->DrawAllPre();
	bool postAll = post && m_Fractorium->DrawAllPost();
	unsigned int bestIndex = -1;
	T bestDist = 10;
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	m_HoverType = HoverNone;

	//If there's a selected/current xform, check it first so it gets precedence over the others.
	if (m_SelectedXform != NULL)
	{
		//These checks prevent highlighting the pre/post selected xform circle, when one is set to show all, and the other
		//is set to show current, and the user hovers over another xform, but doesn't select it, then moves the mouse
		//back over the hidden circle for the pre/post that was set to only show current.
		bool checkSelPre = preAll || (pre && m_HoverXform == m_SelectedXform);
		bool checkSelPost = postAll || (post && m_HoverXform == m_SelectedXform);

		if (CheckXformHover(m_SelectedXform, glCoords, bestDist, checkSelPre, checkSelPost))
		{
			m_HoverXform = m_SelectedXform;
			bestIndex = ember->GetTotalXformIndex(m_SelectedXform);
		}
	}

	//Check all xforms.
	for (unsigned int i = 0; i < ember->TotalXformCount(); i++)
	{
		Xform<T>* xform = ember->GetTotalXform(i);

		if (preAll || (pre && m_HoverXform == xform))//Only check pre affine if they are shown.
		{
			if (CheckXformHover(xform, glCoords, bestDist, true, false))
			{
				m_HoverXform = xform;
				bestIndex = i;
			}
		}

		if (postAll || (post && m_HoverXform == xform))//Only check post affine if they are shown.
		{
			if (CheckXformHover(xform, glCoords, bestDist, false, true))
			{
				m_HoverXform = xform;
				bestIndex = i;
			}
		}
	}

	return bestIndex;
}

/// <summary>
/// Determine the passed in xform's pre/post affine transforms are being hovered over.
/// Meant to be called in succession when checking all xforms for hover, and the best
/// hover distance is recorded in the bestDist reference parameter.
/// Mouse coordinates will be converted internally to world cartesian coordinates for checking.
/// </summary>
/// <param name="xform">A pointer to the xform to check for hover</param>
/// <param name="glCoords">The mouse raster coordinates to check</param>
/// <param name="bestDist">Reference to hold the best distance found so far</param>
/// <param name="pre">True to check pre affine, else don't.</param>
/// <param name="post">True to check post affine, else don't.</param>
/// <returns>True if hovering and the distance is smaller than the bestDist parameter</returns>
template <typename T>
bool GLEmberController<T>::CheckXformHover(Xform<T>* xform, v3T& glCoords, T& bestDist, bool pre, bool post)
{
	bool preFound = false, postFound = false;
	float dist = 0.0f;
	v3T pos;
	Ember<T>* ember = m_FractoriumEmberController->CurrentEmber();

	if (pre)
	{
		v3T translation(xform->m_Affine.C()/* - ember->m_CenterX*/, /*ember->m_CenterY + */xform->m_Affine.F(), 0);
		v3T transScreen = glm::project(translation, m_Modelview, m_Projection, m_Viewport);
		v3T xAxis(xform->m_Affine.A(), xform->m_Affine.D(), 0);
		v3T xAxisScreen = glm::project(translation + xAxis, m_Modelview, m_Projection, m_Viewport);
		v3T yAxis(xform->m_Affine.B(), xform->m_Affine.E(), 0);
		v3T yAxisScreen = glm::project(translation + yAxis, m_Modelview, m_Projection, m_Viewport);

		pos = translation;
		dist = glm::distance(glCoords, transScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverTranslation;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		pos = translation + xAxis;
		dist = glm::distance(glCoords, xAxisScreen);
	
		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverXAxis;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		pos = translation + yAxis;
		dist = glm::distance(glCoords, yAxisScreen);
	
		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverYAxis;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		if (preFound)
			m_AffineType = AffinePre;
	}

	if (post)
	{
		v3T translation(xform->m_Post.C()/* - ember->m_CenterX*/, /*ember->m_CenterY + */xform->m_Post.F(), 0);
		v3T transScreen = glm::project(translation, m_Modelview, m_Projection, m_Viewport);
		v3T xAxis(xform->m_Post.A(), xform->m_Post.D(), 0);
		v3T xAxisScreen = glm::project(translation + xAxis, m_Modelview, m_Projection, m_Viewport);
		v3T yAxis(xform->m_Post.B(), xform->m_Post.E(), 0);
		v3T yAxisScreen = glm::project(translation + yAxis, m_Modelview, m_Projection, m_Viewport);

		pos = translation;
		dist = glm::distance(glCoords, transScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverTranslation;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		pos = translation + xAxis;
		dist = glm::distance(glCoords, xAxisScreen);
	
		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverXAxis;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		pos = translation + yAxis;
		dist = glm::distance(glCoords, yAxisScreen);
	
		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = HoverYAxis;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		if (postFound)
			m_AffineType = AffinePost;
	}

	return preFound || postFound;
}

/// <summary>
/// Calculate the new affine transform when dragging with the x axis with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Rotate and scale only.
/// Local Pivot:
///		Shift: Rotate only about affine center.
///		Alt: Free transform.
///		Shift + Alt: Rotate single axis about affine center.
///		Control: Rotate and scale, snapping to grid.
///		Control + Shift: Rotate only, snapping to grid.
///		Control + Alt: Free transform, snapping to grid.
///		Control + Shift + Alt: Rotate single axis about affine center, snapping to grid.
/// World Pivot:
///		Shift + Alt: Rotate single axis about world center.
///		Control + Shift + Alt: Rotate single axis about world center, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
/// <returns>The new affine transform to be assigned to the selected xform</returns>
template <typename T>
Affine2D<T> GLEmberController<T>::CalcDragXAxis()
{
	v3T t3, newAxis, newPos;
	Affine2D<T> result = m_DragSrcTransform;
	bool worldPivotShiftAlt =  !m_Fractorium->LocalPivot() &&
							  ((m_DragModifier & DragModShift) == DragModShift) &&
							  ((m_DragModifier & DragModAlt) == DragModAlt);

	if (worldPivotShiftAlt)
		t3 = v3T(0, 0, 0);
	else
		t3 = v3T(m_DragSrcTransform.O(), 0);

	if ((m_DragModifier & DragModShift) == DragModShift)
	{
		v3T targetAxis = m_MouseWorldPos - t3;
		v3T norm = glm::normalize(targetAxis);

		if ((m_DragModifier & DragModControl) == DragModControl)
			norm = SnapToNormalizedAngle(norm, 24);
		
		if (worldPivotShiftAlt)
			newAxis = norm * glm::length(m_DragSrcTransform.O() + m_DragSrcTransform.X());
		else
			newAxis = norm * glm::length(m_DragSrcTransform.X());
	}
	else
	{
		if ((m_DragModifier & DragModControl) == DragModControl)
			newPos = SnapToGrid(m_MouseWorldPos);
		else
			newPos = m_MouseWorldPos + m_DragHandleOffset;

		newAxis = newPos - t3;
	}

	if ((m_DragModifier & DragModAlt) == DragModAlt)
	{
		if (worldPivotShiftAlt)
			result.X(v2T(newAxis) - m_DragSrcTransform.O());
		else
			result.X(v2T(newAxis));
	}
	else
	{
		result.RotateScaleXTo(v2T(newAxis));
	}

	m_DragHandlePos = v3T(result.O() + result.X(), 0);

	return result;
}

/// <summary>
/// Calculate the new affine transform when dragging with the y axis with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Rotate and scale only.
/// Local Pivot:
///		Shift: Rotate only about affine center.
///		Alt: Free transform.
///		Shift + Alt: Rotate single axis about affine center.
///		Control: Rotate and scale, snapping to grid.
///		Control + Shift: Rotate only, snapping to grid.
///		Control + Alt: Free transform, snapping to grid.
///		Control + Shift + Alt: Rotate single axis about affine center, snapping to grid.
/// World Pivot:
///		Shift + Alt: Rotate single axis about world center.
///		Control + Shift + Alt: Rotate single axis about world center, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
/// <returns>The new affine transform to be assigned to the selected xform</returns>
template <typename T>
Affine2D<T> GLEmberController<T>::CalcDragYAxis()
{
	v3T t3, newAxis, newPos;
	Affine2D<T> result = m_DragSrcTransform;
	bool worldPivotShiftAlt =  !m_Fractorium->LocalPivot() &&
							  ((m_DragModifier & DragModShift) == DragModShift) &&
							  ((m_DragModifier & DragModAlt) == DragModAlt);

	if (worldPivotShiftAlt)
		t3 = v3T(0, 0, 0);
	else
		t3 = v3T(m_DragSrcTransform.O(), 0);

	if ((m_DragModifier & DragModShift) == DragModShift)
	{
		v3T targetAxis = m_MouseWorldPos - t3;
		v3T norm = glm::normalize(targetAxis);

		if ((m_DragModifier & DragModControl) == DragModControl)
			norm = SnapToNormalizedAngle(norm, 24);

		if (worldPivotShiftAlt)
			newAxis = norm * glm::length(m_DragSrcTransform.O() + m_DragSrcTransform.Y());
		else
			newAxis = norm * glm::length(m_DragSrcTransform.Y());
	}
	else
	{
		if ((m_DragModifier & DragModControl) == DragModControl)
			newPos = SnapToGrid(m_MouseWorldPos);
		else
			newPos = m_MouseWorldPos + m_DragHandleOffset;

		newAxis = newPos - t3;
	}

	if ((m_DragModifier & DragModAlt) == DragModAlt)
	{
		if (worldPivotShiftAlt)
			result.Y(v2T(newAxis) - m_DragSrcTransform.O());
		else
			result.Y(v2T(newAxis));
	}
	else
	{
		result.RotateScaleYTo(v2T(newAxis));
	}

	m_DragHandlePos = v3T(result.O() + result.Y(), 0);

	return result;
}

/// <summary>
/// Calculate the new affine transform when dragging the center with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Free transform.
/// Local Pivot:
///		Shift: Rotate about world center, keeping orientation the same.
///		Control: Free transform, snapping to grid.
///		Control + Shift: Rotate about world center, keeping orientation the same, snapping to grid.
/// World Pivot:
///		Shift: Rotate about world center, rotating orientation.
///		Control + Shift: Rotate about world center, rotating orientation, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
/// <returns>The new affine transform to be assigned to the selected xform</returns>
template <typename T>
Affine2D<T> GLEmberController<T>::CalcDragTranslation()
{
	v3T newPos, newX, newY;
	Affine2D<T> result = m_DragSrcTransform;
	bool worldPivotShift = !m_Fractorium->LocalPivot() && ((m_DragModifier & DragModShift) == DragModShift);

	if ((m_DragModifier & DragModShift) == DragModShift)
	{
		v3T norm = glm::normalize(m_MouseWorldPos);
		
		if ((m_DragModifier & DragModControl) == DragModControl)
			norm = SnapToNormalizedAngle(norm, 12);
		
		newPos = glm::length(m_DragSrcTransform.O()) * norm;

		if (worldPivotShift)
		{
			T startAngle = atan2(m_DragSrcTransform.O().y, m_DragSrcTransform.O().x);
			T endAngle = atan2(newPos.y, newPos.x);
			T angle = startAngle - endAngle;
			
			result.Rotate(angle * RAD_2_DEG);
		}
	}
	else
	{
		if ((m_DragModifier & DragModControl) == DragModControl)
			newPos = SnapToGrid(m_MouseWorldPos);
		else
			newPos = m_MouseWorldPos + m_DragHandleOffset;
	}

	result.O(v2T(newPos));
	m_DragHandlePos = newPos;

	return result;
}

/// <summary>
/// Thin wrapper to check if all controllers are ok and return a pointer to the GLController.
/// </summary>
/// <returns>A pointer to the GLController if everything is ok, else false.</returns>
GLEmberControllerBase* GLWidget::GLController()
{
	if (m_Fractorium && m_Fractorium->ControllersOk())
		return m_Fractorium->m_Controller->GLController();

	return NULL;
}
