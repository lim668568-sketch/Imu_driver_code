#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include "src/serial/ProtocolParser.h"

/**
 * @brief 数据点结构体
 * 
 * 存储单个数据点的值和时间戳
 */
struct DataPoint {
    float value;        // 数据值
    quint64 timestamp;  // 时间戳(相对时间，单位：采样点序号)
};

/**
 * @brief 数据缓冲区类
 * 
 * 管理IMU传感器数据的历史记录，提供:
 * - 线程安全的数据存储
 * - 自动丢弃旧数据(FIFO)
 * - 数据导出/导入功能
 * 
 * 支持的数据类型:
 * - 欧拉角: Roll, Pitch, Yaw
 * - 加速度: AccelX, AccelY, AccelZ
 * - 角速度: GyroX, GyroY, GyroZ
 */
class DataBuffer : public QObject
{
    Q_OBJECT

public:
    explicit DataBuffer(QObject *parent = nullptr);
    
    /**
     * @brief 添加IMU数据
     * @param data IMU数据结构体
     */
    void addData(const IMUData &data);
    
    /**
     * @brief 清空所有数据
     */
    void clear();
    
    /**
     * @brief 设置最大数据点数
     * @param size 最大数据点数
     */
    void setMaxSize(int size);
    
    // 获取各轴数据
    QVector<DataPoint> getRollData() const;
    QVector<DataPoint> getPitchData() const;
    QVector<DataPoint> getYawData() const;
    QVector<DataPoint> getAccelXData() const;
    QVector<DataPoint> getAccelYData() const;
    QVector<DataPoint> getAccelZData() const;
    QVector<DataPoint> getGyroXData() const;
    QVector<DataPoint> getGyroYData() const;
    QVector<DataPoint> getGyroZData() const;
    
    /**
     * @brief 导出数据到CSV文件
     * @param filename 文件名
     * @return 成功返回true，失败返回false
     */
    bool exportToCSV(const QString &filename);
    
    /**
     * @brief 从CSV文件导入数据
     * @param filename 文件名
     * @return 成功返回true，失败返回false
     */
    bool importFromCSV(const QString &filename);
    
    /**
     * @brief 获取数据点数量
     */
    int size() const;
    
    /**
     * @brief 检查缓冲区是否为空
     */
    bool isEmpty() const;
    
signals:
    void dataAdded();       // 数据添加信号
    void bufferCleared();   // 缓冲区清空信号
    
private:
    mutable QMutex m_mutex; // 互斥锁，保证线程安全
    
    // 各轴数据存储
    QVector<DataPoint> m_rollData;
    QVector<DataPoint> m_pitchData;
    QVector<DataPoint> m_yawData;
    QVector<DataPoint> m_accelXData;
    QVector<DataPoint> m_accelYData;
    QVector<DataPoint> m_accelZData;
    QVector<DataPoint> m_gyroXData;
    QVector<DataPoint> m_gyroYData;
    QVector<DataPoint> m_gyroZData;
    
    int m_maxSize;          // 最大数据点数
    quint64 m_timestamp;    // 当前时间戳计数器
};

#endif
