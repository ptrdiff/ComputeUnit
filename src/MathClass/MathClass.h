#ifndef MATH_MODULE_H
#define MATH_MODULE_H

#include <array>
#include <vector>

#include <QVector>

#include "FanucModel/FanucModel.h"
#include "CardModel/CardModel.h"

/**
 * \brief Class with math for params transformation.
 */
class MathModule
{
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
    static constexpr double MAX_SPEED = 0.25;

    /**
     * \brief                   Constructor of math Class.
     * \param[in] startPostion  Current robot position.
     * \param[in] forCard       Flag if this CU would be used for card.
     */
    explicit MathModule(QVector<double> startPostion, bool forCard = false);

    /**
     * \brief                   Function for transformating params sended to RCA.
     * \param[in] params        Params for transforamtion.
     * \return                  Transformed params
     */
    QVector<double> sendToRCATransformation(QVector<double> params);

    /**
     * \brief                   Function for transformating params sended to robot.
     * \param[in] params        Params for transforamtion.
     * \param[in] lastSendPoint Previous command send to robot.
     * \return                  Vector of transformed params for commands for robot.
     */
    QVector<double> sendToRobotTransformation(QVector<double> params);

    /**
     * \brief                   Function for transformating params recived from robot and sended to 
     *                          sensor.
     * \param[in] params        Params for transforamtion.
     * \return                  Transformed params.
     */
    QVector<double> sendToSensorTransformation(QVector<double> params);

    /**
     * \brief                   Function for transformating params recived from sencor and sended to
     *                          RCA. 
     * \param[in] objects       
     * \return                  
     */
    QVector<QVector<double>>  sendAfterSensorTransformation(
        std::vector<std::array<double, 7>> objects);

    /**
     * \brief   Getter for last send point.
     * \return  Lasst send point.
     */
    QVector<double> shutDownCommand();

protected:

    /**
     * \brief Flag is this CU for card.
     */
    bool                _isCard;

    /**
     * \brief Flag if first point was send to robot.
     */
    bool                _wasFirstPointSend{ false };

    /**
     * \brief Array describing last point, sended to robot.
     */
    QVector<double>     _lastSendPoint;

    /**
     * \brief Array describing last point, recieved from robot.
     */
    QVector<double>     _lastReceivedPoint;

    /**
     * \brief Array describing robot default position.
     */
    QVector<double>     _defaultPosition;

    /**
     * \brief Array describing current position of robot in world cordinate system.(only for card)
     */
    QVector<double>     _cur_world_position;

    /**
     * \brief Class for transforming commands for card.
     */
    CardModel           _cardModel;

    /*
     * \brief Direction of moving.
     */
    int spd_coef = 1;
};

#endif // MATH_MODULE_H