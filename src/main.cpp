#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCommandLineParser>

#include "Executor/Executor.h"
#include "RCAConnector/RCAConnector.h"
#include "RobotConnector/RobotConnector.h"
#include "SensorAdapter/SensorAdapter.h"
#include "SensorAdapter/SensorConfig.h"
#include "MathClass/MathClass.h"
#include "ComputerVisionSystem/CVS.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();
  QString msgType;
  switch (type)
  {
  case QtDebugMsg: msgType = "Debug";
    break;
  case QtInfoMsg: msgType = "Info";
    break;
  case QtWarningMsg: msgType = "Warning";
    break;
  case QtCriticalMsg: msgType = "Critical";
    break;
  case QtFatalMsg: msgType = "Fatal";
    break;
  }
  QDateTime now = QDateTime::currentDateTime();
  now.setOffsetFromUtc(now.offsetFromUtc());
  std::ofstream out("log.txt", std::ios_base::app);
  out << QString("%1 %6: %2 (%3, %4:%5)\n").arg(
    now.toString(Qt::ISODateWithMs),
    localMsg.constData(),
    context.function,
    context.file,
    QString::number(context.line),
    msgType
  ).toStdString();
  out.close();
  std::cout << QString("%1 %6: %2 (%3, %4:%5)\n").arg(
    now.toString(Qt::ISODateWithMs),
    localMsg.constData(),
    context.function,
    context.file,
    QString::number(context.line),
    msgType
  ).toStdString();
} // TODO add feature to change stream output and formatting output(table or something like this)

int main(int argc, char *argv[])
{
  try
  {
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("ControllUnit");
    QCoreApplication::setApplicationVersion("0.4");

    QCommandLineParser parser;
    parser.setApplicationDescription("Controll unit for managing robot and sensors connected to \
this robot");
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption targetDirectoryOption(QStringList() << "c" << "config-file",
        QCoreApplication::translate("main", "Path to config file."),
        QCoreApplication::translate("main", "file path"));
    parser.addOption(targetDirectoryOption);


    if (!parser.parse(QCoreApplication::arguments())) {
        qCritical() << "can't parse input parametrs";
        return -2;
    }

    if (parser.isSet(helpOption))
    {
        parser.showHelp();
    }

    if (parser.isSet(versionOption))
    {
        std::cout << "Application name: " << QCoreApplication::applicationName().toStdString() <<
            "\nversion: " << QCoreApplication::applicationVersion().toStdString() << '\n';
        return 0;
    }

    QString configFileName = "config.json";

    if (parser.isSet(targetDirectoryOption))
    {
        configFileName = parser.value(targetDirectoryOption);
    }

    QFile congFile(configFileName);
    if (congFile.open(QIODevice::ReadOnly))
    {
        QString settings = congFile.readAll();
        congFile.close();
        QVariantMap config = QJsonDocument::fromJson(settings.toUtf8()).object().toVariantMap();
        QMap RCAConnectorConfig = config["RCAConnector"].toMap();
        QMap RobotConnectorConfig = config["RobotConnector"].toMap();
        QMap SensorAdapterConfig = config["SensorAdapter"].toMap();
        QMap ComputerVisionSystem = config["ComputerVisionSystem"].toMap();
        QMap MathModuleConfig = config["MathModule"].toMap();
        RCAConnector rcaConnector(RCAConnectorConfig["IPAdress"].toString().toStdString(),
                                  RCAConnectorConfig["Port"].toInt(), 
                                  RCAConnectorConfig["WelcomeCommand"].toString().toStdString());
        RobotConnector robotConnector(RobotConnectorConfig["IPAdress"].toString().toStdString(),
                                      RobotConnectorConfig["Port"].toInt(),
                                      RobotConnectorConfig["WelcomeCommand"].toString().toStdString(),
                                      RobotConnectorConfig["DataBlock"].toInt());
        std::vector<SensorConfig> sensorDescriprion;
        for (int i = 0; i < SensorAdapterConfig["SensorCount"].toInt(); ++i)
        {
            sensorDescriprion.emplace_back(SensorConfig(
                    SensorAdapterConfig["ProgramPath"].toString(),
                    SensorAdapterConfig["ProgramFolder"].toString(),
                    SensorAdapterConfig["Blocks"].toList()[0].toInt(),
                    SensorAdapterConfig["Blocks"].toList()[1].toInt()));
        }
        SensorAdapter sensorAdapter(sensorDescriprion);
        timur::CVS cvs(ComputerVisionSystem["ArucoSqureDimension"].toFloat(),
            ComputerVisionSystem["CountOfMarkers"].toInt(),
            ComputerVisionSystem["MarkerSize"].toInt(),
            ComputerVisionSystem["CameraIndex"].toInt(),
            ComputerVisionSystem["CalibrationFileName"].toString().toStdString());
        if(!cvs.isCameraOpened())
        {
            qCritical() << QString("Can't access camera");
        }
        MathModule mathModule(MathModuleConfig["IsCard"].toInt());
        Executor executor(rcaConnector, robotConnector, sensorAdapter, cvs, mathModule);

        return a.exec();
    }
    qCritical() << QString("Can't open config file: %1").arg(configFileName);
    return -1;
    
  }
  catch (std::exception& exp)
  {
    std::cout << exp.what() << '\n';
    //throw exp;
  }

}