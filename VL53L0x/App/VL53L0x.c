/*******************************************************************************
 * 	VL53L0x.c
 *
 *  Created on: 2020��10��26��
 *
 *  Author: ZhaoSir
 *******************************************************************************/
#include "VL53L0x.h"
#include "Board_GPIO.h" 
#include "Board_Init.h" 


VL53L0X_DeviceInfo_t vl53l0x_dev_info;											// �豸ID�汾��Ϣ
uint8_t AjustOK = 0;															// У׼��־λ

 
/*
 *	VL53L0X������ģʽ����
 *	0��Ĭ��;1:�߾���;2:������;3:����
 */
mode_data Mode_data[]=
{
    {(FixPoint1616_t)(0.25*65536), 
	 (FixPoint1616_t)(18*65536),
	 33000,
	 14,
	 10},																		// Ĭ��
		
	{(FixPoint1616_t)(0.25*65536) ,
	 (FixPoint1616_t)(18*65536),
	 200000, 
	 14,
	 10},																		// �߾���
		
    {(FixPoint1616_t)(0.1*65536) ,
	 (FixPoint1616_t)(60*65536),
	 33000,
	 18,
	 14},																		// ������
	
    {(FixPoint1616_t)(0.25*65536) ,
	 (FixPoint1616_t)(32*65536),
	 20000,
	 14,
	 10},																		// ����
		
};




/********************************************************************************
*               vl53l0x.c
*�������ƣ�	vl53l0x_Addr_set()
*
*�������ã�	����VL53L0X��ַ
*
*����˵����	dev���豸I2C�����ṹ��
*       	newaddr���µ��豸��ַ
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
VL53L0X_Error VL53L0X_SetAddress(VL53L0X_Dev_t *dev, uint8_t newaddr)
{
	uint16_t Id;
	uint8_t FinalAddress;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t sta = 0x00;
	
	FinalAddress = newaddr;
	
	if(FinalAddress == dev->I2cDevAddr)                                         // ���豸I2C��ַ��ɵ�ַһ��,ֱ���˳�
		return VL53L0X_ERROR_NONE;
	//�ڽ��е�һ���Ĵ�������֮ǰ����I2C��׼ģʽ(400Khz)
	Status = VL53L0X_WrByte(dev, 0x88, 0x00);
	if(Status != VL53L0X_ERROR_NONE)
	{
		sta = 0x01;                                                             // ����I2C��׼ģʽ����
		goto set_error;
	}
	//����ʹ��Ĭ�ϵ�0x52��ַ��ȡһ���Ĵ���
	Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
	if(Status != VL53L0X_ERROR_NONE)
	{
		sta = 0x02;                                                             // ��ȡ�Ĵ�������
		goto set_error;
	}
	if(Id == 0xEEAA)
	{
		//�����豸�µ�I2C��ַ
		Status = VL53L0X_SetDeviceAddress(dev, FinalAddress);
		if(Status != VL53L0X_ERROR_NONE)
		{
			sta = 0x03;                                                         // ����I2C��ַ����
			goto set_error;
		}
		//�޸Ĳ����ṹ���I2C��ַ
		dev->I2cDevAddr = FinalAddress;
		//����µ�I2C��ַ��д�Ƿ�����
		Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
		if(Status != VL53L0X_ERROR_NONE)
		{
			sta = 0x04;                                                         // ��I2C��ַ��д����
			goto set_error;
		}	
	}
	set_error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		Status = 1;
	}

	return Status;
}

/********************************************************************************
*               vl53l0x.c
*�������ƣ�	VL53L0x_RESET()
*	
*�������ã�	��ʼ��VL53L0XӲ��
*	
*����˵����	dev���豸I2C�����ṹ��
*	
*�������أ�	��
*	
*�������ߣ�	ZhaoSir
********************************************************************************/
void VL53L0x_RESET(VL53L0X_Dev_t *dev)
{
	uint8_t addr;
	addr = dev->I2cDevAddr;                                                     // �����豸ԭI2C��ַ
    VL53L0X_Xshut(0);                                                           // ʧ��VL53L0X
    DelayMS(30);
	VL53L0X_Xshut(1);                                                           // ʹ��VL53L0X,�ô��������ڹ���(I2C��ַ��ָ�Ĭ��0X52)
	DelayMS(30);
	dev->I2cDevAddr = 0x52;
	VL53L0X_SetAddress(dev,addr);                                                // ����VL53L0X������ԭ���ϵ�ǰԭI2C��ַ
	VL53L0X_DataInit(dev);
}


/********************************************************************************
*               vl53l0x.c
*�������ƣ�	VL53L0X_Init()
*	
*�������ã�	��ʼ��VL53L0XӲ��
*	
*����˵����	dev���豸I2C�����ṹ��
*	
*�������أ�	��
*	
*�������ߣ�	ZhaoSir
********************************************************************************/
VL53L0X_Error VL53L0X_Init(VL53L0X_Dev_t *dev)
{
	/*
	 *	XSHUT 	----->  PB4
	 *	INT		----->  PB5
	 *	I2C_SCL	----->  PB6
	 *	I2C_SDA	----->  PB7
	 */
	VL53L0X_Error Status        = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice    = dev;

	RCC->IOPENR |= 0X02;
	GPIOB->MODER   &= ~(0X3 <<  8);
	GPIOB->MODER   |=  (0X1 <<  8);
	
	GPIOB->OTYPER  &= ~(0X1 <<  4);
	GPIOB->PUPDR   &= ~(0X3 <<  8);
	GPIOB->PUPDR   |=  (0X1 <<  8);
	GPIOB->OSPEEDR &= ~(0X3 <<  8);
	GPIOB->OSPEEDR |=  (0X1 <<  8);	
	
	pMyDevice->I2cDevAddr = VL53L0X_Addr;                                   // I2C��ַ(�ϵ�Ĭ��0x52)
	pMyDevice->comms_type = 1;                                              // I2Cͨ��ģʽ
	pMyDevice->comms_speed_khz = 400;                                       // I2Cͨ������
	
	/*  ��ʼ��IIC����ص�����  */
	VL53L0X_i2c_init();
	VL53L0X_Xshut(0);                                                       // ʧ��VL53L0X
	DelayMS(30);
	VL53L0X_Xshut(1);                                                       // ʹ��VL53L0X,�ô��������ڹ���
	DelayMS(30);
	
	Status = VL53L0X_SetAddress(pMyDevice, 0x54);                           // ����VL53L0X������I2C��ַ
    if(Status != VL53L0X_ERROR_NONE)    goto error;
	Status = VL53L0X_DataInit(pMyDevice);                                   //�豸��ʼ��
	if(Status != VL53L0X_ERROR_NONE)    goto error;
	DelayMS(2);

	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info);           // ��ȡ�豸ID��Ϣ
    if(Status!=VL53L0X_ERROR_NONE) goto error;
/********************************************************************/
//	AT24CXX_Read(0,(uint8_t*)&Vl53l0x_data,sizeof(_vl53l0x_adjust));//��ȡ24c02�����У׼����,����У׼ Vl53l0x_data.adjustok==0xAA
/********************************************************************/

	if(Vl53l0x_data.adjustok == 0xAA)                               		// ��У׼
	  AjustOK=1;	
	else                                                            		// ûУ׼
	  AjustOK=0;
	
	error:
	if(Status!=VL53L0X_ERROR_NONE)
	{
		return Status;
	}
	return Status;
}





