#include "RCAConnector.h"
#include <sstream>
#include <array>
#include <iostream>
#include <QCoreApplication>

RCAConnector::RCAConnector(int port, QObject* parent) :
_port(port),
_socket(std::make_unique<QTcpServer>(this))
{
    connect(_socket.get(), &QTcpServer::newConnection, this, &RCAConnector::slotMakeNewConnection);
}

void RCAConnector::launch()
{
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
        else if (_prevData[0] == 'm' || _prevData[0] == 's')
        {
            std::array<double, 6> coords{};
            size_t pos = 2;
            int param = -1;
            bool flag1 = true;
            for (int i = 0;i < 6;++i)
            {
                flag1 = true;
                coords[i] = atof(_prevData.substr(pos).c_str());
                if (coords[i] == 0.)
                    flag1 = false;
                while (pos < _prevData.size() && _prevData[pos] != ' ' &&
                    (flag1 || _prevData[pos] == '0' || _prevData[pos] == '-' ||
                        _prevData[pos] == '.' || _prevData[pos] == ','))
                    ++pos;
                if (pos == _prevData.size() || _prevData[pos] != ' ')
                {
                    flag = false;
                    break;
                }
                ++pos;
            }
            if (pos + 1 < _prevData.size() && _prevData[pos] >= '0' && _prevData[pos] <= '9')
            {
                param = _prevData[pos] - '0';
                pos += 2;
            }
            if (flag)
            {
                if (_prevData[0] == 'm')
                    emit signalToMoveRobot(coords[0], coords[1], coords[2], coords[3], coords[4],
                        coords[5], param);
                else
                    emit signalToMakeShift(coords[0], coords[1], coords[2], coords[3], coords[4],
                        coords[5], param);
                std::cout << "was parsed: ";
                for (auto elem : coords)
                    std::cout << elem << ' ';
                std::cout << param << std::endl;

                _prevData = _prevData.substr(pos);
                pos = 0;
            }
        }
    }

    _prevData = "";
}

void RCAConnector::slotClientDisconnected()
{
    _clientSocket->close();
}
