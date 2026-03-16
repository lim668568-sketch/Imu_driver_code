#ifndef __MPU6500_H
#define __MPU6500_H

#include "stdint.h"
#include "math.h"
#include "I2C.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6500_ADDR            0x68
#define MPU6500_WHO_AM_I        0x75
#define MPU6500_ACCEL_XOUT_H    0x3B
#define MPU6500_GYRO_XOUT_H     0x43
#define MPU6500_PWR_MGMT_1      0x6B
#define MPU6500_ACCEL_CONFIG    0x1C
#define MPU6500_GYRO_CONFIG     0x1B

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
} MPU6500_Data_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} EulerAngle_t;
extern uint8_t mpu_data_ready;
extern volatile MPU6500_Data_t mpu6500_data;
extern volatile EulerAngle_t euler_angle;

void MPU6500_Init(void);
uint8_t MPU6500_ReadReg(uint8_t reg);
void MPU6500_WriteReg(uint8_t reg, uint8_t data);
void MPU6500_ReadData(void);
void Mahony_Filter(float dt);
void Euler_Calculate(void);

#ifdef __cplusplus
}
#endif

#endif
