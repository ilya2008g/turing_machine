#include "alphabet.h"

#include <QApplication>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    alphabet w;
    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::lightGray);
    w.setPalette(pal);
    w.show();
    return a.exec();
}
