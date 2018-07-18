#ifndef RCA_CONNECTOR_H
#define RCA_CONNECTOR_H

#include <memory>

#include <QObject>
#include <QTcpSocket>
#include <QThread>

#include "../MultiThreadingWorker/MultiThreadingWorker.h"

class RCAConnector : public QObject
{
 Q_OBJECT
 public:

  explicit RCAConnector(std::string serverIP, int port, QObject *parent = nullptr);

  ~RCAConnector() override;

  void doConnect();

  void deInitializeSocket();

 signals:

  void signalNextCommand(QString, QVector<double>);

 public slots:

  void slotToDisconnected();
  void slotToReadyRead();

  void slotWriteToServer(QVector<double>);

 protected:

  MultiThreadingWorker _workerInOtherThread;

  QThread _Thread;

  std::string _serverIP;

  quint16 _port;

  std::unique_ptr<QTcpSocket> _socket;

 signals:

  void signalToInitialise(std::function<void()> func);

};

#endif // RCA_CONNECTOR_H
