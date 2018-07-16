#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <array>

#include <QObject>

#include "../FanucAdapter/FanucAdapter.h"
#include "../RCAConnector/RCAConnector.h"


class Executor : public QObject
{
    Q_OBJECT

public:

    static constexpr double TIME_FOR_RESPONSE = 30.;

    static constexpr double DEFAULT_SPEED = 0.08;
    
    Executor(std::string, int, int, QObject *parent = nullptr);

    void shutDown();

signals:

    void signalToSendNewPointToRobot(double j1, double j2, double j3, double j4, double j5,
        double j6, double speed, int ctrl);

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

    void slotShutDown();

protected:

    bool                    _wasFirstPoint{false};

    std::array<double, 6>   _currentCoords;

    RCAConnector            _rcac;

    FanucAdapter            _robot;
};

#endif // EXECUTOR_H