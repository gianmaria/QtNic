#include "main_window.h"
#include "./ui_main_window.h"
#include <QDebug>
#include <QPushButton>
#include <QShortcut>

#include "nic.h"

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main_Window)
{
    ui->setupUi(this);

    setWindowTitle("All my interfaces");

    auto* quit_shortcut = new QShortcut({Qt::Key_Escape}, this);
    connect(quit_shortcut, &QShortcut::activated,
            this, [this](){this->close();});

    connect(ui->pbSave, &QPushButton::released,
            this, &Main_Window::onPbSaveReleased);

    loadAllNics();
}

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
    // qDebug() << "goodbyeeeeeeeeeeeee";
    QMainWindow::closeEvent(event);
}

Main_Window::~Main_Window()
{
    delete ui;
}

void Main_Window::loadAllNics()
{
    ui->plainTextEdit->clear();

    auto nics = collect_nic_info();

    for (const auto& nic : nics)
    {
        const auto& name = get_name(nic);
        ui->plainTextEdit->appendPlainText(QString::fromUtf8(name.data(), -1));
    }
}

void Main_Window::onPbSaveReleased()
{
    auto content = ui->plainTextEdit->toPlainText().toStdString();
    try
    {
        auto nics = collect_nic_info();
        update_nic_metric(nics, content);
        ui->statusBar->showMessage("All good!", 3000);
    }
    catch (str_cref e)
    {
        auto msg = QString("%1").arg(e.data());
        ui->statusBar->showMessage(msg, 6000);
        // qDebug() << e;
    }
    catch (const std::exception& e)
    {
        auto msg = QString("%1").arg(e.what());
        ui->statusBar->showMessage(msg, 6000);
        // qDebug() << "[EXC] " << e.what();
    }
}
