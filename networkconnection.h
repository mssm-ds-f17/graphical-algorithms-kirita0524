#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QTcpServer>
#include "plugin.h"

class NetworkConnection;
class NetworkClient;

class NetworkServer : public QTcpServer
{
    Q_OBJECT

    NetworkConnection& conn;

public:
    explicit NetworkServer(NetworkConnection& conn, QObject *parent = 0);
    ~NetworkServer();

    void startServer();

signals:

public slots:

protected:

    void incomingConnection(qintptr socketDescriptor);

};

class NetworkConnection : public mssm::Plugin
{
    Q_OBJECT

    NetworkServer server;

public:
    explicit NetworkConnection(QObject *parent = 0);
    virtual ~NetworkConnection();

    void addClient(NetworkClient* client);
    void startServer();
    bool shouldDelete() override;
    void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) override;
};

#endif // NETWORKCONNECTION_H
