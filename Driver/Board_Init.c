/*******************************************************************************
 * Board_Init.c
 *
 *  Created on: 2020��10��26��
 *
 *  Author: ZhaoSir
 *******************************************************************************/

#include "Board_Init.h"
#include "Board_GPIO.h"

#define  		RTC_INT_PRIORITY           	((uint32_t)4U)    					/*!< RTC interrupt priority */
#define  		TICK_INT_PRIORITY           ((uint32_t)3U)    					/*!< tick interrupt priority */
#define  		USART1_INT_PRIORITY         ((uint32_t)2U)    					/*!< USART1 interrupt priority */
#define  		SYSCLOCK_FRE				4194304
static void 	USART1_Init(void);
static void 	SYSTICK_Init(void);
static void 	SYS_IncTick(void);
static void 	RTC_AWU_Init(void);

static __IO uint32_t uwTick;
bool	awuFlag = true;
/********************************************************************************
*               Board_Init.c
*�������ƣ�	Board_Init()
*
*�������ã�	�������Ӳ����ʼ��
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
void Board_Init(void)
{
	GPIO_Init();
	USART1_Init();
	SYSTICK_Init();
	RTC_AWU_Init();
}


/********************************************************************************
*               Board_Init.c
*�������ƣ�	RTC_IRQHandler()
*
*�������ã�	RTC�жϷ�����
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
void RTC_IRQHandler(void)
{
	if(RTC->ISR & (0X1 << 10)) 
	{
		RTC->ISR  &= ~(0X1 << 10);
		awuFlag = 1- awuFlag;
	}
}


/********************************************************************************
*               Board_Init.c
*�������ƣ�	SysTick_Handler()
*
*�������ã�	Systick�жϷ�����
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
void SysTick_Handler(void)
{
	SYS_IncTick();
}

/********************************************************************************
*               Board_Init.c
*�������ƣ�	USART1_Init()
*
*�������ã�	����1��ʼ����������115200
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
static void USART1_Init(void)
{
	uint32_t regTmp = 0;
	uint32_t Baud = 0;
	RCC->APB2ENR |= (0X1 << 14);
	RCC->IOPENR  |= 0X01;
	
	GPIOA->MODER &= ~(0X0F << 18);
	GPIOA->MODER |=  (0X0A << 18);
	
	GPIOA->AFR[1] &= ~(0XFF << 4);
	GPIOA->AFR[1] |=  (0X44 << 4);
	
	regTmp |= (0X1 << 5);														/* RX interrput enable */
	regTmp |= (0X1 << 3);														/* Transmitter is enable */
	regTmp |= (0X1 << 2);														/* Receiver Enable */
	USART1->CR1 = regTmp;
	
	Baud = SYSCLOCK_FRE / 9600;
	USART1->BRR = Baud;
	USART1->CR1 |= (0X1 << 0);													/* USRAT1 Enable */
	NVIC_SetPriority(USART1_IRQn,	USART1_INT_PRIORITY);
}



/********************************************************************************
*               Board_Init.c
*�������ƣ�	SYSTICK_Init()
*
*�������ã�	Systick��ʼ����������ʱ
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
static void SYSTICK_Init(void)
{
	SysTick->LOAD = ((SYSCLOCK_FRE / 1000) - 1);
	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); 			/* set Priority for Systick Interrupt */
	SysTick->VAL = 0UL;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk   |
					SysTick_CTRL_ENABLE_Msk;
	
	NVIC_SetPriority(SysTick_IRQn,	TICK_INT_PRIORITY);
}

/********************************************************************************
*               Board_Init.c
*�������ƣ�	USART1_TX()
*
*�������ã�	���ڷ��ͺ���
*
*����˵����	buff���������ݵ�ָ�룻
*			size���������ݵĴ�С��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
static void RTC_AWU_Init(void)
{
	uint16_t x = 0X7FF;
	
	PWR->CR |=  (0X1 << 8);														// Disable backup write protection
	RCC->CSR |=  (0X1 << 0);													// Enable LSI clocks
	while(x-- & (!(RCC->CSR & 0X2)));
	RCC->CSR |=  (0X1 << 19);													// Reset RTC
	x = 0X7FF;
	while(x--);
	RCC->CSR &= ~(0X1 << 19);													// Finish Reset RTC
	
	RCC->CSR |=  (0X1 << 18);													// Enable RTC Clock
	
	RTC->WPR = 0XCA;
	RTC->WPR = 0X53;
	RCC->CSR &= ~(0X3 << 16);													// CLEAR_BIT
	RCC->CSR |=  (0X2 << 16);													// Config LSI oscillator clock used as RTC clock
	
	EXTI->IMR	|= (0X1 << 20);
	EXTI->RTSR  |= (0X1 << 20);
	EXTI->PR 	|= (0X1 << 20);
	
	RTC->CR &= ~(0X1 << 10);													// Disable Auto  Wakeup Timer
	while(!(RTC->ISR & 0X04)){};												// Waitting for access to config  Wakeup timer register
	RTC->CR &= ~(0X7 << 0);														// RTCCLK(LSI 37Khz) / 16 as Auto Wakeup time clock
	RTC->CR |=  (0X1 << 14);													// Eable Auto  Wakeup Timer interrupt
	RTC->WUTR = 2312-1;															// 37000/16/2312 = 1S
	RTC->CR |=  (0X1 << 10);
	NVIC_SetPriority(RTC_IRQn,	RTC_INT_PRIORITY);
	NVIC_EnableIRQ(RTC_IRQn);
}



/********************************************************************************
*               Board_Init.c
*�������ƣ�	USART1_TX()
*
*�������ã�	���ڷ��ͺ���
*
*����˵����	buff���������ݵ�ָ�룻
*			size���������ݵĴ�С��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
void USART1_TX(const char* buff, uint16_t size)
{
	while(size--)
	{
		while(!(USART1->ISR & (0X1 << 7))){}
			USART1->TDR = *buff++;
	}
	while(!(USART1->ISR & (0X1 << 7))){};
}
 

/********************************************************************************
*               Board_Init.c
*�������ƣ�	HAL_IncTick()
*
*�������ã�	����ϵͳ���ĺ���
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
static void SYS_IncTick(void)
{
	uwTick++;
}

/********************************************************************************
*               Board_Init.c
*�������ƣ�	HAL_GetTick()
*
*�������ã�	��ȡϵͳʱ�ӽ��ĺ���
*
*����˵����	��
*
*�������أ�	��
*
*�������ߣ�	ZhaoSir
********************************************************************************/
uint32_t SYS_GetTick(void)
{
	return uwTick;
}






