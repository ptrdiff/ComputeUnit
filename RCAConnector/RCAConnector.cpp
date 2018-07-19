#include "RCAConnector.h"
#include <chrono>

RCAConnector::RCAConnector(std::string serverIP, int port, QObject *parent) :
    QObject(parent),
    _workerInOtherThread(),
    _Thread(),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _socket(nullptr)
{
  qInfo() << QString("Create with parameters: IP: %1, Port: %2").arg(QString::fromStdString(_serverIP), QString::number(_port));
  auto startChrono = std::chrono::steady_clock::now();

  connect(this, &RCAConnector::signalToInitialise, &_workerInOtherThread,
          &MultiThreadingWorker::slotToDoSomething);

  this->moveToThread(&_Thread);

  _workerInOtherThread.moveToThread(&_Thread);

  _Thread.start();

  emit signalToInitialise([this]()
                          {
                            this->doConnect();
                          });

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Completed the creation: %1 ms").arg(durationChrono / 1000.0);
}

RCAConnector::~RCAConnector()
{
  qInfo() << QString("Start destruction.");
  auto startChrono = std::chrono::steady_clock::now();
  emit signalToInitialise([this]()
                          {
                            this->deInitializeSocket();
                          });
  _Thread.wait();
  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Complete the destruction: %1 ms").arg(durationChrono / 1000.0);
}

void RCAConnector::deInitializeSocket()
{
  qInfo() << QString("Start de-initialization.");
  auto startChrono = std::chrono::steady_clock::now();

  _socket = std::unique_ptr<QTcpSocket>(nullptr);
  _Thread.quit();

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Complete de-initialization: %1 ms").arg(durationChrono / 1000.0);
}

void RCAConnector::slotToReadyRead()
{
  qInfo() << QString("Start reading from server.");
  auto startChrono = std::chrono::steady_clock::now();

  QTextStream locData(_socket.get());
  QString token;
  locData >> token;

  QVector<double> coords;
  while (!locData.atEnd())
  {
    double coord;
    locData >> coord;
    coords.push_back(coord);
  }

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Complete reading from server: %1 ms").arg(durationChrono / 1000.0);

  emit signalNextCommand(token, coords);
}

void RCAConnector::slotToDisconnected()
{
  qInfo() << QString("Start disconnection.");
  if (_socket)
  {
    _socket->close();
    doConnect();
  }
}

void RCAConnector::doConnect()
{
  qInfo() << QString("Start connection.");
  auto startChrono = std::chrono::steady_clock::now();

  _socket = std::make_unique<QTcpSocket>(this);

  connect(_socket.get(), &QTcpSocket::disconnected, this, &RCAConnector::slotToDisconnected);
  connect(_socket.get(), &QTcpSocket::readyRead, this, &RCAConnector::slotToReadyRead);

  _socket->connectToHost(_serverIP.c_str(), _port);

  if (!_socket->waitForConnected(5000))
  {
    qDebug() << "RCAConnector Error: " << _socket->errorString();
  }
  else
  {
    auto endChrono = std::chrono::steady_clock::now();
    auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Complete connection: %1 ms").arg(durationChrono / 1000.0);
  }
}

void RCAConnector::slotWriteToServer(QVector<double> data)
{
  QString dataString;
  for(auto &i : data)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start writing to server. Data: ") + dataString;
  auto startChrono = std::chrono::steady_clock::now();

  QTextStream dataStream(_socket.get());
  for (auto &i : data)
  {
    dataStream << i << ' ';
  }
  dataStream.flush();

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete writing to server: %1 ms").arg(durationChrono / 1000.0);
}
