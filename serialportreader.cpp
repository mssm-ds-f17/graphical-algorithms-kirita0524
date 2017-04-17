
#include "serialportreader.h"

#include <QCoreApplication>

QT_USE_NAMESPACE

SerialPortReader::SerialPortReader(QObject *parent, const std::string& serialPortName) :
    Plugin(parent), serialPortFailed(false)
{
    serialPort.setPortName(QString::fromStdString(serialPortName));
    serialPort.setBaudRate(QSerialPort::Baud9600);

    if (!serialPort.open(QIODevice::ReadOnly))
    {
        serialPortFailed = true;
        serialPortError  = "Error opening serial port";
        return;
    }

    connect(&serialPort, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    connect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
}

SerialPortReader::~SerialPortReader()
{
}

void SerialPortReader::handleReadyRead()
{
    readData.append(serialPort.readAll());
}


void SerialPortReader::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError)
    {
        //m_standardOutput << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
    }
}

bool SerialPortReader::shouldDelete()
{
    return false;
}

void SerialPortReader::call(int /*arg1*/, int /*arg2*/, const std::string& /*arg3*/)
{

}


void SerialPortReader::update(std::function<void(int, int, int, const std::string&)> sendEvent)
{
    if (readData.size() > 0)
    {
        sendEvent(0, 0, 0, readData.toStdString());
        readData.clear();
    }
}
