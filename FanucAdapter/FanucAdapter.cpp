#include "FanucAdapter.h"

#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <cmath>
#include <exception>

#include <QDataStream>

FanucAdapter::FanucAdapter(std::string serverIP, int port, QObject* parent) :
        QObject(parent),
        _workerInOtherThread(),
        _myThread(),
        _serverIP(std::move(serverIP)),
        _port(static_cast<quint16>(port)),
        _socket(nullptr)
{
    connect(_socket.get(), &QTcpSocket::disconnected,this, &FanucAdapter::slotToDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead,this, &FanucAdapter::slotToReadyRead);
    connect(this, &FanucAdapter::signalToInitialise, &_workerInOtherThread,
            &MultiThreadingWorker::slotToDoSomething);

    this->moveToThread(&_myThread);

    _workerInOtherThread.moveToThread(&_myThread);

    _myThread.start();

    emit signalToInitialise([this]() {
        this->doConnect();
    });
}

FanucAdapter::~FanucAdapter() {
    emit signalToInitialise([this]() {
        this->deInitialiseSocket();
    });
    _myThread.wait();
    qDebug() << "robotAdapterShuttedDown";
}

void FanucAdapter::deInitialiseSocket() {
    _socket = std::unique_ptr<QTcpSocket>(nullptr);
    _myThread.quit();
}

void FanucAdapter::doConnect()
{
    _socket = new QTcpSocket(this);

    qDebug() << "Connecting FanucAdapter";

    _socket->connectToHost(_serverIP.c_str(), _port);

    if(_socket->waitForConnected(5000))
    {
        _socket->write("2 0 3 7 1 4 0.01 0");
    }
    else
    {
        qDebug() << "Error: " << _socket->errorString();
    }
}

void FanucAdapter::slotToDisconnected()
{
    _socket->close();
}

void FanucAdapter::slotWriteToServer(QVector<double> data)
{
    QDataStream dataStream;
    dataStream << 1 <<data;
    QByteArray byteArray;
    dataStream >> byteArray;
    _socket->write(byteArray);
}

void FanucAdapter::slotToReadyRead()
{
    QDataStream locData(_socket.get());

    QVector<double> coords;
    while (!locData.atEnd())
    {
        double coord;
        locData >> coord;
        coords.push_back(coord);
    }

    qDebug() << "FanucAdapter read data from server";

    emit signalNextComand(QString("a"),coords);
}