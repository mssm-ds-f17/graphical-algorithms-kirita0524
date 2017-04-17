#include "networkplugin.h"
#include "networkclient.h"
#include "networkserver.h"
#include "graphics.h"
#include <iostream>



NetworkPlugin::NetworkPlugin(QObject *parent, int port) :
    Plugin(parent), serverPort{port}
{
    server.reset(new NetworkServer(*this, this));
    connect(this, SIGNAL(callMakeConnection(const std::string&, int)), this, SLOT(makeConnection(const std::string&, int)), Qt::QueuedConnection);
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
    qDebug() << "Plugin Called on thread " << QThread::currentThreadId();

    // TODO error handling
    switch (arg1) {
    case NetworkPlugin::CMD_SEND:    // send data
        server->queueToSend(arg2, arg3);
        break;
    case NetworkPlugin::CMD_CONNECT: // connect to host
        server->connect(arg3, arg2);
        break;
    default:
        // TODO  Send message/event back
        break;
    }
}

void NetworkPlugin::update(std::function<void(int, int, int, const std::string&)> sendEvent)
{
    // sendEvent params: x, y, arg, data
    // arg should be client id
    // x is command
    // y is extra data

    if (!started && serverPort) {
        server->startServer(serverPort);
        started = true;
    }

    server->sendAllQueued();

    for (const auto& data : receivedData) {
        qDebug() << "Sending an event\n";
        sendEvent(NetworkPlugin::MSG_DATA, 0, data.id, data.data);
    }

    for (const auto& change : stateChanges) {
        sendEvent(NetworkPlugin::MSG_STATUS, static_cast<int>(change.state), change.id, change.msg);
    }

    stateChanges.clear();
    receivedData.clear();
}

void NetworkPlugin::onSocketStateChange(int connectionId, NetworkSocketState state, const std::string& msg)
{
    qDebug() << connectionId << " Got state change " << static_cast<int>(state) << " " << msg.c_str() << " " << QThread::currentThreadId();
    stateChanges.push_back({connectionId, state, msg});
}


void NetworkPlugin::receiver(int connectionId, const std::string& data)
{
    qDebug() << connectionId << " Got some data: " << QString(data.c_str()) << " " << QThread::currentThreadId();
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
