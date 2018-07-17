#include "Executor.h"

#include <sstream>

#include <QCoreApplication>
#include <QDebug>

Executor::Executor(std::string robotServerIP, const int robotServerPort, 
    std::string controlCenterIP, const int controlCenterPort, QObject* parent) try :
QObject(parent),
_wasFirstPoint(false),
_lastSendPoint(),
_controlCenterAdapter(std::move(controlCenterIP), controlCenterPort),
_robotAdapter(std::move(robotServerIP), robotServerPort),
_commandTable({ 
    { "m", {&Executor::sendRobotMoveCommand, 7} },
    { "a", {&Executor::sendControlCenterRobotPosition, 6} },
    { "e", {&Executor::shutDownComputeUnit, 0} }
})
{
    QObject::connect(&_controlCenterAdapter, &RCAConnector::signalNextCommand, this, 
                     &Executor::slotToApplyCommand);
    QObject::connect(&_robotAdapter, &FanucAdapter::signalNextComand, this, 
                     &Executor::slotToApplyCommand);

    QObject::connect(this, &Executor::signalWriteToControlCenter, &_controlCenterAdapter,
                     &RCAConnector::slotWriteToServer);
    QObject::connect(this, &Executor::signalWriteToRobot, &_robotAdapter,
                     &FanucAdapter::slotWriteToServer);
    qInfo() << "Executor started";
}
catch (std::exception& exp) {
    qCritical() << exp.what();
}
// TODO Fix bug: 
//"Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*"


void Executor::slotToApplyCommand(const QString& id,const QVector<double>& params)
{
    if(_commandTable.count(id.toStdString()) == 0)
    {
        qCritical() << "Error unknow command \"" << id << "\"";
    }
    else
    {
        const auto curFunction = _commandTable[id.toStdString()];
        if(curFunction.second > params.size())
        {
            qCritical() << "Error too less arguments for \"" << id << "\" command(need minimum " 
                        << curFunction.second <<", has " << params.size() << ").";
        }
        else
        {
            if(curFunction.second < params.size())
            {
                qWarning() << "Waring too much arguments for \"" << id << "\" command(need "
                           << curFunction.first << ", has " << params.size() << ").";
            }
            (this->*curFunction.first)(params);
        }
    }
}

void Executor::sendRobotMoveCommand(QVector<double> params)
{
    std::stringstream strStream;
    for (auto it : params)
        strStream << ' ' << it;
    
    qDebug() << "move command with parametrs: " << strStream.str().c_str();
    
    double speed = DEFAULT_SPEED;
    
    if (_wasFirstPoint)
    {
        speed = 0.;

        for(size_t i=0;i<6;++i)
        {
            speed += abs(_lastSendPoint[i] - params[i]);
        }

        speed = std::min(speed / TIME_FOR_RESPONSE, MAX_SPEED);
    }

    _wasFirstPoint = true;
    for (size_t i = 0; i<6; ++i)
    {
        _lastSendPoint[i] = params[i];
    }

    params.push_back(speed);
    std::swap(params[6], params[7]);

    emit signalWriteToRobot(params);
}

void Executor::sendControlCenterRobotPosition(QVector<double> params)
{
    std::stringstream strStream;
    for (auto it : params)
        strStream << ' ' << it;
    qDebug() << "answer to client with parametrs: " << strStream.str().c_str();
    
    emit signalWriteToControlCenter(params);
}

void Executor::shutDownComputeUnit(QVector<double> params)
{
    qDebug() << "system is shutting down";
    emit signalWriteToRobot(QVector<double>{_lastSendPoint[0], _lastSendPoint[1], _lastSendPoint[2],
        _lastSendPoint[3], _lastSendPoint[4], _lastSendPoint[5], DEFAULT_SPEED, 1.0});
    QCoreApplication::exit(0);
}
