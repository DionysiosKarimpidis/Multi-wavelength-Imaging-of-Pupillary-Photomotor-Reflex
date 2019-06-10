#ifndef IMAGEREGISTRATIONPANEL_H
#define IMAGEREGISTRATIONPANEL_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QRect>
#include <QPixmap>
#include <QImage>
#include <QTimer>

namespace Ui {
class ImageRegistrationPanel;
}

class ImageRegistrationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageRegistrationPanel(QWidget *parent = 0);
    ~ImageRegistrationPanel();

public slots:
    void handleImages(QPixmap piPx, QPixmap zeroPx);
    void stop();
    void handlleSizeChange(int x, int y);
private slots:
    void closeEvent(QCloseEvent *event);
    void resize();
    void resizeEvent(QResizeEvent *event);
private:
    Ui::ImageRegistrationPanel *ui;

    QRect mFrameRect;

    QGraphicsPixmapItem *mPixmapItem;
    bool imgState;

    int mWidth;
    int mHeight;

signals:
    void beginImgTransaction(bool state);
};

#endif // IMAGEREGISTRATIONPANEL_H
