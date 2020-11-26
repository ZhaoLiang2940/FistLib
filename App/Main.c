/*******************************************************************************
 * MyCode.c
 *
 *  Created on: 2020��10��26��
 *
 *  Author: ZhaoSir
 *******************************************************************************/
#include "stm32l0xx.h"
#include "stdbool.h"
#include "Board_Init.h" 
#include "Board_GPIO.h" 
#include "VL53L0x.h"

#define			SamplePeriod   2

uint8_t TXBuff[7] = {0X5A, 0X00, 0X00, 0X00, 0XFE, 0XFE, 0XA5};
bool 	bitVal = false;
uint32_t testTimes = 2;
int fuck = 10;

void delay(us)
{
	for(int i = 0; i < us; us--)
	{
		__NOP();
	}
}

uint16_t Distance_data = 0;
VL53L0X_Dev_t vl53l0x_dev;														// �豸I2C���ݲ���
VL53L0X_RangingMeasurementData_t VL53L0x_Data;
/********************************************************************************
*               main.c
*�������ƣ�	main()
*
*�������ã�	����������
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
int main(void)
{
	uint32_t SysTickCount = 0UL;
	Board_Init();
	VL53L0X_Init(&vl53l0x_dev);
	GPIO_SET_BIT(GPIOA, 15, 0);
	Systick_DelayMs(5000); 
	GPIO_SET_BIT(GPIOA, 15, 1);
	while(1)
	{		
		if((SYS_GetTick() - SysTickCount) == SamplePeriod)
		{
			GPIO_SET_BIT(GPIOA, 15, 1);
			readVL5Ll0x_PollingtMode(&vl53l0x_dev, &VL53L0x_Data, 0, &Distance_data);
		}
		else if((SYS_GetTick() - SysTickCount) > SamplePeriod)
		{
			GPIO_SET_BIT(GPIOA, 15, 0);
			SysTickCount = SYS_GetTick();
			TXBuff[3] = (Distance_data >> 8) & 0XFF;
			TXBuff[4] = (Distance_data >> 0) & 0XFF;
			TXBuff[5] = TXBuff[4] + TXBuff[3];
			USART2_TX((const char*)&TXBuff, sizeof(TXBuff));
			bitVal = 1 - bitVal;
			GPIO_SET_BIT(GPIOA, 15, 1);
		}
		else
			__WFI;
	}
}





/* 
 RTC ����1Sÿ�Σ�
	��������ģʽ�£�
		��������λ����һ����λ1S����һ�Σ��ڶ�����λ500MS����һ�Σ���������λ100MS����һ�Σ� ����5S�������п����͹ػ�������֮�󣬶̰�1S����������ģʽ���л��������������ڣ�
	����״̬�³�������5S����ػ���
	�ػ�״̬�£�������Դ����5S���뿪���������͹ػ�������˸5��LED��ÿ���л��������ڵ�ʱ����˸һ��LED��

	����ɨ������Ϊ5S��ÿ�ι㲥����1S�����1S��û�����ӣ��ر�������5S֮���ٴι㲥��
	��������֮��͹㲥�������ݣ�

	�����Ĺ㲥�����STM32������STM32ͨ��WKUP���ž����������ߺ͹㲥��ͬʱ����������״̬Ҳ��ͨ�����ŷ�����STM32���Ӷ�������������
*/


