#include "stm32f10x.h"
#include "usart.h"
#include "delay.h" 

u32 seconds = 0;
u8 running =0;
volatile u8 key_flag = 0;

void TIM3_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef t;
	NVIC_InitTypeDef n;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	t.TIM_Period = arr;
	t.TIM_Prescaler = psc;
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3,&t);
	
	TIM_ITConfig(
	TIM3,
	TIM_IT_Update,
	ENABLE);
	
	n.NVIC_IRQChannel = TIM3_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 3;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
	
	TIM_Cmd(TIM3,ENABLE);
}
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!= RESET)
	{
		GPIOA -> ODR ^= (1<<8);
		if(running)seconds++;
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}
void EXTI9_5_IRQHanler(void)
{
	key_flag = 1;
	EXTI_ClearITPendingBit(EXTI_Line5);
}

int main(void)
{
	u16 tick = 0;
	NVIC_InitTypeDef n;
	GPIO_InitTypeDef g;
	EXTI_InitTypeDef e;
	
	delay_init();
	uart_init(9600);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	g.GPIO_Pin = GPIO_Pin_8;
	g.GPIO_Mode = GPIO_Mode_Out_PP;
	g.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&g);
	
	g.GPIO_Pin = GPIO_Pin_5;
	g.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&g);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);
	//端口来源，选是哪一组 GPIO,引脚来源，选这一组里第几个脚s
	
	e.EXTI_Line = EXTI_Line5;
	e.EXTI_Mode = EXTI_Mode_Interrupt;
	e.EXTI_Trigger = EXTI_Trigger_Falling;
	e.EXTI_LineCmd = ENABLE;
	
	EXTI_Init(&e);
	
	n.NVIC_IRQChannel = EXTI9_5_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 3;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
	
	TIM3_Init(9999,7199);
	
	while(1)
	{
		if(key_flag)
		{
			key_flag = 0;
			delay_ms(10);
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) == 0)
			{
				running =! running;
			}
			tick++;
			if(tick >=10)
			{
				tick = 0;
				printf("time=%d s\r\n",seconds);
			}
			delay_ms(100);
		}
	}
}
