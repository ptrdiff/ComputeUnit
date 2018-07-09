#ifndef SENSOR_ADAPTER_H
#define SENSOR_ADAPTER_H

#include <QObject>

#include <memory>
#include "../ComputerVisionSystem/CVS.h"

class SensorAdapter : public QObject
{
    Q_OBJECT
public:

    SensorAdapter(QObject *parent = nullptr);

signals:

    void signalCubeFind(double j1, double j2, double j3, double j4, double j5, double j6);

public slots:

    void slotToFindCube(double j1, double j2, double j3, double j4, double j5,
        double j6);

protected:
    
    timur::CVS _cvs;

};

#endif // SENSOR_ADAPTER_H
