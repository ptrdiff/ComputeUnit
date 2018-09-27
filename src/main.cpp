﻿#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "Executor/Executor.h"
#include "RCAConnector/RCAConnector.h"
#include "RobotConnector/RobotConnector.h"
#include "SensorAdapter/SensorAdapter.h"
#include "SensorAdapter/SensorConfig.h"
#include "MathClass/MathClass.h"

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

    QFile congFile("config.json");
    if (congFile.open(QIODevice::ReadOnly))
    {
        QString settings = congFile.readAll();
        congFile.close();
        QVariantMap config = QJsonDocument::fromJson(settings.toUtf8()).object().toVariantMap();
        QMap RCAConnectorConfig = config["RCAConnector"].toMap();
        QMap RobotConnectorConfig = config["RobotConnector"].toMap();
        QMap SensorAdapterConfig = config["SensorAdapter"].toMap();
        RCAConnector rcaConnector(RCAConnectorConfig["IPAdress"].toString().toStdString(),
                                  RCAConnectorConfig["Port"].toInt());
        RobotConnector robotConnector(RobotConnectorConfig["IPAdress"].toString().toStdString(),
                                      RobotConnectorConfig["Port"].toInt());
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
        MathModule mathModule;
        Executor executor(rcaConnector, robotConnector, sensorAdapter, mathModule);

        return a.exec();
    }
    qCritical() << QString("Can't open config file");
    return -1;
    
  }
  catch (std::exception &exp)
  {
    std::cout << exp.what() << '\n';
    throw exp;
  }

}