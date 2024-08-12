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

    //ui->plainTextEdit->setPlainText(QString("User is admin? %1").arg(is_user_admin()));

    qDebug() << "is running as admin?" << is_running_as_administrator();


    vec<std::shared_ptr<Interface>> nics = collect_nic_info();
    for (const auto& nic: nics)
    {
        auto& cpp_str = get_name(nic);
        ui->plainTextEdit->appendPlainText(QString::fromUtf8(cpp_str.data(), -1));
        // qDebug() << "shared ptr counter:" << nic.use_count();
    }


    // ui->plainTextEdit->appendPlainText(QString("is user admin? %1").arg(is_running_as_administrator()));
    connect(ui->pbLoad, &QPushButton::released,
            this, &Main_Window::onPbLoadReleased);
    connect(ui->pbSave, &QPushButton::released,
            this, &Main_Window::onPbSaveReleased);
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
    qDebug() << "goodbyeeeeeeeeeeeee";
    QMainWindow::closeEvent(event);
}

Main_Window::~Main_Window()
{
    delete ui;
}

void Main_Window::onPbLoadReleased()
{
    ui->plainTextEdit->clear();

    auto nics = collect_nic_info();
    auto cpp_str = dump_nic_info(nics);
    ui->plainTextEdit->setPlainText(QString::fromUtf8(cpp_str.data(), -1));
}

void Main_Window::onPbSaveReleased()
{
    auto content = ui->plainTextEdit->toPlainText().toStdString();
    try
    {
        auto nics = collect_nic_info();
        update_nic_metric(nics, content);
    }
    catch (str_cref e)
    {
        qDebug() << e;
    }
    catch (const std::exception& e)
    {
        qDebug() << "[EXC] " << e.what();
    }
}
