#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsScene>
#include <QRectF>
#include <QImage>
#include <QPixmap>
#include <QTime>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QtCore>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connection Settings
    //ip = "192.168.2.190";
    ip = "192.168.2.190";
    ipZero = "192.168.3.70";
    port = 5005;

    // Camera Variales
    width = 640;
    height = 480;
    rotation = 0;

    // Statistics
    bits = 0;
    bits2 = 0;
    fps=0;
    fps2=0;

    // Sockets
    imgSock = NULL;
    cmdSock = NULL;
    snapSock = NULL;
    imgSockZero = NULL;
    cmdSockZero = NULL;
    snapSockZero = NULL;

    // Control
    camNo = 0;
    size=0;
    sizeZero = 0;
    sizeCmd = 0;
    snapNo=0;
    snapSize = 0;
    ctype = single;
    codec = x264;
    img_codec = Image_JPG;

    compare = false;

    mEyeTestThread = new EyeTestThread(NULL, mEyeParameters);

    connect(&mTimer, SIGNAL(timeout()), this, SLOT(showTime()));

    mPupilMeasureThread = new pupilMeasureThread();

    // Initialize Viewers
    initViewers();

    // Initialize UI
    initUI();

    QXlsx::Document xlsx;

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::serverConnect()
{
    // RPi
    imgSock = new QTcpSocket();
    cmdSock = new QTcpSocket();
    snapSock = new QTcpSocket();

    // Zero
    imgSockZero = new QTcpSocket();
    cmdSockZero = new QTcpSocket();
    snapSockZero = new QTcpSocket();


    //Pi Connection

    imgSock->connectToHost(ip, port);

    if(imgSock->waitForConnected(TIMEOUT))
    {
        imgSock->write("sendFrame");

        cmdSock->connectToHost(ip, port);

        if(cmdSock->waitForConnected(TIMEOUT))
        {
            snapSock->connectToHost(ip, port);

            if(snapSock->waitForConnected(TIMEOUT))
            {
                connect(imgSock, SIGNAL(readyRead()), this, SLOT(readTcpData()));
                connect(snapSock, SIGNAL(readyRead()), this, SLOT(readTcpSnap()));
                connect(cmdSock, SIGNAL(readyRead()), this, SLOT(readTcpCameraSetting()));

                if(ctype == single)
                    return true;
            }
        }
    }

    if(ctype == single)
    {
        QMessageBox::critical(this, "Error connnecting", "Could not connect to Raspberry Pi.");

        serverDisconnect();

        return false;
    }

    //Zero Connection

    imgSockZero->connectToHost(ipZero, port);

    if(imgSockZero->waitForConnected(TIMEOUT))
    {
        imgSockZero->write("sendFrame");

        cmdSockZero->connectToHost(ipZero, port);

        if(cmdSockZero->waitForConnected(TIMEOUT))
        {
            snapSockZero->connectToHost(ipZero, port);

            if(snapSockZero->waitForConnected(TIMEOUT))
            {
                connect(imgSockZero, SIGNAL(readyRead()), this, SLOT(readTcpDataZero()));
                connect(snapSockZero, SIGNAL(readyRead()), this, SLOT(readTcpSnapZero()));

                return true;
            }
        }
    }

    QMessageBox::critical(this, "Error connnecting", "Could not connect to Zero Pi.");

    serverDisconnect();

    return false;
}

void MainWindow::setCameraSettings(QString setting, int value1, int value2)
{
    if(setting == "exp")
    {
        if(value1 == 1)
        {
            ui->buttonAutoExposure->setChecked(true);
            ui->sliderShutter->setEnabled(false);

            timerShutter.start(1000);
        }
        else
        {
            ui->buttonAutoExposure->setChecked(false);
            ui->sliderShutter->setEnabled(true);

            timerShutter.stop();
        }
    }
    else if(setting == "shu")
    {
        ui->sliderShutter->setValue(value1/1000);
    }
    else if(setting == "wbv")
    {
        ui->sliderWBRed->setValue(value1);
        ui->sliderWBBlue->setValue(value2);
    }
    else if(setting == "wbm")
    {
        if(value1 == 0)
        {
            ui->buttonWBManual->setChecked(true);
            ui->buttonWBAuto->setChecked(false);

            ui->sliderWBBlue->setEnabled(true);
            ui->sliderWBRed->setEnabled(true);

            timerWB.stop();
        }
        else
        {
            ui->buttonWBManual->setChecked(false);
            ui->buttonWBAuto->setChecked(true);

            ui->sliderWBBlue->setEnabled(false);
            ui->sliderWBRed->setEnabled(false);

            //timerWB.start(1000);
        }
    }
    else if(setting == "bri")
    {
        ui->sliderBrightness->setValue(value1);
    }
    else if(setting == "con")
    {
        ui->sliderConrtast->setValue(value1);
    }


}

void MainWindow::initDecoders()
{
    if(codec == JPEG)
    {
        decoder = new JPEGDecoder();
        decoderZero = new JPEGDecoder();

        connect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));
        connect(decoderZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));

        decoder->setResolution(width, height);
        decoderZero->setResolution(width, height);

        decoder->start();

        if(ctype != single)
            decoderZero->start();
    }
    else
    {

        decPi = new x264DecoderT();
        decZero = new x264DecoderT();

        connect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));
        connect(decZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));

        decPi->setResolution(width, height);
        decZero->setResolution(width, height);

        decPi->start();

        if(ctype != single)
            decZero->start();
    }
}

void MainWindow::initViewers()
{
    ui->graphicsView->setScene(new QGraphicsScene());
    ui->zeroGraphicsView->setScene(new QGraphicsScene());

    ui->graphicsView->scene()->addItem(&item);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->zeroGraphicsView->scene()->addItem(&itemZero);
    ui->zeroGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->zeroGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsView->setStyleSheet("background-color: black;");
    ui->zeroGraphicsView->setStyleSheet("background-color: black;");

    ui->zeroGraphicsView->hide();

    fs = false;
}

