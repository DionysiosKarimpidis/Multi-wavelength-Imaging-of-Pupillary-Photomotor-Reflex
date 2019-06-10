#ifndef X264DECODERT_H
#define X264DECODERT_H

#include <QThread>
#include <QByteArray>
#include <QMutex>
#include <QPixmap>
#include <QWaitCondition>
#include "x264decoder.h"

class x264DecoderT : public QThread
{
    Q_OBJECT

public:

    x264DecoderT();

    void decode(QByteArray originalData);

    void stop();

    void setResolution(int w, int h);

    void splitChannels(bool b);

    void closeDecoder();

private:

    void channelSplit(uchar *image, uchar *red, uchar *green, uchar *blue);

protected:

    void run();

signals:

    void newFrame(QPixmap f);

    void newChannels(QPixmap red, QPixmap green, QPixmap blue);

private:

    QByteArray data, decData;

    QPixmap p;

    QMutex mutex, mutex2;

    bool frame, stopThread, split;

    int frameWidth, frameHeight;

    x264Decoder dec;

    QWaitCondition condition;
};

#endif // X264DECODERT_H
