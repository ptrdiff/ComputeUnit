#include "RCAConnector.h"

#include <chrono>

RCAConnector::RCAConnector(std::string serverIP, int port, std::string welcomeCommand,
    QObject *parent) :
    QObject(parent),
    _serverIP(std::move(serverIP)),
    _port(static_cast<quint16>(port)),
    _welcomeCommand(welcomeCommand),
    _socket(std::make_unique<QTcpSocket>(this))
{
    qInfo() << QString("Create with parameters: IP: %1, Port: %2").arg(QString::fromStdString(_serverIP),
        QString::number(_port));

    connect(_socket.get(), &QTcpSocket::disconnected, this, &RCAConnector::slotToDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead, this, &RCAConnector::slotToReadyRead);

    qDebug() << QString("Completed the creation.");
}

bool RCAConnector::isConnected() const
{
    return _socket->state() == QTcpSocket::SocketState::ConnectedState;
}

ExectorCommand transformCommand(const QString& token)
{
    if (token == "m")
    {
        return ExectorCommand::SEND_TO_ROBOT;
    }
    if (token == "f")
    {
        return ExectorCommand::SEND_TO_SENSOR;
    }
    if (token == "e")
    {
        return ExectorCommand::SHUT_DOWN;
    }
    return ExectorCommand::INVALID;
}

void RCAConnector::slotToReadyRead()
{
    qInfo() << QString("Start reading from server.");
    const auto startChrono = std::chrono::steady_clock::now();

    QTextStream locData(_socket.get());

    bool isDouble;
    double coord;

    QString token;
    do {
        locData >> token;
        coord = token.toDouble(&isDouble);
        locData.skipWhiteSpace();
    } while (isDouble);

    QVector<double> coords;
    while (!locData.atEnd())
    {
        QString chunk;
        locData >> chunk;
        coord = chunk.toDouble(&isDouble);
        if(isDouble)
        {
            coords.push_back(coord);
        }
        else
        {
            emit signalNextCommand(transformCommand(token), coords);
            coords.clear();
            token = chunk;
        }
        locData.skipWhiteSpace();
    }

    emit signalNextCommand(transformCommand(token), coords);

    const auto endChrono = std::chrono::steady_clock::now();
    const auto durationChrono =
        std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Complete reading from server: %1 ms").arg(durationChrono / 1000.0);
}

void RCAConnector::slotToDisconnected()
{
    qInfo() << QString("Start disconnection.");
    emit signalNextCommand(ExectorCommand::SHUT_DOWN, QVector<double>());
    _socket->close();
    qDebug() << QString("Complete disconnection.");
}

void RCAConnector::slotToConnect()
{
    qInfo() << QString("Start connection.");
    const auto startChrono = std::chrono::steady_clock::now();

    _socket->connectToHost(_serverIP.c_str(), _port);

    if (!_socket->waitForConnected(30000))
    {
        qCritical() << QString("RCAConnector Error: %1").arg(_socket->errorString());
        emit signalSocketError();
    }
    else
    {
        _socket->write(_welcomeCommand.c_str());
        const auto endChrono = std::chrono::steady_clock::now();
        const auto durationChrono =
            std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
        qDebug() << QString("Complete connection: %1 ms").arg(durationChrono / 1000.0);
    }
}

void RCAConnector::slotWriteToServer(QVector<double> data)
{
    QString dataString;
    for (auto &i : data)
    {
        dataString.push_back(QString("%1 ").arg(i));
    }
    qInfo() << QString("Start writing to server. Data: %1").arg(dataString);
    const auto startChrono = std::chrono::steady_clock::now();

    QTextStream dataStream(_socket.get());
    dataStream << R"("cube" : ")";
    for (auto &i : data)
    {
        dataStream << i << ' ';
    }
    dataStream << "\"|";
    dataStream.flush();

    const auto endChrono = std::chrono::steady_clock::now();
    const auto durationChrono =
        std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
    qDebug() << QString("Complete writing to server: %1 ms").arg(durationChrono / 1000.0);
}
