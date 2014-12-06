#include "FractoriumPch.h"
#include "GLEmberController.h"
#include "FractoriumEmberController.h"
#include "Fractorium.h"
#include "GLWidget.h"

/// <summary>
/// Constructor which assigns pointers to the main window and the GLWidget.
/// </summary>
/// <param name="fractorium">Pointer to the main window</param>
/// <param name="glWidget">Pointer to the GLWidget</param>
GLEmberControllerBase::GLEmberControllerBase(Fractorium* fractorium, GLWidget* glWidget)
{
	m_Fractorium = fractorium;
	m_GL = glWidget;
	m_AffineType = AffinePre;
	m_HoverType = HoverNone;
	m_DragState = DragNone;
	m_DragModifier = 0;
}

/// <summary>
/// Empty destructor which does nothing.
/// </summary>
GLEmberControllerBase::~GLEmberControllerBase() { }

/// <summary>
/// Constructor which passes the pointers to the main window the GLWidget to the base,
/// then assigns the pointer to the parent controller.
/// </summary>
/// <param name="fractorium">Pointer to the main window</param>
/// <param name="glWidget">Pointer to the GLWidget</param>
/// <param name="controller">Pointer to the parent controller of the same template type</param>
template <typename T>
GLEmberController<T>::GLEmberController(Fractorium* fractorium, GLWidget* glWidget, FractoriumEmberController<T>* controller)
	: GLEmberControllerBase(fractorium, glWidget)
{
	GridStep = T(1.0 / 8.0);
	m_FractoriumEmberController = controller;

	m_HoverXform = NULL;
	m_SelectedXform = NULL;
	m_CenterDownX = 0;
	m_CenterDownY = 0;
}

/// <summary>
/// Empty destructor which does nothing.
/// </summary>
template <typename T>
GLEmberController<T>::~GLEmberController() { }

/// <summary>
/// Check that the final output size of the current ember matches the dimensions passed in.
/// </summary>
/// <param name="w">The width to compare to</param>
/// <param name="h">The height to compare to</param>
/// <returns>True if any don't match, else false if they are both equal.</returns>
template <typename T>
bool GLEmberController<T>::CheckForSizeMismatch(int w, int h)
{
	return (m_FractoriumEmberController->FinalRasW() != w || m_FractoriumEmberController->FinalRasH() != h);
}

/// <summary>
/// Calculate the scale.
/// Used when dragging the right mouse button.
/// </summary>
/// <returns>The distance dragged in pixels</returns>
template <typename T>
T GLEmberController<T>::CalcScale()
{
	//Can't operate using world coords here because every time scale changes, the world bounds change.
	//So must instead calculate distance traveled based on window coords, which do not change outside of resize events.
	v2T windowCenter((T)m_GL->width() / T(2), (T)m_GL->height() / T(2));
	v2T windowMousePosDistanceFromCenter(m_MousePos.x - windowCenter.x, m_MousePos.y - windowCenter.y);
	v2T windowMouseDownDistanceFromCenter(m_MouseDownPos.x - windowCenter.x, m_MouseDownPos.y - windowCenter.y);

	T lengthMousePosFromCenterInPixels = glm::length(windowMousePosDistanceFromCenter);
	T lengthMouseDownFromCenterInPixels = glm::length(windowMouseDownDistanceFromCenter);

	return lengthMousePosFromCenterInPixels - lengthMouseDownFromCenterInPixels;
}

/// <summary>
/// Calculate the rotation.
/// Used when dragging the right mouse button.
/// </summary>
/// <returns>The angular distance rotated from -180-180</returns>
template <typename T>
T GLEmberController<T>::CalcRotation()
{
	T rotStart = NormalizeDeg180<T>(T(90) - (atan2(-m_MouseDownWorldPos.y, m_MouseDownWorldPos.x) * RAD_2_DEG_T));
	T rot = NormalizeDeg180<T>(T(90) - (atan2(-m_MouseWorldPos.y, m_MouseWorldPos.x) * RAD_2_DEG_T));

	return rotStart - rot;
}

/// <summary>
/// Snap the passed in world cartesian coordinate to the grid for rotation, scale or translation.
/// </summary>
/// <param name="vec">The world cartesian coordinate to be snapped</param>
/// <returns>The snapped world cartesian coordinate</returns>
template <typename T>
typename v3T GLEmberController<T>::SnapToGrid(v3T& vec)
{
	v3T ret;

	ret.x = glm::round(vec.x / GridStep) * GridStep;
	ret.y = glm::round(vec.y / GridStep) * GridStep;
	
	return ret;
}

