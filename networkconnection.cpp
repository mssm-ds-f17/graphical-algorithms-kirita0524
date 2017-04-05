#include "networkconnection.h"
#include "networkclient.h"
#include "graphics.h"

NetworkServer::NetworkServer(NetworkConnection& nc, QObject *parent) : QTcpServer(parent), conn(nc)
{
    qRegisterMetaType< QAbstractSocket::SocketState >();
    qRegisterMetaType< QAbstractSocket::SocketError >();
}

NetworkServer::~NetworkServer()
{

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
    qDebug() << "Bot Connecting";

    auto networkClient = new NetworkClient(socketDescriptor);

    conn.addClient(networkClient);
}

NetworkConnection::NetworkConnection(QObject *parent) :
    Plugin(parent), server(*this, this)
{
}

NetworkConnection::~NetworkConnection()
{

}

void NetworkConnection::addClient(NetworkClient* client)
{

}

bool NetworkConnection::shouldDelete()
{
    return false;
}

void NetworkConnection::update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent)
{

}


namespace mssm
{

void startListening(Graphics& g)
{
    g.registerObject([](QObject* parent)
    { return new NetworkConnection(parent); });
}

}
