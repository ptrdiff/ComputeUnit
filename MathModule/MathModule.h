#ifndef MATH_MODULE_H
#define MATH_MODULE_H

#include <QVector>

/**
 * \brief Class with math for params transformation.
 */
class MathModule
{
public:

  /**
   * \brief             Function for transformating params sended to RCA.
   * \param[in] params  Params for transforamtion.
   * \return            Transformed params
   */
  QVector<double> sendToRCATransformation(QVector<double> params);
  
  /**
   * \brief             Function for transformating params sended to robot.
   * \param[in] params  Params for transforamtion.
   * \return            Transformed params
   */
  QVector<double> sendToRobotTransformation(QVector<double> params);
  
  /**
   * \brief             Function for transformating params recived from robot and sended to sensor.
   * \param[in] params  Params for transforamtion.
   * \return            Transformed params
   */
  QVector<double> sendToSensorTransformation(QVector<double> params);

};

#endif // MATH_MODULE_H