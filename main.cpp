#include "graphics.h"
#include "networkplugin.h"
#include <iostream>
#include <QThread>

#include "filedownloader.h"

using namespace std;
using namespace mssm;

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
                    g.out << "Connected to: " + socketData << std::endl;
                    break;
                case NetworkSocketEvent::disconnected:
                    server.closePlugin();
                    break;
                case NetworkSocketEvent::error:
                    break;
                case NetworkSocketEvent::other:
                    break;
                case NetworkSocketEvent::data:
                    {
                        stringstream ss(socketData);
                        double x;
                        double y;
                        ss >> x;
                        ss >> y;
                        g.line(0,0,x,y);
                    }
                    break;
                }

                continue;
            }

            //g.out << e << endl;

            switch (e.evtType) {
            case EvtType::KeyPress:
                mssm::download(g, "ftp://ftp.swpc.noaa.gov/pub/weekly/Predict.txt");
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
                g.out << e << endl;
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
