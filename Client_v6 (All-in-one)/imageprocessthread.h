#ifndef IMAGEPROCESSTHREAD_H
#define IMAGEPROCESSTHREAD_H

#include <QDebug>
#include <QThread>
#include <QString>
#include <QMutex>
#include <QObject>
#include <QList>
#include <QImage>
#include <QWaitCondition>
#include <QRect>
#include <QRgb>
#include <QColor>

#include "windows.h"
#include "definitions.h"

class ImageProcessThread : public QThread
{
    Q_OBJECT

public:
    ImageProcessThread(QList<processRoutines>pr);

protected:
    void run();

private:
    bool photometryProc;
    QImage *mImageBuffer;
    QRect mSelectionBox;

    volatile bool doStop;
    volatile bool doPause;

    QMutex doStopMutex;
    QMutex doPauseMutex;
    QMutex mCaptureFrameMutex;
    QMutex mCaptureCurrentFrameMutex;
    QWaitCondition pauseCondition;

public slots:
    void handleFrame(QImage *img);
    void handleChangeProcess(QList<processRoutines>pr);
    QString getPixelPhotometry(int x, int y);
    void stop();

private slots:
    void pause();
    void resume();
    void showPhotometry();

signals:
    void sendPhotometry(QString);
};

#endif // IMAGEPROCESSTHREAD_H
