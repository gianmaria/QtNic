#include "main_window.h"
#include "./ui_main_window.h"

#include "nic.h"

#include <QPushButton>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main_Window)
{
    ui->setupUi(this);

    //ui->plainTextEdit->setPlainText(QString("User is admin? %1").arg(is_user_admin()));

    vec<Interface> nics = collect_nic_info();

}

Main_Window::~Main_Window()
{
    delete ui;
}
