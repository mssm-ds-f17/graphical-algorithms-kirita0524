#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H


#include "plugin.h"
#include <vector>
#include <memory>

class NetworkServer;
enum class NetworkSocketState;

class NetworkPlugin : public mssm::Plugin
{
    class NetworkData {
    public:
        int id;
        std::string data;
    };

    class SocketStateChange {
    public:
        int id;
        NetworkSocketState state;
        std::string msg;
    };

    Q_OBJECT

    int serverPort{0};
    std::unique_ptr<NetworkServer> server;

    bool started{false};

    std::vector<NetworkData> receivedData;
    std::vector<SocketStateChange> stateChanges;

public:
    explicit NetworkPlugin(QObject *parent, int serverPort = 0);
    virtual ~NetworkPlugin();

    void startServer();  // do not call from graphicsMain thread
    bool shouldDelete() override;
    void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) override;
    void call(int arg1, int arg2, const std::string& arg3) override;


    void receiver(int connectionId, const std::string& data);

    void onSocketStateChange(int connectionId, NetworkSocketState state, const std::string& msg);

    void connectSocket(const std::string& host, int port);

public slots:
    void makeConnection(const std::string& host, int port);

signals:
    void callMakeConnection(const std::string& host, int port);
};

#endif // NETWORKCONNECTION_H
