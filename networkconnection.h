#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QTcpServer>
#include "plugin.h"
#include <vector>
#include <memory>

class NetworkConnection;
class NetworkClient;

class NetworkServer : public QTcpServer
{
    Q_OBJECT

    int nextClientId{1};

    std::vector<std::unique_ptr<NetworkClient>> clients;

    NetworkConnection& conn;

public:
    explicit NetworkServer(NetworkConnection& conn, QObject *parent = 0);
    ~NetworkServer();

    void startServer();
    void stopServer();

signals:

public slots:

protected:

    void incomingConnection(qintptr socketDescriptor);
public:
    void receiver(int connectionId, const std::string& data);

};

class NetworkConnection : public mssm::Plugin
{
    class NetworkData {
    public:
        int id;
        std::string data;
    };

    Q_OBJECT

    NetworkServer server;
    bool started{false};

    std::vector<NetworkData> receivedData;

public:
    explicit NetworkConnection(QObject *parent = 0);
    virtual ~NetworkConnection();

    void startServer();
    bool shouldDelete() override;
    void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) override;

    void receiver(int connectionId, const std::string& data);
};

#endif // NETWORKCONNECTION_H
