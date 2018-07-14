#include "Executor.h"
#include <iostream>
#include <QCoreApplication>

Executor::Executor(std::string robotIP, int robotPort, int clientPort, QObject* parent):
QObject(parent),
_clientThread(this),
_robotThread(this),
_robot(robotIP, robotPort, this),
_rcac(clientPort, this)
{

    _robot.moveToThread(&_robotThread);

    _rcac.moveToThread(&_clientThread);

    QObject::connect(&_robot, &FanucAdapter::signalToSendCurrentPosition, this,
        &Executor::slotNewRobotPostion);

    QObject::connect(this, &Executor::signalToSendNewPointToRobot, &_robot,
        &FanucAdapter::slotSendNextPosition);

    QObject::connect(this, &Executor::signalToSendCubePostion, &_rcac,
        &RCAConnector::slotToSendCubePosition);

    QObject::connect(this, &Executor::signalToSendCurrentPositionToClient, &_rcac,
        &RCAConnector::slotToSendCurrentRobotPostion);

    QObject::connect(&_rcac, &RCAConnector::signalToSearchCube, this,
        &Executor::slotFoundCubeTask);

    QObject::connect(&_rcac, &RCAConnector::signalToMoveRobot, this,
        &Executor::slotMoveRobot);

    QObject::connect(&_rcac, &RCAConnector::signalShutDown, this,
        &Executor::slotShutDown);

    _rcac.launch();

    try {
        _robot.startConnections();
    }
    catch (std::exception& exp) {
        std::cout << exp.what() << std::endl;
        throw std::exception();
    }

    _robotThread.start();
    _clientThread.start();
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
            abs(_currentCoords[4] - j5) + abs(_currentCoords[5] - j6))/20;
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
    shutDown();
}

void Executor::shutDown()
{
    std::cout << "system is shutting down" << std::endl;
    emit signalToSendNewPointToRobot(_currentCoords[0], _currentCoords[1], _currentCoords[2],
        _currentCoords[3], _currentCoords[4], _currentCoords[5], 0.08, 1);
    _clientThread.exit(0);
    _robotThread.exit(0);
    QCoreApplication::exit(0);
}
