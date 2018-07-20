#include <QtCore/QCoreApplication>
#include <QDateTime>
#include "Executor/Executor.h"

#include <iostream>

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
  std::cerr << QString("%1 %6: %2 (%3, %4:%5)\n").arg(
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

    RCAConnector rcaConnector("localhost", 9099);
    RobotConnector robotConnector("localhost", 59002);
    //Executor executor("172.27.221.60", 59002, 9090);
    Executor executor(rcaConnector,robotConnector);

    return a.exec();
  }
  catch (std::exception &exp)
  {
    std::cout << exp.what() << '\n';
    throw exp;
  }

}