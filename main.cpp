#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(1010, 1010); // чуть больше чтобы небыло ползунков у окна
    w.show();
    return a.exec();
}
