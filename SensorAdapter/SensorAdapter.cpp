#include "SensorAdapter.h"

SensorAdapter::SensorAdapter(QObject* parent)
        :_cvs(0.062,9,4,0,"../ComputerVisionSystem/CamCalibStable.txt")
{
}

void SensorAdapter::slotToFindCube(double j1, double j2, double j3, double j4, double j5, double j6)
{
    try
    {
        auto tmp = _cvs.getMarkerPose(std::array<double, 6>{j1, j2, j3, j4, j5, j6});
        if(tmp[0]!=0 && tmp[1] != 0 && tmp[2] != 0 )
        emit signalCubeFind(tmp[1], tmp[2], tmp[3], -180, 0, 0);
    }
    catch(...)
    {
        
    }
}
