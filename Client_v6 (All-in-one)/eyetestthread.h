#ifndef EYETESTTHREAD_H
#define EYETESTTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QString>

#include "definitions.h"
#include "windows.h"

class EyeTestThread: public QThread
{
    Q_OBJECT

public:
    EyeTestThread(QTcpSocket *cmd_t, EyeParameters eyeParam);

public slots:
    void stop();
private:
    QTcpSocket *cmd;
    EyeParameters mEyeParameters;
    char buffer[11];

protected:
    void run();

signals:
    void setLedBuffer(const char*,int,int);
    void testFinished();

};

#endif // EYETESTTHREAD_H
