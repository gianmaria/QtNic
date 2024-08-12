#include "main_window.h"

#include <QApplication>
#include "nic.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (not is_running_as_administrator())
    {
        restart_as_admin();
        qDebug() << "Adiosssssssssssss";
        return 0;
    }

    Main_Window w;
    // w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint); // Remove maximize button
    // w.setFixedSize(300, 250); // Set a fixed size for the window
    w.show();

    return a.exec();
}
