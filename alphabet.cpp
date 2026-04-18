#include "alphabet.h"
#include "ui_alphabet.h"

alphabet::alphabet(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::alphabet)
{
    ui->setupUi(this);
}

alphabet::~alphabet()
{
    delete ui;
}
