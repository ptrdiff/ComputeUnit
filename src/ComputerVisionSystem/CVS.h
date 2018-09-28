//
// Created by timur on 06.07.18.
//

#ifndef COMPUTERVISIONSYSTEM_CVS_H
#define COMPUTERVISIONSYSTEM_CVS_H

#include <array>
#include <vector>

#include <opencv2/calib3d.hpp>

#include "ArucoMarker/ArucoMarker.h"
#include "CamCalibWI/CamCalibWI.h"


namespace timur
{
    class CVS {
    private:
        timur::ArucoMarkers _arucoMarkers;
        cv::VideoCapture _vid;
        timur::CamCalibWi _camera;

        bool _isCorrect;

    public:
        CVS(float arucoSqureDimension, int cointOfMarkers, int markerSize,
            int cameraIndex,
            std::string calibrationFileName);

        std::vector<std::array<double, 7>> getMarkerPose();

        bool isCorrect();
    };
}

#endif //COMPUTERVISIONSYSTEM_CVS_H

