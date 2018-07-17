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
    
    Executor(std::string, std::string, int, int, QObject *parent = nullptr);

signals:

    void signalWriteToRobot(QVector<double>);

    void signalWriteToBuisness(QVector<double>);

public slots:

    void slotToApplyCommand(const QString& id,const QVector<double>& params);

private:

    using executableCommand = void(Executor::*)(QVector<double>);

    bool                    _wasFirstPoint{false};

    std::array<double, 6>   _currentCoords;

    RCAConnector            _rcac;

    FanucAdapter            _robot;

    std::unordered_map<std::string, executableCommand> _commandTable;

    void moveRobot(QVector<double>);

    void answerClient(QVector<double>);

    void shutDown(QVector<double>);
};

#endif // EXECUTOR_H