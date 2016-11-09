#include "supdate.h"
#include "ui_supdate.h"

supdate::supdate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::supdate)
{
    ui->setupUi(this);
}

supdate::~supdate()
{
    delete ui;
}
