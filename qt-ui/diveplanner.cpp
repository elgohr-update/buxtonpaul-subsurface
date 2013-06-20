#include "diveplanner.h"
#include <QMouseEvent>
#include <QDebug>

DivePlanner* DivePlanner::instance()
{
	static DivePlanner *self = new DivePlanner();
	return self;
}

DivePlanner::DivePlanner(QWidget* parent): QGraphicsView(parent), activeDraggedHandler(0)
{
	setMouseTracking(true);
	setScene( new QGraphicsScene());
	scene()->setSceneRect(0,0,100,100);

	verticalLine = new QGraphicsLineItem(0,0,0, 100);
	verticalLine->setPen(QPen(Qt::DotLine));
	scene()->addItem(verticalLine);

	horizontalLine = new QGraphicsLineItem(0,0,100,0);
	horizontalLine->setPen(QPen(Qt::DotLine));
	scene()->addItem(horizontalLine);
}

void DivePlanner::mouseDoubleClickEvent(QMouseEvent* event)
{
	QPointF mappedPos = mapToScene(event->pos());
	if(isPointOutOfBoundaries(mappedPos))
		return;

	if(handles.count() && handles.last()->x() > mappedPos.x()){
		return;
	}

	DiveHandler  *item = new DiveHandler ();
	item->setRect(-5,-5,10,10);
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	item->setPos( mappedPos );
    scene()->addItem(item);
	handles << item;

	if (lines.empty()){
		QGraphicsLineItem *first = new QGraphicsLineItem(0,0, mappedPos.x(), mappedPos.y());
		item->from = first;
		lines.push_back(first);
		create_deco_stop();
		scene()->addItem(first);
	}else{
		clear_generated_deco();
		DiveHandler *prevHandle = handles.at( handles.count()-2);
		QGraphicsLineItem *line = new QGraphicsLineItem(prevHandle->x(), prevHandle->y(), item->x(), item->y());
		prevHandle->to = line;
		item->from = line;
		lines.push_back(line);
		scene()->addItem(line);
		create_deco_stop();
	}
}

void DivePlanner::clear_generated_deco()
{
	for(int i = handles.count(); i <= lines.count(); i++){
		scene()->removeItem(lines.last());
		delete lines.last();
		lines.removeLast();
	}
}

void DivePlanner::create_deco_stop()
{
	// this needs to create everything
	// for the calculated deco. it should return the *first*
	// line that's calculated, so the
	QGraphicsLineItem *item = new QGraphicsLineItem(handles.last()->x(), handles.last()->y(), 100, 0);
	scene()->addItem(item);
	lines << item;
}

void DivePlanner::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
	fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void DivePlanner::showEvent(QShowEvent* event)
{
    QGraphicsView::showEvent(event);
	fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void DivePlanner::mouseMoveEvent(QMouseEvent* event)
{
	QPointF mappedPos = mapToScene(event->pos());
	if (isPointOutOfBoundaries(mappedPos))
		return;

	verticalLine->setLine(mappedPos.x(), 0, mappedPos.x(), 100);
	horizontalLine->setLine(0, mappedPos.y(), 100, mappedPos.y());

	if(activeDraggedHandler)
		moveActiveHandler(mappedPos);

	if (!handles.count())
		return;

	if (handles.last()->x() > mappedPos.x()){
		verticalLine->setPen( QPen(QBrush(Qt::red), 0, Qt::SolidLine));
		horizontalLine->setPen( QPen(QBrush(Qt::red), 0, Qt::SolidLine));
	}else{
		verticalLine->setPen(QPen(Qt::DotLine));
		horizontalLine->setPen(QPen(Qt::DotLine));
	}
}

void DivePlanner::moveActiveHandler(QPointF pos)
{
	int idx = handles.indexOf(activeDraggedHandler);
	bool moveLines = false;;
	// do not allow it to move between handlers.
	if (handles.count() > 1){
		if (idx == 0 ){ // first
			if (pos.x() < handles[1]->x()){
				activeDraggedHandler->setPos(pos);
				moveLines = true;
			}
		}else if (idx == handles.count()-1){ // last
			if (pos.x() > handles[idx-1]->x()){
				activeDraggedHandler->setPos(pos);
				moveLines = true;
			}
		}else{ // middle
			if (pos.x() > handles[idx-1]->x() && pos.x() < handles[idx+1]->x()){
				activeDraggedHandler->setPos(pos);
				moveLines = true;
			}
		}
	}else{
		activeDraggedHandler->setPos(pos);
		moveLines = true;
	}
	if (moveLines){
		if (activeDraggedHandler->from){
			QLineF f = activeDraggedHandler->from->line();
			activeDraggedHandler->from->setLine(f.x1(), f.y1(), pos.x(), pos.y());
		}

		if (activeDraggedHandler->to){
			QLineF f = activeDraggedHandler->to->line();
			activeDraggedHandler->to->setLine(pos.x(), pos.y(), f.x2(), f.y2());
		}

		if(activeDraggedHandler == handles.last()){
			clear_generated_deco();
			create_deco_stop();
		}
	}
}

bool DivePlanner::isPointOutOfBoundaries(QPointF point)
{
	if (point.x() > sceneRect().width()
	||  point.x() < 0
	||  point.y() < 0
	|| point.y() > sceneRect().height())
	{
		return true;
	}
	return false;
}

void DivePlanner::mousePressEvent(QMouseEvent* event)
{
    QPointF mappedPos = mapToScene(event->pos());
	Q_FOREACH(QGraphicsItem *item, scene()->items(mappedPos)){
		if (DiveHandler *h = qgraphicsitem_cast<DiveHandler*>(item)){
			activeDraggedHandler = h;
			activeDraggedHandler->setBrush(Qt::red);
		}
	}
}

void DivePlanner::mouseReleaseEvent(QMouseEvent* event)
{
	if (activeDraggedHandler)
		activeDraggedHandler = 0;
}

DiveHandler::DiveHandler(): QGraphicsEllipseItem(), from(0), to(0)
{
}

void Ruler::setMaximum(double maximum)
{
	qDeleteAll(ticks);
	max = maximum;
	updateTicks();
}

void Ruler::setMinimum(double minimum)
{
	qDeleteAll(ticks);
	min = minimum;
	updateTicks();
}

Ruler::Ruler() : orientation(Qt::Horizontal)
{
}

void Ruler::setOrientation(Qt::Orientation o)
{
	orientation = o;
	updateTicks();
}

void Ruler::updateTicks()
{

}

void Ruler::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
{

}

void Ruler::setTickInterval(double interval)
{

}
