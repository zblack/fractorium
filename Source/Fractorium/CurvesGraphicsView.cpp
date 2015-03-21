#include "FractoriumPch.h"
#include "CurvesGraphicsView.h"

/// <summary>
/// Construct the scene which will have a fixed rect.
/// Construct all points, pens and axes.
/// </summary>
/// <param name="parent">Pass to the parent</param>
CurvesGraphicsView::CurvesGraphicsView(QWidget* parent)
	: QGraphicsView(parent)
{
	m_Scene.setSceneRect(0, 0, 245, 245);

	m_AllP1 = new EllipseItem(QRectF(-5, -5, 10, 10), 0, 1, this);
	m_AllP1->setBrush(QBrush(Qt::GlobalColor::black));
	m_AllP2 = new EllipseItem(QRectF(-5, -5, 10, 10), 0, 2, this);
	m_AllP2->setBrush(QBrush(Qt::GlobalColor::black));

	m_RedP1 = new EllipseItem(QRectF(-5, -5, 10, 10), 1, 1, this);
	m_RedP1->setBrush(QBrush(Qt::GlobalColor::red));
	m_RedP2 = new EllipseItem(QRectF(-5, -5, 10, 10), 1, 2, this);
	m_RedP2->setBrush(QBrush(Qt::GlobalColor::red));

	m_GrnP1 = new EllipseItem(QRectF(-5, -5, 10, 10), 2, 1, this);
	m_GrnP1->setBrush(QBrush(Qt::GlobalColor::green));
	m_GrnP2 = new EllipseItem(QRectF(-5, -5, 10, 10), 2, 2, this);
	m_GrnP2->setBrush(QBrush(Qt::GlobalColor::green));

	m_BluP1 = new EllipseItem(QRectF(-5, -5, 10, 10), 3, 1, this);
	m_BluP1->setBrush(QBrush(Qt::GlobalColor::blue));
	m_BluP2 = new EllipseItem(QRectF(-5, -5, 10, 10), 3, 2, this);
	m_BluP2->setBrush(QBrush(Qt::GlobalColor::blue));

	m_AxisPen = QPen(Qt::GlobalColor::white);
	m_XLine = new QGraphicsLineItem();
	m_XLine->setPen(m_AxisPen);
	m_XLine->setZValue(0);
	m_YLine = new QGraphicsLineItem();
	m_YLine->setPen(m_AxisPen);
	m_YLine->setZValue(0);
	
	m_Scene.addItem(m_AllP1); m_Points[0].first  = m_AllP1; m_AllP1->setZValue(2);
	m_Scene.addItem(m_AllP2); m_Points[0].second = m_AllP2; m_AllP2->setZValue(2);
	m_Scene.addItem(m_RedP1); m_Points[1].first  = m_RedP1;
	m_Scene.addItem(m_RedP2); m_Points[1].second = m_RedP2;
	m_Scene.addItem(m_GrnP1); m_Points[2].first  = m_GrnP1;
	m_Scene.addItem(m_GrnP2); m_Points[2].second = m_GrnP2;
	m_Scene.addItem(m_BluP1); m_Points[3].first  = m_BluP1;
	m_Scene.addItem(m_BluP2); m_Points[3].second = m_BluP2;
	m_Scene.addItem(m_XLine);
	m_Scene.addItem(m_YLine);

	m_APen = QPen(Qt::GlobalColor::black); m_Pens[0] = &m_APen;
	m_RPen = QPen(Qt::GlobalColor::red);   m_Pens[1] = &m_RPen;
	m_GPen = QPen(Qt::GlobalColor::green); m_Pens[2] = &m_GPen;
	m_BPen = QPen(Qt::GlobalColor::blue);  m_Pens[3] = &m_BPen;

	m_APen.setWidth(2);
	m_RPen.setWidth(2);
	m_GPen.setWidth(2);
	m_BPen.setWidth(2);

	setScene(&m_Scene);
	SetTop(CurveIndex::ALL);
	//qDebug() << "Original scene rect before setting anything is: " << sceneRect();
	m_OriginalRect = sceneRect();
	show();
	//qDebug() << "Original scene rect is: " << m_OriginalRect;
}

