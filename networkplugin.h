#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H


#include "plugin.h"
#include <vector>
#include <memory>

class NetworkServer;
enum class NetworkSocketState;

class NetworkPlugin : public mssm::Plugin
{
    Q_OBJECT

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

    int serverPort{0};
    std::unique_ptr<NetworkServer> server;

    bool started{false};

    std::vector<NetworkData> receivedData;
    std::vector<SocketStateChange> stateChanges;

public:
    static constexpr int CMD_CONNECT = 1;  // arg2 = port arg3 = hostname
    static constexpr int CMD_SEND    = 2;  // arg2 = client id  arg3 = data

    static constexpr int MSG_DATA    = 1;  // data received.  arg = client id, y = reserved, data = data
    static constexpr int MSG_STATUS  = 2;  // socket status.  arg = client id, y = message#, data = msgString

    static constexpr int MSG_STATUS_CONNECTED    = 1;
    static constexpr int MSG_STATUS_DISCONNECTED = 0;

    explicit NetworkPlugin(QObject *parent, int serverPort = 0);
    virtual ~NetworkPlugin();

    void startServer();  // do not call from graphicsMain thread
    bool shouldDelete() override;
    void update(std::function<void(int, int, int, const std::string&)> sendEvent) override;
    void call(int arg1, int arg2, const std::string& arg3) override;


    void receiver(int connectionId, const std::string& data);

    void onSocketStateChange(int connectionId, NetworkSocketState state, const std::string& msg);
};

#endif // NETWORKCONNECTION_H
