#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <mutex>
#include <condition_variable>

class NetworkServer;

class NetworkClient : public QObject
{
    Q_OBJECT

    //QMutex       mutex;

    int            connectionId;
    NetworkServer *server;

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

    NetworkClient(int id, NetworkServer* server, qintptr socketId);
   ~NetworkClient();

    void queueToSend(const std::string& data);
    void sendQueued();
    int id() { return connectionId; }
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
