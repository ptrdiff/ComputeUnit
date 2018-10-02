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
    static constexpr double MAX_SPEED = 2.;

    /**
     * \brief                   Constructor of math Class.
     * \param[in] forCard       Flag if this CU would be used for card.
     */
    explicit MathModule(bool forCard = false);

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
     * \brief Class for transforming commands for card.
     */
    CardModel           _cardModel;
};

#endif // MATH_MODULE_H