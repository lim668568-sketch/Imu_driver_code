#include "DeviceManager.h"
#include <QDebug>

DeviceManager* DeviceManager::m_instance = nullptr;

DeviceManager* DeviceManager::instance()
{
    if (!m_instance) {
        m_instance = new DeviceManager();
    }
    return m_instance;
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new SerialPort(this))
    , m_protocolParser(new ProtocolParser(this))
    , m_dataBuffer(new DataBuffer(this))
    , m_deviceName("Unknown")
    , m_connected(false)
{
    connect(m_serialPort, &SerialPort::dataReceived, this, &DeviceManager::onSerialDataReceived);
    connect(m_serialPort, &SerialPort::errorOccurred, this, &DeviceManager::onSerialError);
    connect(m_serialPort, &SerialPort::portClosed, this, &DeviceManager::deviceDisconnected);
    
    connect(m_protocolParser, &ProtocolParser::eulerAngleReceived, this, &DeviceManager::onEulerAngleReceived);
    connect(m_protocolParser, &ProtocolParser::rawDataReceived, this, &DeviceManager::onRawDataReceived);
    connect(m_protocolParser, &ProtocolParser::imuDataReceived, this, &DeviceManager::onIMUDataReceived);
}

DeviceManager::~DeviceManager()
{
    disconnectDevice();
}

bool DeviceManager::connectDevice(const QString &portName, qint32 baudRate)
{
    if (m_serialPort->openPort(portName, baudRate)) {
        m_connected = true;
        m_deviceName = portName;
        emit deviceConnected(portName);
        return true;
    }
    return false;
}

void DeviceManager::disconnectDevice()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->closePort();
        m_connected = false;
        m_protocolParser->clearBuffer();
    }
}

bool DeviceManager::isConnected() const
{
    return m_connected;
}

SerialPort* DeviceManager::getSerialPort()
{
    return m_serialPort;
}

ProtocolParser* DeviceManager::getProtocolParser()
{
    return m_protocolParser;
}

DataBuffer* DeviceManager::getDataBuffer()
{
    return m_dataBuffer;
}

void DeviceManager::sendData(const QByteArray &data)
{
    if (m_connected) {
        m_serialPort->writeData(data);
    }
}

void DeviceManager::clearBuffer()
{
    m_dataBuffer->clear();
    m_protocolParser->clearBuffer();
}

void DeviceManager::setDeviceName(const QString &name)
{
    m_deviceName = name;
}

QString DeviceManager::getDeviceName() const
{
    return m_deviceName;
}

void DeviceManager::onSerialDataReceived(const QByteArray &data)
{
    m_protocolParser->parseData(data);
}

void DeviceManager::onSerialError(const QString &error)
{
    m_connected = false;
    emit connectionError(error);
}

void DeviceManager::onEulerAngleReceived(float roll, float pitch, float yaw)
{
    Q_UNUSED(roll);
    Q_UNUSED(pitch);
    Q_UNUSED(yaw);
}

void DeviceManager::onRawDataReceived(float ax, float ay, float az, float gx, float gy, float gz)
{
    Q_UNUSED(ax);
    Q_UNUSED(ay);
    Q_UNUSED(az);
    Q_UNUSED(gx);
    Q_UNUSED(gy);
    Q_UNUSED(gz);
}

void DeviceManager::onIMUDataReceived(const IMUData &data)
{
    m_dataBuffer->addData(data);
    emit dataReceived(data);
}
