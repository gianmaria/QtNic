#include "nic_widget.h"
#include "ui_nic_widget.h"

Nic_Widget::Nic_Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Nic_Widget)
{
    ui->setupUi(this);
}

Nic_Widget::~Nic_Widget()
{
    delete ui;
}
