#include "networkserver.h"
#include "networkclient.h"
#include "networkplugin.h"

#include <QThread>

NetworkServer::NetworkServer(NetworkPlugin& nc, QObject *parent) : QTcpServer(parent), conn(nc)
{
    qRegisterMetaType< QAbstractSocket::SocketState >();
    qRegisterMetaType< QAbstractSocket::SocketError >();
}

NetworkServer::~NetworkServer()
{
    qDebug() << "NetworkServer destructor\n";
}

void NetworkServer::socketStateChange(int connectionId, NetworkSocketState state, const std::string& msg)
{
    conn.onSocketStateChange(connectionId, state, msg);
}

void NetworkServer::stopServer()
{
    clients.clear();
    close();
}

void NetworkServer::startServer()
{
    int port = 1234;
    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening to port " << port << "...";
    }
}

// This function is called by QTcpServer when a new connection is available.

void NetworkServer::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection
    qDebug() << "Socket connecting on thread: " << QThread::currentThreadId();

    clients.emplace_back(new NetworkClient(nextClientId++, this, socketDescriptor));
}

int NetworkServer::connect(const std::string& host, int port)
{
    // We have a new connection
    qDebug() << "Creating socket connection on thread: " << QThread::currentThreadId();

    clients.emplace_back(new NetworkClient(nextClientId, this, host, port));

    return nextClientId++;
}


void NetworkServer::receiver(int connectionId, const std::string& data)
{
    conn.receiver(connectionId, data);
}

bool NetworkServer::queueToSend(int connectionId, const std::string& data)
{
    for (auto& client : clients) {
        if (client->id() == connectionId) {
            client->queueToSend(data);
            return true;
        }
    }
    return false;
}

void NetworkServer::sendAllQueued()
{
    for (auto& client : clients) {
        client->sendQueued();
    }
}
