#include "MathClass.h"

#include <array>

#include <opencv2/calib3d.hpp>

#include "CardModel/CardModel.h"


MathModule::MathModule(bool forCard):
_isCard(forCard),
_wasFirstPointSend(false),
_lastSendPoint({ 985, 0, 940, -180, 0, 0 }),
_lastReceivedPoint({ 0, 0, 0, 0, -90, 0 })
{
}

QVector<double> MathModule::sendToRCATransformation(QVector<double> params)
{
    _lastReceivedPoint = params;
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

        params.push_back(speed);
        std::swap(params[6], params[7]);
        return params;
    }
    else
    {
        if (params.size() >= 8)
            return { params[0], params[1], params[6], params[7] };
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
    const std::array<double, 6> jointCorners)
{
    nikita::FanucModel _fanuc = nikita::FanucModel();
    const cv::Mat p6 = _fanuc.fanucForwardTask(jointCorners);
    cv::Mat res = p6 * _fanuc.getToCamera() * transformationMatrix * _fanuc.getToSixth();
    return nikita::FanucModel::getCoordsFromMat(res);
}

QVector<std::array<double, 7>> transformMarkerPosition(
    const std::array<double, 6> jointCorners, std::vector<std::array<double, 7>> objects)
{
    QVector<std::array<double, 7>> result;
    for (auto &i : objects)
    {
        cv::Mat transformMatrix = createTransformationMatrix(cv::Vec3d(i[1], i[2], i[3]),
            cv::Vec3d(i[4], i[5], i[6]));
        std::array<double, 6> res = calculateMarkerPose(transformMatrix, jointCorners);
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
        lastPoint[i] = _lastReceivedPoint[i];
    }

    auto newMarkerPos = transformMarkerPosition(lastPoint, objects);

    QVector<QVector<double>> result;
    for (auto &i : objects)
    {
        result.push_back({i[0], i[1], i[2], i[3], i[4], i[5], i[6]});
    }
    return result;
}

QVector<double> MathModule::shutDownCommand()
{
    if (!_isCard)
    {
        auto res = _lastSendPoint;

        res.push_back(DEFAULT_SPEED);
        res.push_back(1);

        return res;
    }
    else
    {
        return QVector<double>();
    }
}
