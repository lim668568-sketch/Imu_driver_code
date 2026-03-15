#include "SerialPort.h"
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化串口对象和超时定时器，连接信号槽
 */
SerialPort::SerialPort(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_timeoutTimer(new QTimer(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialPort::readData);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
    connect(m_timeoutTimer, &QTimer::timeout, [this]() {
        if (m_serial->isOpen()) {
            m_timeoutTimer->start(1000);
        }
    });
}

/**
 * @brief 析构函数
 * 
 * 关闭串口连接，释放资源
 */
SerialPort::~SerialPort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
}

/**
 * @brief 扫描可用串口
 * @return 串口列表，格式为"端口名 - 描述"
 * 
 * 遍历系统中所有可用的串口设备，返回串口名和描述信息
 */
QStringList SerialPort::scanPorts()
{
    QStringList portList;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        portList << info.portName() + " - " + info.description();
    }
    return portList;
}

/**
 * @brief 打开串口
 * @param portName 串口名称(如"COM1"或"COM1 - 描述")
 * @param baudRate 波特率，默认115200
 * @return 成功返回true，失败返回false
 * 
 * 配置串口参数并打开连接:
 * - 数据位: 8位
 * - 校验位: 无校验
 * - 停止位: 1位
 * - 流控制: 无流控制
 */
bool SerialPort::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    
    QString purePortName = portName.split(" - ").first();
    m_serial->setPortName(purePortName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_timeoutTimer->start(1000);
        emit portOpened(portName);
        return true;
    } else {
        emit errorOccurred(m_serial->errorString());
        return false;
    }
}

/**
 * @brief 关闭串口
 * 
 * 关闭当前串口连接，停止超时定时器
 */
void SerialPort::closePort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        m_timeoutTimer->stop();
        emit portClosed();
    }
}

/**
 * @brief 检查串口是否已打开
 * @return 已打开返回true，否则返回false
 */
bool SerialPort::isOpen() const
{
    return m_serial->isOpen();
}

/**
 * @brief 写入数据到串口
 * @param data 要发送的数据
 * @return 实际写入的字节数，失败返回-1
 */
qint64 SerialPort::writeData(const QByteArray &data)
{
    if (m_serial->isOpen()) {
        return m_serial->write(data);
    }
    return -1;
}

/**
 * @brief 设置波特率
 * @param baudRate 波特率值
 */
void SerialPort::setBaudRate(qint32 baudRate)
{
    m_serial->setBaudRate(baudRate);
}

/**
 * @brief 设置数据位
 * @param dataBits 数据位数(5/6/7/8)
 */
void SerialPort::setDataBits(QSerialPort::DataBits dataBits)
{
    m_serial->setDataBits(dataBits);
}

/**
 * @brief 设置校验位
 * @param parity 校验模式(无校验/奇校验/偶校验)
 */
void SerialPort::setParity(QSerialPort::Parity parity)
{
    m_serial->setParity(parity);
}

/**
 * @brief 设置停止位
 * @param stopBits 停止位数(1位/1.5位/2位)
 */
void SerialPort::setStopBits(QSerialPort::StopBits stopBits)
{
    m_serial->setStopBits(stopBits);
}

/**
 * @brief 设置流控制
 * @param flowControl 流控制模式(无/硬件/软件)
 */
void SerialPort::setFlowControl(QSerialPort::FlowControl flowControl)
{
    m_serial->setFlowControl(flowControl);
}

/**
 * @brief 读取串口数据(私有槽函数)
 * 
 * 当串口有数据可读时自动调用，将数据追加到缓冲区并发射dataReceived信号
 */
void SerialPort::readData()
{
    QByteArray data = m_serial->readAll();
    m_buffer.append(data);
    
    emit dataReceived(data);
}

/**
 * @brief 处理串口错误(私有槽函数)
 * @param error 错误类型
 * 
 * 当串口发生错误时自动调用，发射errorOccurred信号
 * 忽略无错误和超时错误
 */
void SerialPort::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        emit errorOccurred(m_serial->errorString());
    }
}
