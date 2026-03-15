#include "ProtocolParser.h"
#include <QDebug>
#include <QtEndian>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化协议解析器，设置默认的帧头帧尾:
 * - 帧头: 0x55 0xAA
 * - 帧尾: 0x0D 0x0A (回车换行)
 * - 最大缓冲区: 4096字节
 */
ProtocolParser::ProtocolParser(QObject *parent)
    : QObject(parent)
    , m_header1(0x55)
    , m_header2(0xAA)
    , m_tail1(0x0D)
    , m_tail2(0x0A)
    , m_maxBufferSize(4096)
{
}

/**
 * @brief 解析接收到的数据
 * @param data 原始二进制数据
 * 
 * 将接收到的数据追加到缓冲区，然后循环查找并解析完整的数据帧
 * 如果缓冲区超过最大大小，则丢弃旧数据
 */
void ProtocolParser::parseData(const QByteArray &data)
{
    m_buffer.append(data);
    
    if (m_buffer.size() > m_maxBufferSize) {
        m_buffer.remove(0, m_buffer.size() - m_maxBufferSize);
    }
    
    QByteArray frame;
    while (findFrame(frame)) {
        parseFrame(frame);
    }
}

/**
 * @brief 设置协议格式
 * @param header1 帧头第一个字节
 * @param header2 帧头第二个字节
 * @param tail1 帧尾第一个字节
 * @param tail2 帧尾第二个字节
 */
void ProtocolParser::setProtocolFormat(quint8 header1, quint8 header2, quint8 tail1, quint8 tail2)
{
    m_header1 = header1;
    m_header2 = header2;
    m_tail1 = tail1;
    m_tail2 = tail2;
}

/**
 * @brief 清空接收缓冲区
 */
void ProtocolParser::clearBuffer()
{
    m_buffer.clear();
}

/**
 * @brief 从缓冲区中查找完整数据帧
 * @param frame 输出参数，找到的完整帧
 * @return 找到返回true，否则返回false
 * 
 * 帧格式: [帧头2B][帧长1B][类型1B][数据NB][CRC1B][帧尾2B]
 * 
 * 查找步骤:
 * 1. 查找帧头位置
 * 2. 读取帧长度
 * 3. 检查是否接收到完整帧
 * 4. 验证帧尾
 */
bool ProtocolParser::findFrame(QByteArray &frame)
{
    int headerPos = -1;
    for (int i = 0; i < m_buffer.size() - 1; ++i) {
        if ((quint8)m_buffer[i] == m_header1 && (quint8)m_buffer[i + 1] == m_header2) {
            headerPos = i;
            break;
        }
    }
    
    if (headerPos == -1) {
        return false;
    }
    
    if (headerPos > 0) {
        m_buffer.remove(0, headerPos);
    }
    
    if (m_buffer.size() < 4) {
        return false;
    }
    
    quint8 frameLen = (quint8)m_buffer[2];
    
    int totalLen = 2 + 1 + 1 + frameLen + 1 + 2;
    
    if (m_buffer.size() < totalLen) {
        return false;
    }
    
    int tailPos = totalLen - 2;
    if ((quint8)m_buffer[tailPos] == m_tail1 && (quint8)m_buffer[tailPos + 1] == m_tail2) {
        frame = m_buffer.left(totalLen);
        m_buffer.remove(0, totalLen);
        return true;
    } else {
        m_buffer.remove(0, 2);
        return false;
    }
}

/**
 * @brief 解析单个数据帧
 * @param frame 完整的数据帧
 * @return 解析成功返回true，否则返回false
 * 
 * 解析步骤:
 * 1. 提取帧长度和数据类型
 * 2. 校验CRC
 * 3. 根据数据类型解析数据
 * 4. 发射相应的信号
 */
bool ProtocolParser::parseFrame(const QByteArray &frame)
{
    if (frame.size() < 7) {
        return false;
    }
    
    quint8 frameLen = (quint8)frame[2];
    quint8 dataType = (quint8)frame[3];
    
    quint8 crcReceived = (quint8)frame[frame.size() - 3];
    quint8 crcCalculated = calculateCRC(frame, 4, frameLen);
    
    if (crcReceived != crcCalculated) {
        emit parseError(QString("CRC校验失败: 接收=%1, 计算=%2").arg(crcReceived).arg(crcCalculated));
        return false;
    }
    
    const char *dataPtr = frame.constData() + 4;
    
    IMUData imuData;
    imuData.timestamp = 0;
    
    switch (dataType) {
        case EULER_ANGLE: {
            if (frameLen >= 12) {
                imuData.roll = qFromLittleEndian<float>(dataPtr);
                imuData.pitch = qFromLittleEndian<float>(dataPtr + 4);
                imuData.yaw = qFromLittleEndian<float>(dataPtr + 8);
                
                emit eulerAngleReceived(imuData.roll, imuData.pitch, imuData.yaw);
                emit imuDataReceived(imuData);
            }
            break;
        }
        
        case RAW_DATA: {
            if (frameLen >= 24) {
                imuData.accel_x = qFromLittleEndian<float>(dataPtr);
                imuData.accel_y = qFromLittleEndian<float>(dataPtr + 4);
                imuData.accel_z = qFromLittleEndian<float>(dataPtr + 8);
                imuData.gyro_x = qFromLittleEndian<float>(dataPtr + 12);
                imuData.gyro_y = qFromLittleEndian<float>(dataPtr + 16);
                imuData.gyro_z = qFromLittleEndian<float>(dataPtr + 20);
                
                emit rawDataReceived(imuData.accel_x, imuData.accel_y, imuData.accel_z,
                                    imuData.gyro_x, imuData.gyro_y, imuData.gyro_z);
                emit imuDataReceived(imuData);
            }
            break;
        }
        
        case ALL_DATA: {
            if (frameLen >= 40) {
                imuData.roll = qFromLittleEndian<float>(dataPtr);
                imuData.pitch = qFromLittleEndian<float>(dataPtr + 4);
                imuData.yaw = qFromLittleEndian<float>(dataPtr + 8);
                imuData.accel_x = qFromLittleEndian<float>(dataPtr + 12);
                imuData.accel_y = qFromLittleEndian<float>(dataPtr + 16);
                imuData.accel_z = qFromLittleEndian<float>(dataPtr + 20);
                imuData.gyro_x = qFromLittleEndian<float>(dataPtr + 24);
                imuData.gyro_y = qFromLittleEndian<float>(dataPtr + 28);
                imuData.gyro_z = qFromLittleEndian<float>(dataPtr + 32);
                imuData.temperature = qFromLittleEndian<float>(dataPtr + 36);
                
                emit eulerAngleReceived(imuData.roll, imuData.pitch, imuData.yaw);
                emit rawDataReceived(imuData.accel_x, imuData.accel_y, imuData.accel_z,
                                    imuData.gyro_x, imuData.gyro_y, imuData.gyro_z);
                emit imuDataReceived(imuData);
            }
            break;
        }
        
        default:
            emit parseError(QString("未知数据类型: %1").arg(dataType));
            return false;
    }
    
    return true;
}

/**
 * @brief 计算CRC校验值
 * @param data 数据
 * @param start 起始位置
 * @param length 数据长度
 * @return CRC校验值(累加和)
 * 
 * 使用简单的累加和作为校验值
 */
quint8 ProtocolParser::calculateCRC(const QByteArray &data, int start, int length)
{
    quint8 crc = 0;
    for (int i = start; i < start + length; ++i) {
        crc += (quint8)data[i];
    }
    return crc;
}