void MainWindow::initUI()
{
    //Starting Values of Command Panel
    ui->sliderBrightness->setEnabled(false);
    ui->sliderShutter->setEnabled(false);
    ui->sliderFrameRate->setEnabled(false);
    ui->sliderWBBlue->setEnabled(false);
    ui->sliderWBRed->setEnabled(false);
    ui->sliderJPEGQuality->setEnabled(false);
    ui->buttonAutoExposure->setEnabled(false);
    ui->comboBoxISO->setEnabled(false);
    ui->buttonRotationClock->setEnabled(false);
    ui->buttonRotationCounterClock->setEnabled(false);
    ui->buttonWBAuto->setEnabled(false);
    ui->buttonWBManual->setEnabled(false);
    ui->buttonSnap->setEnabled(false);
    ui->buttonChange->setEnabled(false);
    ui->buttonChangeCamera->setEnabled(false);
    ui->buttonConfig264->setEnabled(false);
    ui->comboBoxResolution->setEnabled(false);
    ui->sliderConrtast->setEnabled(false);

    //Values for ISO and Resolution
    ui->comboBoxISO->blockSignals(true);
    ui->comboBoxISO->addItem("Auto");
    ui->comboBoxISO->addItem("60");
    ui->comboBoxISO->addItem("100");
    ui->comboBoxISO->addItem("200");
    ui->comboBoxISO->addItem("320");
    ui->comboBoxISO->addItem("400");
    ui->comboBoxISO->addItem("500");
    ui->comboBoxISO->addItem("640");
    ui->comboBoxISO->addItem("800");
    ui->comboBoxISO->blockSignals(false);

    ui->comboBoxResolution->blockSignals(true);
    ui->comboBoxResolution->addItem("320x280");
    ui->comboBoxResolution->addItem("640x480");
    ui->comboBoxResolution->addItem("800x600");
    ui->comboBoxResolution->addItem("1024x768");
    ui->comboBoxResolution->addItem("1280x720");
    ui->comboBoxResolution->addItem("1640x1232");
    ui->comboBoxResolution->addItem("1600x900");
    ui->comboBoxResolution->addItem("1920x1080");
    ui->comboBoxResolution->addItem("2592x1944");
    ui->comboBoxResolution->addItem("3280x2464");
    ui->comboBoxResolution->setCurrentIndex(5);
    ui->comboBoxResolution->blockSignals(false);

    ui->lineIP->setText(ip);
    ui->linePort->setText(QString::number(port));

    ui->labelBitrate_2->hide();
    ui->labelFPS2->hide();

    initLedGUI();

    action = new QAction(this);

    action->setShortcut(QKeySequence(Qt::Key_Escape));

    action->setCheckable(true);

    this->addAction(action);

    connect(action, SIGNAL(triggered(bool)), this, SLOT(on_actionFullscreen_toggled(bool)));

    connect(&config264, SIGNAL(option(int,int)), this, SLOT(sendH264Option(int,int)));

    connect(&fscreen, SIGNAL(exitFS(bool)), this, SLOT(on_actionFullscreen_toggled(bool)));

    connect(&timerShutter, SIGNAL(timeout()), this, SLOT(getShutter()));
    connect(&timerWB, SIGNAL(timeout()), this, SLOT(getWB()));
}

void MainWindow::initRightLedList()
{
    if(!rightLedList.isEmpty())
        rightLedList.clear();


    rightLedList.append("430nm");
    rightLedList.append("460nm");
    rightLedList.append("470nm");
    rightLedList.append("515nm");
    rightLedList.append("535nm");
    rightLedList.append("569nm");
    rightLedList.append("587nm");
    rightLedList.append("605nm");
    rightLedList.append("645nm");
    rightLedList.append("700nm");
    rightLedList.append("730nm");
    rightLedList.append("white");

    ledNumMatRight = new int[rightLedList.size()];

    ledNumMatRight[0] = 24;      //right_430nm
    ledNumMatRight[1] = 23;      //right_460nm
    ledNumMatRight[2] = 22;      //right_470nm
    ledNumMatRight[3] = 21;      //right_515nm
    ledNumMatRight[4] = 20;      //right_535nm
    ledNumMatRight[5] = 19;      //right_569nm
    ledNumMatRight[6] = 18;      //right_587nm
    ledNumMatRight[7] = 17;      //right_605nm
    ledNumMatRight[8] = 16;      //right_645nm
    ledNumMatRight[9] = 27;      //right_700nm
    ledNumMatRight[10] = 26;     //right_730nm
    ledNumMatRight[11] = 25;     //right_white


    mDominantLed = rightLedList.at(0);
    mDominantLedNum = 0;
}

void MainWindow::initLeftLedList()
{
    if(!leftLedList.isEmpty())
        leftLedList.clear();

    leftLedList.append("430nm");
    leftLedList.append("460nm");
    leftLedList.append("470nm");
    leftLedList.append("515nm");
    leftLedList.append("535nm");
    leftLedList.append("569nm");
    leftLedList.append("587nm");
    leftLedList.append("605nm");
    leftLedList.append("645nm");
    leftLedList.append("700nm");
    leftLedList.append("730nm");
    leftLedList.append("white");

    ledNumMatLeft = new int[leftLedList.size()];

    ledNumMatLeft[0] = 8;      //left_430nm
    ledNumMatLeft[1] = 7;      //left_460nm
    ledNumMatLeft[2] = 6;      //left_470nm
    ledNumMatLeft[3] = 5;      //left_515nm
    ledNumMatLeft[4] = 4;      //left_535nm
    ledNumMatLeft[5] = 3;      //left_569nm
    ledNumMatLeft[6] = 2;      //left_587nm
    ledNumMatLeft[7] = 1;      //left_605nm
    ledNumMatLeft[8] = 0;      //left_645nm
    ledNumMatLeft[9] = 11;     //left_700nm
    ledNumMatLeft[10] = 10;    //left_730nm
    ledNumMatLeft[11] = 9;     //left_white

    mDominantLed = leftLedList.at(0);
    mDominantLedNum = 0;
}

void MainWindow::initLedGUI()
{
    initRightLedList();
    initLeftLedList();

    ui->ledValueLabel->setText("0");

    ui->ledValueSlider->setEnabled(false);

    ui->ledGroupBox->setEnabled(false);

    //right eye
    if(rightLedList.isEmpty())
        return;

    ui->rightComboLedBox->blockSignals(true);
    for(int i=0;i<rightLedList.size();i++)
    {
        QString tmp = rightLedList[i];
        ui->rightComboLedBox->addItem(tmp);
    }
    ui->rightComboLedBox->blockSignals(false);


    //left eye
    if(leftLedList.isEmpty())
        return;

    ui->leftComboLedBox->blockSignals(true);
    for(int i=0;i<leftLedList.size();i++)
    {
        QString tmp = leftLedList[i];
        ui->leftComboLedBox->addItem(tmp);
    }
    ui->leftComboLedBox->blockSignals(false);

    ui->waitLineEdit->setText(QString::number(mEyeParameters.waitTime));
    ui->stimLineEdit->setText(QString::number(mEyeParameters.stimulationTime));
    ui->recoveryLineEdit->setText(QString::number(mEyeParameters.recoveryTime));

    ui->minThresSlider->setValue(mEyeParameters.minThresValue);
    ui->maxThresSlider->setValue(mEyeParameters.maxThresValue);

    ui->minThresLabel->setText(QString::number(mEyeParameters.minThresValue));
    ui->maxThresLabel->setText(QString::number(mEyeParameters.maxThresValue));

    ui->setValuesRadioButton->setChecked(true);
    mLedSlidersState = SetState;
}

