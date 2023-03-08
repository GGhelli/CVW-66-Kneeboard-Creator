#include "aboutkneebuilder.h"
#include "ui_aboutkneebuilder.h"

AboutKneebuilder::AboutKneebuilder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutKneebuilder)
{
    ui->setupUi(this);
}

AboutKneebuilder::~AboutKneebuilder()
{
    delete ui;
}
