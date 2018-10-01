#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <memory>

#include <QObject>
#include <QTcpSocket>

#include "Executor/ExecutorCommandList.h"

class RobotConnector : public QObject
{
 Q_OBJECT

 public:

  RobotConnector(std::string serverIP, int port, std::string welcomeCommand, int inputBlock,
      QObject *parent = nullptr);

  bool isConnected() const;

  bool isNotMoving() const;

 signals:

  void signalSocketError();
  void signalNextCommand(ExectorCommand, QVector<double>);

 public slots:

  void slotToConnect();
  void slotToDisconnected();
  void slotToReadyRead();

  void slotWriteToServer(QVector<double>);

 protected:

  std::string _serverIP;

  quint16 _port;

  std::string _welcomeCommand;

  int _inputBlock;

  size_t _sendedCommands;

  size_t _recievedCommands;

  std::unique_ptr<QTcpSocket> _socket;
};

#endif // FANUC_ADAPTER_H