void MainWindow::enableUI()
{
    ui->lineIP->setEnabled(false);
    ui->linePort->setEnabled(false);
    ui->sliderBrightness->setEnabled(true);
    ui->sliderFrameRate->setEnabled(true);
    ui->buttonAutoExposure->setEnabled(true);
    ui->sliderShutter->setEnabled(true);
    ui->comboBoxISO->setEnabled(true);
    ui->buttonRotationClock->setEnabled(true);
    ui->buttonRotationCounterClock->setEnabled(true);
    ui->buttonSnap->setEnabled(true);
    ui->buttonChange->setEnabled(true);
    ui->buttonWBAuto->setEnabled(true);
    ui->buttonWBManual->setEnabled(true);
    ui->comboBoxCodec->setEnabled(false);
    ui->comboBoxMode->setEnabled(false);
    ui->sliderConrtast->setEnabled(true);
    ui->comboBoxResolution->setEnabled(true);

    if(codec == JPEG)
        ui->sliderJPEGQuality->setEnabled(true);

    ui->buttonConnect->setText("Disconnect");

    timerbitrate.start();
    timerfps.start();

    if(ctype == dual_preview)
    {
        timerbitrate2.start();
        timerfps2.start();

        ui->labelBitrate_2->show();
        ui->labelFPS2->show();
    }
    else if(ctype == dual)
    {
        ui->buttonChangeCamera->setEnabled(true);

        timerbitrate2.start();
        timerfps2.start();
    }

    QTimer::singleShot(50, this, SLOT(resize()));
}

void MainWindow::destroyDecoders()
{
    if(codec == JPEG)
    {
        disconnect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

        decoder->stop();

        decoder->wait();

        delete decoder;
    }
    else
    {
        disconnect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

        decPi->stop();

        decPi->wait();

        delete decPi;
    }
}

void MainWindow::resetUI()
{
    ui->lineIP->setEnabled(true);
    ui->linePort->setEnabled(true);
    ui->buttonSnap->setEnabled(false);
    ui->sliderBrightness->setEnabled(false);
    ui->sliderShutter->setEnabled(false);
    ui->sliderFrameRate->setEnabled(false);
    ui->sliderWBBlue->setEnabled(false);
    ui->sliderWBRed->setEnabled(false);
    ui->sliderJPEGQuality->setEnabled(false);
    ui->buttonAutoExposure->setEnabled(false);
    ui->comboBoxISO->setEnabled(false);
    ui->buttonRotationClock->setEnabled(false);
    ui->buttonRotationCounterClock->setEnabled(false);
    ui->buttonWBAuto->setEnabled(false);
    ui->buttonWBManual->setEnabled(false);
    ui->buttonChangeCamera->setEnabled(false);
    ui->comboBoxCodec->setEnabled(true);
    ui->comboBoxMode->setEnabled(true);
    ui->buttonConfig264->setEnabled(false);
    ui->sliderConrtast->setEnabled(false);
    ui->comboBoxResolution->setEnabled(false);

    config264.hide();

    ui->buttonConnect->setText("Connect");

    ui->labelBitrate->setText("Bitrate");
    ui->labelFPS->setText("FPS");
    ui->labelBitrate_2->setText("Bitrate 2");
    ui->labelFPS2->setText("FPS 2");

    updateScreen(QPixmap());
    updateScreenZero(QPixmap());

    ui->labelBitrate_2->hide();
    ui->labelFPS2->hide();
    ui->zeroGraphicsView->hide();


    ui->graphicsView->show();
    ui->labelBitrate->show();
    ui->labelFPS->show();

    fscreen.hide();
    fscreen.showZeroViewer(false);
    fscreen.showViewer(true);

    timerShutter.stop();
}

void MainWindow::handleSetBuffer(const char *cmd, int value1, int value2)
{
    setBuffer(buffer, cmd, value1, value2);
    cmdSock->write(buffer, 11);
}

void MainWindow::setBuffer(char *buffer, const char *cmd, int value1, int value2)
{
    char *t1 = (char*)&value1;
    char *t2 = (char*)&value2;

    buffer[0] = cmd[0];
    buffer[1] = cmd[1];
    buffer[2] = cmd[2];

    buffer[3] = t1[0];
    buffer[4] = t1[1];
    buffer[5] = t1[2];

    buffer[7] = t2[0];
    buffer[8] = t2[1];
    buffer[9] = t2[2];
    buffer[10] = t2[3];
}

void MainWindow::serverDisconnect()
{
    disconnect(imgSock, SIGNAL(readyRead()), this, SLOT(readTcpData()));
    disconnect(snapSock, SIGNAL(readyRead()), this, SLOT(readTcpSnap()));
    disconnect(imgSockZero, SIGNAL(readyRead()), this, SLOT(readTcpDataZero()));
    disconnect(snapSockZero, SIGNAL(readyRead()), this, SLOT(readTcpSnapZero()));

    imgSock->close();
    cmdSock->close();
    snapSock->close();
    imgSockZero->close();
    cmdSockZero->close();
    snapSockZero->close();

    delete imgSock;
    delete cmdSock;
    delete snapSock;
    delete imgSockZero;
    delete cmdSockZero;
    delete snapSockZero;

    imgSock = NULL;
    cmdSock = NULL;
    snapSock = NULL;
    imgSockZero = NULL;
    cmdSockZero = NULL;
    snapSockZero = NULL;
}

void MainWindow::readTcpSnap()
{
    dataSnap.append(snapSock->readAll());

    if(snapSize==0)
    {
        uchar *temp = (uchar*)dataSnap.data();

        snapSize = (temp[3] << 24) | (temp[2] << 16) | (temp[1] << 8) | temp[0];

        qDebug() << snapSize;

        dataSnap.clear();

        snapSock->write("ok");
    }

    else if(dataSnap.size()==snapSize)
    {
        QPixmap p;

        if(img_codec == Image_JPG)
        {
            p.loadFromData(dataSnap, "jpg");
        }
        else if(img_codec == Image_PNG)
        {
            p.loadFromData(dataSnap, "png");
        }
        else
        {
            framePi = QImage((uchar*)dataSnap.data(), MAX_WIDTH, MAX_HEIGHT, QImage::Format_RGB888);

            p = QPixmap::fromImage(framePi);
        }

        rect.setWidth(MAX_WIDTH);
        rect.setHeight(MAX_HEIGHT);

        resizeEvent(0);

        updateScreen(p);

        snapSize=0;

        ui->buttonSave->setEnabled(true);

        ui->buttonSnap->setEnabled(true);
    }
}

void MainWindow::readTcpSnapZero()
{
    dataSnapZero.append(snapSockZero->readAll());

    if(snapSizeZero==0)
    {
        uchar *temp = (uchar*)dataSnapZero.data();

        snapSizeZero = (temp[3] << 24) | (temp[2] << 16) | (temp[1] << 8) | temp[0];

        qDebug() << snapSizeZero;

        dataSnapZero.clear();

        snapSockZero->write("ok");
    }

    else if(dataSnapZero.size()==snapSizeZero)
    {
        QPixmap p;

        if(img_codec == Image_JPG)
        {
            p.loadFromData(dataSnapZero, "jpg");
        }
        else
        {
            frameZero = QImage((uchar*)dataSnapZero.data(), MAX_WIDTH, MAX_HEIGHT, QImage::Format_RGB888);

            p = QPixmap::fromImage(frameZero);
        }

        rect.setWidth(MAX_WIDTH);
        rect.setHeight(MAX_HEIGHT);

        resizeEvent(0);

        updateScreenZero(p);

        snapSizeZero=0;

        ui->buttonSave->setEnabled(true);

        ui->buttonSnap->setEnabled(true);
    }
}

