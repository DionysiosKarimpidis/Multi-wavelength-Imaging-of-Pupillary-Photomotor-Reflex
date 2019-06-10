#ifndef PUPILMEASURETHREAD_H
#define PUPILMEASURETHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QString>
#include <QImage>
#include <QPainter>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "windows.h"
#include <vector>
#include "definitions.h"

class pupilMeasureThread: public QThread
{
    Q_OBJECT
public:
    pupilMeasureThread();

public slots:
    void updateImage(QPixmap img);
    void stop();
    void setMeasureSpecs(EyeParameters param);
    void setMinThresValue(int mt);
    void setMaxThresValue(int mt);
    void setAreaValue(int val);
protected:
    void run();

signals:
    void sendDiameter(int);
    void sendImage(QImage);

private slots:
    void pupilMeasure();

private:
    double mDiameter;
    bool stopThread;
    bool running;
    QImage *mImage;

    EyeParameters mParam;
};

#endif // PUPILMEASURETHREAD_H
