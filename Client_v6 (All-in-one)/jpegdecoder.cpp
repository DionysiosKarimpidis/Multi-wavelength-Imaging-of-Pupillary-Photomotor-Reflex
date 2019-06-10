#include "jpegdecoder.h"

#include <QDebug>
#include <QRectF>
#include <jpeglib.h>
#include <turbojpeg.h>
#include <QTime>

JPEGDecoder::JPEGDecoder():
    QThread()
{
    frame=false;

    stopThread = false;
}

void JPEGDecoder::decode(QByteArray originalData)
{
    mutex.lock();

    data = originalData;

    frame = true;

    mutex.unlock();
}

void JPEGDecoder::stop()
{
    mutex.lock();

    stopThread = true;

    data.clear();

    mutex.unlock();
}

void JPEGDecoder::setResolution(int w, int h)
{
    mutex.lock();

    frameWidth = w;

    frameHeight = h;

    mutex.unlock();
}

void JPEGDecoder::run()
{

    tjhandle decompressor = tjInitDecompress();
    QByteArray decData(3280*2464*3, 0);
    QPixmap p;

    while(true)
    {
        mutex.lock();

        if(stopThread)
            break;

        if(frame)
        {

            tjDecompressHeader2(decompressor, (uchar*) data.data(), data.size(), &frameWidth, &frameHeight, &jpegSubsamp);

            tjDecompress2(decompressor, (uchar*) data.data(), data.size(), (uchar*)decData.data(), frameWidth, 0/*pitch*/, frameHeight, TJPF_RGB, TJFLAG_FASTDCT);

            p =QPixmap::fromImage( QImage((uchar*)decData.data(), frameWidth, frameHeight, QImage::Format_RGB888));

            emit newFrame(p);

            frame = false;
        }

        mutex.unlock();
    }

    tjDestroy(decompressor);

    stopThread = false;

    mutex.unlock();
}

