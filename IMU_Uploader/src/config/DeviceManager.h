#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "src/serial/SerialPort.h"
#include "src/serial/ProtocolParser.h"
#include "src/data/DataBuffer.h"

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    static DeviceManager* instance();
    
    bool connectDevice(const QString &portName, qint32 baudRate = 115200);
    void disconnectDevice();
    bool isConnected() const;
    
    SerialPort* getSerialPort();
    ProtocolParser* getProtocolParser();
    DataBuffer* getDataBuffer();
    
    void sendData(const QByteArray &data);
    void clearBuffer();
    
    void setDeviceName(const QString &name);
    QString getDeviceName() const;
    
signals:
    void deviceConnected(const QString &portName);
    void deviceDisconnected();
    void connectionError(const QString &error);
    void dataReceived(const IMUData &data);
    
private slots:
    void onSerialDataReceived(const QByteArray &data);
    void onSerialError(const QString &error);
    void onEulerAngleReceived(float roll, float pitch, float yaw);
    void onRawDataReceived(float ax, float ay, float az, float gx, float gy, float gz);
    void onIMUDataReceived(const IMUData &data);
    
private:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();
    
    static DeviceManager *m_instance;
    
    SerialPort *m_serialPort;
    ProtocolParser *m_protocolParser;
    DataBuffer *m_dataBuffer;
    
    QString m_deviceName;
    bool m_connected;
};

#endif
