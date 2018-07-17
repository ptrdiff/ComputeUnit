#include <QtCore/QCoreApplication>

#include "Executor/Executor.h"

#include <iostream>

Q_DECLARE_METATYPE(std::function<void()>)

void initialise()
{
    qRegisterMetaType<std::function<void()>>("myLyambda");
    qRegisterMetaType<QVector<double>>("myQVectorDouble");
    qInfo() << "Meta tpes registered";
}

int main(int argc, char *argv[])
{
    try {
        QCoreApplication a(argc, argv);

        initialise();

        //Executor executor("172.27.221.60", 59002, 9090);
        Executor executor("localhost", 59002, "localhost", 9090);

        return a.exec();
    }
    catch (std::exception& exp) {
        std::cout << exp.what() << '\n';
        throw exp;
    }

}