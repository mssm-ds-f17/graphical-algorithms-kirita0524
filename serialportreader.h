#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QtSerialPort/QSerialPort>

#include <QTextStream>
#include <QTimer>
#include <QByteArray>
#include <QObject>

QT_USE_NAMESPACE


#include "plugin.h"

class SerialPortReader : public mssm::Plugin
{
    Q_OBJECT
private:
    QSerialPort serialPort;
    QByteArray  readData;
    bool        serialPortFailed;
    std::string serialPortError;
public:
    explicit SerialPortReader(QObject *parent, const std::string& portName);
    virtual ~SerialPortReader();
    bool shouldDelete() override;
    void update(std::function<void(int, int, int, const std::string&)> sendEvent) override;
    void call(int arg1, int arg2, const std::string& arg3) override;
private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
};

#endif
