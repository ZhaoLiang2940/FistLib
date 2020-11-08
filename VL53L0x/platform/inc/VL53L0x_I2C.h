#ifndef __VL53L0_I2C_H
#define __VL53L0_I2C_H
#include "stdint.h"
#include "stm32l0xx.h"

#define			VL_READ_SDA				((GPIOB->IDR & (0X1 << 7)) ? 1 : 0)
#define			VL_SDA_IN()				(GPIOB->MODER &= ~(0X3 << 12))
#define			VL_SDA_OUT() 			(GPIOB->MODER |=  (0X1 << 12))
#define			VL_IIC_SDA_H			(GPIOB->ODR |=  (0X1 << 7))
#define			VL_IIC_SDA_L			(GPIOB->ODR &= ~(0X1 << 7))
#define			VL_IIC_SCL_H			(GPIOB->ODR |=  (0X1 << 6))
#define			VL_IIC_SCL_L			(GPIOB->ODR &= ~(0X1 << 6))
#define 		VL53L0x_DelayUS(x)


//״̬
#define STATUS_OK       0x00
#define STATUS_FAIL     0x01

//IIC��������
extern  void VL53L0X_i2c_init(void);//��ʼ��IIC��IO��
extern  uint8_t VL53L0X_write_byte(uint8_t address,uint8_t index,uint8_t data);              //IICдһ��8λ����
extern  uint8_t VL53L0X_write_word(uint8_t address,uint8_t index,uint16_t data);             //IICдһ��16λ����
extern  uint8_t VL53L0X_write_dword(uint8_t address,uint8_t index,uint32_t data);            //IICдһ��32λ����
extern  uint8_t VL53L0X_write_multi(uint8_t address, uint8_t index,uint8_t *pdata,uint16_t count);//IIC����д

extern  uint8_t VL53L0X_read_byte(uint8_t address,uint8_t index,uint8_t *pdata);             //IIC��һ��8λ����
extern  uint8_t VL53L0X_read_word(uint8_t address,uint8_t index,uint16_t *pdata);            //IIC��һ��16λ����
extern  uint8_t VL53L0X_read_dword(uint8_t address,uint8_t index,uint32_t *pdata);           //IIC��һ��32λ����
extern  uint8_t VL53L0X_read_multi(uint8_t address,uint8_t index,uint8_t *pdata,uint16_t count);  //IIC������



#endif 


