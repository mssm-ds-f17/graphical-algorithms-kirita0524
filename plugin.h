#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <functional>

namespace mssm
{

class Graphics;

class Plugin : public QObject
{
    Q_OBJECT
public:
    explicit Plugin(QObject *parent);
   ~Plugin();
private:
    virtual bool shouldDelete() = 0;
    virtual void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) = 0;
signals:

public slots:

    friend class Graphics;
};

}
#endif // PLUGIN_H
