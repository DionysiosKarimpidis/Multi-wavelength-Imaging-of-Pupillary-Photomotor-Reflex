#include "eyetestthread.h"

EyeTestThread::EyeTestThread(QTcpSocket *cmd_t, EyeParameters eyeParam)
{
    cmd = cmd_t;

    mEyeParameters = eyeParam;
}

void EyeTestThread::run()
{
    Sleep(mEyeParameters.waitTime * 1000);
    emit setLedBuffer("led", mEyeParameters.ledNum, mEyeParameters.ledValue);
    qDebug()<<"Led num: "<<mEyeParameters.ledNum<<" Led value: "<<mEyeParameters.ledValue;
    Sleep(mEyeParameters.stimulationTime * 1000);
    emit setLedBuffer("led", mEyeParameters.ledNum, 0);
    Sleep(mEyeParameters.recoveryTime * 1000);
    emit testFinished();
}

void EyeTestThread::stop()
{
    setLedBuffer("led", mEyeParameters.ledNum, 0);
}
