#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <memory>

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>

#include "../MultiThreadingWorker/MultiThreadingWorker.h"

class FanucAdapter : public QObject
{
    Q_OBJECT

public:

    FanucAdapter(std::string serverIP, int port, QObject* parent = nullptr);

    ~FanucAdapter();

    void startConnections();

    void deInitialiseSocket();

signals:

    void signalToSendCurrentPosition(double j1, double j2, double j3, double j4, double j5,
        double j6);

public slots:

    void slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, double j6,
        double speed, int ctrl);

private slots:

    void slotReadFromServer();

    void slotServerDisconnected();

protected:

    MultiThreadingWorker            _workerInOtherThread;

    QThread                         _myThread;

    quint16                         _serverPort;

    std::unique_ptr<QTcpSocket>     _socket;

    std::string                     _serverIP;

    static constexpr int            DEFUALT_TIME_FOR_CONNECT = 1'000;

    bool TryConnect(int timeOut = DEFUALT_TIME_FOR_CONNECT);

    void makeConnection();

signals:

    void signalToInitialise(std::function<void()> func);
};

#endif // FANUC_ADAPTER_H