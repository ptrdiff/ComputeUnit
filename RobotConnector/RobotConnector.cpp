#include "RobotConnector.h"

#include <cmath>

RobotConnector::RobotConnector(std::string serverIP, int port, QObject *parent) :
    QObject(parent),
    _workerInOtherThread(),
    _myThread(),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _socket(nullptr)
{
  connect(this, &RobotConnector::signalToInitialise, &_workerInOtherThread,
          &MultiThreadingWorker::slotToDoSomething);

  this->moveToThread(&_myThread);

  _workerInOtherThread.moveToThread(&_myThread);

  _myThread.start();

  emit signalToInitialise([this]()
                          {
                            this->doConnect();
                          });
}

RobotConnector::~RobotConnector()
{
  emit signalToInitialise([this]()
                          {
                            this->deInitialiseSocket();
                          });
  _myThread.wait();
  qDebug() << "robotAdapterShuttedDown";
}

void RobotConnector::deInitialiseSocket()
{
  _socket = std::unique_ptr<QTcpSocket>(nullptr);
  _myThread.quit();
}

void RobotConnector::doConnect()
{
  _socket = std::make_unique<QTcpSocket>(this);

  connect(_socket.get(), &QTcpSocket::disconnected, this, &RobotConnector::slotToDisconnected);
  connect(_socket.get(), &QTcpSocket::readyRead, this, &RobotConnector::slotToReadyRead);

  qDebug() << "Connecting RobotConnector";

  _socket->connectToHost(_serverIP.c_str(), _port);

  if (_socket->waitForConnected(5000))
  {
    _socket->write("2 0 3 7 1 4 0.01 0");
  } else
  {
    qDebug() << "Fanuc Error: " << _socket->errorString();
  }
}

void RobotConnector::slotToDisconnected()
{
  if (_socket)
  {
    _socket->close();
    qDebug() << "RobotConnector disconnected from server";
    doConnect();
  }
}

void RobotConnector::slotWriteToServer(QVector<double> data)
{
  qInfo() << "RobotConnector try write!";
  QTextStream dataStream(_socket.get());
  dataStream << 1;
  for (auto i = 0; i + 1 < data.size(); ++i)
  {
    dataStream << ' ' << lround(data.at(i) * 1000);
  }
  dataStream << data.at(data.size() - 1);
  dataStream.flush();
  qInfo() << "RobotConnector finish write!";
}

void RobotConnector::slotToReadyRead()
{
  qInfo() << "RobotConnector try read!";
  QTextStream locData(_socket.get());

  QVector<double> coords;
  while (!locData.atEnd())
  {
    double coord;
    locData >> coord;
    coords.push_back(coord);
  }

  qDebug() << "RobotConnector read data from server";

  emit signalNextComand(QString("a"), coords);
}