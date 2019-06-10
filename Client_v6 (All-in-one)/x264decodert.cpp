#include "x264decodert.h"
#include <QDebug>

x264DecoderT::x264DecoderT()
{
    frame=false;

    stopThread = false;
}

void x264DecoderT::decode(QByteArray originalData)
{
    mutex2.lock();

    data = originalData;

    condition.wakeOne();

    mutex2.unlock();
}

void x264DecoderT::stop()
{
    mutex2.lock();

    stopThread = true;

    condition.wakeOne();

    mutex2.unlock();

}

void x264DecoderT::setResolution(int w, int h)
{
    mutex.lock();

    frameWidth = w;

    frameHeight = h;

    dec.initialize(w, h);

    decData = QByteArray(frameWidth*frameHeight*3, 0);

    mutex.unlock();
}

void x264DecoderT::splitChannels(bool b)
{
    split = b;
}

void x264DecoderT::closeDecoder()
{
    dec.close();
}

void x264DecoderT::channelSplit(uchar *image, uchar *red, uchar *green, uchar *blue)
{
    for(int i=0; i< frameWidth*frameHeight*3; i=i+3)
    {
        int index = i/3;

        red[index] = image[i];
        green[index] = image[i+1];
        blue[index] = image[i+2];
    }

}

void x264DecoderT::run()
{
    decData = QByteArray(frameWidth*frameHeight*3, 0);

    QPixmap p;

    mutex2.lock();

    while(true)
    {
        mutex.lock();
        mutex2.unlock();
        condition.wait(&mutex);

        mutex.unlock();

        mutex2.lock();

        if(stopThread)
            break;

        if(dec.decodeFrame((char*)data.data(), data.size(), (char*)decData.data()))
        {
            p =QPixmap::fromImage( QImage((uchar*)decData.data(), frameWidth, frameHeight, QImage::Format_RGB888));

            emit newFrame(p);
        }
    }

    stopThread = false;

    dec.close();

    mutex2.unlock();
}
