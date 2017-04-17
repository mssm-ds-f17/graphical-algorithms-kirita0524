#include "graphics.h"
#include "networkplugin.h"
#include <iostream>
#include <QThread>

using namespace std;
using namespace mssm;

class NetworkClientWrapper {
private:
    Graphics& g;

    int networkPluginId;
    int socketId;

    int port;
    std::string hostname;
public:
    NetworkClientWrapper(Graphics& g, int port, const std::string& hostname);

    bool handleEvent(const Event& evt);
    bool send(const std::string& data);
    bool isConnected() { return socketId; }

    int  pluginId() { return networkPluginId; }
};

NetworkClientWrapper::NetworkClientWrapper(Graphics& g, int port, const std::string& hostname)
  : g{g}, port{port}, hostname{hostname}
{
    networkPluginId = g.registerPlugin([](QObject* parent) { return new NetworkPlugin(parent); });
}

bool NetworkClientWrapper::handleEvent(const Event& e)
{
    switch (e.evtType) {
    case EvtType::PluginCreated:
        if (e.pluginId == networkPluginId) {
            g.callPlugin(networkPluginId, NetworkPlugin::CMD_CONNECT, port, hostname);
            return true;
        }
        break;
    case EvtType::PluginMessage:
        if (e.pluginId == networkPluginId && e.x == NetworkPlugin::MSG_STATUS) {
            if (e.y == NetworkPlugin::MSG_STATUS_CONNECTED) {  // connected
                g.out << "Connected to server" << endl;
                socketId = e.arg;
                return true;
            }
            else if (e.y == NetworkPlugin::MSG_STATUS_DISCONNECTED) {
                g.out << "Disconnected from server" << endl;
                socketId = 0;
                return true;
            }
        }
    }

    return false;
}

bool NetworkClientWrapper::send(const std::string& data)
{
    if (socketId) {
        g.callPlugin(networkPluginId, NetworkPlugin::CMD_SEND, socketId, data);
        return true;
    }
    return false;
}


void graphicsMain(Graphics& g)
{
    g.out << "Graphics Main" << QThread::currentThreadId() << endl;

    NetworkClientWrapper server(g, 1234, "localhost");

    while (g.draw())
    {
        g.rect(10,10,20,50);

        auto events = g.events();

        for (unsigned int i = 0; i < events.size(); ++i) {
            Event e = events[i];

            if (server.handleEvent(e)) {
                continue;
            }

            g.out << e << endl;

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
                g.out << e.x << endl;
                break;
            case EvtType::MouseRelease:
                break;
            case EvtType::PluginCreated:
                break;
            case EvtType::PluginMessage:
                if (e.pluginId == server.pluginId()) {
                    switch (e.x) {
                    case NetworkPlugin::MSG_DATA:
                        g.out << "MSG_DATA:   Client = " << e.arg << " Data = " << e.data << endl;
                        break;
                    }
                }
            }
        }
    }
}

int main()
{
    cout << "Main Thread" << QThread::currentThreadId() << endl;

    Graphics g("Graphics App", 300, 300, graphicsMain);
}
