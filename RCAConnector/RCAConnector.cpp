#include "RCAConnector.h"
#include <sstream>
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

RCAConnector::RCAConnector(int port, QObject* parent) :
QObject(parent),
_port(static_cast<quint16>(port))
{
    this->moveToThread(&_myThread);

    auto func = [this]() {
        this->launch();
    };

    _initialiser.moveToThread(&_myThread);

    connect(this, &RCAConnector::signalToInitialise, &_initialiser, &Worker::slotToDoSomething);
    
    _myThread.start();

    emit signalToInitialise(func);
}

RCAConnector::~RCAConnector() {
    auto func = [this]() {
        this->deInitialiseSocket();
    };
    emit signalToInitialise(func);
    //_myThread.quit();
    _myThread.wait();
    std::cout << "RCAConnectorShuttiedDown\n";
}

void RCAConnector::launch()
{
    _socket.swap(std::unique_ptr<QTcpServer>(new QTcpServer(this)));
    //_socket = std::make_unique<QTcpServer>(this);
    connect(_socket.get(), &QTcpServer::newConnection, this, &RCAConnector::slotMakeNewConnection);
    _socket->listen(QHostAddress::AnyIPv4, _port);
    _isInitialiesd = true;
}

void RCAConnector::deInitialiseSocket()
{
    _socket.swap(std::unique_ptr<QTcpServer>(nullptr));
    _myThread.quit();
}

void RCAConnector::slotToSendCubePosition(double x, double y, double z, double w, double p,
    double r)
{
    if (!_isInitialiesd)
        launch();
    std::stringstream str;
    str << "a " << x << ' ' << y << ' ' << z << ' ' << w << ' ' << p << ' ' << r << ' ';
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotToSendCurrentRobotPostion(double j1, double j2, double j3, double j4,
    double j5, double j6)
{
    if (!_isInitialiesd)
        launch();
    std::stringstream str;
    str << "r " << j1 << ' ' << j2 << ' ' << j3 << ' ' << j4 << ' ' << j5 << ' ' << j6 << ' ';
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotMakeNewConnection()
{
    if (!_isInitialiesd)
        launch();
    std::cout << "new client connected!" << std::endl;

    _clientSocket = _socket->nextPendingConnection();

    connect(_clientSocket, &QTcpSocket::disconnected, this, &RCAConnector::slotClientDisconnected);
    connect(_clientSocket, &QTcpSocket::readyRead, this, &RCAConnector::slotReadFromClient);
}

void RCAConnector::slotReadFromClient()
{
    if (!_isInitialiesd)
        launch();
    std::string token;
    std::stringstream locData = std::stringstream(_clientSocket->readAll().toStdString());

    while (locData >> token) {
        if (token == "f") {
            emit signalToSearchCube();
        } else if (token == "e") {
            emit signalShutDown();
            break;
        } else if (token == "m") {
            std::array<double, 6> coords{};
            int param;
            for (double &coord : coords) {
                locData >> coord;
            }
            locData >> param;
            emit signalToMoveRobot(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], param);
        }
        else {
            locData.clear();
        }
    }
}

void RCAConnector::slotClientDisconnected()
{
    if (!_isInitialiesd)
        launch();
    _clientSocket->close();
}