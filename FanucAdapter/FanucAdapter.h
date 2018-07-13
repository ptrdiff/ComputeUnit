#ifndef FANUC_ADAPTER_H
#define FANUC_ADAPTER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

#include <memory>

class FanucAdapter : public QObject
{
    Q_OBJECT

public:

    FanucAdapter(std::string serverIP, int port, QObject* parent = nullptr);

    void startConnections();

signals:

    void signalToSendCurrentPosition(double j1, double j2, double j3, double j4, double j5,
        double j6);

public slots:

    void slotSendNextPosition(double j1, double j2, double j3, double j4, double j5, double j6,
        int ctrl);

private slots:

    void slotReadFromServer();

    void slotServerDisconnected();

protected:

    std::string                     _prevData;

    quint16 _serverPort;

    std::unique_ptr<QTcpSocket>     _socket;

    std::string                     _serverIP;

    bool TryConnect(int timeOut = 1'000) const;

    void makeConnection();

};

#endif // FANUC_ADAPTER_H