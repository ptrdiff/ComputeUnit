#include "Executor.h"
#include <iostream>

Executor::Executor(QObject* parent):
QObject(parent)
{
}

void Executor::slotNewRobotPostion(double j1, double j2, double j3, double j4, double j5, double j6)
{
    std::cout << "Executor::slotNewRobotPostion" << std::endl;
    emit signalToSendCurrentPositionToClient(j1, j2, j3, j4, j5, j6);
}

void Executor::slotMoveRobot(double j1, double j2, double j3, double j4, double j5, double j6,
    int ctrl)
{
    std::cout << "move command" << std::endl;

    double speed=0.01;
    if (_wasFirstPoint) 
    {
        speed = (abs(_currentCoords[0] - j1) + abs(_currentCoords[1] - j2) + 
            abs(_currentCoords[2] - j3) + abs(_currentCoords[3] - j4) + 
            abs(_currentCoords[4] - j5) + abs(_currentCoords[5] - j6))/2;
    }

    _wasFirstPoint = true;
    _currentCoords[0] = j1;
    _currentCoords[1] = j2;
    _currentCoords[2] = j3;
    _currentCoords[3] = j4;
    _currentCoords[4] = j5;
    _currentCoords[5] = j6;
    emit signalToSendNewPointToRobot(j1, j2, j3, j4, j5, j6, speed, ctrl);
}

void Executor::slotFoundCubeTask()
{
    std::cout << "Executor::slotFoundCubeTask" << std::endl;
    if(_wasFirstPoint)
    {
        emit signalToFindCubePostion(_currentCoords[0], _currentCoords[1], _currentCoords[2],
            _currentCoords[3], _currentCoords[4], _currentCoords[5]);
    }
}

void Executor::slotNewCubePostion(double x, double y, double z, double w, double p, double r)
{
    std::cout << "Executor::slotNewCubePostion" << std::endl;
    emit signalToSendCubePostion(x, y, z, w, p, r);
}

void Executor::slotShutDown()
{
    std::cout << "system is shutting down" << std::endl;
    emit signalToSendNewPointToRobot(_currentCoords[0], _currentCoords[1], _currentCoords[2],
        _currentCoords[3], _currentCoords[4], _currentCoords[5], 1);
}
