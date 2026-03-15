#include "mpu6500.h"
#include "i2c.h"

// 陀螺仪灵敏度：250dps量程，16位ADC，转换为度/秒
#define GYRO_SCALE    (250.0f / 32768.0f)
// 加速度计灵敏度：2g量程，16位ADC，转换为g
#define ACCEL_SCALE   (2.0f / 32768.0f)
// Mahony滤波器比例增益：控制加速度计对姿态的修正强度
#define Kp            2.0f
// Mahony滤波器积分增益：控制积分项对陀螺仪零偏的补偿强度
#define Ki            0.0f
// 采样周期的一半，用于姿态更新计算
#define halfT         0.001f

// MPU6500原始数据结构体（加速度计和陀螺仪数据）
volatile MPU6500_Data_t mpu6500_data;
// 欧拉角结构体（横滚角、俯仰角、偏航角）
volatile EulerAngle_t euler_angle;

// 四元数初始值：q0=1表示初始姿态为水平
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
// 积分项：用于补偿陀螺仪的零偏和漂移
static float integral_x = 0.0f, integral_y = 0.0f, integral_z = 0.0f;

/**
 * @brief MPU6500初始化函数
 * @details 配置MPU6500传感器的基本参数：
 *          1. 电源管理：唤醒传感器，选择PLL时钟源
 *          2. 加速度计：±2g量程
 *          3. 陀螺仪：±250dps量程
 *          这些配置适用于大多数姿态解算应用
 */
void MPU6500_Init(void)
{
    // 电源管理寄存器1：0x01 = 唤醒传感器，使用PLL作为时钟源
    MPU6500_WriteReg(MPU6500_PWR_MGMT_1, 0x01);
    // 加速度计配置：0x00 = ±2g量程
    MPU6500_WriteReg(MPU6500_ACCEL_CONFIG, 0x00);
    // 陀螺仪配置：0x00 = ±250dps量程
    MPU6500_WriteReg(MPU6500_GYRO_CONFIG, 0x00);
}

/**
 * @brief 读取MPU6500指定寄存器的值
 * @param reg 寄存器地址
 * @return 寄存器中的8位数据
 */
uint8_t MPU6500_ReadReg(uint8_t reg)
{
    uint8_t data;
    I2C_ReadReg(MPU6500_ADDR, reg, &data);
    return data;
}

/**
 * @brief 向MPU6500指定寄存器写入数据
 * @param reg 寄存器地址
 * @param data 要写入的8位数据
 */
void MPU6500_WriteReg(uint8_t reg, uint8_t data)
{
    I2C_WriteReg(MPU6500_ADDR, reg, data);
}

/**
 * @brief 读取MPU6500传感器数据并转换为物理量
 * @details 从MPU6500读取14字节数据：
 *          - 加速度计：6字节（X/Y/Z轴，每轴2字节）
 *          - 温度：2字节（本次读取暂不使用）
 *          - 陀螺仪：6字节（X/Y/Z轴，每轴2字节）
 *          数据格式：高字节在前，低字节在后
 *          转换：将原始ADC值转换为物理量（g和度/秒）
 */
void MPU6500_ReadData(void)
{
    uint8_t buf[14];
    
    // 从加速度计X轴高字节寄存器开始连续读取14字节
    I2C_ReadMulti(MPU6500_ADDR, MPU6500_ACCEL_XOUT_H, buf, 14);
    
    // 加速度计数据转换：16位有符号数 × 灵敏度 = 物理量(g)
    mpu6500_data.accel_x = ((int16_t)(buf[0] << 8 | buf[1])) * ACCEL_SCALE;
    mpu6500_data.accel_y = ((int16_t)(buf[2] << 8 | buf[3])) * ACCEL_SCALE;
    mpu6500_data.accel_z = ((int16_t)(buf[4] << 8 | buf[5])) * ACCEL_SCALE;
    
    // 陀螺仪数据转换：16位有符号数 × 灵敏度 = 物理量(度/秒)
    mpu6500_data.gyro_x = ((int16_t)(buf[8] << 8 | buf[9])) * GYRO_SCALE;
    mpu6500_data.gyro_y = ((int16_t)(buf[10] << 8 | buf[11])) * GYRO_SCALE;
    mpu6500_data.gyro_z = ((int16_t)(buf[12] << 8 | buf[13])) * GYRO_SCALE;
}

/**
 * @brief 向量归一化函数
 * @details 将三维向量转换为单位向量，模长为1
 *          用于加速度计数据的归一化处理
 * @param v 三维向量指针 [x, y, z]
 */
static void Normalize(float *v)
{
    // 计算向量的模长
    float mag = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    // 将每个分量除以模长，得到单位向量
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
}

/**
 * @brief 向量叉积函数
 * @details 计算两个三维向量的叉积，结果垂直于两个输入向量
 *          用于Mahony滤波器中计算重力向量与测量向量的误差
 * @param v1 第一个向量 [x1, y1, z1]
 * @param v2 第二个向量 [x2, y2, z2]
 * @param out 输出向量 [x, y, z] = v1 × v2
 */
