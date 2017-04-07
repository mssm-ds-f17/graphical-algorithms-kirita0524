#include "networkplugin.h"
#include "networkclient.h"
#include "networkserver.h"
#include "graphics.h"
#include <iostream>



NetworkPlugin::NetworkPlugin(QObject *parent) :
    Plugin(parent)
{
    server.reset(new NetworkServer(*this, this));
}

NetworkPlugin::~NetworkPlugin()
{
    qDebug() << "Closing\n";
    server->stopServer();
    qDebug() << "NetworkConnection destructor\n";
}

bool NetworkPlugin::shouldDelete()
{
    return false;
}

void NetworkPlugin::call(int arg1, int arg2, const std::string& arg3)
{

    // TODO error handling
    switch (arg1) {
    case 1: // send data
        server->queueToSend(arg2, arg3);
        break;
    }
}

void NetworkPlugin::update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent)
{

    if (!started) {
        server->startServer();
        started = true;
    }

    server->sendAllQueued();

    for (auto& data : receivedData) {
        qDebug() << "Sending an event\n";
        sendEvent("TCP", 0, 0, data.id, data.data);
    }

    receivedData.clear();
}

void NetworkPlugin::receiver(int connectionId, const std::string& data)
{
    qDebug() << connectionId << " Got some data: " << QString(data.c_str()) << "\n";
    receivedData.push_back({connectionId, data});
}

namespace mssm
{

void startListening(Graphics& g)
{
    g.registerPlugin([](QObject* parent)
    { return new NetworkPlugin(parent); });
}

}
