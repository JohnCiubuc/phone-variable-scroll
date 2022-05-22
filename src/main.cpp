#include "PhoneVariableScroll.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PhoneVariableScroll w;
    w.show();
    return a.exec();
}