static void CrossProduct(float *v1, float *v2, float *out)
{
    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/**
 * @brief Mahony姿态解算滤波器
 * @details Mahony算法是一种互补滤波器，用于融合陀螺仪和加速度计数据：
 *          
 *          **算法原理：**
 *          1. 陀螺仪：短期精度高，但有零偏漂移，用于快速响应姿态变化
 *          2. 加速度计：长期稳定，但受运动加速度干扰，用于修正陀螺仪漂移
 *          3. 四元数：用于表示姿态，避免万向锁问题
 *          
 *          **计算步骤：**
 *          1. 从当前四元数计算重力向量（机体坐标系下）
 *          2. 计算重力向量与加速度计测量向量的误差（叉积）
 *          3. 使用PI控制器修正陀螺仪数据
 *          4. 使用修正后的陀螺仪数据更新四元数
 *          5. 归一化四元数，确保数值稳定性
 *          
 *          **参数说明：**
 *          - Kp：比例增益，控制加速度计修正强度（通常1-5）
 *          - Ki：积分增益，控制零偏补偿强度（通常0-0.1）
 *          - dt：采样周期（秒），影响姿态更新精度
 *          
 * @param dt 采样周期（秒），例如0.01表示10ms采样一次
 */
void Mahony_Filter(float dt)
{
    // 准备加速度计和陀螺仪数据
    float accel[3] = {mpu6500_data.accel_x, mpu6500_data.accel_y, mpu6500_data.accel_z};
    float gyro[3] = {mpu6500_data.gyro_x, mpu6500_data.gyro_y, mpu6500_data.gyro_z};
    float ex, ey, ez;  // 误差向量
    float halfq0 = q0 * 0.5f, halfq1 = q1 * 0.5f, halfq2 = q2 * 0.5f;
    float halfq3 = q3 * 0.5f;
    float a[3], v[3];  // a:重力向量，v:误差向量
    
    // 归一化加速度计数据，得到单位重力向量
    Normalize(accel);
    
    // 从当前四元数计算机体坐标系下的重力向量
    // 四元数到旋转矩阵的转换公式
    a[0] = 2 * (halfq1 * halfq3 - halfq0 * halfq2);
    a[1] = 2 * (halfq0 * halfq1 + halfq2 * halfq3);
    a[2] = halfq0 * halfq0 - halfq1 * halfq1 - halfq2 * halfq2 + halfq3 * halfq3;
    
    // 计算重力向量与测量向量的误差（叉积）
    // 误差向量表示需要修正的姿态偏差
    CrossProduct(a, accel, v);
    
    ex = v[0];
    ey = v[1];
    ez = v[2];
    
    // 积分项：用于补偿陀螺仪的零偏和长期漂移
    integral_x += ex * Ki * dt;
    integral_y += ey * Ki * dt;
    integral_z += ez * Ki * dt;
    
    // 使用PI控制器修正陀螺仪数据
    // Kp*ex：比例项，快速响应姿态误差
    // integral_x：积分项，补偿零偏
    gyro[0] += Kp * ex + integral_x;
    gyro[1] += Kp * ey + integral_y;
    gyro[2] += Kp * ez + integral_z;
    
    // 使用修正后的陀螺仪数据更新四元数
    // 四元数微分方程的离散化实现
    q0 += (-halfq1 * gyro[0] - halfq2 * gyro[1] - halfq3 * gyro[2]) * dt;
    q1 += (halfq0 * gyro[0] + halfq2 * gyro[2] - halfq3 * gyro[1]) * dt;
    q2 += (halfq0 * gyro[1] - halfq1 * gyro[2] + halfq3 * gyro[0]) * dt;
    q3 += (halfq0 * gyro[2] + halfq1 * gyro[1] - halfq2 * gyro[0]) * dt;
    
    // 归一化四元数，确保数值稳定性
    // 防止由于累积误差导致四元数模长偏离1
    float norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 /= norm;
    q1 /= norm;
    q2 /= norm;
    q3 /= norm;
}

/**
 * @brief 从四元数计算欧拉角
 * @details 将四元数转换为横滚角、俯仰角和偏航角：
 *          
 *          **欧拉角定义：**
 *          - Roll（横滚角）：绕X轴旋转，范围-180°~180°
 *          - Pitch（俯仰角）：绕Y轴旋转，范围-90°~90°
 *          - Yaw（偏航角）：绕Z轴旋转，范围-180°~180°
 *          
 *          **转换公式：**
 *          使用四元数到欧拉角的数学转换公式
 *          57.2958是弧度转角度的转换因子（180/π）
 *          
 *          **注意：**
 *          - 俯仰角在±90°附近会出现万向锁现象
 *          - 偏航角需要磁力计数据才能获得绝对方向
 *          - 当前实现仅使用加速度计和陀螺仪，偏航角会有漂移
 */
void Euler_Calculate(void)
{
    // 横滚角：绕X轴的旋转角度
    euler_angle.roll = atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2)) * 57.2958f;
    // 俯仰角：绕Y轴的旋转角度
    euler_angle.pitch = asin(2 * (q0 * q2 - q1 * q3)) * 57.2958f;
    // 偏航角：绕Z轴的旋转角度
    euler_angle.yaw = atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3)) * 57.2958f;
}
