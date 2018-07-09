#include "Executor.h"
#include <iostream>

Executor::Executor(QObject* parent)
{
}

void Executor::slotNewRobotPostion(double j1, double j2, double j3, double j4, double j5, double j6)
{
    _wasFirstPoint = true;
    _currentCoords[0] = j1;
    _currentCoords[1] = j2;
    _currentCoords[2] = j3;
    _currentCoords[3] = j4;
    _currentCoords[4] = j5;
    _currentCoords[5] = j6;
    emit signalToSendCurrentPositionToClient(j1, j2, j3, j4, j5, j6);
}

void Executor::slotShiftRobot(double j1, double j2, double j3, double j4, double j5, double j6,
    int ctrl)
{
    std::cout << "shift command" << std::endl;
    if(_wasFirstPoint)
    {
        emit signalToSendNewPointToRobot(j1 + _currentCoords[0], j2 + _currentCoords[1],
            j3 + _currentCoords[2], j4 + _currentCoords[3], j5 + _currentCoords[4],
            j6 + _currentCoords[5], ctrl);
    }
}

void Executor::slotMoveRobot(double j1, double j2, double j3, double j4, double j5, double j6,
    int ctrl)
{
    std::cout << "move command" << std::endl;
    emit signalToSendNewPointToRobot(j1, j2, j3, j4, j5, j6, ctrl);
}

void Executor::slotFoundCubeTask()
{
    if(_wasFirstPoint)
    {
        emit signalToFindCubePostion(_currentCoords[0], _currentCoords[1], _currentCoords[2],
            _currentCoords[3], _currentCoords[4], _currentCoords[5]);
    }
}

void Executor::slotNewCubePostion(double x, double y, double z, double w, double p, double r)
{
    emit signalToSendCubePostion(x, y, z, w, p, r);
}
