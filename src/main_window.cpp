#include "main_window.h"
#include "./ui_main_window.h"

#include "nic.h"

#include <QPushButton>
Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main_Window)
{
    ui->setupUi(this);

    auto* quit_shortcut = new QShortcut({Qt::Key_Escape}, this);
    connect(quit_shortcut, &QShortcut::activated,
            this, [this](){this->close();});

    //ui->plainTextEdit->setPlainText(QString("User is admin? %1").arg(is_user_admin()));

    vec<Interface> nics = collect_nic_info();

void Main_Window::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A)
    {
        qDebug() << "A key pressed!";
    }

    QMainWindow::keyPressEvent(event);

}

void Main_Window::closeEvent(QCloseEvent *event)
{
    qDebug() << "goodbyeeeeeeeeeeeee";
    QMainWindow::closeEvent(event);
}

Main_Window::~Main_Window()
{
    delete ui;
}
