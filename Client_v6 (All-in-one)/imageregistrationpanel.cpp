#include "imageregistrationpanel.h"
#include "ui_imageregistrationpanel.h"

ImageRegistrationPanel::ImageRegistrationPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageRegistrationPanel)
{
    ui->setupUi(this);

    mPixmapItem = ui->graphicsView->getItem();

    connect(ui->graphicsView, SIGNAL(resizeSignal(QResizeEvent*)), this, SLOT(resizeEvent(QResizeEvent*)));
    connect(ui->graphicsView, SIGNAL(frameMove(qreal,int,int)), this, SIGNAL(frameMove(qreal,int,int)));
    connect(ui->graphicsView, SIGNAL(zoomFactor(qreal)), this, SIGNAL(zoomFactor(qreal)));
    connect(ui->graphicsView, SIGNAL(zoomLowest()), this, SIGNAL(zoomLowest()));
    connect(this, SIGNAL(getFactor(qreal)), ui->graphicsView, SLOT(getFactor(qreal)));
    connect(this, SIGNAL(getFrameMove(qreal,int,int)), ui->graphicsView, SLOT(getFrameMove(qreal,int,int)));
    mFrameRect.setWidth(1640);
    mFrameRect.setHeight(1232);

    QTimer::singleShot(200, this, SLOT(resize()));

    imgState = false;
}

ImageRegistrationPanel::~ImageRegistrationPanel()
{
    delete ui;
}

void ImageRegistrationPanel::resize()
{
    resizeEvent(0);
}

void ImageRegistrationPanel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->graphicsView->fitInView(mFrameRect, Qt::KeepAspectRatio);

    ui->graphicsView->setZoomLowLimt();
}

void ImageRegistrationPanel::handlleSizeChange(int x, int y)
{
    mFrameRect.setWidth(x);
    mFrameRect.setHeight(y);

    QTimer::singleShot(200, this, SLOT(resize()));
}

void ImageRegistrationPanel::handleImages(QPixmap piPx, QPixmap zeroPx)
{
    QImage piImg = piPx.toImage().convertToFormat(QImage::Format_RGB888);
    QImage zeroImg = zeroPx.toImage().convertToFormat(QImage::Format_RGB888);

    int h = piImg.height();
    int w = piImg.width();

    QImage diffImg(w, h, QImage::Format_RGB888);

    const int bpp = zeroImg.depth()/8;

    for(int y = 0; y < h; y++)
    {
        uchar *rowData = piImg.scanLine(y);
        uchar *rowData2 = zeroImg.scanLine(y);
        uchar *rowDataRes = diffImg.scanLine(y);

        for(int x = 0; x < w; x++)
        {

            if((*(rowData2+1) - *(rowData+1)) > 0)
                *rowDataRes = (*(rowData2+1) - *(rowData+1));
            else
                *(rowDataRes+2) = (*(rowData+1) - *(rowData2+1));

            *(rowDataRes+1) = 0;

            rowData+=bpp;
            rowData2+=bpp;
            rowDataRes+=bpp;
        }
    }

    mPixmapItem->setPixmap(QPixmap::fromImage(diffImg));
}

void ImageRegistrationPanel::stop()
{
    close();
}

void ImageRegistrationPanel::closeEvent(QCloseEvent *event)
{
    emit beginImgTransaction(false);
}

