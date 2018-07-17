#include "Executor.h"

#include <sstream>

#include <QCoreApplication>
#include <QDebug>

Executor::Executor(std::string robotIP, int robotPort, int clientPort, QObject* parent)try :
QObject(parent),
_wasFirstPoint(false),
_currentCoords(),
_rcac(clientPort),
_robot(std::move(robotIP), robotPort),
_commandTable({ 
    { "m", &Executor::moveRobot },
    { "a", &Executor::answerClient },
    { "e", &Executor::shutDown }
})
{
    QObject::connect(&_robot, &FanucAdapter::signalToSendCurrentPosition, this,
        &Executor::slotNewRobotPostion, Qt::QueuedConnection);

    QObject::connect(this, &Executor::signalToSendNewPointToRobot, &_robot,
        &FanucAdapter::slotSendNextPosition, Qt::QueuedConnection);

    QObject::connect(this, &Executor::signalToSendCubePostion, &_rcac,
        &RCAConnector::slotToSendCubePosition, Qt::QueuedConnection);

    QObject::connect(this, &Executor::signalToSendCurrentPositionToClient, &_rcac,
        &RCAConnector::slotToSendCurrentRobotPostion, Qt::QueuedConnection);

    QObject::connect(&_rcac, &RCAConnector::signalToSearchCube, this,
        &Executor::slotFoundCubeTask, Qt::QueuedConnection);

    QObject::connect(&_rcac, &RCAConnector::signalToMoveRobot, this,
        &Executor::slotMoveRobot, Qt::QueuedConnection);

    QObject::connect(&_rcac, &RCAConnector::signalShutDown, this,
        &Executor::slotShutDown, Qt::QueuedConnection);
    
    qInfo() << "Executor started";
}
catch (std::exception& exp) {
    qCritical() << exp.what();
    throw exp;
}

void Executor::slotNewRobotPostion(double j1, double j2, double j3, double j4, double j5, double j6)
{
    qWarning() << "used deprecated answerClient metod!";
    slotToApplyCommand("a", { j1, j2, j3, j4, j5, j6 });
}

void Executor::slotMoveRobot(double j1, double j2, double j3, double j4, double j5, double j6,
    int ctrl)
{
    qWarning() << "used deprecated moveRobot metod!";
    slotToApplyCommand("m",{ j1, j2, j3, j4, j5, j6, static_cast<double>(ctrl) });
}

void Executor::slotFoundCubeTask()
{
    qDebug() << "Executor::slotFoundCubeTask";
    if(_wasFirstPoint)
    {
        emit signalToFindCubePostion(_currentCoords[0], _currentCoords[1], _currentCoords[2],
            _currentCoords[3], _currentCoords[4], _currentCoords[5]);
    }
}

void Executor::slotNewCubePostion(double x, double y, double z, double w, double p, double r)
{
    qDebug() << "Executor::slotNewCubePostion";
    emit signalToSendCubePostion(x, y, z, w, p, r);
}

void Executor::slotShutDown()
{
    qWarning() << "used deprecated shutDown metod!";
    slotToApplyCommand("e", {});
}

void Executor::slotToApplyCommand(const QString& id,const QVector<double>& params)
{
    if(_commandTable.count(id.toStdString()) == 0)
    {
        qWarning() << "Unknow command: " << id;
    }
    else
    {
        (this->*_commandTable[id.toStdString()])(params);
    }
}

void Executor::moveRobot(QVector<double> j)
{
    if (j.size() < 7)
    {
        qCritical() << "too less arguments for moving robot";
        throw std::exception();
    }
    if (j.size() > 7)
    {
        qWarning() << "too many arguments for moving robot";
    }

    std::stringstream strStream;
    for (auto it : j)
        strStream << ' ' << it;
    qDebug() << "move command" << strStream.str().c_str();
    
    double speed = DEFAULT_SPEED;
    
    if (_wasFirstPoint)
    {
        speed = 0.;

        for(size_t i=0;i<6;++i)
        {
            speed += abs(_currentCoords[i] - j[i]);
        }

        speed /= TIME_FOR_RESPONSE;
    }

    _wasFirstPoint = true;
    for (size_t i = 0; i<6; ++i)
    {
        _currentCoords[i] = j[i];
    }

    j.push_back(speed);
    std::swap(j[6], j[7]);

    emit signalWriteToRobot(j);
}

void Executor::answerClient(QVector<double> j)
{
    if(j.size() < 6)
    {
        qCritical() << "too less arguments for answer";
        throw std::exception();
    }
    if(j.size() > 6)
    {
        qWarning() << "too many arguments for answer";
    }
    
    std::stringstream strStream;
    for (auto it : j)
        strStream << ' ' << it;
    qDebug() << "answer to client" << strStream.str().c_str();
    
    emit signalWriteToBuisness(j);
}

void Executor::shutDown(QVector<double> params)
{
    if(!params.empty())
    {
        qWarning() << "too much parametrs in shutting down";
    }
    
    qDebug() << "system is shutting down";
    emit signalToSendNewPointToRobot(_currentCoords[0], _currentCoords[1], _currentCoords[2],
        _currentCoords[3], _currentCoords[4], _currentCoords[5], DEFAULT_SPEED, 1);
    QCoreApplication::exit(0);
}

//todo improve number of parametrs checking
//todo remove redundent metods