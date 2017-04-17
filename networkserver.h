#ifndef NETWORKSEVER_H
#define NETWORKSEVER_H

#include <QTcpServer>
#include <memory>

class NetworkPlugin;
class NetworkClient;

enum class NetworkSocketState {
    waiting,
    connected,
    error,
    closed,
    other
};

class NetworkServer : public QTcpServer
{
    Q_OBJECT

    int nextClientId{1};

    std::vector<std::unique_ptr<NetworkClient>> clients;

    NetworkPlugin& conn;

public:
    explicit NetworkServer(NetworkPlugin& conn, QObject *parent = 0);
    ~NetworkServer();

    void startServer(int port);
    void stopServer();

signals:

public slots:

protected:

    void incomingConnection(qintptr socketDescriptor);
public:
    int  connect(const std::string& host, int port);
    void receiver(int connectionId, const std::string& data);
    bool queueToSend(int connectionId, const std::string& data);
    void sendAllQueued();
    void socketStateChange(int connectionId, NetworkSocketState state, const std::string& msg);
};

#endif // NETWORKSEVER_H
