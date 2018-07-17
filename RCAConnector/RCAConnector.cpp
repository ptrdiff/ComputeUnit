#include "RCAConnector.h"

#include <sstream>
#include <array>
#include <memory>
#include <iostream>
#include <string>
#include <utility> #include <vector>
#include <chrono>

#include <QCoreApplication>
#include <QtCore/QDataStream>

RCAConnector::RCAConnector(std::string serverIP, int port, QObject* parent) :
QObject(parent),
_workerInOtherThread(),
_myThread(),
_serverIP(std::move(serverIP)),
_port(static_cast<quint16>(port)),
_socket(nullptr)
{
    connect(_socket.get(), &QTcpSocket::disconnected,this, &RCAConnector::slotToDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead,this, &RCAConnector::slotToReadyRead);
    connect(this, &RCAConnector::signalToInitialise, &_workerInOtherThread,
            &MultiThreadingWorker::slotToDoSomething);

    this->moveToThread(&_myThread);
    
    _workerInOtherThread.moveToThread(&_myThread);
    
    _myThread.start();

    emit signalToInitialise([this]() {
        this->doConnect();
    });
}

RCAConnector::~RCAConnector() {
    emit signalToInitialise([this]() {
        this->deInitialiseSocket();
    });
    _myThread.wait();
    qDebug() << "RCAConnectorShuttiedDown";
}

void RCAConnector::deInitialiseSocket()
{
    _socket = std::unique_ptr<QTcpSocket>(nullptr);
    _myThread.quit();
    qDebug() << "RCAConnector deInitialiseSocket";
}

void RCAConnector::slotToReadyRead()
{
    QDataStream locData(_socket.get());

    QString token;
    locData >> token;

    QVector<double> coords;
    while (!locData.atEnd())
    {
        double coord;
        locData >> coord;
        coords.push_back(coord);
    }

    qDebug() << "RCAConnector read data from server";

    emit signalNextComand(token,coords);
}

void RCAConnector::slotToDisconnected()
{
    _socket->close();
    qDebug() << "RCAConnector disconnected from server";
}

void RCAConnector::doConnect()
{
    _socket = new QTcpSocket(this);

    qDebug() << "Connecting RCAConnector";

    _socket->connectToHost(_serverIP.c_str(), _port);

    if(!_socket->waitForConnected(5000))
    {
        qDebug() << "Error: " << _socket->errorString();
    }
}

void RCAConnector::slotWriteToServer(QVector<double> data)
{
    QDataStream dataStream;
    dataStream << data;
    QByteArray byteArray;
    dataStream >> byteArray;
    _socket->write(byteArray);
}
