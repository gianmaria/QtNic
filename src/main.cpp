#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Main_Window w;
    // w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint); // Remove maximize button
    // w.setFixedSize(300, 250); // Set a fixed size for the window
    w.show();

    return a.exec();
}
