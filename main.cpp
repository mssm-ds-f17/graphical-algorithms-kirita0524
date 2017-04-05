#include "graphics.h"

using namespace std;
using namespace mssm;

//g.registerObject(
//            [url](QObject* parent) { return new FileDownloader(parent, url); });

void graphicsMain(Graphics& g)
{
    while (g.draw())
    {
        auto events = g.events();

        for (unsigned int i = 0; i < events.size(); ++i) {
            auto e = events[i];
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
            }
        }
    }
}

int main()
{
     Graphics g("Graphics App", 300, 300, graphicsMain);
}
