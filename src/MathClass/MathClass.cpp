#include "MathClass.h"

#include <array>

#include <opencv2/calib3d.hpp>

#include "CardModel/CardModel.h"


MathModule::MathModule(bool forCard):_isCard(forCard)
{
}

QVector<double> MathModule::sendToRCATransformation(QVector<double> params)
{
    return params;
}

QVector<double> MathModule::sendToRobotTransformation(QVector<double> params)
{
    if(!_isCard)
    {
        return params;
    }

    CardModle cardModel(0,0);

    if(params.size() == 4)
    {
        return cardModel.linerMoving(params);
    }
    if(params.size() == 2)
    {
        return cardModel.RotationMoving(params);
    }
    return QVector<double>();
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

QVector<std::array<double, 7>> MathModule::sendAfterSensorTransformation(const std::array<double, 6> jointCorners,
    std::vector<std::array<double, 7>> objects)
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
