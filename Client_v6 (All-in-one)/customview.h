#ifndef CUSTOMVIEW_H
#define CUSTOMVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QTimeLine>
#include <QPointF>
#include <QGroupBox>
#include <QEvent>
#include <QResizeEvent>


class CustomView: public QGraphicsView
{
    Q_OBJECT

public:

    explicit CustomView(QWidget *parent = 0);

    void setMultiCursorsMode(bool b);

    void initializeCursor();

    void adjustViewer(QGroupBox *b);

    void setZoomLowLimt();

private:

    void zoom(int num);

public slots:

     void getCursorPosition(QPointF pos);

     void getHideCursor();

private slots:

    void wheelEvent (QWheelEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event);

    void scalingTime(qreal x);

    void animFinished();

    void getFactor(qreal factor);

    void getFrameMove(qreal scaling, int hval, int vval);

    void displayCursor();

    void leaveEvent(QEvent *event);

    bool viewportEvent(QEvent *event);

public slots:
    QGraphicsPixmapItem* getItem();

signals:

    void zoomFactor(qreal factor);

    void frameMove(qreal scaling, int hval, int vval);

    void cursorPosition(QPointF pos);

    void doubleClicked();

    void mouseHoveringHere();

    void zoomLowest();

    void hideCursor();

    //Testing Signals

    void zoomSignal(int,int,int,int);

    void positionSignal(int,int,int,int);

    void plotingPosition(int,int);

    void plotingSavePosition();

private:

    QGraphicsPixmapItem cursor;

    QGraphicsPixmapItem *item;

    QTimeLine *anim;

    qreal zoomLowLimit;

    int numScheduledScalings, X, Y;

    bool leftButtonPressed, mouseHover, multiCursorsMode;

    qreal length;

};

#endif // CUSTOMVIEW_H
