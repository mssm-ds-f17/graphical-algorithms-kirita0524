#include "networkclient.h"
#include <sstream>
#include <qbytearray.h>
#include <iostream>

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

NetworkClient::NetworkClient(qintptr socketId) : gotResponse(false)
{
    //cout << "Constructing AI on Thread: " << QThread::currentThreadId() << endl;
    socketDescriptor = socketId;
    socket = new QTcpSocket();

    wasDisconnected = false;

    // set the ID
    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        qDebug() << "Error setting socket descriptor";
        return;
    }
    // connect socket and signal
    // note - Qt::DirectConnection is used because it's multithreaded
    // This makes the slot to be invoked immediately, when the signal is emitted.
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)), Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::QueuedConnection);

    qDebug() << socketDescriptor << " Client connected";

    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    //socket->write("Blocked 0 0\r\n.\r\n");
}

NetworkClient::~NetworkClient()
{
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

void NetworkClient::sendData(QByteArray data)
{
    //qDebug() << "locking in order to queue data";

    std::unique_lock<std::mutex> lock(commLock);

    //qDebug() << "queue data to send: '" << data << "'";

    outgoingData.append(data);
}

void NetworkClient::readyRead()
{
    //cout << "In readyRead: " << QThread::currentThreadId() << endl;

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

            // will write on server side window
            //qDebug() << " Received Data: " << clean(QByteArray::fromStdString(response));
        }

        botCv.notify_one();
    }
}

void NetworkClient::disconnected()
{
    std::cout << " Disconnected\n";
    setDisconnected();
}



bool NetworkClient::asyncWait()
{
    //cout << "RemoteAI::asyncWait on Thread: " << QThread::currentThreadId() << endl;

    return handleCommand();
}

void NetworkClient::commUpdate()
{
    std::unique_lock<std::mutex> lock(commLock);

    if (outgoingData.size() > 0)
    {
        //qDebug() << "Found data to send: "  << outgoingData;

        if (socket)
        {
            socket->write(outgoingData);
            socket->flush();

            //cout << "Sending Data on Thread: " << QThread::currentThreadId() << endl;
        }
        else
        {
            qDebug() << "Not Sending (disconnected): " << outgoingData;
        }

        outgoingData.clear();
    }
}


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

void NetworkClient::setDisconnected()
{
    wasDisconnected = true;
}

bool NetworkClient::isDisconnected()
{
    return wasDisconnected;
}
