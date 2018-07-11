#ifndef RCA_CONNECTOR_H
#define RCA_CONNECTOR_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

#include <memory>

class RCAConnector : public QObject
{
    Q_OBJECT
public:

    RCAConnector(int port, QObject *parent = nullptr);

    void launch();

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

    int                             _port;

    QTcpSocket *                    _clientSocket = nullptr;

    std::unique_ptr<QTcpServer>	    _socket;

};

#endif // RCA_CONNECTOR_H
