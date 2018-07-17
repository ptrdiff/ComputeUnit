#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <memory>

#include <QObject>
#include <QTcpSocket>
#include <QThread>

#include "../MultiThreadingWorker/MultiThreadingWorker.h"


class FanucAdapter : public QObject {
Q_OBJECT

public:

    FanucAdapter(std::string serverIP, int port, QObject *parent = nullptr);

    ~FanucAdapter() override;

    void doConnect();

    void deInitialiseSocket();

signals:

    void signalNextComand(QString, QVector<double>);

public slots:

    void slotToDisconnected();

    void slotToReadyRead();

    void slotWriteToServer(QVector<double>);

protected:

    MultiThreadingWorker _workerInOtherThread;

    QThread _myThread;

    std::string _serverIP;

    quint16 _port;

    std::unique_ptr<QTcpSocket> _socket;

signals:

    void signalToInitialise(std::function<void()> func);
};

#endif // FANUC_ADAPTER_H