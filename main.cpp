#include <QtCore/QCoreApplication>

#include "Executor/Executor.h"

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Executor executor("172.27.221.60", 59002, 9090);
    //Executor executor("127.0.0.1", 59002, 9090);
    
    return a.exec();
}
