//
// Created by timur on 06.07.18.
//

#include "CVS.h"
#include <exception>


timur::CVS::CVS(float arucoSqureDimension, int cointOfMarkers, int markerSize,
                int cameraIndex,
                std::string calibrationFileName)
        :_arucoMarkers(arucoSqureDimension,cointOfMarkers,markerSize)
        ,_vid(cameraIndex)
        ,_camera(calibrationFileName)
{
    if (!_vid.isOpened())
    {
        throw std::exception();
    }
}

std::vector<std::array<double, 7>> timur::CVS::getMarkerPose()
{
    cv::Mat frame;
    std::vector<cv::Vec3d> rotationVectors, translationVectors;
    std::vector<int> markerIds;
    std::vector<std::array<double, 7>> result;
    if (!_vid.read(frame))
    {
        throw std::exception();
    }

    bool foundMarkers = _arucoMarkers.estimateMarkersPose(frame, _camera.cameraMatrix(),
                                                               _camera.distortionCoefficients(),
                                                               rotationVectors,
                                                               translationVectors, markerIds);
    if(foundMarkers)
    {
        for(std::size_t i = 0; i < markerIds.size(); ++i)
        {
            std::array<double,7> markerInfo{static_cast<double>(i),
                                            translationVectors[i][0],
                                            translationVectors[i][1],
                                            translationVectors[i][2],
                                            rotationVectors[i][0],
                                            rotationVectors[i][1],
                                            rotationVectors[i][2]};
            result.push_back(markerInfo);
        }
    }
    return result;
}
