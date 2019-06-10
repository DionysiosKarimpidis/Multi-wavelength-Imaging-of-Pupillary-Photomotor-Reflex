#include "pupilmeasurethread.h"

pupilMeasureThread::pupilMeasureThread()
{
    mDiameter = 0;
    stopThread = true;
    running = false;

    mImage = new QImage();
}

void pupilMeasureThread::run()
{

    stopThread = false;

    while(!stopThread)
    {
        Sleep(30);
        if(!mImage->isNull())
        {
            pupilMeasure();
        }
        delete mImage;
        mImage = new QImage();
    }
}

void pupilMeasureThread::stop()
{
    stopThread = true;
}

void pupilMeasureThread::setMeasureSpecs(EyeParameters param)
{
    mParam = param;
}

void pupilMeasureThread::updateImage(QPixmap img)
{
    if(!stopThread)
    {
        if(!running)
        {
            *mImage = img.toImage();
        }
    }
}

void pupilMeasureThread::setMaxThresValue(int mt)
{
    mParam.maxThresValue = mt;
}

void pupilMeasureThread::setMinThresValue(int mt)
{
    mParam.minThresValue = mt;
}

void pupilMeasureThread::setAreaValue(int val)
{
    mParam.areaValue = val;
}

void pupilMeasureThread::pupilMeasure()
{
    if(mImage->isNull())
        return;

    running = true;

    //    *************************************************************************

    cv::Mat tmp(mImage->height(),mImage->width(),CV_8UC3,(uchar*)mImage->bits(),mImage->bytesPerLine());

    cv::Mat gray;
    cv::cvtColor(tmp, gray, CV_BGR2GRAY);

    cv::threshold(gray, gray, mParam.minThresValue, mParam.maxThresValue, cv::THRESH_BINARY);

    // Find all contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(gray.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // Fill holes in each contour
    cv::drawContours(gray, contours, -1, CV_RGB(255, 255, 255), -1);

    for (int i = 0; i <contours.size() ; i++)
    {
        double area = cv::contourArea(contours[i]);
        cv::Rect rect = cv::boundingRect(contours[i]);
        int radius = rect.width / 2;

        // If contour is big enough and has round shape
        // Then it is the pupil
        if (area >= mParam.areaValue &&
                std::abs(1 - ((double)rect.width / (double)rect.height)) <= 0.2 &&
                std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
        {

            QPen myPen(Qt::red, 6, Qt::SolidLine);
            QPainter painter(mImage);
            painter.setPen(myPen);
            painter.drawEllipse(QPoint(rect.x, rect.y), radius, radius);
            painter.end();

            mDiameter = radius*2;
        }
    }

    emit sendDiameter(mDiameter);
    emit sendImage(*mImage);

    delete mImage;
    mImage = new QImage();
    running = false;
}
