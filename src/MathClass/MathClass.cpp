#include "MathClass.h"

#include <array>

#include <opencv2/calib3d.hpp>

#include "CardModel/CardModel.h"


MathModule::MathModule(QVector<double> curPosition, bool forCard):
_isCard(forCard),
_wasFirstPointSend(false),
_lastSendPoint(curPosition),
_lastReceivedPoint({ 0, 0, 0, 0, -90, 0 }),
_defaultPosition(curPosition)
{
    
}

QVector<double> MathModule::sendToRCATransformation(QVector<double> params)
{
    _lastReceivedPoint = params;
    nikita::FanucModel fanuc = nikita::FanucModel();
    std::array<double, 6> jointCoords{};
    for (size_t i = 0; i < _lastReceivedPoint.size(); ++i)
    {
        jointCoords[i] = _lastReceivedPoint[i];
    }
    cv::Mat p6 = fanuc.fanucForwardTask(jointCoords);
    std::array<double, 6> worldCoords = nikita::FanucModel::getCoordsFromMat(p6);
    QVector<double> qWorldCoords;
    for (size_t i = 0; i < worldCoords.size(); ++i)
    {
        if(i < 3)
        {
            qWorldCoords.push_back(worldCoords[i]);
        }
        else
        {
            qWorldCoords.push_back(worldCoords[i] * 180.0/nikita::PI);
        }
    }
    params.append(qWorldCoords);
    return params;
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
            auto coords = _cardModel.secondTypeOfMoving(_lastSendPoint[0], _lastSendPoint[1], 
                params[0], params[1]);
            _lastSendPoint = params;
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
                                          const std::array<double, 6> jointCoords)
{
    nikita::FanucModel _fanuc = nikita::FanucModel();
    const cv::Mat p6 = _fanuc.fanucForwardTask(jointCoords);
    cv::Mat res = p6 * _fanuc.getToCamera() * transformationMatrix * _fanuc.getToSixth();
    return nikita::FanucModel::getCoordsFromMat(res);
}

QVector<std::array<double, 7>> transformMarkerPosition(
        const std::array<double, 6> jointCoords, std::vector<std::array<double, 7>> objects)
{
    QVector<std::array<double, 7>> result;
    for (auto &i : objects)
    {
        cv::Mat transformMatrix = createTransformationMatrix(cv::Vec3d(i[4], i[5], i[6]),
                                                             cv::Vec3d(i[1], i[2], i[3]));
        std::array<double, 6> res = calculateMarkerPose(transformMatrix, jointCoords);
        result.push_back(std::array<double, 7>{i[0],
                                               res[0],
                                               res[1],
                                               res[2],
                                               res[3]*180.0/nikita::PI,
                                               res[4]*180.0/nikita::PI,
                                               res[5]*180.0/nikita::PI});
    }
    return result;
}

QVector<QVector<double>> MathModule::sendAfterSensorTransformation(
    std::vector<std::array<double, 7>> objects)
{
    std::array<double, 6> lastJointPoint;

    for (int i = 0; i < 6; ++i)
    {
        lastJointPoint[i] = _lastReceivedPoint[i];
    }

    auto newMarkerPos = transformMarkerPosition(lastJointPoint, objects);

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

        return command;
    }
}
