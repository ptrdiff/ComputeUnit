#include "RCAConnector.h"

#include <chrono>

RCAConnector::RCAConnector(std::string serverIP, int port, QObject *parent) :
    QObject(parent),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _socket(std::make_unique<QTcpSocket>(this))
{
  qInfo() << QString("Create with parameters: IP: %1, Port: %2").arg(QString::fromStdString(_serverIP),
                                                                     QString::number(_port));

  connect(_socket.get(), &QTcpSocket::disconnected, this, &RCAConnector::slotToDisconnected);
  connect(_socket.get(), &QTcpSocket::readyRead, this, &RCAConnector::slotToReadyRead);

  qDebug() << QString("Completed the creation.");
}

bool RCAConnector::isConnected() const
{
  return _socket->state() == QTcpSocket::SocketState::ConnectedState;
}

void RCAConnector::slotToReadyRead()
{
  qInfo() << QString("Start reading from server.");
  const auto startChrono = std::chrono::steady_clock::now();

  QTextStream locData(_socket.get());

  QString token;
  locData >> token;

  QVector<double> coords;
  while (!locData.atEnd())
  {
    double coord;
    locData >> coord;
    coords.push_back(coord);
    locData.skipWhiteSpace();
  }

  ExectorCommand command;

  if(token == "m")
  {
    command = ExectorCommand::SEND_TO_ROBOT;
  }
  else if(token == "f")
  {
    command = ExectorCommand::SEND_TO_SENSOR;
  }
  else
  {
    command = ExectorCommand::INVALID;
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono = 
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete reading from server: %1 ms").arg(durationChrono / 1000.0);
  
  emit signalNextCommand(command, coords);
}

void RCAConnector::slotToDisconnected()
{
  qInfo() << QString("Start disconnection.");
  emit signalNextCommand(ExectorCommand::SHUT_DOWN, QVector<double>());
  _socket->close();
  qDebug() << QString("Complete disconnection.");
}

void RCAConnector::slotToConnect()
{
  qInfo() << QString("Start connection.");
  const auto startChrono = std::chrono::steady_clock::now();

  _socket->connectToHost(_serverIP.c_str(), _port);

  if (!_socket->waitForConnected(30000))
  {
    qCritical() << QString("RCAConnector Error: %1").arg(_socket->errorString());
    emit signalSocketError();
  } else
  {
    _socket->write("f");
    const auto endChrono = std::chrono::steady_clock::now();
    const auto durationChrono = 
      std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Complete connection: %1 ms").arg(durationChrono / 1000.0);
  }
}

void RCAConnector::slotWriteToServer(QVector<double> data)
{
  QString dataString;
  for (auto &i : data)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start writing to server. Data: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  QTextStream dataStream(_socket.get());
  dataStream << R"("data" : ")";
  for (auto &i : data)
  {
    dataStream << i << ' ';
  }
  dataStream << "\"|";
  dataStream.flush();

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono = 
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete writing to server: %1 ms").arg(durationChrono / 1000.0);
}
