#include "networkplugin.h"
#include "networkclient.h"
#include "networkserver.h"
#include "graphics.h"
#include <iostream>



NetworkPlugin::NetworkPlugin(QObject *parent) :
    Plugin(parent)
{
    server.reset(new NetworkServer(*this, this));
    connect(this, SIGNAL(callMakeConnection(const std::string&, int)), this, SLOT(makeConnection(const std::string&, int)));
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

void NetworkPlugin::requestConnection(const std::string& host, int port)
{
    emit callMakeConnection(host, port);
}

void NetworkPlugin::call(int arg1, int arg2, const std::string& arg3)
{
    qDebug() << "Plugin Called on thread " << QThread::currentThreadId();

    // TODO error handling
    switch (arg1) {
    case 1: // send data
        server->queueToSend(arg2, arg3);
        break;
    case 2: // make socket connection  (arg2 is port arg3 is hostname)
        requestConnection(arg3, arg2);
        break;
    }
}

void NetworkPlugin::update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent)
{

    if (!started) {
        qDebug() << "NOT STARTING SERVER\n";
        //server->startServer();
        started = true;
    }

    server->sendAllQueued();

    for (const auto& data : receivedData) {
        qDebug() << "Sending an event\n";
        sendEvent("TCP", 0, 0, data.id, data.data);
    }

    for (const auto& change : stateChanges) {
        sendEvent("TCP_STATE", static_cast<int>(change.state), 0, change.id, change.msg);
    }

    stateChanges.clear();
    receivedData.clear();
}

void NetworkPlugin::makeConnection(const std::string& host, int port)
{
    qDebug() << "makeConnection is on thread: " << QThread::currentThreadId();
    server->connect(host, port);
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
