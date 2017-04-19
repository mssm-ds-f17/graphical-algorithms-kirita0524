#include "graphics.h"
#include "networkplugin.h"
#include <iostream>
#include <QThread>

using namespace std;
using namespace mssm;

class NetworkClientPlugin {
private:
    Graphics& g;

    int networkPluginId;
    int socketId;

    int port;
    std::string hostname;
public:
    NetworkClientPlugin(Graphics& g, int port, const std::string& hostname);

    bool handleEvent(const Event& evt, NetworkSocketEvent& socketEvent, string& data);
    bool send(const std::string& data);
    bool isConnected() { return socketId; }
    int  pluginId() { return networkPluginId; }
};

NetworkClientPlugin::NetworkClientPlugin(Graphics& g, int port, const std::string& hostname)
  : g{g}, port{port}, hostname{hostname}
{
    networkPluginId = g.registerPlugin([](QObject* parent) { return new NetworkPlugin(parent); });
}

bool NetworkClientPlugin::handleEvent(const Event& e, NetworkSocketEvent& socketEvent, string& data)
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
                g.out << "Connected to server" << endl;
                socketId = e.arg;
                socketEvent = NetworkSocketEvent::connected;
                data = e.data;
                return true;
            case NetworkSocketEvent::disconnected:
                g.out << "Disconnected from server" << endl;
                socketId = 0;
                socketEvent = NetworkSocketEvent::disconnected;
                data = e.data;
                return true;
            case NetworkSocketEvent::error:
                g.out << "Error: " << e.arg << " " << e.data << endl;
                socketEvent = NetworkSocketEvent::error;
                data = e.data;
                return true;
            case NetworkSocketEvent::other:
                g.out << "Other: " << e.arg << " " << e.data << endl;
                socketEvent = NetworkSocketEvent::other;
                data = e.data;
                return true;
            case NetworkSocketEvent::data:
                g.out << "Data: " << e.arg << " " << e.data << endl;
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




void graphicsMain(Graphics& g)
{
   // g.out << "Graphics Main" << QThread::currentThreadId() << endl;

    NetworkClientPlugin server(g, 1234, "localhost");

    while (g.draw())
    {
        g.clear();

        g.rect(10,10,20,50);
        g.rect(30,30,20,50);
        g.rect(15,60,20,50);

        auto events = g.events();

        for (unsigned int i = 0; i < events.size(); ++i) {
            Event e = events[i];

            NetworkSocketEvent socketEvent;
            string socketData;

            if (server.handleEvent(e, socketEvent, socketData)) {
                // got a network event of some sort
                switch (socketEvent)
                {
                case NetworkSocketEvent::connected:
                    break;
                case NetworkSocketEvent::disconnected:
                    break;
                case NetworkSocketEvent::error:
                    break;
                case NetworkSocketEvent::other:
                    break;
                case NetworkSocketEvent::data:
                    break;
                }

                continue;
            }

            //g.out << e << endl;

            switch (e.evtType) {
            case EvtType::KeyPress:
                break;
            case EvtType::KeyRelease:
                break;
            case EvtType::MouseMove:
                if (server.isConnected()) {
                    stringstream ss;
                    ss << e.x << " " << e.y << "\n";
                    server.send(ss.str());
                }
                break;
            case EvtType::MousePress:
                if (server.isConnected()) {
                    stringstream ss;
                    ss << e.x << " " << e.y << "\n";
                    server.send(ss.str());
                }
                g.line(0,0,e.x, e.y);
               // g.out << e.x << endl;
                break;
            case EvtType::MouseRelease:
                break;
            case EvtType::PluginCreated:
                break;
            case EvtType::PluginMessage:
                break;
            }
        }
    }
}

int main()
{
    cout << "Main Thread" << QThread::currentThreadId() << endl;

    Graphics g("Graphics App", 300, 300, graphicsMain);
}
