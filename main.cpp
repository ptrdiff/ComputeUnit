#include <QtCore/QCoreApplication>
#include "FanucAdapter/FanucAdapter.h"
#include "RCAConnector/RCAConnector.h"
#include "Executor/Executor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FanucAdapter fanucAdapter("127.0.0.1",59002);

    RCAConnector rcaConnector(9090);

    Executor executor;

    QObject::connect(&fanucAdapter, &FanucAdapter::signalToSendCurrentPosition, &executor, 
        &Executor::slotNewRobotPostion);

    QObject::connect(&executor, &Executor::signalToSendNewPointToRobot, &fanucAdapter, 
        &FanucAdapter::slotSendNextPosition);

    QObject::connect(&executor, &Executor::signalToSendCubePostion, &rcaConnector,
        &RCAConnector::slotToSendCubePosition);

    QObject::connect(&executor, &Executor::signalToSendCurrentPositionToClient, &rcaConnector,
        &RCAConnector::slotToSendCurrentRobotPostion);
    
    QObject::connect(&rcaConnector, &RCAConnector::signalToSearchCube, &executor,
        &Executor::slotFoundCubeTask);

    QObject::connect(&rcaConnector, &RCAConnector::signalToMoveRobot, &executor,
        &Executor::slotMoveRobot);

    rcaConnector.launch();

    fanucAdapter.startConnections();

    return a.exec();
}
