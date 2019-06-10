#ifndef FSMODE_H
#define FSMODE_H

#include <QWidget>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QRect>
#include <QAction>

namespace Ui {
class FSMode;
}

class FSMode : public QWidget

{
    Q_OBJECT

public:
    explicit FSMode(QWidget *parent = 0);

    ~FSMode();

    void setPixmap(QPixmap p);

    void setPixmap_2(QPixmap p);

    void setRect(QRect r);

    void showZeroViewer(bool b);

    void showViewer(bool b);

private:

    void resizeEvent(QResizeEvent *event);

signals:

    void exitFS(bool b);

private slots:

    void exit(bool b);

private:
    Ui::FSMode *ui;

    QGraphicsPixmapItem item, item_2;

    QRect rect;

    QAction *action;
};

#endif // FSMODE_H
