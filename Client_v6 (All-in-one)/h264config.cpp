#include "h264config.h"
#include "ui_h264config.h"

#include <QDebug>

H264Config::H264Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::H264Config)
{
    ui->setupUi(this);

    ui->comboProfile->blockSignals(true);
    ui->comboProfile->setCurrentIndex(3);
    ui->comboProfile->blockSignals(false);

    ui->comboLevel->blockSignals(true);
    ui->comboLevel->setCurrentIndex(13);
    ui->comboLevel->blockSignals(false);

    ui->lineBitrate->setText("3000");

    ui->comboNAL->blockSignals(true);
    ui->comboNAL->setCurrentIndex(0);
    ui->comboNAL->blockSignals(false);

    ui->comboIntra->blockSignals(true);
    ui->comboIntra->setCurrentIndex(2);
    ui->comboIntra->blockSignals(false);

    ui->lineIntraPeriod->setText("20");

    ui->comboRate->blockSignals(true);
    ui->comboRate->setCurrentIndex(3);
    ui->comboRate->blockSignals(false);

    ui->lineQP->setText("30");

    ui->lineMaxQuant->setText("51");

    ui->lineMinQuant->setText("14");
}

H264Config::~H264Config()
{
    delete ui;
}

void H264Config::on_comboProfile_currentIndexChanged(int index)
{
    emit option(PROFILE, PROFILE + index + 1);
}

void H264Config::on_comboLevel_currentIndexChanged(int index)
{
    emit option(LEVEL, LEVEL + index + 1);
}

void H264Config::on_lineBitrate_returnPressed()
{
    emit option(BITRATE, ui->lineBitrate->text().toInt() * 1000);
}

void H264Config::on_comboNAL_currentIndexChanged(int index)
{
    emit option(NAL_FORMAT, NAL_FORMAT + index + 1);
}

void H264Config::on_comboIntra_currentIndexChanged(int index)
{
    emit option(INTRA_MODE, INTRA_MODE + index + 1);
}

void H264Config::on_lineIntraPeriod_returnPressed()
{
    emit option(INTRA_PERIOD, ui->lineIntraPeriod->text().toInt());
}

void H264Config::on_comboRate_currentIndexChanged(int index)
{
    emit option(RATE, RATE + index + 1);
}

void H264Config::on_lineQP_returnPressed()
{
    emit option(ENCODE_QP, ui->lineQP->text().toInt());
}

void H264Config::on_lineMinQuant_returnPressed()
{
    emit option(MIN_QUANT, ui->lineMinQuant->text().toInt());
}

void H264Config::on_lineMaxQuant_returnPressed()
{
    emit option(MAX_QUANT, ui->lineMaxQuant->text().toInt());
}

void H264Config::on_checkMinFrag_clicked(bool checked)
{
    if(checked)
        emit option(MINIMIZE_FRAGMENTATION, 1);
    else
        emit option(MINIMIZE_FRAGMENTATION, 0);
}
