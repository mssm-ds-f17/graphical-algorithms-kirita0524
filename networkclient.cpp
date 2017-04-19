#include "networkclient.h"
#include <sstream>
#include <qbytearray.h>
#include "networkserver.h"

using namespace std;

QByteArray clean(QByteArray data)
{
    for (int i=0;i<data.size();i++)
    {
        switch (data[i])
        {
        case '\n':
            data[i] = '\\';
            break;
        case '\r':
            data[i] = '/';
            break;
        }
    }

    return data;
}

NetworkClient::NetworkClient(int clientId, NetworkServer* server, qintptr socketId)
    : QObject(), server{server}
{
    connectionId = clientId;

    //qDebug() << "Constructing NetworkClient on Thread: " << QThread::currentThreadId() << endl;
    auto s = new QTcpSocket();

    if(!s->setSocketDescriptor(socketId))
    {
        qDebug() << "Error setting socket descriptor";
        return;
    }

    setSocket(s);
}

NetworkClient::NetworkClient(int clientId, NetworkServer* server, const std::string& host, int port)
    : QObject(), server{server}
{
    connectionId = clientId;

    //qDebug() << "Creating socket to port: " << port << " on thread: " << QThread::currentThreadId();

    auto s = new QTcpSocket();

    setSocket(s);

    s->connectToHost(host.c_str(), port);
}

void NetworkClient::setSocket(QTcpSocket *socket)
{
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)), Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::QueuedConnection);

    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
}

NetworkClient::~NetworkClient()
{
    //qDebug() << "NetworkClient destructing\n";
    closeSocket();
}

void NetworkClient::closeSocket()
{
    //qDebug() << "closeSocket on thread: " << QThread::currentThreadId() << endl;
    if (socket)
    {
        auto tmp = socket;
        socket = nullptr;
        tmp->disconnectFromHost();
        tmp->close();
        tmp->deleteLater();
    }
}


void NetworkClient::socketStateChanged(QAbstractSocket::SocketState ss)
{
    string state = "Unknown";
    NetworkSocketEvent ns = NetworkSocketEvent::other;

    switch (ss) {
    case QAbstractSocket::UnconnectedState:
        state = "UnconnectedState";
        ns = NetworkSocketEvent::disconnected;
        break;
    case QAbstractSocket::HostLookupState :
        state = "HostLookupState";
        ns = NetworkSocketEvent::other;
        break;
    case QAbstractSocket::ConnectingState :
        state = "ConnectingState";
        ns = NetworkSocketEvent::other;
        break;
    case QAbstractSocket::ConnectedState  :
        state = "ConnectedState";
        ns = NetworkSocketEvent::connected;
        break;
    case QAbstractSocket::BoundState      :
        state = "BoundState";
        ns = NetworkSocketEvent::other;
        break;
    case QAbstractSocket::ListeningState  :
        state = "ListeningState";
        ns = NetworkSocketEvent::other;
        break;
    case QAbstractSocket::ClosingState    :
        state = "ClosingState";
        ns = NetworkSocketEvent::other;
        break;
    }

    server->socketStateChange(connectionId, ns, state);

    //qDebug() << "Socket State Changed: " << state.c_str();
}