/// <summary>
/// Snap the passed in world cartesian coordinate to the grid for rotation only.
/// </summary>
/// <param name="vec">The world cartesian coordinate to be snapped</param>
/// <param name="divisions">The divisions of a circle to use for snapping</param>
/// <returns>The snapped world cartesian coordinate</returns>
template <typename T>
typename v3T GLEmberController<T>::SnapToNormalizedAngle(v3T& vec, uint divisions)
{
	T rsq, theta;
	T bestRsq = numeric_limits<T>::max();
	v3T c, best;

	best.x = 1;
	best.y = 0;

	for (uint i = 0; i < divisions; i++)
	{
		theta = 2.0 * M_PI * (T)i / (T)divisions;
		c.x = cos(theta);
		c.y = sin(theta);
		rsq = glm::distance(vec, c);

		if (rsq < bestRsq)
		{
			best = c;
			bestRsq = rsq;
		}
	}

	return best;
}

/// <summary>
/// Convert raster window coordinates to world cartesian coordinates.
/// </summary>
/// <param name="v">The window coordinates to convert</param>
/// <param name="flip">True to flip vertically, else don't.</param>
/// <returns>The converted world cartesian coordinates</returns>
template <typename T>
typename v3T GLEmberController<T>::WindowToWorld(v3T& v, bool flip)
{
	v3T mouse(v.x, flip ? m_Viewport[3] - v.y : v.y, 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	v3T newCoords = glm::unProject(mouse, m_Modelview, m_Projection, m_Viewport);//Perform the calculation.

	newCoords.z = 0;//For some reason, unProject() always comes back with the z coordinate as something other than 0. It should be 0 at all times.
	return newCoords;
}

/// <summary>
/// Template specialization for querying the viewport, modelview and projection
/// matrices as floats.
/// </summary>
template <>
void GLEmberController<float>::QueryVMP()
{
	m_GL->glGetIntegerv(GL_VIEWPORT, glm::value_ptr(m_Viewport));
	m_GL->glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(m_Modelview));
	m_GL->glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(m_Projection));
}

#ifdef DO_DOUBLE
/// <summary>
/// Template specialization for querying the viewport, modelview and projection
/// matrices as doubles.
/// </summary>
template <>
void GLEmberController<double>::QueryVMP()
{
	m_GL->glGetIntegerv(GL_VIEWPORT, glm::value_ptr(m_Viewport));
	m_GL->glGetDoublev(GL_MODELVIEW_MATRIX, glm::value_ptr(m_Modelview));
	m_GL->glGetDoublev(GL_PROJECTION_MATRIX, glm::value_ptr(m_Projection));
}
#endif

/// <summary>
/// Template specialization for multiplying the current matrix
/// by an m4<float>.
/// </summary>
template <>
void GLEmberController<float>::MultMatrix(glm::detail::tmat4x4<float, glm::defaultp>& mat)
{
	m_GL->glMultMatrixf(glm::value_ptr(mat));
}

#ifdef DO_DOUBLE
/// <summary>
/// Template specialization for multiplying the current matrix
/// by an m4<double>.
/// </summary>
template <>
void GLEmberController<double>::MultMatrix(glm::detail::tmat4x4<double, glm::defaultp>& mat)
{
	m_GL->glMultMatrixd(glm::value_ptr(mat));
}
#endif

/// <summary>
/// Query the matrices currently being used.
/// Debugging function, unused.
/// </summary>
/// <param name="print">True to print values, else false.</param>
template <typename T>
void GLEmberController<T>::QueryMatrices(bool print)
{
	RendererBase* renderer = m_FractoriumEmberController->Renderer();

	if (renderer)
	{
		double unitX = fabs(renderer->UpperRightX(false) - renderer->LowerLeftX(false)) / 2.0;
		double unitY = fabs(renderer->UpperRightY(false) - renderer->LowerLeftY(false)) / 2.0;
		
		m_GL->glMatrixMode(GL_PROJECTION);
		m_GL->glPushMatrix();
		m_GL->glLoadIdentity();
		m_GL->glOrtho(-unitX, unitX, -unitY, unitY, -1, 1);
		m_GL->glMatrixMode(GL_MODELVIEW);
		m_GL->glPushMatrix();
		m_GL->glLoadIdentity();

		QueryVMP();

		m_GL->glMatrixMode(GL_PROJECTION);
		m_GL->glPopMatrix();
		m_GL->glMatrixMode(GL_MODELVIEW);
		m_GL->glPopMatrix();

		if (print)
		{
			for (int i = 0; i < 4; i++)
				qDebug() << "Viewport[" << i << "] = " << m_Viewport[i] << endl;

			for (int i = 0; i < 16; i++)
				qDebug() << "Modelview[" << i << "] = " << glm::value_ptr(m_Modelview)[i] << endl;

			for (int i = 0; i < 16; i++)
				qDebug() << "Projection[" << i << "] = " << glm::value_ptr(m_Projection)[i] << endl;
		}
	}
}