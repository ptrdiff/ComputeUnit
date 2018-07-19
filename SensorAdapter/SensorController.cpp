#include "SensorController.h"

SensorController::SensorController(QString sensorProgramName, QObject* parent):
QObject(parent),
_programName(std::move(sensorProgramName))
{
    this->moveToThread(&_thread);

    _multiThreadingWorker.moveToThread(&_thread);

    connect(this, &SensorController::signalToComputeInAnoutherThread, &_multiThreadingWorker,
        &MultiThreadingWorker::slotToDoSomething);

    _thread.start();

    emit signalToComputeInAnoutherThread([this]()
    {
        _sensorProcess = std::make_unique<QProcess>();

        _sensorProcess->setProgram(_programName);

        if (!_directoryForProcess.isEmpty())
        {
            _sensorProcess->setWorkingDirectory(_directoryForProcess);
        }

        //todo add connections
    });
}

SensorController::~SensorController()
{
    emit signalToComputeInAnoutherThread([this]()
    {
        this->initialiseProcess();
    });
}

void SensorController::initialiseProcess()
{
    
}

void SensorController::deinitialiseProcess()
{
    if (_sensorProcess != nullptr)
    {
        _sensorProcess->kill();
    }

    _sensorProcess = nullptr;

    _thread.quit();
}
