#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "jpegdecoder.h"
#include "x264decoder.h"
#include "x264decodert.h"
#include "fsmode.h"
#include "h264config.h"
#include "eyetestthread.h"

#include <QMainWindow>
#include <QTcpSocket>
#include <QGraphicsPixmapItem>
#include <QByteArray>
#include <QCloseEvent>
#include <QTime>
#include <QRect>
#include <QResizeEvent>
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QAction>
#include <QRgb>
#include <QPainter>
#include <QPen>
#include <windows.h>
#include <xlsxdocument.h>
#include <thread>

#include "imageregistrationpanel.h"
#include "pupilmeasurethread.h"

#define MAX_WIDTH 3296
#define MAX_HEIGHT 2464
#define TIMEOUT 100

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:

    bool serverConnect();

    void serverDisconnect();

    void setCameraSettings(QString setting, int value1, int value2);

    void initDecoders();

    void initViewers();

    void initUI();

    void enableUI();

    void destroyDecoders();

    void resetUI();

    void setBuffer(char *buffer, const char *cmd, int value1, int value2);

    bool compare;
private slots:

    void readTcpSnap();

    void readTcpData();

    void updateScreen(QPixmap p);

    void on_lineEditCommand_returnPressed();

    void closeEvent(QCloseEvent *event);

    void serverCommmunication();

    void on_buttonChange_clicked();

    void on_buttonWBAuto_toggled(bool checked);

    void on_buttonWBManual_toggled(bool checked);

    void on_buttonConnect_clicked();

    void on_sliderShutter_valueChanged(int value);

    void on_sliderWBBlue_valueChanged(int value);

    void on_sliderWBRed_valueChanged(int value);

    void on_sliderBrightness_valueChanged(int value);

    void on_sliderJPEGQuality_valueChanged(int value);

    void on_sliderFrameRate_valueChanged(int value);

    void on_lineIP_editingFinished();

    void on_linePort_editingFinished();

    void on_buttonAutoExposure_clicked(bool checked);

    void resizeEvent(QResizeEvent *event);

    void on_sliderShutter_sliderReleased();

    void on_comboBoxISO_currentIndexChanged(const QString &arg1);

    void on_comboBoxResolution_currentIndexChanged(const QString &arg1);

    void on_sliderFrameRate_sliderReleased();

    void on_buttonRotationCounterClock_clicked();

    void on_buttonRotationClock_clicked();

    void on_buttonWBAuto_clicked();

    void on_buttonWBManual_clicked();

    void on_sliderWBBlue_sliderReleased();

    void on_sliderWBRed_sliderReleased();

    void on_sliderBrightness_sliderReleased();

    void on_sliderJPEGQuality_sliderReleased();

    void on_buttonSnap_clicked(bool checked);

    void readTcpWB();

    void readTcpCameraSetting();

    void on_buttonSave_clicked();

    void on_buttonChangeCamera_clicked();

    void readTcpDataZero();

    void updateScreenZero(QPixmap p);

    void on_comboBoxCodec_currentIndexChanged(int index);

    void on_comboBoxMode_activated(int index);

    void resize();

    void readTcpSnapZero();

    void on_comboBox_currentIndexChanged(int index);

    void on_actionFullscreen_toggled(bool arg1);

    void on_buttonConfig264_clicked();

    void sendH264Option(int option, int value);

    void getShutter();

    void getWB();

    void on_sliderConrtast_valueChanged(int value);

    void on_sliderConrtast_sliderReleased();

    void handleDifferences(bool state);

    void on_startIRLedButton_clicked(bool state);

    void on_ledOffPushButton_clicked();

    void on_startTestButton_clicked();

    void on_ledValueSlider_valueChanged(int value);

    void initLedGUI();
    void on_rightComboLedBox_currentIndexChanged(const QString &arg1);

    void initRightLedList();

    void initLeftLedList();

    void on_leftEyecheckBox_toggled(bool checked);

    void on_rightEyecheckBox_toggled(bool checked);

    void on_leftComboLedBox_currentIndexChanged(const QString &arg1);

    void on_eyeAppButton_clicked(bool checked);

    void on_minThresSlider_sliderMoved(int position);

    void on_maxThresSlider_sliderMoved(int position);

    void on_setValuesRadioButton_clicked();

    void on_testValueRadioButton_clicked();

    void initEyeCameraSettings();
    void on_stopTestButton_clicked();

    void startEyeTest();
    void handleSetBuffer(const char *cmd, int value1, int value2);
    void showTime();
    void handleEyeTestFinished();
    void updateScreen(QImage img);
    void on_ledValueSlider_sliderReleased();

    void handleSetDiameter(int diameter);

    void on_patientInfoButton_clicked();
    void on_finishButton_clicked();


    void on_test2Button_clicked();

    void on_areaHorizontalSlider_sliderReleased();

    void on_areaHorizontalSlider_sliderMoved(int position);

private:

    enum CamType{
        single = 0,
        dual,
        dual_preview,
    };

    enum ledSlidersState{
        SetState,
        TestState
    };

    enum Codec{
        JPEG = 0,
        x264
    };

    enum Image_Codec{
        Image_JPG,
        Image_PNG,
        Image_RAW
    };

    enum Indexes{
        Command,
        EyeApp,
        PatientInfo,
        Main
    };

    Ui::MainWindow *ui;

    QGraphicsPixmapItem item,itemZero;

    QTcpSocket *imgSock, *cmdSock, *snapSock, *imgSockZero, *cmdSockZero, *snapSockZero;

    QByteArray data, dataZero, dataSnap, dataSnapZero, dataCmd;

    QImage framePi, frameZero;

    QString ip, ipZero;

    QString mDominantLed;
    int mDominantLedNum;

    QAction *action;

    QRect rect;

    QTime timerbitrate, timerbitrate2, timerfps, timerfps2;

    QTimer timerShutter, timerWB;

    JPEGDecoder *decoder, *decoderZero;

    x264DecoderT *decPi, *decZero;

    H264Config config264;

    FSMode fscreen;

    CamType ctype;

    QTime mTime;

    QTimer mTimer;

    Codec codec;

    EyeParameters mEyeParameters;

    Image_Codec img_codec;

    QList<QString> rightLedList;

    QList<QString> leftLedList;

    std::thread t;

    EyeTestThread *mEyeTestThread;

    pupilMeasureThread *mPupilMeasureThread;

    int *ledNumMatRight;
    int *ledNumMatLeft;

    int mLedSlidersState;

    int size, sizeZero, sizeCmd, port, rotation, width, height, snapSize, snapSizeZero, bits, bits2, fps, fps2, camNo, snapNo;

    char buffer[11];

    ImageRegistrationPanel *imgDifference;

    bool fs;

    QPixmap *zeroPx, *piPx;

    QString dtime;
    QList<QString> diameterStr;
    QList<QString> timeStr;
    QXlsx::Document xlsx;
};

#endif // MAINWINDOW_H
