#include "Executor.h"

#include <sstream>

#include <QCoreApplication>
#include <QDebug>

Executor::Executor(std::string RCCenterIP, std::string robotIP, int robotPort, int clientPort, QObject* parent)try :
QObject(parent),
_wasFirstPoint(false),
_currentCoords(),
_rcac(std::move(RCCenterIP), clientPort),
_robot(std::move(robotIP), robotPort),
_commandTable({ 
    { "m", &Executor::moveRobot },
    { "a", &Executor::answerClient },
    { "e", &Executor::shutDown }
})
{
    QObject::connect(&_rcac, &RCAConnector::signalNextComand, this, &Executor::slotToApplyCommand);
    QObject::connect(&_robot, &FanucAdapter::signalNextComand, this, &Executor::slotToApplyCommand);

    QObject::connect(this, &Executor::signalWriteToBuisness, &_rcac, &RCAConnector::slotWriteToServer);
    QObject::connect(this, &Executor::signalWriteToRobot, &_robot, &FanucAdapter::slotWriteToServer);
    qInfo() << "Executor started";
}
catch (std::exception& exp) {
    qCritical() << exp.what();
}
// TODO Fix bug "Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*"


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
    emit signalWriteToRobot(QVector<double>{_currentCoords[0], _currentCoords[1], _currentCoords[2],
        _currentCoords[3], _currentCoords[4], _currentCoords[5], DEFAULT_SPEED, 1.0});
    QCoreApplication::exit(0);
}

//todo improve number of parametrs checking
//todo remove redundent metods