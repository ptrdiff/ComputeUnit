#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <memory>

#include <QObject>
#include <QTcpSocket>

class RobotConnector : public QObject
{
 Q_OBJECT

 public:

  RobotConnector(std::string serverIP, int port, QObject *parent = nullptr);
  void doConnect();

 signals:

  void signalNextComand(QString, QVector<double>);

 public slots:

  void slotToDisconnected();
  void slotToReadyRead();

  void slotWriteToServer(QVector<double>);

 protected:

  std::string _serverIP;

  quint16 _port;

  std::unique_ptr<QTcpSocket> _socket;
};

#endif // FANUC_ADAPTER_H