#include "RobotConnector.h"

#include <cmath>
#include <chrono>

RobotConnector::RobotConnector(std::string serverIP, int port, QObject *parent) :
    QObject(parent),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _socket(std::make_unique<QTcpSocket>(this))
{
  qInfo() << QString("Create with parameters: IP: %1, Port: %2").arg(QString::fromStdString(_serverIP),
                                                                     QString::number(_port));

  connect(_socket.get(), &QTcpSocket::disconnected, this, &RobotConnector::slotToDisconnected);
  connect(_socket.get(), &QTcpSocket::readyRead, this, &RobotConnector::slotToReadyRead);

  qDebug() << QString("Completed the creation.");
}

bool RobotConnector::isConnected() const
{
  return _socket->state() == QTcpSocket::SocketState::ConnectedState;
}

void RobotConnector::slotToConnect()
{
  qInfo() << QString("Start connection.");
  const auto startChrono = std::chrono::steady_clock::now();

  _socket->connectToHost(_serverIP.c_str(), _port);

  if (_socket->waitForConnected(30000))
  {
    _socket->write("2 0 3 7 4 1 0");

    const auto endChrono = std::chrono::steady_clock::now();
    const auto durationChrono = 
      std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Completed connection: %1 ms").arg(durationChrono / 1000.0);
  } else
  {
    qCritical() << QString("Fanuc Error: %1").arg(_socket->errorString());
    emit signalSocketError();
  }
}

void RobotConnector::slotToDisconnected()
{
  qInfo() << QString("Start disconnection.");
  _socket->close();
  emit signalSocketError();
  qDebug() << QString("Complete disconnection");
}

void RobotConnector::slotWriteToServer(QVector<double> data)
{
  QString dataString;
  for (auto &i : data)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start writing to server. Data: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  QTextStream dataStream(_socket.get());
  dataStream << "1 ";
  for (auto i = 0; i + 1 < data.size(); ++i)
  {
    dataStream << lround(data.at(i) * 1000) << ' ';
  }
  dataStream << data.at(data.size() - 1);
  dataStream.flush();

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono = 
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete writing to server: %1 ms").arg(durationChrono / 1000.0);
}

void RobotConnector::slotToReadyRead()
{
  qInfo() << QString("Start reading from server.");
  const auto startChrono = std::chrono::steady_clock::now();

  QTextStream locData(_socket.get());

  QVector<double> coords;
  while (!locData.atEnd())
  {
    double coord;
    locData >> coord;
    coords.push_back(coord);
    locData.skipWhiteSpace();
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono = 
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed reading from server: %1 ms").arg(durationChrono / 1000.0);

  emit signalNextCommand(ExectorCommand::SEND_TO_RCA, coords);
}