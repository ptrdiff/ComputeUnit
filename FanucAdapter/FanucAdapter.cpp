#include "FanucAdapter.h"

#include <qthread.h>

#include "thread"
#include "chrono"
#include <sstream>
#include <iostream>
#include <cmath>
#include <exception>

FanucAdapter::FanucAdapter(std::string serverIP, int port, QObject* parent):
QObject(parent),
_prevData(""),
_serverPort(static_cast<quint16>(port)),
_socket(std::make_unique<QTcpSocket>(this)),
_serverIP(std::move(serverIP))
{
    connect(_socket.get(), &QTcpSocket::readyRead, this, &FanucAdapter::slotReadFromServer);

    connect(_socket.get(), &QTcpSocket::disconnected, this, &FanucAdapter::slotServerDisconnected,
        Qt::QueuedConnection);
}

void FanucAdapter::startConnections()
{
    //QThread *qth = QThread::create([this]() {this->makeConnection(); });
    //this->moveToThread(qth);
    //qth->start();
    bool flag = false;

    for (int i = 0; i < 3; ++i) {
        if (TryConnect(10000))
        {
            flag = true;
            break;
        }
        std::cout << "Can't create first connect to server" << std::endl;
    }
    if (!flag)
        throw std::exception();
    std::cout << "Connect to server!" << std::endl;
}

void FanucAdapter::slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, 
    double j6, double speed, int ctrl)
{
    std::stringstream sstr;
    sstr << "1 " << lround(j1 * 1'000) << ' ' << lround(j2 * 1'000) << ' ' << lround(j3 * 1'000)
        << ' ' << lround(j4 * 1'000) << ' ' << lround(j5 * 1'000) << ' ' << lround(j6 * 1'000)
        << " " << lround(speed * 1'000) << " " << ctrl << ' ';
    std::cout << "was send: " << sstr.str() << std::endl;
    if (_socket->isOpen())
        _socket->write(sstr.str().c_str());
    else
        std::cout << "error with robot connection" << std::endl;
}

void FanucAdapter::slotReadFromServer()
{
    std::cout << "FanucAdapter::slotReadFromServer()" <<std::endl;
    _prevData += _socket->readAll().toStdString();

    std::cout<<_prevData<<std::endl;

    std::vector<double> coords;
    coords.reserve(6);

    for(size_t i=0;i<_prevData.size();++i)
    {
        if (_prevData[i] != ' ' && _prevData[i]<'0' && _prevData[i]>'9')
            continue;
        if(_prevData[i] == ' ')
        {
            if(coords.size() == 6)
            {
                emit signalToSendCurrentPosition(coords[0], coords[1], coords[2], coords[3],
                    coords[4], coords[5]);
                coords.clear();
                _prevData = _prevData.substr(i);
                i = 0;
            }
            coords.emplace_back(atof(_prevData.substr(i).c_str()));
        }
    }
    if (coords.size() == 6)
    {
        emit signalToSendCurrentPosition(coords[0], coords[1], coords[2], coords[3],
            coords[4], coords[5]);
        _prevData = "";
    }
    _prevData = "";
    //todo rewrite
}

void FanucAdapter::slotServerDisconnected()
{
    _socket->close();
    makeConnection();
}

bool FanucAdapter::TryConnect(int timeOut) const
{
    if(_socket->isOpen())
    {
        return true;
    }

    _socket->connectToHost(_serverIP.c_str(), _serverPort);

    if(_socket->waitForConnected(timeOut))
    {
        _socket->write("2 0 3 7 1 4 0.01 0");///check it
        return true;
    }
    return false;
}

void FanucAdapter::makeConnection()
{
    while (!TryConnect())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
        std::cout << "try to connect ot robot" << std::endl;
    }
    std::cout << "connected to robot" << std::endl;
}
