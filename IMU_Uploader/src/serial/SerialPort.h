#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include <QTimer>

/**
 * @brief 串口通信类
 * 
 * 封装Qt的QSerialPort，提供串口扫描、连接、数据收发等功能
 * 支持自定义波特率、数据位、校验位、停止位等参数配置
 */
class SerialPort : public QObject
{
    Q_OBJECT

public:
    explicit SerialPort(QObject *parent = nullptr);
    ~SerialPort();
    
    QStringList scanPorts();
    bool openPort(const QString &portName, qint32 baudRate = 115200);
    void closePort();
    bool isOpen() const;
    qint64 writeData(const QByteArray &data);
    
    void setBaudRate(qint32 baudRate);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setParity(QSerialPort::Parity parity);
    void setStopBits(QSerialPort::StopBits stopBits);
    void setFlowControl(QSerialPort::FlowControl flowControl);
    
signals:
    void dataReceived(const QByteArray &data);
    void portOpened(const QString &portName);
    void portClosed();
    void errorOccurred(const QString &error);
    
private slots:
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    
private:
    QSerialPort *m_serial;
    QByteArray m_buffer;
    QTimer *m_timeoutTimer;
};

#endif
