#include "RCAConnector.h"
#include <sstream>
#include <array>
#include <iostream>
#include <QCoreApplication>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>

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
    std::stringstream str;
    str << "a " << x << ' ' << y << ' ' << z << ' ' << w << ' ' << p << ' ' << r;
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotToSendCurrentRobotPostion(double j1, double j2, double j3, double j4,
    double j5, double j6)
{
    std::stringstream str;
    str << "r " << j1 << ' ' << j2 << ' ' << j3 << ' ' << j4 << ' ' << j5 << ' ' << j6;
    std::cout << "answer to client:" << str.str() << std::endl;
    _clientSocket->write(str.str().c_str());
}

void RCAConnector::slotMakeNewConnection()
{
    std::cout << "new client connected!" << std::endl;

    _clientSocket = _socket->nextPendingConnection();

    connect(_clientSocket, &QTcpSocket::disconnected, this, &RCAConnector::slotClientDisconnected);
    connect(_clientSocket, &QTcpSocket::readyRead, this, &RCAConnector::slotReadFromClient);
}

void RCAConnector::slotReadFromClient()
{
    double timeSum = 0;

    //std::ofstream out("input.txt");
    auto start = std::chrono::high_resolution_clock::now();

    std::chrono::time_point<std::chrono::steady_clock> t1, t2;

    std::string locData = _clientSocket->readAll().toStdString();

    std::cout << "From client was recieved:" << locData <<'|' << '\n';

    auto tmp = std::chrono::high_resolution_clock::now();

    auto cMes = locData.c_str();

    for(auto it = cMes; it-cMes < locData.size();)
    {
        if (*it == 'f')
        {
            emit signalToSearchCube();
            ++it;
        }
        else if(*it == 'e')
        {
            emit signalShutDown();
            QCoreApplication::exit(0);
            break;
        }
        else if (*it == 'm')
        {
            std::array<double, 6> coords;
            char* tmp;
            ++it;
            for (size_t i = 0; i < 6; ++i) {
                coords[i] = strtod(it, &tmp);
                it = tmp;
            }
            int param = strtol(it, &tmp, 10);
            it = tmp;
            t1 = std::chrono::high_resolution_clock::now();
            emit signalToMoveRobot(coords[0], coords[1], coords[2], coords[3], coords[4],
                                   coords[5], param);
            t2 = std::chrono::high_resolution_clock::now();
            timeSum += static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
                t2 - t1).count()) / 1000;
        }
        if (*it == ' ')
            ++it;
    }
    // todo: rewrite it later

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "time for read part: "
        << static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
            tmp - start).count()) / 1000 << '\n';
    std::cout << "time for parse part: "
        << static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
            end - tmp).count()) / 1000 << '\n';
    std::cout << "time for signal emits: " << timeSum << std::endl;
}

void RCAConnector::slotClientDisconnected()
{
    _clientSocket->close();
}