void MainWindow::readTcpData()
{
    data.append(imgSock->readAll());

    if(size==0)
    {
        uchar *temp = (uchar*)data.data();
        uchar *t = (uchar*)&size;

        t[0] = temp[0];
        t[1] = temp[1];
        t[2] = temp[2];
        t[3] = temp[3];

        bits+=data.size();

        data.clear();

        imgSock->write("sendFrame");
    }
    else if(data.size()==size)
    {

        if(codec == JPEG)
            decoder->decode(data);
        else
        {
            decPi->decode(data);
        }

        bits+=data.size();

        data.clear();

        size=0;

        imgSock->write("sendFrame");
    }

    if(timerbitrate.elapsed()>=1000)
    {
        ui->labelBitrate->setText("Bitrate 1: " + QString::number((float)(bits*8)/1000000,'f',2) + " Mbps");

        bits = 0;

        timerbitrate.restart();
    }
}

void MainWindow::readTcpDataZero()
{
    dataZero.append(imgSockZero->readAll());

    if(sizeZero==0)
    {
        uchar *temp = (uchar*)dataZero.data();

        sizeZero = (temp[3] << 24) | (temp[2] << 16) | (temp[1] << 8) | temp[0];

        bits2+=dataZero.size();

        dataZero.clear();

        imgSockZero->write("sendFrame");
    }
    else if(dataZero.size()==sizeZero)
    {
        if(codec == JPEG)
            decoderZero->decode(dataZero);
        else
            decZero->decode(dataZero);

        bits2+=dataZero.size();

        dataZero.clear();

        sizeZero=0;

        imgSockZero->write("sendFrame");
    }


    if(timerbitrate2.elapsed()>=1000)
    {
        ui->labelBitrate_2->setText("Bitrate 2: " + QString::number((float)(bits2*8)/1000000,'f',2) + " Mbps");

        bits2 = 0;

        timerbitrate2.restart();
    }
}

void MainWindow::updateScreen(QPixmap p)
{
    QImage img = p.toImage();
    
    

    QRgb rgb = img.pixel(img.width()/2, img.height()/2);

    ui->photometryLabel->setText("R: " + QString::number(qRed(rgb)) + " G: " + QString::number(qGreen(rgb)) + " B: " + QString::number(qBlue(rgb)));

    img.detach();

//    QPainter *paint = new QPainter(&p);
//    QPen myPen(Qt::red, 3, Qt::SolidLine);
//    paint->setPen(myPen);
//    paint->drawEllipse((int)(p.width()/2),(int)(p.height()/2),20,20);
//    paint->end();

    if(fs)
        fscreen.setPixmap(p);
    else
        item.setPixmap(p);

    if(compare)
        *piPx = p;

    ++fps;


    if(timerfps.elapsed()>=1000)
    {
        ui->labelFPS->setText("FPS: " + QString::number(fps));

        fps=0;

        timerfps.restart();
    }

}

void MainWindow::updateScreen(QImage img)
{
    QPixmap p = QPixmap::fromImage(img);

    if(fs)
        fscreen.setPixmap(p);
    else
        item.setPixmap(p);
}

void MainWindow::updateScreenZero(QPixmap p)
{

    if(fs)
        fscreen.setPixmap_2(p);
    else
        itemZero.setPixmap(p);

    if(compare)
    {
        *zeroPx = p;
    //    imgDifference->handleImages(*piPx, *zeroPx);
    }

    ++fps2;

    if(timerfps2.elapsed()>=1000)
    {
        ui->labelFPS2->setText("FPS 2: " + QString::number(fps2));

        fps2=0;

        timerfps2.restart();
    }
}

void MainWindow::on_lineEditCommand_returnPressed()
{
    QString s = ui->lineEditCommand->text();

    ui->textEditCommand->append(">> " + s);
    ui->lineEditCommand->clear();

    QString cmd = s.section(' ',0,0);
    int value1 = s.section(' ',1,1).toInt();
    int value2 = s.section(' ',2,2).toInt();

    setBuffer(buffer, cmd.toStdString().c_str(), value1, value2);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(ui->buttonConnect->text() == "Disconnect")
    {
        if(cmdSock->isOpen())
            cmdSock->close();

        if(imgSock->isOpen())
            imgSock->close();
    }

    event->accept();

    exit(0);
}

void MainWindow::serverCommmunication()
{
    QByteArray incomingData = cmdSock->readAll();

    ui->textEditCommand->append("SERVER: " + QString(incomingData));
}

void MainWindow::on_buttonChange_clicked()
{
    if(ui->stackedWidget_3->currentIndex())
    {
        ui->stackedWidget_3->setCurrentIndex(0);    
    }else
    {
        ui->stackedWidget_3->setCurrentIndex(1);
    }
}

void MainWindow::on_buttonWBAuto_toggled(bool checked)
{
    if(checked)
    {
        ui->sliderWBBlue->setEnabled(false);
        ui->sliderWBRed->setEnabled(false);
    }
}

void MainWindow::on_buttonWBManual_toggled(bool checked)
{
    if(checked)
    {
        ui->sliderWBBlue->setEnabled(true);
        ui->sliderWBRed->setEnabled(true);
    }
}

void MainWindow::on_buttonConnect_clicked()
{
    if(ui->buttonConnect->text() == "Connect")
    {
        rect.setWidth(width);
        rect.setHeight(height);

        if(ctype == dual_preview)
        {
            ui->zeroGraphicsView->show();

            fscreen.showZeroViewer(true);
        }
        else
        {
            ui->zeroGraphicsView->hide();

            fscreen.showZeroViewer(false);
        }

        initDecoders();

        if(serverConnect())
        {
            enableUI();

            char buffer[11];

            if(codec==JPEG)
                setBuffer(buffer, "sta", 0, 0);
            else
                setBuffer(buffer, "sta", 1, 0);

            cmdSock->write(buffer, 11);

            if(ctype == dual_preview)
            {
                cmdSockZero->write(buffer, 11);
            }

//            setBuffer(buffer, "gex", 0, 0);

//            cmdSock->write(buffer, 11);

//            setBuffer(buffer, "gwm", 0, 0);

//            cmdSock->write(buffer, 11);

//            setBuffer(buffer, "gsh", 0, 0);

//            cmdSock->write(buffer, 11);

//            setBuffer(buffer, "gwb", 0, 0);

//            cmdSock->write(buffer, 11);

//            setBuffer(buffer, "gbr", 0, 0);

//            cmdSock->write(buffer, 11);

        }
    }
    else
    {
        destroyDecoders();

        serverDisconnect();

        data.clear();
        dataZero.clear();
        dataCmd.clear();

        size=0;
        sizeZero=0;
        sizeCmd = 0;

        bits = 0;
        bits2 = 0;
        camNo = 0;
        snapNo=0;
        rotation = 0;

        resetUI();
    }
}

