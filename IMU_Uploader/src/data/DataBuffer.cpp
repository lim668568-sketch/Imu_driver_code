#include "DataBuffer.h"
#include <QDateTime>
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化数据缓冲区:
 * - 最大数据点数: 10000
 * - 时间戳计数器: 0
 */
DataBuffer::DataBuffer(QObject *parent)
    : QObject(parent)
    , m_maxSize(10000)
    , m_timestamp(0)
{
}

/**
 * @brief 添加IMU数据到缓冲区
 * @param data IMU数据结构体
 * 
 * 将IMU数据添加到各轴的数据队列中，如果超过最大容量则自动丢弃最旧的数据(FIFO)
 * 使用互斥锁保证线程安全
 */
void DataBuffer::addData(const IMUData &data)
{
    QMutexLocker locker(&m_mutex);
    
    DataPoint point;
    point.timestamp = m_timestamp++;
    
    // 添加欧拉角数据
    point.value = data.roll;
    m_rollData.append(point);
    
    point.value = data.pitch;
    m_pitchData.append(point);
    
    point.value = data.yaw;
    m_yawData.append(point);
    
    // 添加加速度数据
    point.value = data.accel_x;
    m_accelXData.append(point);
    
    point.value = data.accel_y;
    m_accelYData.append(point);
    
    point.value = data.accel_z;
    m_accelZData.append(point);
    
    // 添加角速度数据
    point.value = data.gyro_x;
    m_gyroXData.append(point);
    
    point.value = data.gyro_y;
    m_gyroYData.append(point);
    
    point.value = data.gyro_z;
    m_gyroZData.append(point);
    
    // 如果超过最大容量，移除最旧的数据(FIFO)
    while (m_rollData.size() > m_maxSize) {
        m_rollData.removeFirst();
        m_pitchData.removeFirst();
        m_yawData.removeFirst();
        m_accelXData.removeFirst();
        m_accelYData.removeFirst();
        m_accelZData.removeFirst();
        m_gyroXData.removeFirst();
        m_gyroYData.removeFirst();
        m_gyroZData.removeFirst();
    }
    
    emit dataAdded();
}

/**
 * @brief 清空所有数据
 * 
 * 清空所有数据队列并重置时间戳计数器
 */
void DataBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    
    m_rollData.clear();
    m_pitchData.clear();
    m_yawData.clear();
    m_accelXData.clear();
    m_accelYData.clear();
    m_accelZData.clear();
    m_gyroXData.clear();
    m_gyroYData.clear();
    m_gyroZData.clear();
    m_timestamp = 0;
    
    emit bufferCleared();
}

/**
 * @brief 设置最大数据点数
 * @param size 最大数据点数
 */
void DataBuffer::setMaxSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = size;
}

/**
 * @brief 获取Roll数据
 * @return Roll数据点向量
 */
QVector<DataPoint> DataBuffer::getRollData() const
{
    QMutexLocker locker(&m_mutex);
    return m_rollData;
}

/**
 * @brief 获取Pitch数据
 * @return Pitch数据点向量
 */
QVector<DataPoint> DataBuffer::getPitchData() const
{
    QMutexLocker locker(&m_mutex);
    return m_pitchData;
}

/**
 * @brief 获取Yaw数据
 * @return Yaw数据点向量
 */
QVector<DataPoint> DataBuffer::getYawData() const
{
    QMutexLocker locker(&m_mutex);
    return m_yawData;
}

/**
 * @brief 获取X轴加速度数据
 * @return X轴加速度数据点向量
 */
QVector<DataPoint> DataBuffer::getAccelXData() const
{
    QMutexLocker locker(&m_mutex);
    return m_accelXData;
}

/**
 * @brief 获取Y轴加速度数据
 * @return Y轴加速度数据点向量
 */
QVector<DataPoint> DataBuffer::getAccelYData() const
{
    QMutexLocker locker(&m_mutex);
    return m_accelYData;
}

/**
 * @brief 获取Z轴加速度数据
 * @return Z轴加速度数据点向量
 */
QVector<DataPoint> DataBuffer::getAccelZData() const
{
    QMutexLocker locker(&m_mutex);
    return m_accelZData;
}

/**
 * @brief 获取X轴角速度数据
 * @return X轴角速度数据点向量
 */
QVector<DataPoint> DataBuffer::getGyroXData() const
{
    QMutexLocker locker(&m_mutex);
    return m_gyroXData;
}

/**
 * @brief 获取Y轴角速度数据
 * @return Y轴角速度数据点向量
 */
QVector<DataPoint> DataBuffer::getGyroYData() const
{
    QMutexLocker locker(&m_mutex);
    return m_gyroYData;
}

/**
 * @brief 获取Z轴角速度数据
 * @return Z轴角速度数据点向量
 */
QVector<DataPoint> DataBuffer::getGyroZData() const
{
    QMutexLocker locker(&m_mutex);
    return m_gyroZData;
}

/**
 * @brief 导出数据到CSV文件
 * @param filename CSV文件名
 * @return 成功返回true，失败返回false
 * 
 * CSV格式:
 * Timestamp,Roll,Pitch,Yaw,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ
 */
bool DataBuffer::exportToCSV(const QString &filename)
{
    QMutexLocker locker(&m_mutex);
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setRealNumberPrecision(6);
    
    // 写入CSV表头
    out << "Timestamp,Roll,Pitch,Yaw,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ\n";
    
    // 写入数据行
    for (int i = 0; i < m_rollData.size(); ++i) {
        out << m_rollData[i].timestamp << ","
            << m_rollData[i].value << ","
            << m_pitchData[i].value << ","
            << m_yawData[i].value << ","
            << m_accelXData[i].value << ","
            << m_accelYData[i].value << ","
            << m_accelZData[i].value << ","
            << m_gyroXData[i].value << ","
            << m_gyroYData[i].value << ","
            << m_gyroZData[i].value << "\n";
    }
    
    file.close();
    return true;
}

/**
 * @brief 从CSV文件导入数据
 * @param filename CSV文件名
 * @return 成功返回true，失败返回false
 */
bool DataBuffer::importFromCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    clear();
    
    QTextStream in(&file);
    in.readLine(); // 跳过表头
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(',');
        
        if (values.size() >= 10) {
            IMUData data;
            data.roll = values[1].toFloat();
            data.pitch = values[2].toFloat();
            data.yaw = values[3].toFloat();
            data.accel_x = values[4].toFloat();
            data.accel_y = values[5].toFloat();
            data.accel_z = values[6].toFloat();
            data.gyro_x = values[7].toFloat();
            data.gyro_y = values[8].toFloat();
            data.gyro_z = values[9].toFloat();
            
            addData(data);
        }
    }
    
    file.close();
    return true;
}

/**
 * @brief 获取数据点数量
 * @return 数据点数量
 */
int DataBuffer::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_rollData.size();
}

/**
 * @brief 检查缓冲区是否为空
 * @return 为空返回true，否则返回false
 */
bool DataBuffer::isEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_rollData.isEmpty();
}
