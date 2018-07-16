#include "FanucAdapter.h"

#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <cmath>
#include <exception>

FanucAdapter::FanucAdapter(std::string serverIP, int port, QObject* parent):
QObject(parent),
_workerInOtherThread(),
_myThread(),
_serverPort(static_cast<quint16>(port)),
_socket(),
_serverIP(std::move(serverIP))
{
    this->moveToThread(&_myThread);

    _workerInOtherThread.moveToThread(&_myThread);

    connect(this, &FanucAdapter::signalToInitialise, &_workerInOtherThread,
        &MultiThreadingWorker::slotToDoSomething);

    _myThread.start();

    emit signalToInitialise([this]() {
        this->startConnections();
    });
}

FanucAdapter::~FanucAdapter() {
    emit signalToInitialise([this]() {
        this->deInitialiseSocket();
    });
    _myThread.wait();
    std::cout << "robotAdapterShuttedDown" << std::endl;
}
void FanucAdapter::startConnections()
{
    _socket.swap(std::unique_ptr<QTcpSocket>(new QTcpSocket(this)));
    connect(_socket.get(), &QTcpSocket::readyRead, this, &FanucAdapter::slotReadFromServer);

    connect(_socket.get(), &QTcpSocket::disconnected, this, &FanucAdapter::slotServerDisconnected,
        Qt::QueuedConnection);
    
    bool flag = false;

    constexpr int LONG_CONNECTION = 10'000;
    
    for (int i = 0; i < 3; ++i) {
        if (TryConnect(LONG_CONNECTION))
        {
            flag = true;
            break;
        }
        std::cout << "Can't create first connect to server" << std::endl;
    }//*/
    if (!flag)
        throw std::exception();
    std::cout << "Connect to server!" << std::endl;
    
}

void FanucAdapter::deInitialiseSocket() {
    _socket.swap(std::unique_ptr<QTcpSocket>(nullptr));
    _myThread.quit();
}

void FanucAdapter::slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, 
    double j6, double speed, int ctrl)
{
    std::stringstream sstr;
    sstr << "1 " << lround(j1 * 1'000) << ' ' << lround(j2 * 1'000) << ' ' << lround(j3 * 1'000)
        << ' ' << lround(j4 * 1'000) << ' ' << lround(j5 * 1'000) << ' ' << lround(j6 * 1'000)
        << ' ' << lround(speed * 1'000) << ' ' << ctrl << ' ';
    std::cout << "was send: " << sstr.str() << '|' << std::endl;
    if (_socket->isOpen())
        _socket->write(sstr.str().c_str());
    else
        std::cout << "error with robot connection" << std::endl;
}

void FanucAdapter::slotReadFromServer()
{
    std::cout << "FanucAdapter::slotReadFromServer()" <<std::endl;
    std::string  newData = _socket->readAll().toStdString();

    std::cout<< newData << std::endl;

    std::vector<double> coords;
    coords.reserve(6);

    for(size_t i=0;i<newData.size();++i)
    {
        if (newData[i] != ' ' && newData[i]<'0' && newData[i]>'9')
            continue;
        if(newData[i] == ' ')
        {
            if(coords.size() == 6)
            {
                emit signalToSendCurrentPosition(coords[0], coords[1], coords[2], coords[3],
                    coords[4], coords[5]);
                coords.clear();
                newData = newData.substr(i);
                i = 0;
            }
            coords.emplace_back(atof(newData.substr(i).c_str()));
        }
    }
    if (coords.size() == 6)
    {
        emit signalToSendCurrentPosition(coords[0], coords[1], coords[2], coords[3],
            coords[4], coords[5]);
    }
    //todo rewrite
}

void FanucAdapter::slotServerDisconnected()
{
    _socket->close();
    makeConnection();
}

bool FanucAdapter::TryConnect(int timeOut)
{
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        return true;
    }

    if(_socket->isOpen())
    {
        _socket->close();
    }

    _socket->connectToHost(_serverIP.c_str(), _serverPort);

    if(_socket->waitForConnected(timeOut))
    {
        _socket->write("2 0 3 7 1 4 0.01 0");
        //todo check it
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
