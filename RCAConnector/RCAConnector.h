#ifndef RCA_CONNECTOR_H
#define RCA_CONNECTOR_H

#include <memory>

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>

#include "../MultiThreadingWorker/MultiThreadingWorker.h"


class RCAConnector : public QObject
{
    Q_OBJECT
public:

    RCAConnector(int port, QObject *parent = nullptr);

    ~RCAConnector();

    void launch();

    void deInitialiseSocket();

signals:

    void signalToMoveRobot(double j1, double j2, double j3, double j4, double j5, double j6,
        int ctrl);

    void signalToSearchCube();

    void signalShutDown();

public slots:

    void slotToSendCubePosition(double x, double y, double z, double w, double p, double r);

    void slotToSendCurrentRobotPostion(double j1, double j2, double j3, double j4, double j5,
        double j6);

private slots:

    void slotMakeNewConnection();

    void slotReadFromClient();

    void slotClientDisconnected();

protected:

    MultiThreadingWorker        _workerInOtherThread;

    QThread                     _myThread;

    quint16                     _port;

    QTcpSocket *                _clientSocket = nullptr;

    std::unique_ptr<QTcpServer> _socket;

signals:

    void signalToInitialise(std::function<void()> func);

};

#endif // RCA_CONNECTOR_H
