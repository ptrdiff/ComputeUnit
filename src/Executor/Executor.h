#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <unordered_map>

#include <QObject>
#include <QString>
#include <QVector>

#include "RobotConnector/RobotConnector.h"
#include "RCAConnector/RCAConnector.h"
#include "SensorAdapter/SensorAdapter.h"
#include "MathClass/MathClass.h"
#include "ExecutorCommandList.h"
#include "ComputerVisionSystem/CVS.h"

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
     * \brief                             Constructor executor and connecting to robot server and
     *                                    control center server.
     * \param[in] controlCenterConnector  Instance of RCAConnector.
     * \param[in] robotConnector          Instance of RobotConnector.
     * \param[in] sensorAdapter           Insatnce of SensorAdapter.
     * \param[in] cvs                     Instance of CVS.
     * \param[in] mathModule              Instance of MathModule.
     * \param[in] parent                  Previous qt object.
     */
    Executor(RCAConnector& controlCenterConnector, RobotConnector& robotConnector, 
      SensorAdapter& sensorAdapter, timur::CVS& cvs, MathModule& mathModule, 
        QObject *parent = nullptr);

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

    /**
     * \brief Signal for make connection to robot and RCA.
     */
    void signalToConnect();

public slots:

    /**
     * \brief               Slot for processing commands from services.
     * \param[in] command   Id of command.
     * \param[in] params    Parametrs for this command.
     */
    void slotToApplyCommand(ExectorCommand command, QVector<double> params);

    /**
     * \brief Slot for processing socket errors.
     */
    void slotToSocketError();

private:

    /**
     * \brief Designation for link to function.
     */
    using executableCommand = void(Executor::*)(QVector<double>);

    /**
     * \brief Flag if first point was send to robot.
     */
    bool                                                                   _wasFirstPoint{false};

    /**
     * \brief Array describing last point, sended to robot.
     */
    QVector<double>                                                        _lastSendPoint;

    /**
     * \brief Adaptor for communication with buismess layer.
     */
    RCAConnector&                                                          _controlCenterConnector;

    /**
     * \brief Adaptor for communication with robot.
     */
    RobotConnector&                                                        _robotConnector;

    /**
     * \brief Adaptor for adding sensor to CU.
     */
    SensorAdapter&                                                         _sensorAdapter;

    /**
     * \brief Lib for computer vision.
     */
    timur::CVS&                                                            _cvs;

    /**
     * \brief Math for params transformation.
     */
    MathModule&                                                            _mathModule;

    /**
     * \brief Table of comprasion id of command with function for this command and number of it
     *        parametrs.
     */
    std::unordered_map<ExectorCommand, std::pair<executableCommand, int>> _commandTable;

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
     * \brief               Function for shutting down compute unit.
     * \param[in] params    Un used parametr(for adding this function in command table)
     */
    void shutDownComputeUnit(QVector<double> params);

    /**
     * \brief            Function for processing new data from sensor.
     * \param[in] params First number is id of sensor, other is data from this sensor.
     */
    void newSensorData(QVector<double> params);

    /**
     * \brief            Function for sending data to sensor.
     * \param[in] params First number is id of sensor, other is data from this sensor.
     */
    void askSensor(QVector<double> params);

    /**
     * \brief               Function for scanning scene and returning object's positions.
     * \param[in] params    Just for template.
     */
    void getComputerVisionSystemData(QVector<double> params);
};

//todo complit doxygen

#endif // EXECUTOR_H