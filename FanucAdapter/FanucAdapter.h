#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>

#include "../worker/worker.h"

#include <memory>

class FanucAdapter : public QObject
{
    Q_OBJECT

public:

    FanucAdapter(std::string serverIP, int port, QObject* parent = nullptr);

    ~FanucAdapter();

signals:

    void signalToSendCurrentPosition(double j1, double j2, double j3, double j4, double j5,
        double j6);

    void signalToInitialise(std::function<void()> func);

public slots:

    void startConnections();

    void deInitialiseSocket();

    void slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, double j6,
        double speed, int ctrl);

private slots:

    void slotReadFromServer();

    void slotServerDisconnected();

protected:

    Worker                          _initialiser;

    bool                            _isInitialised{ false };

    QThread                         _myThread;

    std::string                     _prevData;

    quint16 _serverPort;

    std::unique_ptr<QTcpSocket>     _socket;

    std::string                     _serverIP;

    bool TryConnect(int timeOut = 1'000);

    void makeConnection();

};

#endif // FANUC_ADAPTER_H