void NetworkClient::socketError(QAbstractSocket::SocketError se)
{
    string errMsg;
    switch (se) {
    case QAbstractSocket::ConnectionRefusedError          : errMsg = "ConnectionRefusedError"; break;
    case QAbstractSocket::RemoteHostClosedError           : errMsg = "RemoteHostClosedError"; break;
    case QAbstractSocket::HostNotFoundError               : errMsg = "HostNotFoundError"; break;
    case QAbstractSocket::SocketAccessError               : errMsg = "SocketAccessError"; break;
    case QAbstractSocket::SocketResourceError             : errMsg = "SocketResourceError"; break;
    case QAbstractSocket::SocketTimeoutError              : errMsg = "SocketTimeoutError"; break;
    case QAbstractSocket::DatagramTooLargeError           : errMsg = "DatagramTooLargeError"; break;
    case QAbstractSocket::NetworkError                    : errMsg = "NetworkError"; break;
    case QAbstractSocket::AddressInUseError               : errMsg = "AddressInUseError"; break;
    case QAbstractSocket::SocketAddressNotAvailableError  : errMsg = "SocketAddressNotAvailableError"; break;
    case QAbstractSocket::UnsupportedSocketOperationError : errMsg = "UnsupportedSocketOperationError"; break;
    case QAbstractSocket::UnfinishedSocketOperationError  : errMsg = "UnfinishedSocketOperationError"; break;
    case QAbstractSocket::ProxyAuthenticationRequiredError: errMsg = "ProxyAuthenticationRequiredError"; break;
    case QAbstractSocket::SslHandshakeFailedError         : errMsg = "SslHandshakeFailedError"; break;
    case QAbstractSocket::ProxyConnectionRefusedError     : errMsg = "ProxyConnectionRefusedError"; break;
    case QAbstractSocket::ProxyConnectionClosedError      : errMsg = "ProxyConnectionClosedError"; break;
    case QAbstractSocket::ProxyConnectionTimeoutError     : errMsg = "ProxyConnectionTimeoutError"; break;
    case QAbstractSocket::ProxyNotFoundError              : errMsg = "ProxyNotFoundError"; break;
    case QAbstractSocket::ProxyProtocolError              : errMsg = "ProxyProtocolError"; break;
    case QAbstractSocket::OperationError                  : errMsg = "OperationError"; break;
    case QAbstractSocket::SslInternalError                : errMsg = "SslInternalError"; break;
    case QAbstractSocket::SslInvalidUserDataError         : errMsg = "SslInvalidUserDataError"; break;
    case QAbstractSocket::TemporaryError                  : errMsg = "TemporaryError"; break;
    default: errMsg = "UnknownSocketError"; break;
    }

    server->socketStateChange(connectionId, NetworkSocketEvent::error, errMsg);

    //qDebug() << "Socket Error Signal: " << errMsg.c_str();
}

void NetworkClient::queueToSend(const std::string& data)
{
    //qDebug() << "locking in order to queue data";

    std::unique_lock<std::mutex> lock(commLock);

    //qDebug() << "queue data to send: '" << data.c_str() << "'";

    outgoingData.append(data.c_str());
}

void NetworkClient::readyRead()
{
    //qDebug()  << "In readyRead: " << QThread::currentThreadId() << "\n";

    if (!socket) {
        //qDebug() << "Socket already closed\n";
        return;
    }

    // get the information
    QByteArray newdata = socket->readAll();

    incomingData.append(newdata);

    if (incomingData.endsWith('\n'))
    {
        response = incomingData.constData();
        incomingData.clear();

        gotResponse = true;

        server->receiver(connectionId, response);

        // will write on server side window
        //qDebug() << " Received Data: " << clean(QByteArray::fromStdString(response));
    }
}

void NetworkClient::disconnected()
{
    //qDebug()  << " Disconnected NOTIFY SOMEONE!! " << QThread::currentThreadId() << "\n";
    server->socketStateChange(connectionId, NetworkSocketEvent::disconnected, "disconnected");
    closeSocket();
}

void NetworkClient::sendQueued()
{
    std::unique_lock<std::mutex> lock(commLock);

    if (outgoingData.size() > 0)
    {
        //qDebug() << "Found data to send: "  << outgoingData;

        if (socket)
        {
            socket->write(outgoingData);
            socket->flush();

            //qDebug()  << "Sending Data on Thread: " << QThread::currentThreadId() << "\n";
        }
        else
        {
            //qDebug() << "Not Sending (disconnected): " << outgoingData;
        }

        outgoingData.clear();
    }
}
