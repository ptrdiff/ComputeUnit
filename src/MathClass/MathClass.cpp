#include "MathClass.h"

#include <array>
#include <math.h>

#include <opencv2/calib3d.hpp>
#include <QDebug>

#include "CardModel/CardModel.h"


MathModule::MathModule(QVector<double> curPosition, bool forCard):
_isCard(forCard),
_wasFirstPointSend(false),
_lastSendPoint(curPosition),
_lastReceivedPoint({ 0, 0, 0, 0, -90, 0 }),
_defaultPosition(curPosition),
_cur_world_position(curPosition)
{
    if(_isCard)
    {
        _lastReceivedPoint = { 0,0,0,0,0,0 };
    }
}

QVector<double> MathModule::sendToRCATransformation(QVector<double> params)
{
    if(_isCard)
    {
        qInfo() << QString("start transform cordinates. %1 coords").arg(params.size());
        if(params.size() == 7)
        {
            if(abs(params.at(6)) != 0)
            {
                _lastReceivedPoint = params;
                for(int i=0;i<6;i+=2)
                {
                    _lastReceivedPoint[i] = 0;
                }
                return _cur_world_position;
            }

            QVector<double> distance_differ;
            for(int i=0;i<6;i+=2)
            {
                distance_differ.push_back(params.at(i) - _lastReceivedPoint.at(i));
            }
            if(abs(params.at(1) - params.at(3))<3 && abs(params.at(3) - params.at(5))<3)
            {
                const auto mid_angel = (params.at(1) + params.at(3) + params.at(5)) / 3;

                double mid_distance = 0.;
                for(const auto& elem: distance_differ)
                {
                    mid_distance += elem / distance_differ.size();
                }
                {
                    QString dataString;
                    for (auto &i : _cur_world_position)
                    {
                        dataString.push_back(QString("%1 ").arg(i));
                    }
                    qInfo() << QString("before transformation: %1").arg(dataString);
                }

                _cur_world_position = {_cur_world_position.at(0) - 
                        cos((mid_angel-175)*acos(-1.)/180)*mid_distance,
                    _cur_world_position.at(1) -
                        sin((mid_angel - 175)*acos(-1.) / 180)*mid_distance };

                {
                    QString dataString;
                    for (auto &i : _cur_world_position)
                    {
                        dataString.push_back(QString("%1 ").arg(i));
                    }
                    qInfo() << QString("after transformation: %1").arg(dataString);
                }
            }
            _lastReceivedPoint = params;
            return _cur_world_position;
        }
        return {};
    }
    else
    {
        _lastReceivedPoint = params;
        return params;
    }
}

QVector<double> MathModule::sendToRobotTransformation(QVector<double> params)
{
    if(!_isCard)
    {
        double speed = DEFAULT_SPEED;

        if (_wasFirstPointSend)
        {
            speed = 0.;

            for (size_t i = 0; i < 6; ++i)
            {
                speed += abs(_lastSendPoint.at(i) - params.at(i));
            }

            speed = std::min(speed / TIME_FOR_RESPONSE, MAX_SPEED);
        }

        _wasFirstPointSend = true;
        for (size_t i = 0; i < 6; ++i)
        {
            _lastSendPoint[i] = params.at(i);
        }

        // TODO (Danila) remove this line.
        speed = DEFAULT_SPEED;

        params.push_back(speed);
        std::swap(params[6], params[7]);

        for(int i=0; i<7; ++i)
        {
            params[i] *= 1000;
        }

        return params;
    }
    else
    {
        if (params.size() >= 8)
        {
            auto coords = _cardModel.secondTypeOfMoving(params[0], params[1], params[6], params[7]);
            QVector<double> message;
            for(auto& coord: coords)
            {
                message.push_back(coord);
            }
            return message;
        }
        if(params.size() >= 2)
        {
            auto coords = _cardModel.secondTypeOfMoving(_cur_world_position[0], _cur_world_position[1],
                params[0], params[1]);
            _lastSendPoint = params;
            if (coords.at(1) > 0)
                spd_coef = -1;
            else
                spd_coef = 1;
            QVector<double> message;
            for (auto& coord : coords)
            {
                message.push_back(coord);
            }
            return message;
        }
        return {};
    }
}

QVector<double> MathModule::sendToSensorTransformation(QVector<double> params)
{
    return params;
}

cv::Mat createTransformationMatrix(const cv::Vec3d& rotationVector,
    const cv::Vec3d& translationVector) {
    cv::Mat rotationMatrix;
    cv::Rodrigues(rotationVector, rotationMatrix);
    cv::Mat transformationMatrix = cv::Mat::zeros(4, 4, cv::DataType<double>::type);
    rotationMatrix.copyTo(transformationMatrix(cv::Rect(0, 0, 3, 3)));
    cv::Mat(translationVector * 1000).copyTo(transformationMatrix(cv::Rect(3, 0, 1, 3)));
    transformationMatrix.at<double>(3, 3) = 1;
    return transformationMatrix;
}

std::array<double, 6> calculateMarkerPose(const cv::Mat &transformationMatrix,
    const std::array<double, 6> cartesianCoords)
{
    nikita::FanucModel _fanuc = nikita::FanucModel();
    const cv::Mat p6 = nikita::FanucModel::matrixFromCartesianCoords(cartesianCoords);
    cv::Mat res = p6 * _fanuc.getToCamera() * transformationMatrix * _fanuc.getToSixth();
    return nikita::FanucModel::getCoordsFromMat(res);
}

QVector<std::array<double, 7>> transformMarkerPosition(
    const std::array<double, 6> jointAngles, std::vector<std::array<double, 7>> objects)
{
    QVector<std::array<double, 7>> result;
    for (auto &i : objects)
    {
        cv::Mat transformMatrix = createTransformationMatrix(cv::Vec3d(i[1], i[2], i[3]),
            cv::Vec3d(i[4], i[5], i[6]));
        std::array<double, 6> res = calculateMarkerPose(transformMatrix, jointAngles);
        result.push_back(std::array<double, 7>{i[0], res[0], res[1], res[2], res[3], res[4], res[5]});
    }
    return result;
}

QVector<QVector<double>> MathModule::sendAfterSensorTransformation(
    std::vector<std::array<double, 7>> objects)
{
    std::array<double, 6> lastPoint;

    for (int i = 0; i < 6; ++i)
    {
        lastPoint[i] = _lastSendPoint[i];
    }

    auto newMarkerPos = transformMarkerPosition(lastPoint, objects);

    QVector<QVector<double>> result;
    for (auto &i : newMarkerPos)
    {
        QVector<double> object;
        for(auto&j : i)
        {
            object.push_back(j);
        }
        result.push_back(object);
    }
    return result;
}

QVector<double> MathModule::shutDownCommand()
{
    if (!_isCard)
    {
        auto res = _defaultPosition;

        res.push_back(DEFAULT_SPEED);
        res.push_back(1);

        return res;
    }
    else
    {
        auto command = sendToRobotTransformation(_defaultPosition);

        //command.push_back(-1);

        return command;
    }
}
