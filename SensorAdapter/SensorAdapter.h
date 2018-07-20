#ifndef SENSOR_ADAPTER_H
#define SENSOR_ADAPTER_H

#include <memory>
#include <vector>

#include <QObject>

#include "../ComputerVisionSystem/CVS.h"
#include "SensorController.h"

class SensorAdapter : public QObject
{
    Q_OBJECT
public:

    SensorAdapter(const std::vector<std::tuple<QString, int, int, QString>>& sensorsDescription,
        QObject *parent = nullptr);

    bool isOpen(size_t id);

    void sendCurPosition(int id, QVector<double>);

protected:
    
    std::vector<SensorController> _sensorsProcessControllers;

signals:

    void signalSendPosition(QVector<double>);

    void signalGenerateCommand(const QString& id, QVector<double> params);
    
protected slots:

    void slotToGetNewParametrs(int id, QVector<double> data);
};

//todo rewite initialise with using config

#endif // SENSOR_ADAPTER_H
