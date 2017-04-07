#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H


#include "plugin.h"
#include <vector>
#include <memory>

class NetworkServer;

class NetworkPlugin : public mssm::Plugin
{
    class NetworkData {
    public:
        int id;
        std::string data;
    };

    Q_OBJECT

    std::unique_ptr<NetworkServer> server;

    bool started{false};

    std::vector<NetworkData> receivedData;

public:
    explicit NetworkPlugin(QObject *parent = 0);
    virtual ~NetworkPlugin();

    void startServer();
    bool shouldDelete() override;
    void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) override;
    void call(int arg1, int arg2, const std::string& arg3) override;

    void receiver(int connectionId, const std::string& data);
};

#endif // NETWORKCONNECTION_H
