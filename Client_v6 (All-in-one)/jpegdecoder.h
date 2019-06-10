#ifndef JPEGDECODER_H
#define JPEGDECODER_H

#include <QThread>
#include <QByteArray>
#include <QMutex>
#include <QPixmap>


class JPEGDecoder : public QThread
{
    Q_OBJECT

public:

    JPEGDecoder();

    void decode(QByteArray originalData);

    void stop();

    void setResolution(int w, int h);

protected:

    void run();

signals:

    void newFrame(QPixmap f);

private:

    QByteArray data;

    QPixmap p;

    QMutex mutex;

    bool frame, stopThread;

    int frameWidth, frameHeight, jpegSubsamp;
};

#endif // JPEGDECODER_H
