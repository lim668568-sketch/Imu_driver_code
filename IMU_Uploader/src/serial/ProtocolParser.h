#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

/**
 * @brief IMU传感器数据结构体
 * 
 * 存储从传感器接收到的所有姿态数据
 */
struct IMUData {
    float roll;         // 滚转角，单位：度
    float pitch;        // 俯仰角，单位：度
    float yaw;          // 偏航角，单位：度
    float accel_x;      // X轴加速度，单位：g
    float accel_y;      // Y轴加速度，单位：g
    float accel_z;      // Z轴加速度，单位：g
    float gyro_x;       // X轴角速度，单位：deg/s
    float gyro_y;       // Y轴角速度，单位：deg/s
    float gyro_z;       // Z轴角速度，单位：deg/s
    float temperature;  // 温度，单位：摄氏度
    quint32 timestamp;  // 时间戳
};

/**
 * @brief 协议解析类
 * 
 * 解析从串口接收的二进制数据帧，支持多种数据类型:
 * - 欧拉角数据(roll, pitch, yaw)
 * - 原始传感器数据(加速度计、陀螺仪)
 * - 四元数数据
 * - 全部数据
 * 
 * 协议格式: 帧头(2B) + 帧长(1B) + 数据类型(1B) + 数据(NB) + 校验(1B) + 帧尾(2B)
 */
class ProtocolParser : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolParser(QObject *parent = nullptr);
    
    /**
     * @brief 解析接收到的数据
     * @param data 原始二进制数据
     */
    void parseData(const QByteArray &data);
    
    /**
     * @brief 设置协议格式
     * @param header1 帧头第一个字节
     * @param header2 帧头第二个字节
     * @param tail1 帧尾第一个字节
     * @param tail2 帧尾第二个字节
     */
    void setProtocolFormat(quint8 header1, quint8 header2, quint8 tail1, quint8 tail2);
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearBuffer();
    
    /**
     * @brief 数据类型枚举
     */
    enum DataType {
        EULER_ANGLE = 0x01,   // 欧拉角数据
        RAW_DATA = 0x02,      // 原始传感器数据
        QUATERNION = 0x03,    // 四元数数据
        ALL_DATA = 0x04       // 全部数据
    };

signals:
    /**
     * @brief 接收到欧拉角数据时发射
     */
    void eulerAngleReceived(float roll, float pitch, float yaw);
    
    /**
     * @brief 接收到原始数据时发射
     */
    void rawDataReceived(float ax, float ay, float az, float gx, float gy, float gz);
    
    /**
     * @brief 接收到完整IMU数据时发射
     */
    void imuDataReceived(const IMUData &data);
    
    /**
     * @brief 解析错误时发射
     */
    void parseError(const QString &error);
    
private:
    /**
     * @brief 从缓冲区中查找完整数据帧
     * @param frame 输出参数，找到的完整帧
     * @return 找到返回true，否则返回false
     */
    bool findFrame(QByteArray &frame);
    
    /**
     * @brief 解析单个数据帧
     * @param frame 完整的数据帧
     * @return 解析成功返回true，否则返回false
     */
    bool parseFrame(const QByteArray &frame);
    
    /**
     * @brief 计算CRC校验值
     * @param data 数据
     * @param start 起始位置
     * @param length 数据长度
     * @return CRC校验值
     */
    quint8 calculateCRC(const QByteArray &data, int start, int length);
    
    QByteArray m_buffer;    // 接收缓冲区
    quint8 m_header1;       // 帧头第一个字节(默认0x55)
    quint8 m_header2;       // 帧头第二个字节(默认0xAA)
    quint8 m_tail1;         // 帧尾第一个字节(默认0x0D)
    quint8 m_tail2;         // 帧尾第二个字节(默认0x0A)
    int m_maxBufferSize;    // 最大缓冲区大小
};

#endif
