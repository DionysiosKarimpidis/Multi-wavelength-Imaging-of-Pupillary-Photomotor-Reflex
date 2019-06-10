#ifndef H264CONFIG_H
#define H264CONFIG_H

#include "definitions.h"

#include <QWidget>

#define PROFILE 100
#define LEVEL 120
#define BITRATE 140
#define NAL_FORMAT 160
#define INTRA_MODE 180
#define INTRA_PERIOD 200
#define RATE 220
#define ENCODE_QP 240
#define MIN_QUANT 260
#define MAX_QUANT 280
#define MINIMIZE_FRAGMENTATION 300

namespace Ui {
class H264Config;
}

class H264Config : public QWidget
{
    Q_OBJECT

public:
    explicit H264Config(QWidget *parent = 0);
    ~H264Config();

signals:

    void option(int param, int value);

private slots:
    void on_comboProfile_currentIndexChanged(int index);

    void on_comboLevel_currentIndexChanged(int index);

    void on_lineBitrate_returnPressed();

    void on_comboNAL_currentIndexChanged(int index);

    void on_comboIntra_currentIndexChanged(int index);

    void on_lineIntraPeriod_returnPressed();

    void on_comboRate_currentIndexChanged(int index);

    void on_lineQP_returnPressed();

    void on_lineMinQuant_returnPressed();

    void on_lineMaxQuant_returnPressed();

    void on_checkMinFrag_clicked(bool checked);

private:
    Ui::H264Config *ui;
};

#endif // H264CONFIG_H