/// <summary>
/// Get the position of a given point within a given curve.
/// </summary>
/// <param name="curveIndex">The curve whose point value will be retrieved, 0-3.</param>
/// <param name="pointIndex">The point within the curve value will be retrieved, 1-2.</param>
/// <param name="point">The position of the point. X,Y will each be within 0-1.</param>
void CurvesGraphicsView::PointChanged(int curveIndex, int pointIndex, const QPointF& point)
{
	double x = point.x() / width();
	double y = (height() - point.y()) / height();

	emit PointChangedSignal(curveIndex, pointIndex, QPointF(x, y));
}

/// <summary>
/// Get the position of a given point within a given curve.
/// </summary>
/// <param name="curveIndex">The curve whose point value will be retrieved, 0-3.</param>
/// <param name="pointIndex">The point within the curve whose value will be retrieved, 1-2.</param>
/// <returns>The position of the point. X,Y will each be within 0-1.</returns>
QPointF CurvesGraphicsView::Get(int curveIndex, int pointIndex)
{
	if (curveIndex < 4)
	{
		EllipseItem* item = (pointIndex == 1) ? m_Points[curveIndex].first : m_Points[curveIndex].second;

		return QPointF(item->pos().x() / width(), (height() - item->pos().y()) / height());
	}

	return QPointF();
}

/// <summary>
/// Set the position of a given point within a given curve.
/// </summary>
/// <param name="curveIndex">The curve whose point will be set, 0-3.</param>
/// <param name="pointIndex">The point within the curve which will be set, 1-2</param>
/// <param name="point">The position to set the point to. X,Y will each be within 0-1.</param>
void CurvesGraphicsView::Set(int curveIndex, int pointIndex, const QPointF& point)
{
	if (curveIndex < 4)
	{
		if (pointIndex == 1)
			m_Points[curveIndex].first->setPos(point.x() * width(), (1.0 - point.y()) * height());//Scale to scene dimensions, Y axis is flipped.
		else
			m_Points[curveIndex].second->setPos(point.x() * width(), (1.0 - point.y()) * height());
	}
}

/// <summary>
/// Set the topmost curve but setting its Z value.
/// All other curves will get a value one less.
/// </summary>
/// <param name="curveIndex">The curve to set</param>
void CurvesGraphicsView::SetTop(CurveIndex curveIndex)
{
	int index;

	switch (curveIndex)
	{
	case CurveIndex::ALL:
		index = 0;
		break;
	case CurveIndex::RED:
		index = 1;
		break;
	case CurveIndex::GREEN:
		index = 2;
		break;
	case CurveIndex::BLUE:
	default:
		index = 3;
	}

	for (int i = 0; i < 4; i++)
	{
		if (i == index)
		{
			m_Points[i].first->setZValue(2);
			m_Points[i].second->setZValue(2);
		}
		else
		{
			m_Points[i].first->setZValue(1);
			m_Points[i].second->setZValue(1);
		}
	}
}

/// <summary>
/// Overridden paint even which draws the points, axes and curves.
/// </summary>
/// <param name="e">Ignored</param>
void CurvesGraphicsView::paintEvent(QPaintEvent* e)
{
	QGraphicsView::paintEvent(e);
	
	int i;
	QRectF rect = scene()->sceneRect();
	double w2 = width() / 2;
	double h2 = height() / 2;

	//Draw axis lines.
	m_XLine->setLine(QLineF(0, h2, width(), h2));
	m_YLine->setLine(QLineF(w2, 0, w2, height()));

	//This must be constructed every time and cannot be a member.
	QPainter painter(viewport());
	painter.setClipRect(rect);
	painter.setRenderHint(QPainter::Antialiasing);

	//Create 4 new paths. These must be constructed every time and cannot be members.
	QPainterPath paths[4] =
	{
		QPainterPath(mapFromScene(rect.bottomLeft())),
		QPainterPath(mapFromScene(rect.bottomLeft())),
		QPainterPath(mapFromScene(rect.bottomLeft())),
		QPainterPath(mapFromScene(rect.bottomLeft()))
	};

	//Draw paths for all but the topmost curve.
	for (i = 0; i < 4; i++)
	{
		paths[i].cubicTo(m_Points[i].first->pos(), m_Points[i].second->pos(), mapFromScene(rect.topRight()));

		if (m_Points[i].first->zValue() == 1)
		{
			painter.setPen(*m_Pens[i]);
			painter.drawPath(paths[i]);
		}
	}

	//Draw the topmost curve.
	for (i = 0; i < 4; i++)
	{
		if (m_Points[i].first->zValue() == 2)
		{
			painter.setPen(*m_Pens[i]);
			painter.drawPath(paths[i]);
			break;
		}
	}
}