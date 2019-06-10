#include "fsmode.h"
#include "ui_fsmode.h"
#include <QGraphicsScene>
#include <QDebug>

FSMode::FSMode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FSMode)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene());

    ui->graphicsView->scene()->addItem(&item);

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsView->setStyleSheet("background-color: black;");

    ui->graphicsView_2->setScene(new QGraphicsScene());

    ui->graphicsView_2->scene()->addItem(&item_2);

    ui->graphicsView_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsView_2->setStyleSheet("background-color: black;");

    ui->graphicsView_2->hide();

    action = new QAction(this);

    action->setShortcut(QKeySequence(Qt::Key_Escape));

    this->addAction(action);

    connect(action, SIGNAL(triggered(bool)), this, SLOT(exit(bool)));
}

FSMode::~FSMode()
{
    delete ui;
}

void FSMode::setPixmap(QPixmap p)
{
    item.setPixmap(p);
}

void FSMode::setPixmap_2(QPixmap p)
{
    item_2.setPixmap(p);
}

void FSMode::setRect(QRect r)
{
    rect = r;

    resizeEvent(0);
}

void FSMode::showZeroViewer(bool b)
{
    if(b)
        ui->graphicsView_2->show();
    else
        ui->graphicsView_2->hide();
}

void FSMode::showViewer(bool b)
{
    if(b)
        ui->graphicsView->show();
    else
        ui->graphicsView->hide();
}

void FSMode::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->graphicsView->fitInView(rect, Qt::KeepAspectRatio);

    ui->graphicsView_2->fitInView(rect, Qt::KeepAspectRatio);
}

void FSMode::exit(bool b)
{
    emit exitFS(false);
}


