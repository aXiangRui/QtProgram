#include "Seller"
#include "ui_bookmechrant.h"

BookMechrant::BookMechrant(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BookMechrant)
{
    ui->setupUi(this);
}

BookMechrant::~BookMechrant()
{
    delete ui;
}
