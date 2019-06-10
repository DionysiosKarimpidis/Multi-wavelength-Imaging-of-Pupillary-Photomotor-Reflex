#include "imageprocessthread.h"

ImageProcessThread::ImageProcessThread(QList<processRoutines>pr) : QThread(),
    doStop(false),
    doPause(false)
{
    photometryProc = pr.contains(PHOTOMETRY) ? true : false;
    mImageBuffer = new QImage();
}

void ImageProcessThread::run()
{
    while(1)
    {

        doStopMutex.lock();
        if(doStop)
        {
            doStop=false;
            doStopMutex.unlock();
            break;
        }
        doStopMutex.unlock();

        while(doPause)
        {
            Sleep(10);
        }

        if(photometryProc && !mImageBuffer->isNull())
            showPhotometry();
    }
}

void ImageProcessThread::handleFrame(QImage *img)
{
    pause();
    mImageBuffer = img;
    resume();
}

void ImageProcessThread::handleChangeProcess(QList<processRoutines> pr)
{
    pause();
    photometryProc = pr.contains(PHOTOMETRY) ? true : false;
    resume();
}

QString ImageProcessThread::getPixelPhotometry(int x, int y)
{
    if(photometryProc)
    {
        const int bpp = mImageBuffer->depth()/8;

        if(bpp==3 || bpp==4)
        {
            QColor color(mImageBuffer->pixel(x,y));
            QString phot = "Red: " + QString::number(color.red(),'f', 2) + " Green: " + QString::number(color.green(),'f', 2) + " Blue: " + QString::number(color.blue(),'f', 2);
            return phot;
        }
        else
        {
            return "Intensity: " + QString::number(qGray(mImageBuffer->pixel(x, y)), 'f', 2);
        }
    }
    else
        return "Error_EmptyImageButffer";
}

void ImageProcessThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop=true;
}

void ImageProcessThread::pause()
{
    QMutexLocker locker(&doPauseMutex);
    doPause = true;
}

void ImageProcessThread::resume()
{
    QMutexLocker locker(&doPauseMutex);
    doPause = false;
    pauseCondition.wakeAll();
}

void ImageProcessThread::showPhotometry()
{
    mSelectionBox.setWidth(PHOT_BOX_WIDTH);
    mSelectionBox.setHeight(PHOT_BOX_HEIGH);
    mSelectionBox.setLeft(mImageBuffer->width()/2.0 - 10);
    mSelectionBox.setTop(mImageBuffer->height()/2.0 - 10);

    const int bpp = mImageBuffer->depth()/8;

    if(bpp==3 || bpp==4)
    {
        ulong avR=0;
        ulong avG=0;
        ulong avB=0;
        QRgb pixel;

        for(int y=0; y<mSelectionBox.height(); y++){
            for(int x=0; x<mSelectionBox.width(); x++){
                pixel = mImageBuffer->pixel(mSelectionBox.x() + x, mSelectionBox.y() + y);
                QColor color(pixel);
                avR += color.red();
                avG += color.green();
                avB += color.blue();
            }
        }

        double avgR = (double)avR/(double)(mSelectionBox.height()*mSelectionBox.width());
        double avgG = (double)avG/(double)(mSelectionBox.height()*mSelectionBox.width());
        double avgB = (double)avB/(double)(mSelectionBox.height()*mSelectionBox.width());

        QString phot = "Red: " + QString::number(avgR,'f', 2) + " Green: " + QString::number(avgG,'f', 2) + " Blue: " + QString::number(avgB,'f', 2);

        emit sendPhotometry(phot);
    }
    else
    {
        ulong avR=0;
        for(int y=0; y<mSelectionBox.height(); y++){
            for(int x=0; x<mSelectionBox.width(); x++){
                avR += qGray(mImageBuffer->pixel(mSelectionBox.x() + x, mSelectionBox.y() + y));
            }
        }

        emit sendPhotometry(QString::number((double)avR/(double)(mSelectionBox.height()*mSelectionBox.width()),'f', 2));
    }
}
