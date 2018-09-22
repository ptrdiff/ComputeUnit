#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDebug>

#include "Executor/Executor.h"
#include "RCAConnector/RCAConnector.h"
#include "RobotConnector/RobotConnector.h"
#include "SensorAdapter/SensorAdapter.h"
#include "SensorAdapter/SensorConfig.h"
#include "MathModule/MathModule.h"

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
  std::cout << QString("%1 %6: %2 (%3, %4:%5)\n").arg(
    now.toString(Qt::ISODateWithMs),
    localMsg.constData(),
    context.function,
    context.file,
    QString::number(context.line),
    msgType
  ).toStdString();
  out.close();
} // TODO add feature to change stream output and formatting output(table or something like this)

void skipComments(QTextStream& in)
{
  if(!in.atEnd())
  {
    in.skipWhiteSpace();
    QString token = in.read(1);
    while(token == "#")
    {
      in.readLine();
      if (!in.atEnd())
      {
        token = in.read(1);
      }
    }
    if(!in.atEnd())
    {
      in.seek(in.pos() - 1);
    }
  }
}

int main(int argc, char *argv[])
{
  try
  {
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);

    QFile congFile("config.txt");

    if (congFile.open(QIODevice::ReadOnly))
    {
      QTextStream in(&congFile);

      skipComments(in);
      QString RCAIpAdress;
      int RCAPort;
      in >> RCAIpAdress >> RCAPort;
      RCAConnector rcaConnector(RCAIpAdress.toStdString(), RCAPort);
      //RobotConnector robotConnector("172.27.221.60", 59002);
      
      skipComments(in);
      QString RobotIpAdress;
      int RobotPort;
      in >> RobotIpAdress >> RobotPort;
      RobotConnector robotConnector(RobotIpAdress.toStdString(), RobotPort);
      
      skipComments(in);
      int numberOfSensors;
      in >> numberOfSensors;
      std::vector<SensorConfig> sensorDescriprion;
      for(int i=0;i<numberOfSensors;++i)
      {
        skipComments(in);
        const QString programName = in.readLine();
        skipComments(in);
        const QString programDirectory = in.readLine();
        skipComments(in);
        int inputBlock, outputBlock;
        in >> inputBlock >> outputBlock;
        sensorDescriprion.emplace_back(SensorConfig(programName, programDirectory, inputBlock, 
          outputBlock));
      }
      SensorAdapter sensorAdapter(sensorDescriprion);
      MathModule mathModule;
      Executor executor(rcaConnector, robotConnector, sensorAdapter, mathModule);
    }
    else
    {
      qCritical() << QString("Can't open config file");
    }

    return a.exec();
  }
  catch (std::exception &exp)
  {
    std::cout << exp.what() << '\n';
    throw exp;
  }

}