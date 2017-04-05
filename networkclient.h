#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <mutex>
#include <condition_variable>

class NetworkClient: public QObject
{
    Q_OBJECT

    //QMutex       mutex;

    std::mutex  commLock;
    std::mutex  botLock;
    std::condition_variable botCv;
    bool        gotResponse;
    std::string response;

    QTcpSocket *socket;
    qintptr     socketDescriptor;
    QByteArray  incomingData;
    QByteArray  outgoingData;

     bool         wasDisconnected;

public:

    NetworkClient(qintptr socketId);
   ~NetworkClient();

    void commUpdate();
    //void handleEvents(std::vector<BotEvent> events, double currentTime);

    bool asyncWait();

private:
    void sendData(QByteArray data);
    bool handleCommand();
    bool handleCommand(std::string cmd);

public slots:

    void readyRead();
    void disconnected();
    void socketStateChanged(QAbstractSocket::SocketState);
    void socketError(QAbstractSocket::SocketError);
public:
    void setDisconnected();
    bool isDisconnected();
};

#endif // NETWORKCLIENT_H
