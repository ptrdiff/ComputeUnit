#include "FanucAdapter.h"
#include "thread"
#include "chrono"
#include <sstream>
#include <iostream>

FanucAdapter::FanucAdapter(std::string serverIP, int port, QObject* parent):
_prevData(""),
_serverPort(port),
_socket(std::make_unique<QTcpSocket>(this)),
_serverIP(std::move(serverIP))
{
    connect(_socket.get(), &QTcpSocket::readyRead, this, &FanucAdapter::slotReadFromServer);

    connect(_socket.get(), &QTcpSocket::disconnected, this, &FanucAdapter::slotServerDisconnected,
        Qt::QueuedConnection);

    //std::cout<<"Fanuc Apadter"
}

void FanucAdapter::startConnections()
{
    makeConnection();//
}

void FanucAdapter::slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, 
    double j6, int ctrl)
{
    std::stringstream sstr;
    sstr << "1 " << static_cast<int>(j1 * 1'000 + 0.5) << ' ' << static_cast<int>(j2 * 1'000 + 0.5)
        << ' ' << static_cast<int>(j3 * 1'000 + 0.5) << ' ' << static_cast<int>(j4 * 1'000 + 0.5)
        << ' ' << static_cast<int>(j5 * 1'000 + 0.5) << ' ' << static_cast<int>(j6 * 1'000 + 0.5) 
        << " 80 " << ctrl << ' ';
    std::cout << "was send: " << sstr.str() << std::endl;
    if (_socket->isOpen())
        _socket->write(sstr.str().c_str());
    else
        std::cout << "error with robot connection" << std::endl;
}

void FanucAdapter::slotReadFromServer()
{
    _prevData += _socket->readAll().toStdString();

    std::vector<double> coords;
    coords.reserve(6);

    for(size_t i=0;i<_prevData.size();++i)
    {
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
}

void FanucAdapter::slotServerDisconnected()
{
    makeConnection();
}

bool FanucAdapter::TryConnect(int timeOut) const
{
    if(_socket->isOpen())
    {
        _socket->close();
    }

    _socket->connectToHost(_serverIP.c_str(), _serverPort);

    if(_socket->waitForConnected(timeOut))
    {
        _socket->write("2 0 3 7 1 4 0.01 0");
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
    std::cout << "connected ot robot" << std::endl;
}
