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
    wasDisconnected = false;

    qDebug() << "Constructing NetworkClient on Thread: " << QThread::currentThreadId() << endl;
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
    wasDisconnected = false;

    qDebug() << "Constructing NetworkClient on Thread: " << QThread::currentThreadId() << endl;

    auto s = new QTcpSocket();

    s->connectToHost(host.c_str(), port);

    if(s->waitForConnected(5000))
    {
        qDebug() << "Connected!";

        setSocket(s);
    }
    else
    {
        qDebug() << "Failed to connect!";
        wasDisconnected = true;
    }
}

void NetworkClient::setSocket(QTcpSocket *socket)
{
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)), Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::QueuedConnection);

    qDebug() << socket->socketDescriptor() << " Socket connected";

    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
}

NetworkClient::~NetworkClient()
{
    qDebug() << "NetworkClient destructing\n";
    if (socket)
    {
        socket->close();
        delete socket;
    }
}

void NetworkClient::socketStateChanged(QAbstractSocket::SocketState ss)
{
    qDebug() << "Socket State Changed: " << (int)ss;
}

void NetworkClient::socketError(QAbstractSocket::SocketError se)
{
    qDebug() << "Socket Error Signal: " << (int)se;
}

/*

Fire();
Scan(fieldOfView); // maximum field of view is given by maxFieldOfView function
Move(speed);       // maximum speed is given by maxSpeed function
Turn(radians);

*/

void NetworkClient::queueToSend(const std::string& data)
{
    //qDebug() << "locking in order to queue data";

    std::unique_lock<std::mutex> lock(commLock);

    qDebug() << "queue data to send: '" << data.c_str() << "'";

    outgoingData.append(data.c_str());
}

void NetworkClient::readyRead()
{
    qDebug()  << "In readyRead: " << QThread::currentThreadId() << "\n";

    // get the information
    QByteArray newdata = socket->readAll();

    {
        std::unique_lock<std::mutex> lock(botLock);

        incomingData.append(newdata);

        if (incomingData.endsWith('\n'))
        {
            response = incomingData.constData();
            incomingData.clear();

            gotResponse = true;

            server->receiver(connectionId, response);

            // will write on server side window
            qDebug() << " Received Data: " << clean(QByteArray::fromStdString(response));
        }

        botCv.notify_one();
    }
}

void NetworkClient::disconnected()
{
    qDebug()  << " Disconnected\n";
    setDisconnected();
}

void NetworkClient::sendQueued()
{
    std::unique_lock<std::mutex> lock(commLock);

    if (outgoingData.size() > 0)
    {
        qDebug() << "Found data to send: "  << outgoingData;

        if (socket)
        {
            socket->write(outgoingData);
            socket->flush();

            qDebug()  << "Sending Data on Thread: " << QThread::currentThreadId() << "\n";
        }
        else
        {
            qDebug() << "Not Sending (disconnected): " << outgoingData;
        }

        outgoingData.clear();
    }
}
/*

bool NetworkClient::handleCommand()
{
    //cout << "RemoteAI::handleCommand on Thread: " << QThread::currentThreadId() << endl;

    std::unique_lock<std::mutex> lock(botLock);

    if (!botCv.wait_for(lock, std::chrono::milliseconds(10000), [this]{ return gotResponse || wasDisconnected ; }))
    {
        cout << "timeout" << endl;
        // timeout, apparently
        setDisconnected();
        return false;
    }

    if (wasDisconnected)
    {
        cout << "wasDisconnected" << endl;
        return false;
    }

    if (gotResponse)
    {
        gotResponse = false;

        if (!response.empty())
        {
            string cmd;
            swap(cmd, response);
            return handleCommand(cmd);
        }

        cout << "Unexpected thing" << endl;
        return false;
    }

    cout << "Definitely don't expect to get here" << endl;

    return false;
}
*/

void NetworkClient::setDisconnected()
{
    wasDisconnected = true;
}

bool NetworkClient::isDisconnected()
{
    return wasDisconnected;
}
