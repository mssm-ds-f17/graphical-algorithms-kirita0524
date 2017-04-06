#include "graphics.h"
#include "networkconnection.h"
#include <iostream>
#include <QThread>

using namespace std;
using namespace mssm;

//g.registerObject(
//            [url](QObject* parent) { return new FileDownloader(parent, url); });

void graphicsMain(Graphics& g)
{
    cout << "Graphics Main" << QThread::currentThreadId() << endl;

    g.registerObject([](QObject* parent)
    { return new NetworkConnection(parent); });

    while (g.draw())
    {
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
                g.out << e.data << endl;
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
