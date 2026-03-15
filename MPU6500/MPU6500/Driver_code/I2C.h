#ifndef __I2C_H
#define __I2C_H

#include "stdint.h"
#include "gpio.h"

#define SCL_PIN   GPIO_PIN_6
#define SDA_PIN   GPIO_PIN_7
#define I2C_PORT  GPIOB

void I2C_Delay(void);
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WriteByte(uint8_t data);
uint8_t I2C_ReadByte(uint8_t ack);
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *data);
void I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
void I2C_ReadMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif
