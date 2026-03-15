#include "i2c.h"
#include "gpio.h"

#define SCL_PIN   GPIO_PIN_6
#define SDA_PIN   GPIO_PIN_7
#define I2C_PORT  GPIOB

// 使用HAL库替代标准库的GPIO操作
#define GPIO_SetBits(port, pin)     HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define GPIO_ResetBits(port, pin)   HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define GPIO_ReadInputDataBit(port, pin)  (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET ? 1 : 0)

/**
 * @brief I2C软件延时函数
 * @details 通过空循环实现微秒级延时，确保I2C时序符合规范
 *          延时长度取决于系统时钟频率和循环次数
 */
void I2C_Delay(void)
{
    for (volatile int i = 0; i < 1000; i++);
}

/**
 * @brief I2C起始信号
 * @details I2C起始条件：SCL为高电平时，SDA由高变低
 *          时序步骤：
 *          1. SDA置高
 *          2. SCL置高
 *          3. SDA拉低（起始条件）
 *          4. SCL拉低（准备发送数据）
 */
void I2C_Start(void)
{
    GPIO_SetBits(I2C_PORT, SDA_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_PORT, SDA_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
}

/**
 * @brief I2C停止信号
 * @details I2C停止条件：SCL为高电平时，SDA由低变高
 *          时序步骤：
 *          1. SDA拉低（总线空闲状态）
 *          2. SCL置高
 *          3. SDA置高（停止条件）
 */
void I2C_Stop(void)
{
    GPIO_ResetBits(I2C_PORT, SDA_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SDA_PIN);
    I2C_Delay();
}

/**
 * @brief I2C写入一个字节
 * @details 通过软件模拟I2C时序，逐位发送数据
 *          MSB优先发送（最高位在前）
 *          每位发送流程：
 *          1. 设置SDA数据线电平
 *          2. SCL置高（数据被从设备读取）
 *          3. SCL拉低（准备下一位）
 *          发送完8位后释放SDA线
 * @param data 要发送的字节数据
 */
void I2C_WriteByte(uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) {
            GPIO_SetBits(I2C_PORT, SDA_PIN);
        } else {
            GPIO_ResetBits(I2C_PORT, SDA_PIN);
        }
        I2C_Delay();
        GPIO_SetBits(I2C_PORT, SCL_PIN);
        I2C_Delay();
        GPIO_ResetBits(I2C_PORT, SCL_PIN);
        I2C_Delay();
        data <<= 1;
    }
    GPIO_SetBits(I2C_PORT, SDA_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
}

/**
 * @brief I2C读取一个字节
 * @details 通过软件模拟I2C时序，逐位读取数据
 *          MSB优先读取（最高位在前）
 *          每位读取流程：
 *          1. SCL置高（从设备将数据放到SDA线上）
 *          2. 读取SDA线电平
 *          3. SCL拉低（准备读取下一位）
 *          ACK/NACK控制：
 *          - ack=1: 发送ACK（继续读取）
 *          - ack=0: 发送NACK（结束读取）
 * @param ack 应答信号控制位：1=ACK，0=NACK
 * @return 读取到的字节数据
 */
uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t data = 0;
    
    GPIO_SetBits(I2C_PORT, SDA_PIN);
    for (int i = 0; i < 8; i++) {
        I2C_Delay();
        GPIO_SetBits(I2C_PORT, SCL_PIN);
        I2C_Delay();
        data <<= 1;
        if (GPIO_ReadInputDataBit(I2C_PORT, SDA_PIN)) {
            data |= 0x01;
        }
        GPIO_ResetBits(I2C_PORT, SCL_PIN);
        I2C_Delay();
    }
    
    if (ack) {
        GPIO_ResetBits(I2C_PORT, SDA_PIN);
    } else {
        GPIO_SetBits(I2C_PORT, SDA_PIN);
    }
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_PORT, SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_PORT, SDA_PIN);
    
    return data;
}

/**
 * @brief I2C读取寄存器值
 * @details 标准I2C读取流程：起始→从设备地址(写)→寄存器地址→重复起始→从设备地址(读)→读数据→停止
 *          适用于MPU6500等传感器寄存器读取
 * @param addr 从设备地址（7位地址）
 * @param reg 寄存器地址
 * @param data 数据指针，用于存储读取的数据
 * @return 0表示成功
 */
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *data)
{
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x00);
    I2C_WriteByte(reg);
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x01);
    *data = I2C_ReadByte(0);
    I2C_Stop();
    return 0;
}

/**
 * @brief I2C写入寄存器值
 * @details 标准I2C写入流程：起始→从设备地址(写)→寄存器地址→写数据→停止
 *          适用于MPU6500等传感器寄存器配置
 * @param addr 从设备地址（7位地址）
 * @param reg 寄存器地址
 * @param data 要写入的数据
 */
void I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)
{
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x00);
    I2C_WriteByte(reg);
    I2C_WriteByte(data);
    I2C_Stop();
}

/**
 * @brief I2C连续读取多个字节
 * @details 用于读取传感器的多字节数据（如加速度计/陀螺仪原始数据）
 *          读取过程中连续发送ACK，最后一个字节发送NACK结束
 * @param addr 从设备地址（7位地址）
 * @param reg 起始寄存器地址
 * @param buf 数据缓冲区指针
 * @param len 要读取的字节数
 */
void I2C_ReadMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x00);
    I2C_WriteByte(reg);
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x01);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = I2C_ReadByte(i < len - 1);
    }
    I2C_Stop();
}