void MainWindow:: readTcpCameraSetting()
{
    dataCmd.append(cmdSock->readAll());

    if(dataCmd.size()>=11)
    {
        int value1;
        int value2;
        char cmd[4];
        uchar *t1 = (uchar*)&value1;
        uchar *t2 = (uchar*)&value2;
        char *temp = dataCmd.data();

        cmd[0] = temp[0];
        cmd[1] = temp[1];
        cmd[2] = temp[2];
        cmd[3] = '\0';

        t1[0] = temp[3];
        t1[1] = temp[4];
        t1[2] = temp[5];
        t1[3] = temp[6];

        t2[0] = temp[7];
        t2[1] = temp[8];
        t2[2] = temp[9];
        t2[3] = temp[10];

        setCameraSettings(QString(cmd), value1, value2);

        dataCmd.remove(0, 11);
    }
}

void MainWindow::on_sliderShutter_valueChanged(int value)
{
    ui->shutterValue->setText(QString::number(value));
}

void MainWindow::on_sliderWBBlue_valueChanged(int value)
{
    double realBlueValue = double( value / 10.0 ); // float value
    ui->blueValue->setText(QString::number(realBlueValue));
}

void MainWindow::on_sliderWBRed_valueChanged(int value)
{
    double realRedValue = double( value / 10.0 ); // float value
    ui->redValue->setText(QString::number(realRedValue));
}

void MainWindow::on_sliderBrightness_valueChanged(int value)
{
    ui->brightnessValue->setText(QString::number(value));
}

void MainWindow::on_sliderJPEGQuality_valueChanged(int value)
{
    ui->jPEGQualityValue->setText(QString::number(value));
}

void MainWindow::on_sliderFrameRate_valueChanged(int value)
{
    ui->frameRateValue->setText(QString::number(value));
}

void MainWindow::on_lineIP_editingFinished()
{
    ip = ui->lineIP->text();
}

void MainWindow::on_linePort_editingFinished()
{
    port = ui->linePort->text().toInt();
}

void MainWindow::on_buttonAutoExposure_clicked(bool checked)
{
    if(checked)
    {
        setBuffer(buffer, "exp", 1, 0);
    }
    else
    {
        setBuffer(buffer, "exp", 0, 0);
    }

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->graphicsView->fitInView(rect, Qt::KeepAspectRatio);
    ui->zeroGraphicsView->fitInView(rect, Qt::KeepAspectRatio);
}

void MainWindow::on_sliderShutter_sliderReleased()
{
    int value = ui->sliderShutter->value();

    setBuffer(buffer, "shu", value, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_comboBoxISO_currentIndexChanged(const QString &arg1)
{
    if(arg1=="Auto")
    {
        setBuffer(buffer, "iso",0,0);
    }
    else
    {
        setBuffer(buffer, "iso",arg1.toInt(),0);
    }

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_comboBoxResolution_currentIndexChanged(const QString &arg1)
{
    width = arg1.section('x',0,0).toInt();
    height = arg1.section('x',1,1).toInt();

    if(cmdSock == NULL)
    {
        return;
    }

    if(codec == JPEG)
    {
        disconnect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

        if(ctype != single)
            disconnect(decoderZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));

    }
    else
    {
        disconnect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

        if(ctype != single)
            disconnect(decZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
    }

    updateScreen(QPixmap());

    char buffer[11];

    setBuffer(buffer, "res", arg1.section('x',0,0).toInt(), arg1.section('x',1,1).toInt());

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);

    rect.setWidth(width);
    rect.setHeight(height);

    fscreen.setRect(rect);

  //  imgDifference->handlleSizeChange(width, height);

    if(codec == JPEG)
    {
        decoder->setResolution(width, height);
        decoderZero->setResolution(width, height);

        connect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));
        connect(decoderZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
    }
    else
    {
        decPi->setResolution(width, height);
        decZero->setResolution(width, height);

        connect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));
        connect(decZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
    }

     QTimer::singleShot(50, this, SLOT(resize()));

}

