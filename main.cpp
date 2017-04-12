#include "graphics.h"
#include "networkplugin.h"
#include <iostream>
#include <QThread>

using namespace std;
using namespace mssm;

//g.registerObject(
//            [url](QObject* parent) { return new FileDownloader(parent, url); });

void graphicsMain(Graphics& g)
{
    cout << "Graphics Main" << QThread::currentThreadId() << endl;

    int networkServerId = g.registerPlugin([](QObject* parent) { return new NetworkPlugin(parent); });

    g.draw(); // make sure plugin is actually registered

    auto network = static_pointer_cast<NetworkPlugin>(g.getPlugin(networkServerId));

    network->connectSocket("localhost", 1234);

    while (g.draw())
    {
        g.rect(10,10,20,50);

        auto events = g.events();

        for (unsigned int i = 0; i < events.size(); ++i) {
            Event e = events[i];
            switch (e.evtType) {
            case EvtType::KeyPress:
                break;
            case EvtType::KeyRelease:
                break;
            case EvtType::MouseMove:
                break;
            case EvtType::MousePress:
                g.line(0,0,e.x, e.y);
                g.out << e.x << endl;
                break;
            case EvtType::MouseRelease:
                break;
            case EvtType::Message:
                g.out << "Got some data: " << e.data << endl;
              //  network->send(e.arg, "Test");
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
