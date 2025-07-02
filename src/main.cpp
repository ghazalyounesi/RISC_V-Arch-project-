#include "assemblerwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    
    QApplication a(argc, argv);

    
    AssemblerWindow w;
    w.show();

    
    return a.exec();
}
