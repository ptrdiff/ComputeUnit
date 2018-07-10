//
// Created by timur on 06.07.18.
//

#ifndef COMPUTERVISIONSYSTEM_CVS_H
#define COMPUTERVISIONSYSTEM_CVS_H

#include <opencv2/calib3d.hpp>

#include "ArucoMarker/ArucoMarker.h"
#include "CamCalibWI/CamCalibWI.h"
#include "FanucModel/FanucModel.h"


namespace timur
{
    class CVS {
    private:
        timur::ArucoMarkers _arucoMarkers;
        cv::VideoCapture _vid;
        timur::CamCalibWi _camera;
        nikita::FanucModel _fanuc;

        cv::Mat createTransformationMatrix(const cv::Vec3d& rotationVector,
                                                  const cv::Vec3d& translationVector);
        std::array<double, 3> calculateMarkerPose(cv::Mat transformationMatrix,
                                                         std::array<double, 6> jointCorners);
    public:
        CVS(float arucoSqureDimension, int cointOfMarkers, int markerSize,
            int cameraIndex,
            std::string calibrationFileName);

        std::array<double, 3> getMarkerPose(std::array<double, 6> jointCorners);
    };
}

#endif //COMPUTERVISIONSYSTEM_CVS_H
