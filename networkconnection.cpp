#include "networkconnection.h"
#include "networkclient.h"
#include "graphics.h"
#include <iostream>

NetworkServer::NetworkServer(NetworkConnection& nc, QObject *parent) : QTcpServer(parent), conn(nc)
{
    qRegisterMetaType< QAbstractSocket::SocketState >();
    qRegisterMetaType< QAbstractSocket::SocketError >();
}

NetworkServer::~NetworkServer()
{
    qDebug() << "NetworkServer destructor\n";
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

void NetworkServer::receiver(int connectionId, const std::string& data)
{
    conn.receiver(connectionId, data);
}

NetworkConnection::NetworkConnection(QObject *parent) :
    Plugin(parent), server(*this, this)
{
}

NetworkConnection::~NetworkConnection()
{
    qDebug() << "Closing\n";
    server.stopServer();
    qDebug() << "NetworkConnection destructor\n";
}

bool NetworkConnection::shouldDelete()
{
    return false;
}

void NetworkConnection::update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent)
{

    if (!started) {
        server.startServer();
        started = true;
    }


    for (auto& data : receivedData) {
        qDebug() << "Sending an event\n";
        sendEvent("TCP", data.id, 0, 0, data.data);
    }

    receivedData.clear();
}

void NetworkConnection::receiver(int connectionId, const std::string& data)
{
    qDebug() << connectionId << " Got some data: " << QString(data.c_str()) << "\n";
    receivedData.push_back({connectionId, data});
}

namespace mssm
{

void startListening(Graphics& g)
{
    g.registerObject([](QObject* parent)
    { return new NetworkConnection(parent); });
}

}
