#include "customview.h"

#include <QGraphicsScene>
#include <QScrollBar>
#include <QDebug>
#include <QPixmap>
#include <QTransform>


CustomView::CustomView(QWidget *parent):
    QGraphicsView(parent),
    numScheduledScalings(0),
    leftButtonPressed(false),
    mouseHover(true),
    multiCursorsMode(false)

{
    //setStyleSheet("QCustomView { border-style: solid; background-color: transparent;}");

    //Create a new Graphics Scene for the Graphics Viewer
    setScene(new QGraphicsScene());

    item = new QGraphicsPixmapItem();
    scene()->addItem(item);

    //Mouse preferences
    setEnabled(true);
    setMouseTracking(true);

    //Scroll Bars preferences
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAttribute(Qt::WA_AcceptTouchEvents);

    //    totalScaleFactor = 1;
}

void CustomView::setMultiCursorsMode(bool b)
{
    multiCursorsMode = b;

    if(!b){

        cursor.hide();

        mouseHover=true;
    }

}

void CustomView::initializeCursor()
{
    //    scene()->addItem(&cursor);
    //    cursor.setPixmap(QPixmap(":/Cursors/crossw"));
    //    cursor.setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    //    cursor.hide();
}


void CustomView::adjustViewer(QGroupBox *b)
{
    QPointF p = mapFromScene(0,0);
    QPoint p2 = mapToParent(QPoint(width(), 0));
    QPoint p3 = b->mapToParent(QPoint(0,0));

    int x = (int)p.x();
    int y = (int)p.y();

    if(x>5)
    {
        setFixedWidth(width()-(2*x));

        p2 = mapToParent(QPoint(width(), 0));
        p3 = b->mapToParent(QPoint(0,0));

        if(p2.x()>p3.x())
        {
            setFixedWidth(width() - (p2.x() - p3.x()));
        }
    }
    else if(p2.x()>p3.x())
    {
        setFixedWidth(width() - (p2.x() - p3.x()));
    }
    else if(y>5)
    {
        y = 3*y;

        int bw =  b->width();
        int bmw = b->minimumWidth();

        if((bw - y)>bmw)
        {
            setFixedWidth(width() + y);
        }
        else
        {
            setFixedWidth(width() + (bw - bmw));
        }

    }

    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void CustomView::setZoomLowLimt()
{
    zoomLowLimit = transform().m11();
}


void CustomView::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    numScheduledScalings += numSteps;

    if(numScheduledScalings * numSteps < 0)
        numScheduledScalings = numSteps;

    zoom(numScheduledScalings);

}


void CustomView::zoom(int num){

    numScheduledScalings=num;

    anim = new QTimeLine(400, this);

    anim->setUpdateInterval(25);

    connect(anim, SIGNAL (valueChanged(qreal)), SLOT (scalingTime(qreal)));
    connect(anim, SIGNAL (finished()), SLOT (animFinished()));

    anim->start();
}

void CustomView::getFactor(qreal factor)
{
    scale(factor, factor);
}


void CustomView::scalingTime(qreal x)
{
    Q_UNUSED(x);

    qreal factor = 1.0 + qreal(numScheduledScalings) / 300.0;

    qreal temp = transform().m11()*factor;

    if(temp<zoomLowLimit)
    {
        anim->stop();

        emit zoomLowest();

        resizeEvent(0);

        return;
    }
    else if(temp>1)
    {
        anim->stop();

        return;
    }

    scale(factor, factor);

    emit zoomFactor(factor);
}

void CustomView::animFinished()
{
    if (numScheduledScalings > 0)
        numScheduledScalings--;
    else
        numScheduledScalings++;


    /**************************************************/
    /* Test Code */

    QPointF tmp = this->mapToScene(0,0);
    QPointF tmp1 = this->mapToScene(this->width(), this->height());
    int x1 = tmp.x();
    int y1 = tmp.y();
    int mwidth = tmp1.x() - x1;
    int mheight = tmp1.y() - y1;
    emit zoomSignal(x1,y1,mwidth,mheight);

    /***************************************************/


    sender()->~QObject();

}


