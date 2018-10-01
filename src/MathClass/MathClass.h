#ifndef MATH_MODULE_H
#define MATH_MODULE_H

#include <array>
#include <vector>

#include <QVector>

#include "FanucModel/FanucModel.h"

/**
 * \brief Class with math for params transformation.
 */
class MathModule
{
public:

    /**
     * \brief               Constructor of math Class.
     * \param[in] forCard   Flag if this CU would be used for card.
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
     * \brief                   Function for transformating params recived from sencor and sended to RCA.
     * \param[in] jointCorners  
     * \param[in] objects       
     * \return                  
     */
    QVector<std::array<double, 7>>  sendAfterSensorTransformation(
        std::array<double, 6> jointCorners, std::vector<std::array<double, 7>> objects);

protected:

    /**
     * \brief Flag is this CU for card.
     */
    bool _isCard;

};

#endif // MATH_MODULE_H