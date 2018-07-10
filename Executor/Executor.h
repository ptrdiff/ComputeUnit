#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <QObject>

#include <array>

class Executor : public QObject
{
    Q_OBJECT

public:
    
    Executor(QObject *parent = nullptr);

signals:

    void signalToSendNewPointToRobot(double j1, double j2, double j3, double j4, double j5,
        double j6, int ctrl);

    void signalToSendCurrentPositionToClient(double j1, double j2, double j3, double j4, 
        double j5, double j6);

    void signalToSendCubePostion(double x, double y, double z, double w, double p, double r);

    void signalToFindCubePostion(double j1, double j2, double j3, double j4, double j5,
        double j6);

public slots:

    void slotNewRobotPostion(double j1, double j2, double j3, double j4, double j5, double j6);

    void slotMoveRobot(double j1, double j2, double j3, double j4, double j5,
        double j6, int ctrl);

    void slotFoundCubeTask();

    void slotNewCubePostion(double x, double y, double z, double w, double p, double r);

protected:

    bool                    _wasFirstPoint{false};

    std::array<double, 6>   _currentCoords;
};

#endif // EXECUTOR_H