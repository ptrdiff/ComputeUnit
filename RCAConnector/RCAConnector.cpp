#include "RCAConnector.h"
#include <sstream>
#include <array>
#include <iostream>
#include <QCoreApplication>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &str, const std::string &delim)
{
    const auto delim_pos = str.find(delim);

    if (delim_pos == std::string::npos)
        return {str};

    std::vector<std::string> ret{str.substr(0, delim_pos)};
    auto tail = split(str.substr(delim_pos + delim.size(), std::string::npos), delim);

    ret.insert(ret.end(), tail.begin(), tail.end());

    return ret;
}

RCAConnector::RCAConnector(int port, QObject* parent) :
QObject(parent),
_port(port),
_socket(std::make_unique<QTcpServer>(this))
{
    connect(_socket.get(), &QTcpServer::newConnection, this, &RCAConnector::slotMakeNewConnection);
}

void RCAConnector::launch()
{
    std::cout << "RCAConnector::launch()" <<std::endl;
    _socket->listen(QHostAddress::AnyIPv4, _port);
}

void RCAConnector::slotToSendCubePosition(double x, double y, double z, double w, double p,
    double r)
{
    std::cout << "RCAConnector::slotToSendCubePosition" <<std::endl;
    std::stringstream str;
    str << "a " << x << ' ' << y << ' ' << z << ' ' << w << ' ' << p << ' ' << r;
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotToSendCurrentRobotPostion(double j1, double j2, double j3, double j4,
    double j5, double j6)
{
    std::cout << "RCAConnector::slotToSendCurrentRobotPostion" <<std::endl;
    std::stringstream str;
    str << "r " << j1 << ' ' << j2 << ' ' << j3 << ' ' << j4 << ' ' << j5 << ' ' << j6;
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotMakeNewConnection()
{
    std::cout << "RCAConnector::slotMakeNewConnection()" << std::endl;
    std::cout << "new client connected!" << std::endl;

    _clientSocket = _socket->nextPendingConnection();

    connect(_clientSocket, &QTcpSocket::disconnected, this, &RCAConnector::slotClientDisconnected);
    connect(_clientSocket, &QTcpSocket::readyRead, this, &RCAConnector::slotReadFromClient);
}

void RCAConnector::slotReadFromClient()
{
    std::cout << "RCAConnector::slotReadFromClient()" << std::endl;
    _prevData += _clientSocket->readAll().toStdString();

    std::cout << "From client was recieved:" << _prevData <<'|' << std::endl;

    bool flag = true;

    while (flag && !_prevData.empty())
    {
        if (_prevData[0] == 'f')
        {
            emit signalToSearchCube();
            _prevData = _prevData.substr(2);
        }
        else if(_prevData[0] == 'e')
        {
            QCoreApplication::exit(0);
        }
        else if (_prevData[0] == 'm')
        {
            auto splitString = split(_prevData," ");
            emit signalToMoveRobot(atof(splitString[1].c_str()),
                                   atof(splitString[2].c_str()),
                                   atof(splitString[3].c_str()),
                                   atof(splitString[4].c_str()),
                                   atof(splitString[5].c_str()),
                                   atof(splitString[6].c_str()),
                                   atoi(splitString[7].c_str()));
            break;
        }
    }
    // todo: rewrite it later
    _prevData = "";
}

void RCAConnector::slotClientDisconnected()
{
    std::cout << "RCAConnector::slotClientDisconnected()" << std::endl;
    _clientSocket->close();
}