#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H


#include "plugin.h"
#include <vector>
#include <memory>
#include "networkserver.h"


class NetworkPlugin : public mssm::Plugin
{
    Q_OBJECT

    class NetworkEvent {
    public:
        int id;
        NetworkSocketEvent state;
        std::string data;
    };

    int serverPort{0};
    std::unique_ptr<NetworkServer> server;

    bool started{false};

    std::vector<NetworkEvent> networkEvents;

public:
    static constexpr int CMD_CONNECT = 1;  // arg2 = port arg3 = hostname
    static constexpr int CMD_SEND    = 2;  // arg2 = client id  arg3 = data

    explicit NetworkPlugin(QObject *parent, int serverPort = 0);
    virtual ~NetworkPlugin();

    void startServer();  // do not call from graphicsMain thread
    bool shouldDelete() override;
    void update(std::function<void(int, int, int, const std::string&)> sendEvent) override;
    void call(int arg1, int arg2, const std::string& arg3) override;


    void receiver(int connectionId, const std::string& data);

    void onSocketStateChange(int connectionId, NetworkSocketEvent state, const std::string& msg);
};

#endif // NETWORKCONNECTION_H
