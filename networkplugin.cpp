#include "networkplugin.h"
#include "networkclient.h"
#include "networkserver.h"
#include "graphics.h"
#include <iostream>

NetworkPlugin::NetworkPlugin(QObject *parent, int port) :
    Plugin(parent), serverPort{port}
{
    server.reset(new NetworkServer(*this, this));
}

NetworkPlugin::~NetworkPlugin()
{
    //qDebug() << "Closing\n";
    server->stopServer();
    //qDebug() << "NetworkConnection destructor\n";
}

bool NetworkPlugin::shouldDelete()
{
    return closed;
}

void NetworkPlugin::call(int arg1, int arg2, const std::string& arg3)
{
    //qDebug() << "Plugin Called on thread " << QThread::currentThreadId();

    // TODO error handling
    switch (arg1) {
    case NetworkPlugin::CMD_SEND:    // send data
        server->queueToSend(arg2, arg3);
        break;
    case NetworkPlugin::CMD_CONNECT: // connect to host
        server->connect(arg3, arg2);
        break;
    case NetworkPlugin::CMD_CLOSE_PLUGIN:
        closePlugin();
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

    for (const auto& ne : networkEvents) {
        //qDebug() << "Sending an event\n";
        sendEvent(static_cast<int>(ne.state), 0, ne.id, ne.data);
    }

    networkEvents.clear();
}

void NetworkPlugin::onSocketStateChange(int connectionId, NetworkSocketEvent state, const std::string& msg)
{
    //qDebug() << connectionId << " Got state change " << static_cast<int>(state) << " " << msg.c_str() << " " << QThread::currentThreadId();
    networkEvents.push_back({connectionId, state, msg});
}


void NetworkPlugin::receiver(int connectionId, const std::string& data)
{
    //qDebug() << connectionId << " Got some data: " << QString(data.c_str()) << " " << QThread::currentThreadId();
    networkEvents.push_back({connectionId, NetworkSocketEvent::data, data});
}


NetworkClientPlugin::NetworkClientPlugin(mssm::Graphics& g, int port, const std::string& hostname)
  : g{g}, port{port}, hostname{hostname}
{
    networkPluginId = g.registerPlugin([](QObject* parent) { return new NetworkPlugin(parent); });
}

void NetworkClientPlugin::closePlugin()
{
    g.callPlugin(networkPluginId, NetworkPlugin::CMD_CLOSE_PLUGIN, 0, "");
}

bool NetworkClientPlugin::handleEvent(const mssm::Event& e, NetworkSocketEvent& socketEvent, std::string& data)
{
    switch (e.evtType) {
    case EvtType::PluginCreated:
        if (e.pluginId == networkPluginId) {
            g.callPlugin(networkPluginId, NetworkPlugin::CMD_CONNECT, port, hostname);
            socketEvent = NetworkSocketEvent::other;
            data = "Plugin Registered";
            return true;
        }
        break;
    case EvtType::PluginMessage:
        if (e.pluginId == networkPluginId)
        {
            switch (static_cast<NetworkSocketEvent>(e.x))
            {
            case NetworkSocketEvent::connected:
                //g.out << "Connected to server" << std::endl;
                socketId = e.arg;
                socketEvent = NetworkSocketEvent::connected;
                data = e.data;
                return true;
            case NetworkSocketEvent::disconnected:
                //g.out << "Disconnected from server" << std::endl;
                socketId = 0;
                socketEvent = NetworkSocketEvent::disconnected;
                data = e.data;
                return true;
            case NetworkSocketEvent::error:
                //g.out << "Error: " << e.arg << " " << e.data << std::endl;
                socketEvent = NetworkSocketEvent::error;
                data = e.data;
                return true;
            case NetworkSocketEvent::other:
                //g.out << "Other: " << e.arg << " " << e.data << std::endl;
                socketEvent = NetworkSocketEvent::other;
                data = e.data;
                return true;
            case NetworkSocketEvent::data:
                //g.out << "Data: " << e.arg << " " << e.data << std::endl;
                socketEvent = NetworkSocketEvent::data;
                data = e.data;
                return true;
            }
        }
        break;
    default:
        break;
    }
    return false;
}

bool NetworkClientPlugin::send(const std::string& data)
{
    if (socketId) {
        g.callPlugin(networkPluginId, NetworkPlugin::CMD_SEND, socketId, data);
        return true;
    }
    return false;
}

NetworkServerPlugin::NetworkServerPlugin(mssm::Graphics& g, int port)
  : g{g}, port{port}
{
    networkPluginId = g.registerPlugin([port](QObject* parent) { return new NetworkPlugin(parent, port); });
}

void NetworkServerPlugin::closePlugin()
{
    g.callPlugin(networkPluginId, NetworkPlugin::CMD_CLOSE_PLUGIN, 0, "");
}

bool NetworkServerPlugin::handleEvent(const mssm::Event& e, NetworkSocketEvent& netEventType, int& clientId, std::string& data)
{
    switch (e.evtType) {
    case EvtType::PluginCreated:
        if (e.pluginId == networkPluginId) {
            netEventType = NetworkSocketEvent::other;
            clientId = 0;
            data.clear();
            return true;
        }
        break;
    case EvtType::PluginMessage:
        if (e.pluginId == networkPluginId) {
            switch (static_cast<NetworkSocketEvent>(e.x))
            {
            case NetworkSocketEvent::connected:
                netEventType = NetworkSocketEvent::connected;
                clientId = e.arg;
                data.clear();
                return true;
            case NetworkSocketEvent::disconnected:
                netEventType = NetworkSocketEvent::disconnected;
                clientId = e.arg;
                data.clear();
                return true;
            case NetworkSocketEvent::error:
                netEventType = NetworkSocketEvent::other;
                clientId = e.arg;
                data = e.data;
                return true;
            case NetworkSocketEvent::other:
                netEventType = NetworkSocketEvent::other;
                clientId = e.arg;
                data = e.data;
                return true;
            case NetworkSocketEvent::data:
                //g.out << "Data: " << e.arg << " " << e.data << endl;
                netEventType = NetworkSocketEvent::data;
                clientId = e.arg;
                data = e.data;
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

void NetworkServerPlugin::send(int socketId, const std::string& data)
{
    g.callPlugin(networkPluginId, NetworkPlugin::CMD_SEND, socketId, data);
}