void CustomView::mousePressEvent(QMouseEvent *event){


    if(event->button()==Qt::LeftButton){

        leftButtonPressed=true;

        setCursor(Qt::ClosedHandCursor);

        X=event->globalX();
        Y=event->globalY();

    }
}

void CustomView::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->button()==Qt::LeftButton){

        leftButtonPressed=false;

        setCursor(Qt::ArrowCursor);
    }

    else if(event->button()==Qt::RightButton)
    {

        /**************************************************/
        /* Test Code */

        QPointF tmp = this->mapToScene(0,0);
        QPointF tmp1 = this->mapToScene(this->width(), this->height());
        int x1 = tmp.x();
        int y1 = tmp.y();
        int mwidth = tmp1.x() - x1;
        int mheight = tmp1.y() - y1;

        emit positionSignal(x1, y1, mwidth, mheight);
        QPointF point;
        point.setX(event->x());
        point.setY(event->y());
        QPointF p = this->mapToScene(event->x(),event->y());
        //emit positionSignal(p.rx(), p.ry(), event->x(), event->y());
        /**************************************************/
        emit plotingSavePosition();
    }

}

void CustomView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point;
    point.setX(event->x());
    point.setY(event->y());
    QPointF p = this->mapToScene(event->x(),event->y());
    emit plotingPosition(p.rx(), p.ry());

    if(leftButtonPressed){

        int x=event->globalX();
        int y=event->globalY();

        int hval = horizontalScrollBar()->value() + X - x;
        int vval = verticalScrollBar()->value() + Y - y;

        horizontalScrollBar()->setValue(hval);
        verticalScrollBar()->setValue(vval);

        emit frameMove(transform().m11(), hval, vval);

        X=x;
        Y=y;
    }

    if(multiCursorsMode){

        if(mouseHover){

            mouseHover=false;

            cursor.hide();

            emit mouseHoveringHere();
        }

        QPointF p = mapToScene(event->pos());

        emit cursorPosition(p);
    }
}

void CustomView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked();
}

void CustomView::getFrameMove(qreal scaling, int hval, int vval)
{
    Q_UNUSED(scaling);

    horizontalScrollBar()->setValue(hval);

    verticalScrollBar()->setValue(vval);
}

void CustomView::getCursorPosition(QPointF pos)
{
    QPoint p = mapFromScene(pos);

    pos = mapToScene(p.x()-22, p.y()-18);

    cursor.setPos(pos);
}

void CustomView::getHideCursor()
{
    cursor.hide();
}

void CustomView::displayCursor()
{
    mouseHover=true;

    cursor.show();
}

void CustomView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    mouseHover=true;

    emit hideCursor();
}

bool CustomView::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

        if (touchPoints.count() == 2)
        {
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

            length =  QLineF(touchPoint0.pos(), touchPoint1.pos()).length();
        }

        break;
    }
    case QEvent::TouchUpdate:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

        if (touchPoints.count() == 2)
        {
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

            qreal currentScaleFactor = 1.02;

            qreal newlength =  QLineF(touchPoint0.pos(), touchPoint1.pos()).length();

            if(length > newlength)
                currentScaleFactor = 0.98;
            else if(length == newlength)
                break;

            length = newlength;

            qreal temp = transform().m11()*currentScaleFactor;

            if(temp<zoomLowLimit)
            {
                resizeEvent(0);

                emit zoomLowest();

                return true;
            }
            else if(temp>1)
            {
                return true;
            }

            scale(currentScaleFactor, currentScaleFactor);

            emit zoomFactor(currentScaleFactor);
        }

        return true;
    }
    default:
        break;
    }

    return QGraphicsView::viewportEvent(event);
}

QGraphicsPixmapItem *CustomView::getItem()
{
    return item;
}


