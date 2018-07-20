#include "Executor.h"

#include <sstream>

#include <QCoreApplication>

/**
 * \brief               Function for get string from elements of QVector
 * \param[in] params    Qvector for casting.
 * \return              String with values of vector.
 */
QString toQString(QVector<double> params)
{
    std::stringstream strStream;
    for (auto it : params)
        strStream << ' ' << it;
    return strStream.str().c_str();
}

Executor::Executor(std::string robotServerIP, const int robotServerPort, 
    std::string controlCenterIP, const int controlCenterPort, QObject* parent) try :
QObject(parent),
_wasFirstPoint(false),
_lastSendPoint(),
_controlCenterConnector(std::move(controlCenterIP), controlCenterPort),
_robotConnector(std::move(robotServerIP), robotServerPort),
_sensorAdapter({ {"SensorAdapter/tmp/echo.exe",6} }),
_commandTable({ 
    { "m", {&Executor::sendRobotMoveCommand, 7} },
    { "a", {&Executor::sendControlCenterRobotPosition, 6} },
    { "e", {&Executor::shutDownComputeUnit, 0} },
    { "s", { &Executor::NewSensorData, -1 }},
    { "f", { &Executor::aksSensor, -1}}
})
{
    qDebug() << "robot serverIP: \"" << robotServerIP.c_str() << "\" robot server port: " 
            << robotServerPort << " control center server IP: \"" << controlCenterIP.c_str()
            << "\" control center server port: " << controlCenterPort;
    const auto start = std::chrono::steady_clock::now();
    
    QObject::connect(&_controlCenterConnector, &RCAConnector::signalNextCommand, this, 
                     &Executor::slotToApplyCommand);
    QObject::connect(&_robotConnector, &RobotConnector::signalNextComand, this,
                     &Executor::slotToApplyCommand);

    QObject::connect(this, &Executor::signalWriteToControlCenter, &_controlCenterConnector,
                     &RCAConnector::slotWriteToServer);
    QObject::connect(this, &Executor::signalWriteToRobot, &_robotConnector,
                     &RobotConnector::slotWriteToServer);
    
    qInfo() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count()/1000.;
}
catch (std::exception& exp) {
    qCritical() << exp.what();
}
// TODO Fix bug: 
//"Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*"


void Executor::slotToApplyCommand(const QString& id, const QVector<double>& params)
{
    qDebug() << "id: \"" << id << "\" params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();

    if(_commandTable.count(id.toStdString()) == 0)
    {
        qCritical() << "Unknow command \"" << id << "\"";
    }
    else
    {
        const auto curFunction = _commandTable[id.toStdString()];
        if(curFunction.second != -1 && curFunction.second > params.size())
        {
            qCritical() << "Error too less arguments for \"" << id << "\" command(need minimum " 
                        << curFunction.second <<", has " << params.size() << ").";
        }
        else
        {
            if(curFunction.second != -1 && curFunction.second < params.size())
            {
                qWarning() << "Waring too much arguments for \"" << id << "\" command(need "
                           << curFunction.first << ", has " << params.size() << ").";
            }
            (this->*curFunction.first)(params);
        }
    }

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::sendRobotMoveCommand(QVector<double> params)
{
    qDebug() << "params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();

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

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::sendControlCenterRobotPosition(QVector<double> params)
{
    qDebug() << "params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();
    
    emit signalWriteToControlCenter(params);

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::shutDownComputeUnit(QVector<double> params)
{
    qDebug() << "params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();

    emit signalWriteToRobot(QVector<double>{_lastSendPoint[0], _lastSendPoint[1], _lastSendPoint[2],
        _lastSendPoint[3], _lastSendPoint[4], _lastSendPoint[5], DEFAULT_SPEED, 1.0});
    QCoreApplication::exit(0);

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::NewSensorData(QVector<double> params)
{
    qDebug() << "params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();

    emit signalWriteToControlCenter(params);

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::aksSensor(QVector<double> params)
{
    qDebug() << "params: " << toQString(params);
    const auto start = std::chrono::steady_clock::now();

    for(auto elem : params)
    {
        if (_sensorAdapter.isOpen(elem))
        {
            _sensorAdapter.sendCurPosition(elem, _lastSendPoint);
        }
    }

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;

}


//todo add logging to new functions