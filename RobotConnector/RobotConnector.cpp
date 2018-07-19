#include "RobotConnector.h"

#include <cmath>

RobotConnector::RobotConnector(std::string serverIP, int port, QObject *parent) :
    QObject(parent),
    _workerInOtherThread(),
    _Thread(),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _socket(nullptr)
{
  qInfo() << QString("Create with parameters: IP: %1, Port: %2").arg(QString::fromStdString(_serverIP),
                                                                     QString::number(_port));
  auto startChrono = std::chrono::steady_clock::now();
  connect(this, &RobotConnector::signalToInitialise, &_workerInOtherThread,
          &MultiThreadingWorker::slotToDoSomething);

  this->moveToThread(&_Thread);

  _workerInOtherThread.moveToThread(&_Thread);

  _Thread.start();

  emit signalToInitialise([this]()
                          {
                            this->doConnect();
                          });
  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed the creation: %1 ms").arg(durationChrono / 1000.0);
}

RobotConnector::~RobotConnector()
{
  qInfo() << QString("Start destruction.");
  auto startChrono = std::chrono::steady_clock::now();

  emit signalToInitialise([this]()
                          {
                            this->deInitialiseSocket();
                          });
  _Thread.wait();

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed the destruction: %1 ms").arg(durationChrono / 1000.0);
}

void RobotConnector::deInitialiseSocket()
{
  qInfo() << QString("Start de-initialization.");
  auto startChrono = std::chrono::steady_clock::now();

  _socket = std::unique_ptr<QTcpSocket>(nullptr);
  _Thread.quit();

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed de-initialization: %1 ms").arg(durationChrono / 1000.0);
}

void RobotConnector::doConnect()
{
  qInfo() << QString("Start connection.");
  auto startChrono = std::chrono::steady_clock::now();

  _socket = std::make_unique<QTcpSocket>(this);

  connect(_socket.get(), &QTcpSocket::disconnected, this, &RobotConnector::slotToDisconnected);
  connect(_socket.get(), &QTcpSocket::readyRead, this, &RobotConnector::slotToReadyRead);

  qDebug() << "Connecting RobotConnector";

  _socket->connectToHost(_serverIP.c_str(), _port);

  if (_socket->waitForConnected(5000))
  {
    _socket->write("2 0 3 7 4 1 0");
    auto endChrono = std::chrono::steady_clock::now();
    auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Completed connection: %1 ms").arg(durationChrono / 1000.0);
  } else
  {
    qCritical() << QString("Fanuc Error: %1").arg(_socket->errorString());
    emit signalNextComand(QString("e"),QVector<double>());
  }
}

void RobotConnector::slotToDisconnected()
{
  qInfo() << QString("Start disconnection.");

  if (_socket)
  {
    _socket->close();
    doConnect();
  }
}

void RobotConnector::slotWriteToServer(QVector<double> data)
{
  QString dataString;
  for (auto &i : data)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start writing to server. Data: %1").arg(dataString);
  auto startChrono = std::chrono::steady_clock::now();

  QTextStream dataStream(_socket.get());
  dataStream << "1 ";
  for (auto i = 0; i + 1 < data.size(); ++i)
  {
    dataStream << lround(data.at(i) * 1000) << ' ';
  }
  dataStream << data.at(data.size() - 1);
  dataStream.flush();

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete writing to server: %1 ms").arg(durationChrono / 1000.0);
}

void RobotConnector::slotToReadyRead()
{
  qInfo() << QString("Start reading from server.");
  auto startChrono = std::chrono::steady_clock::now();

  QTextStream locData(_socket.get());

  QVector<double> coords;
  while (!locData.atEnd())
  {
    double coord;
    locData >> coord;
    coords.push_back(coord);
    locData.skipWhiteSpace();
  }

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed reading from server: %1 ms").arg(durationChrono / 1000.0);

  emit signalNextComand(QString("a"), coords);
}