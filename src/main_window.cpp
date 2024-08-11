#include "main_window.h"
#include "nic_widget.h"
#include "./ui_main_window.h"

#include <QPushButton>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main_Window)
{
    ui->setupUi(this);

    for (int i = 0; i < 12; ++i) {
        //QPushButton *button = new QPushButton("Button " + QString::number(i + 1));
        Nic_Widget* w = new Nic_Widget(this);
        ui->verticalLayout->addWidget(w);
    }

}

Main_Window::~Main_Window()
{
    delete ui;
}
