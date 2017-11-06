#include "graphics.h"
#include "networkplugin.h"
#include <iostream>
#include <QThread>

#include "filedownloader.h"

using namespace std;
using namespace mssm;

class Segment {
public:
    Vec2d p1;
    Vec2d p2;
    double distance;
};

Segment closestPointsBruteForce(std::vector<Vec2d>& points) {
    double minDistance = numeric_limits<double>::max();
    Segment best;
    for (size_t i = 0; i < points.size(); i++) {
        for (size_t j = i+1; j < points.size(); j++) {
            Vec2d p1 = points[i];
            Vec2d p2 = points[j];
            double dist = (p1-p2).magnitude();
            if (dist < minDistance) {
                minDistance = dist;
                best.p1 = p1;
                best.p2 = p2;
                best.distance = minDistance;
            }
        }
    }
    return best;
}

void genPoints(Graphics &g, vector<Vec2d>& vec, int numPoints) {

    double x = 0;
    double y = 0;
    Vec2d v;
    for (int i = 0; i < numPoints; i++) {
        x = g.randomDouble(0, g.width());
        y = g.randomDouble(0, g.height());
        v = {x, y};
        vec.push_back(v);
        g.point(x, y, WHITE);
    }
}

bool compareByX(Vec2d &a, Vec2d &b) {
    if (a.x < b.x) {
        return true;
    }
    else {
        return false;
    }
}

bool compareByY(Vec2d &a, Vec2d &b) {
    if (a.y < b.y) {
        return true;
    }
    else {
        return false;
    }
}

Segment findClosestPoints(vector<Vec2d>& points) {

    if (points.size() <= 3) {
        return closestPointsBruteForce(points);
    }

    sort(points.begin(), points.end(), compareByX);

    vector<Vec2d> a;
    vector<Vec2d> b;

    for (unsigned int i = 0; i < points.size(); i++) {
        if (i < points.size()/2) {
            a.push_back(points[i]);
        }
        else {
            b.push_back(points[i]);
        }
    }

    double divideLine = (a.back().x + b[0].x)/2;

    Segment segA = findClosestPoints(a);
    Segment segB = findClosestPoints(b);

    vector<Vec2d> strip;
    double minDist;

    Segment s;

    if (segA.distance < segB.distance) {
        minDist = segA.distance;
        s = segA;
    }
    else {
        minDist = segB.distance;
        s = segB;
    }

    double maxLine = divideLine + minDist;
    double minLine = divideLine - minDist;

    for (unsigned int i = 0; i < a.size(); i++) {
        if ((a[i].x < maxLine) && (a[i].x > minLine)) {
            strip.push_back(a[i]);
        }
    }
    for (unsigned int i = 0; i < b.size(); i++) {
        if ((b[i].x < maxLine) && (b[i].x > minLine)) {
            strip.push_back(b[i]);
        }
    }

    sort(strip.begin(), strip.end(), compareByY);

    for (unsigned int i = 0; i < strip.size(); i++) {
        for (unsigned int j = i+1; j < strip.size(); j++) {
            if (abs(strip[i].y - strip[j].y) > minDist) {
                break;
            }
            double dist = (strip[i]+strip[j]).magnitude();
            if (dist < s.distance) {
                s.distance = dist;
                s.p1 = strip[i];
                s.p2 = strip[j];
            }
        }
    }
    return s;
}

/*void mergeSort(vector<double>& values, function<bool(Vec2d &a, Vec2d &b)> comp) {
    if (values.size() == 1 || values.size() == 0) {
        return;
    }

    // split into two halves
    vector<double> a;
    vector<double> b;
    for (unsigned int i = 0; i < values.size(); i++) {
        if (i < values.size()/2) {
            a.push_back(values[i]);
        }
        else if (i < values.size()){
            b.push_back(values[i]);
        }
    }

    // sort each half
    mergeSort(a, comp);
    mergeSort(b,comp);

    // merge the two halves back into the original values vector
    vector<double> retVec;
    unsigned int ai = 0;
    unsigned int bi = 0;

    while(true) {
        if (comp(a[ai], b[bi])) {
            retVec.push_back(a[ai]);
            ai++;
        } else if (comp(b[bi], a[ai])) {
            retVec.push_back(b[bi]);
            bi++;
        } else {
            retVec.push_back(a[ai]);
            ai++;
        }

        if (ai == a.size()) {
            for (unsigned int i = bi; i < b.size(); i++) {
                retVec.push_back(b[i]);
            }
            break;
        } else if (bi == b.size()) {
            for (unsigned int i = ai; i < a.size(); i++) {
                retVec.push_back(a[i]);
            }
            break;
        }
    }
    values = retVec;
    return;
}*/

void printVector(Graphics &g, vector<Vec2d> vec) {
    g.out << "( ";
    for (int i = 0; i < vec.size(); i++) {
        g.out << "{" << vec[i].x << ", " << vec[i].y << "}";
    }
    g.out << ")" << endl;
}

void graphicsMain(Graphics& g)
{
    vector<Vec2d> v;

    Segment s;

    genPoints(g, v, 100);

    s = findClosestPoints(v);

    while (g.draw())
    {
        g.line(s.p1, s.p2, RED);

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
                break;
            case EvtType::MouseRelease:
                break;
            case EvtType::MouseWheel:
                break;
            case EvtType::PluginCreated:
                break;
            case EvtType::PluginClosed:
                break;
            case EvtType::PluginMessage:
                break;
            case EvtType::MusicEvent:
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
