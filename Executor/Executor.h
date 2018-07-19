#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <array>
#include <unordered_map>

#include <QObject>
#include <QString>

#include "../RobotConnector/RobotConnector.h"
#include "../RCAConnector/RCAConnector.h"


/**
 * \brief Executor for processing elememtary commands.
 */
class Executor : public QObject
{
    Q_OBJECT

public:

    /**
     * \brief Default time for response.
     */
    static constexpr double TIME_FOR_RESPONSE = 30.;

    /**
     * \brief Default robot speed.
     */
    static constexpr double DEFAULT_SPEED = 0.1;

    /**
     * \brief Maximium robot speed.
     */
    static constexpr double MAX_SPEED = 2.;
    
    /**
     * \brief                       Constructor executor and connecting to robot server and control 
     *                              center server.
     * \param[in] robotServerIP     IP adress of robot server.
     * \param[in] robotServerPort   Port of robot server.
     * \param[in] controlCenterIP   IP adress of control center server.
     * \param[in] controlCenterPort Port of control center server.
     * \param[in] parent 
     */
    Executor(RCAConnector& controlCenterConnector, RobotConnector& robotConnector, QObject *parent = nullptr);

signals:

    /**
     * \brief               Signal for writing answer to client.
     * \params[in] params   Parametrs of answer for clients.
     */
    void signalWriteToRobot(QVector<double> params);

    /**
     * \brief               Signal for sending moving message to robot.
     * \param[in] params    Parametrs for moving.
     */
    void signalWriteToControlCenter(QVector<double> params);

public slots:

    /**
     * \brief               Slot for processing commands from services.
     * \param[in] id        Id of command.
     * \param[in] params    Parametrs for this command.
     */
    void slotToApplyCommand(const QString& id, QVector<double> params);

private:

    /**
     * \brief Designation for link to function.
     */
    using executableCommand = void(Executor::*)(QVector<double>);

    /**
     * \brief Flag if first point was send to robot.
     */
    bool                                                                _wasFirstPoint{false};

    /**
     * \brief Array describing last point, sended to robot.
     */
    std::array<double, 6>                                               _lastSendPoint;

    /**
     * \brief Adaptor for communication with buismess layer.
     */
    RCAConnector&                                                        _controlCenterConnector;

    /**
     * \brief Adaptor for communication with robot.
     */
    RobotConnector&                                                      _robotConnector;

    /**
     * \brief Table of comprasion id of command with function for this command and number of it
     *        parametrs.
     */
    std::unordered_map<std::string, std::pair<executableCommand, int>> _commandTable;

    /**
     * \brief               Function for sending next point to robot.
     * \param[in] params    Coordinates and parametrs for next point.
     */
    void sendRobotMoveCommand(QVector<double> params);

    /**
     * \brief               Function for sending answer ot client.
     * \param[in] params    Current coordinates of robot.
     */
    void sendControlCenterRobotPosition(QVector<double> params);

    /**
     * \brief               Function for shutting down comrute unit.
     * \param[in] params    Un used parametr(for adding this function in command table)
     */
    void shutDownComputeUnit(QVector<double> params);
};

#endif // EXECUTOR_H