void MainWindow::on_sliderFrameRate_sliderReleased()
{
    int value = ui->sliderFrameRate->value();

    setBuffer(buffer, "fra", value, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_buttonRotationCounterClock_clicked()
{
    if(rotation==0)
        rotation = 270;
    else
        rotation-=90;

    setBuffer(buffer, "rot", rotation, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_buttonRotationClock_clicked()
{
    if(rotation==270)
        rotation = 0;
    else
        rotation+=90;

    setBuffer(buffer, "rot", rotation, 0);

    cmdSock->write(buffer, 11);
}

void MainWindow::on_buttonWBAuto_clicked()
{
    ui->buttonWBManual->setChecked(false);

    setBuffer(buffer, "wbm", 1 ,0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);

    //timerWB.start(1000);
}

void MainWindow::on_buttonWBManual_clicked()
{
    ui->buttonWBAuto->setChecked(false);

    setBuffer(buffer, "wbm", 0 ,0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);

    timerWB.stop();
}

void MainWindow::on_sliderWBBlue_sliderReleased()
{
    int valueRed = ui->sliderWBRed->value();
    int valueBlue = ui->sliderWBBlue->value();

    setBuffer(buffer, "wbv", valueRed, valueBlue);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_sliderWBRed_sliderReleased()
{
    int valueRed = ui->sliderWBRed->value();
    int valueBlue = ui->sliderWBBlue->value();

    setBuffer(buffer, "wbv", valueRed, valueBlue);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_sliderBrightness_sliderReleased()
{
    int value = ui->sliderBrightness->value();

    setBuffer(buffer, "bri", value, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_sliderJPEGQuality_sliderReleased()
{
    int value = ui->sliderJPEGQuality->value();

    setBuffer(buffer, "qua", value, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::on_buttonSnap_clicked(bool checked)
{
    char buffer[11];

    if(checked)
    {
        if(codec == JPEG)
        {
            disconnect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

            if(ctype != single)
                disconnect(decoderZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
        }
        else
        {
            disconnect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

            if(ctype != single)
                disconnect(decZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
        }

        if(img_codec == Image_JPG)
        {
            setBuffer(buffer, "snp", 0, 0);
        }
        else if(img_codec == Image_PNG)
        {
            setBuffer(buffer, "snp", 100, 0);
        }
        else
        {
            setBuffer(buffer, "snp", 1000, 0);
        }

        cmdSock->write(buffer, 11);
        cmdSockZero->write(buffer, 11);

        ui->buttonSnap->setText("Preview");
        ui->buttonSnap->setEnabled(false);
    }
    else
    {
        if(codec == JPEG)
        {
            connect(decoder, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

            if(ctype != single)
                connect(decoderZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
        }
        else
        {
            connect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

            if(ctype != single)
                connect(decZero, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreenZero(QPixmap)));
        }

        dataSnap.clear();
        dataSnapZero.clear();

        framePi = QImage();
        frameZero = QImage();

        if(codec == x264)
        {
            setBuffer(buffer, "sta", 1, 0);
        }
        else
        {
            setBuffer(buffer, "sta", 0, 0);
        }

        cmdSock->write(buffer, 11);
        cmdSockZero->write(buffer, 11);


        rect.setWidth(width);
        rect.setHeight(height);

        resizeEvent(0);

        ui->buttonSave->setEnabled(false);
        ui->buttonSnap->setText("Snap");

    }

}

void MainWindow::readTcpWB()
{
    QByteArray d = cmdSock->readAll();
    QString values(d);

    float wbred = values.section(',',0,0).toFloat();
    float wbblue = values.section(',',1,1).toFloat();

    ui->sliderWBRed->setValue(wbred*10);

    ui->sliderWBBlue->setValue(wbblue*10);
}

void MainWindow::on_buttonSave_clicked()
{

    QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("PNG Images(*.png)"));

    QImage saveImage = item.pixmap().toImage();

    saveImage.save(path);

    if(ctype == single)
    {
//        if(img_codec == Image_JPG)
//        {
//            QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("JPEG Images(*.jpg)"));

//            QFile file(path);
//            file.open(QIODevice::WriteOnly);
//            file.write(dataSnap);
//            file.close();
//        }
//        else if(img_codec == Image_PNG)
//        {
//            QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("PNG Images(*.png)"));

//            QFile file(path);
//            file.open(QIODevice::WriteOnly);
//            file.write(dataSnap);
//            file.close();
//        }
//        else
//        {
//            QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("PNG Images(*.png)"));

//            framePi.save(path, "PNG");
//        }

//        QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("PNG Images(*.png)"));

//        QImage saveImage = item.pixmap().toImage();

//        saveImage.save(path);


    }
    else
    {

        if(img_codec == Image_JPG)
        {
            QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("JPEG Images(*.jpg)"));

            path.replace(QString("."), QString("_1."));

            QFile file(path);
            file.open(QIODevice::WriteOnly);
            file.write(dataSnap);
            file.close();

            path.replace(QString("_1."), QString("_2."));

            file.setFileName(path);
            file.open(QIODevice::WriteOnly);
            file.write(dataSnapZero);
            file.close();
        }
        else
        {
            QString path = QFileDialog::getSaveFileName(this, tr("Save File")," ", tr("PNG Images(*.png)"));

            path.replace(QString("."), QString("_1."));

            framePi.save(path, "PNG");

            path.replace(QString("_1."), QString("_2."));

            frameZero.save(path, "PNG");
        }
    }
}

void MainWindow::on_buttonChangeCamera_clicked()
{
    if(camNo == 0)
    {
        camNo = 1;

        setBuffer(buffer, "sto", 0, 0);

        cmdSock->write(buffer, 11);

        fscreen.showZeroViewer(true);
        fscreen.showViewer(false);
        ui->zeroGraphicsView->show();
        ui->graphicsView->hide();


        if(codec==JPEG)
            setBuffer(buffer, "sta", 0, 0);
        else
            setBuffer(buffer, "sta", 1, 0);

        cmdSockZero->write(buffer, 11);

        QTimer::singleShot(10, this, SLOT(resize()));

        ui->labelBitrate->hide();
        ui->labelFPS->hide();
        ui->labelBitrate_2->show();
        ui->labelFPS2->show();

    }
    else
    {
        camNo = 0;

        setBuffer(buffer, "sto", 0, 0);

        cmdSockZero->write(buffer, 11);

        fscreen.showZeroViewer(false);
        fscreen.showViewer(true);
        ui->graphicsView->show();
        ui->zeroGraphicsView->hide();

        if(codec==JPEG)
            setBuffer(buffer, "sta", 0, 0);
        else
            setBuffer(buffer, "sta", 1, 0);

        cmdSock->write(buffer, 11);

        QTimer::singleShot(10, this, SLOT(resize()));

        ui->labelBitrate_2->hide();
        ui->labelFPS2->hide();
        ui->labelBitrate->show();
        ui->labelFPS->show();
    }

}

void MainWindow::on_comboBoxCodec_currentIndexChanged(int index)
{
    if(index == 1)
        codec = JPEG;
    else
        codec = x264;
}

void MainWindow::on_comboBoxMode_activated(int index)
{
    if(index == 0)
    {
        ctype = single;
        ui->buttonConfig264->setEnabled(false);
    }
    else if(index == 1)
    {
        ctype = dual;
        ui->buttonConfig264->setEnabled(false);
    }
    else
    {
        ctype = dual_preview;
        ui->buttonConfig264->setEnabled(true);
    }
}

void MainWindow::resize()
{
    resizeEvent(0);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(index == 0)
        img_codec = Image_JPG;
    else if(index == 1)
        img_codec = Image_PNG;
    else
        img_codec = Image_RAW;
}

void MainWindow::on_actionFullscreen_toggled(bool arg1)
{
    fs = arg1;

    if(fs)
    {
        fscreen.showFullScreen();

        item.setPixmap(QPixmap());

        itemZero.setPixmap(QPixmap());
    }
    else
    {
        fscreen.hide();

        fscreen.setPixmap(QPixmap());

        action->setChecked(false);

        ui->actionFullscreen->setChecked(false);
    }
}

void MainWindow::on_buttonConfig264_clicked()
{
//    if(config264.isVisible())
//        return;

//    config264.show();
   // imgDifference = new ImageRegistrationPanel();
    zeroPx = new QPixmap();
    piPx = new QPixmap();

  //  connect(imgDifference, SIGNAL(beginImgTransaction(bool)), this, SLOT(handleDifferences(bool)));

  //  imgDifference->show();
    compare = true;
}

void MainWindow::sendH264Option(int option, int value)
{
    setBuffer(buffer, "264", value, 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::getShutter()
{
    setBuffer(buffer, "gsh", 0, 0);

    if(cmdSock != NULL)
        cmdSock->write(buffer, 11);
}

void MainWindow::getWB()
{
    setBuffer(buffer, "gwb", 0, 0);

    if(cmdSock != NULL)
        cmdSock->write(buffer, 11);
}

void MainWindow::on_sliderConrtast_valueChanged(int value)
{
    ui->labelContrast->setText(QString::number(value));
}

void MainWindow::on_sliderConrtast_sliderReleased()
{
    setBuffer(buffer, "con", ui->sliderConrtast->value(), 0);

    cmdSock->write(buffer, 11);
    cmdSockZero->write(buffer, 11);
}

void MainWindow::handleDifferences(bool state)
{
    zeroPx = new QPixmap();
    piPx = new QPixmap();

   // imgDifference->close();

    compare = state;
}

void MainWindow::on_startIRLedButton_clicked(bool checked)
{
    if(checked)
    {
        ui->ledGroupBox->setEnabled(true);
        ui->leftComboLedBox->setEnabled(false);
        ui->rightComboLedBox->setEnabled(false);

        ui->comboBoxMode->setCurrentIndex(2);
        on_comboBoxMode_activated(2);

        if(ui->buttonConnect->text() == "Connect")
            on_buttonConnect_clicked();

    //    initEyeCameraSettings();

         //left eye IR_leds
         setBuffer(buffer, "led", 12, 250);
         cmdSock->write(buffer, 11);

         //right eye IR_leds
         setBuffer(buffer, "led", 28, 250);
         cmdSock->write(buffer, 11);

    }
    else
    {

    }
}

void MainWindow::initEyeCameraSettings()
{
    ui->comboBoxResolution->setCurrentIndex(1);

    Sleep(180);
    ui->buttonAutoExposure->click();
    Sleep(180);
    ui->comboBoxISO->setCurrentIndex(2);
    Sleep(180);
    ui->sliderShutter->setValue(100);
    on_sliderShutter_sliderReleased();
    Sleep(180);
    ui->buttonWBManual->click();
    Sleep(180);
    ui->sliderWBBlue->setValue(13);
    ui->sliderWBRed->setValue(12);
    on_sliderWBRed_sliderReleased();
    Sleep(180);
    on_sliderWBBlue_sliderReleased();

    Sleep(180);
    setBuffer(buffer, "rot", 180, 0);
    cmdSock->write(buffer, 11);
}

void MainWindow::on_ledOffPushButton_clicked()
{
    ui->ledValueSlider->setValue(0);
    on_ledValueSlider_valueChanged(0);
}

void MainWindow::on_startTestButton_clicked()
{
    if(ui->startIRLedButton->isChecked())
    {
        mEyeParameters.stimulationTime = ui->stimLineEdit->text().toInt();
        mEyeParameters.recoveryTime = ui->recoveryLineEdit->text().toInt();
        mEyeParameters.waitTime = ui->waitLineEdit->text().toInt();
        mEyeParameters.areaValue = ui->areaHorizontalSlider->value();

        mEyeTestThread = new EyeTestThread(cmdSock, mEyeParameters);
        connect(mEyeTestThread, SIGNAL(setLedBuffer(const char*,int,int)), this, SLOT(handleSetBuffer(const char*,int,int)));
        connect(mEyeTestThread, SIGNAL(testFinished()), this, SLOT(handleEyeTestFinished()));

        mPupilMeasureThread->setMeasureSpecs(mEyeParameters);

        mEyeTestThread->start();
        mPupilMeasureThread->start();
        mTime.start();
        mTimer.start(50);

        disconnect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

        connect(mPupilMeasureThread, SIGNAL(sendImage(QImage)), this, SLOT(updateScreen(QImage)));
        connect(mPupilMeasureThread, SIGNAL(sendDiameter(int)), this, SLOT(handleSetDiameter(int)));
        connect(decPi, SIGNAL(newFrame(QPixmap)), mPupilMeasureThread, SLOT(updateImage(QPixmap)));

        QFont* font = new QFont("Kozuka Gothic Pr6N M");
        font->setPointSize(10);
        font->setBold(true);
        ui->clockLabel->setFont(*font);
        ui->eyeStateLabel->setFont(*font);
        ui->eyeStateLabel->setStyleSheet("color: blue");
        ui->eyeStateLabel->setText("-Wait Time-");
    }
    else
    {
        QMessageBox::warning(this, "IR Led off","Please turn IR Led on");
        disconnect(mEyeTestThread, SIGNAL(setLedBuffer(const char*,int,int)), this, SLOT(handleSetBuffer(const char*,int,int)));
    }
}

void MainWindow::handleSetDiameter(int diameter)
{
    ui->diameterLabel->setText("Diameter: " + QString::number(diameter));
    //append stis listes
    diameterStr.append(QString::number(diameter));
    timeStr.append(dtime);

}

void MainWindow::on_ledValueSlider_valueChanged(int value)
{
    ui->ledValueLabel->setText(QString::number(value));
}

void MainWindow::on_leftComboLedBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.contains("No"))
        ui->ledValueSlider->setEnabled(false);
    else
    {
        ui->ledValueSlider->setEnabled(true);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

        mDominantLedNum = leftLedList.indexOf(arg1);
        mDominantLed = arg1;

        mEyeParameters.ledNum = ledNumMatLeft[mDominantLedNum];
    }

}

void MainWindow::on_rightComboLedBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.contains("No"))
        ui->ledValueSlider->setEnabled(false);
    else
    {
        ui->ledValueSlider->setEnabled(true);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

        mDominantLedNum = rightLedList.indexOf(arg1);
        mDominantLed = arg1;

        mEyeParameters.ledNum = ledNumMatRight[mDominantLedNum];
    }
}

void MainWindow::on_leftEyecheckBox_toggled(bool checked)
{
    if(checked)
    {
        ui->rightEyecheckBox->setChecked(false);
        ui->rightComboLedBox->setEnabled(false);
        ui->leftComboLedBox->setEnabled(true);

        //init slider
        ui->ledValueSlider->setEnabled(true);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

        //switch off left leds
        setBuffer(buffer, "led", ledNumMatRight[mDominantLedNum], 0);
        cmdSock->write(buffer, 11);

        mEyeParameters.eyeNum = LEFT;

        //++init combo box

    }
    else
    {
        ui->leftComboLedBox->setEnabled(false);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

    }
}

void MainWindow::handleEyeTestFinished()
{

    disconnect(mPupilMeasureThread, SIGNAL(sendImage(QImage)), this, SLOT(updateScreen(QImage)));
    disconnect(mPupilMeasureThread, SIGNAL(sendDiameter(int)), this, SLOT(handleSetDiameter(int)));
    disconnect(decPi, SIGNAL(newFrame(QPixmap)), mPupilMeasureThread, SLOT(updateImage(QPixmap)));
    mPupilMeasureThread->stop();

    disconnect(mEyeTestThread, SIGNAL(setLedBuffer(const char*,int,int)), this, SLOT(handleSetBuffer(const char*,int,int)));
    mTimer.stop();
    QFont* font = new QFont("Kozuka Gothic Pr6N M");
    font->setPointSize(10);
    font->setBold(true);
    ui->clockLabel->setFont(*font);
    ui->eyeStateLabel->setFont(*font);
    ui->eyeStateLabel->setStyleSheet("color: red");
    ui->eyeStateLabel->setText("Test Finished!");
    ui->diameterLabel->setText("");

    connect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));

//  store in excell
    xlsx.write("B12", "time");
    xlsx.write("C12", "diameter");
    int j = 13;
    for(int i=0;i<diameterStr.length();i++)
    {
        xlsx.write((QString("B%1").arg(QString::number(j))), timeStr[i].toDouble());
        xlsx.write((QString("C%1").arg(QString::number(j))), diameterStr[i].toInt());
        j++;
    }

    QDateTime timestamp = QDateTime::currentDateTime();
    xlsx.saveAs(QString("%1_%2.xlsx").arg(timestamp.toString("MMMd_h.m.ss"),ui->lastNameEdit->text()));


}

void MainWindow::on_rightEyecheckBox_toggled(bool checked)
{
    if(checked)
    {
        ui->leftEyecheckBox->setChecked(false);
        ui->leftComboLedBox->setEnabled(false);
        ui->rightComboLedBox->setEnabled(true);

        //init slider
        ui->ledValueSlider->setEnabled(true);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

        //switch off right leds
        setBuffer(buffer, "led", ledNumMatLeft[mDominantLedNum], 0);
        cmdSock->write(buffer, 11);

        mEyeParameters.eyeNum = RIGHT;
        //++init combo box
    }
    else
    {
        ui->rightComboLedBox->setEnabled(false);
        ui->ledValueSlider->setValue(0);
        on_ledValueSlider_valueChanged(0);

    }
}

void MainWindow::on_eyeAppButton_clicked(bool checked)
{
    if(checked)
        ui->stackedWidget_3->setCurrentIndex(EyeApp);
    else
        ui->stackedWidget_3->setCurrentIndex(Main);
}

void MainWindow::startEyeTest()
{

}

void MainWindow::on_minThresSlider_sliderMoved(int position)
{
    ui->minThresLabel->setText(QString::number(position));
    mEyeParameters.minThresValue = position;
    mPupilMeasureThread->setMinThresValue(position);
}

void MainWindow::on_maxThresSlider_sliderMoved(int position)
{
    ui->maxThresLabel->setText(QString::number(position));
    mEyeParameters.maxThresValue = position;
    mPupilMeasureThread->setMaxThresValue(position);
}

void MainWindow::on_setValuesRadioButton_clicked()
{
    on_ledValueSlider_valueChanged(0);
    mLedSlidersState = SetState;
}

void MainWindow::showTime()
{
    int secs = mTime.elapsed() / 1000;
    secs = secs % 60;
    int msecs = mTime.elapsed() - secs*1000;

    dtime = QString::number(secs) + "." + QString::number(msecs);

    ui->clockLabel->setText(QString::number(secs) + ":" + QString::number(msecs));

    if(secs > mEyeParameters.waitTime)
    {
        QFont* font = new QFont("Kozuka Gothic Pr6N M");
        font->setPointSize(10);
        font->setBold(true);
        ui->clockLabel->setFont(*font);
        ui->eyeStateLabel->setFont(*font);
        ui->eyeStateLabel->setStyleSheet("color: green");
        ui->eyeStateLabel->setText("-Stimulation Time-");
    }
    else if(secs > (mEyeParameters.stimulationTime+mEyeParameters.waitTime))
    {
        QFont* font = new QFont("Kozuka Gothic Pr6N M");
        font->setPointSize(10);
        font->setBold(true);
        ui->clockLabel->setFont(*font);
        ui->eyeStateLabel->setFont(*font);
        ui->eyeStateLabel->setStyleSheet("color: black");
        ui->eyeStateLabel->setText("-Recovery Time-");
    }
}

void MainWindow::on_testValueRadioButton_clicked()
{
    mLedSlidersState = TestState;
}

void MainWindow::on_stopTestButton_clicked()
{
    if(mEyeTestThread->isRunning())
    {
        disconnect(mPupilMeasureThread, SIGNAL(sendImage(QImage)), this, SLOT(updateScreen(QImage)));
        disconnect(mPupilMeasureThread, SIGNAL(sendDiameter(int)), this, SLOT(handleSetDiameter(int)));
        mPupilMeasureThread->stop();
        connect(decPi, SIGNAL(newFrame(QPixmap)), this, SLOT(updateScreen(QPixmap)));
        mEyeTestThread->stop();
        disconnect(mEyeTestThread, SIGNAL(setLedBuffer(const char*,int,int)), this, SLOT(handleSetBuffer(const char*,int,int)));
        mTimer.stop();
        QFont* font = new QFont("Kozuka Gothic Pr6N M");
        font->setPointSize(10);
        font->setBold(true);
        ui->eyeStateLabel->setFont(*font);
        ui->eyeStateLabel->setStyleSheet("color: red");
        ui->eyeStateLabel->setText("Test Stopped!");
        ui->diameterLabel->setText("");
    }

}

void MainWindow::on_ledValueSlider_sliderReleased()
{
    int value = ui->ledValueSlider->value();
    ui->ledValueLabel->setText(QString::number(value));
    mEyeParameters.ledValue = value;

    if(mLedSlidersState == TestState)
    {
        if(ui->leftComboLedBox->isEnabled())
            setBuffer(buffer, "led", ledNumMatLeft[mDominantLedNum], value);
        else
            setBuffer(buffer, "led", ledNumMatRight[mDominantLedNum], value);

        cmdSock->write(buffer, 11);
    }
}

void MainWindow::on_patientInfoButton_clicked()
{
     ui->stackedWidget_3->setCurrentIndex(2);
}

void MainWindow::on_finishButton_clicked()
{
//  QXlsx::Document xlsx;
//   QDateTime::currentDateTime().toString()

   QString lastName = ui->lastNameEdit->text();
   QString firstName = ui->firstNameEdit->text();
   QString day = ui->dayEdit->text();
   QString month = ui->monthEdit->text();
   QString year = ui->yearEdit->text();
   QString city = ui->cityEdit->text();
   QString address = ui->addressEdit->text();
   QString homePhone = ui->homePhoneEdit->text();
   QString mobilePhone = ui->mobilePhoneEdit->text();
   QString notes = ui->notesEdit->toPlainText();

   xlsx.write("B2", "Last Name:");
   xlsx.write("C2", lastName);
   xlsx.write("B3", "First Name:");
   xlsx.write("C3", firstName);
   xlsx.write("B4", "Date of Birth:");
   xlsx.write("C4", day);
   xlsx.write("D4", month);
   xlsx.write("E4", year);
   xlsx.write("B5", "City:");
   xlsx.write("C5", city);
   xlsx.write("B6", "Address:");
   xlsx.write("C6", address);
   xlsx.write("B7", "Home phone:");
   xlsx.write("C7", homePhone);
   xlsx.write("B8", "Mobile phone:");
   xlsx.write("C8", mobilePhone);
   xlsx.write("B9", "Notes:");
   xlsx.write("C9", notes);

   xlsx.save();
   ui->stackedWidget_3->setCurrentIndex(EyeApp);
}


void MainWindow::on_test2Button_clicked()
{

    ui->stackedWidget_3->setCurrentIndex(1);


}

void MainWindow::on_areaHorizontalSlider_sliderReleased()
{
    ui->areaLabel->setText(QString::number(ui->areaHorizontalSlider->value()));
    mEyeParameters.areaValue = ui->areaHorizontalSlider->value();
    mPupilMeasureThread->setAreaValue(mEyeParameters.areaValue);
}

void MainWindow::on_areaHorizontalSlider_sliderMoved(int position)
{
    ui->areaLabel->setText(QString::number(ui->areaHorizontalSlider->value()));
    mEyeParameters.areaValue = ui->areaHorizontalSlider->value();
}
