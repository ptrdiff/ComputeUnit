#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <array>
#include <unordered_map>

#include <QObject>
#include <QString>

#include "../FanucAdapter/FanucAdapter.h"
#include "../RCAConnector/RCAConnector.h"


class Executor : public QObject
{
    Q_OBJECT

public:

    static constexpr double TIME_FOR_RESPONSE = 30.;

    static constexpr double DEFAULT_SPEED = 0.1;
    
    Executor(std::string, int, int, QObject *parent = nullptr);

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

    void slotToApplyCommand(const QString& id,const QVector<double>& params);

private:

    using executableCommand = void(Executor::*)(const QVector<double>&);

    bool                    _wasFirstPoint{false};

    std::array<double, 6>   _currentCoords;

    RCAConnector            _rcac;

    FanucAdapter            _robot;

    std::unordered_map<std::string, executableCommand> _commandTable;

    void moveRobot(const QVector<double>&);

    void answerClient(const QVector<double>&);

    void shutDown(const QVector<double>&);
};

#endif // EXECUTOR_H