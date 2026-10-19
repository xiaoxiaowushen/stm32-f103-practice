#include "stm32f10x.h"
#include "usart.h"

void delay(u32 count)
{
	u32 i;
	for(i=0;i<count;i++);
}
volatile u8 key_flag = 0;         

void EXTI9_5_IRQHandler(void)
{
	key_flag = 1;
	EXTI_ClearITPendingBit(EXTI_Line5);
}

int main(void)
{
	
	u8 on =0;
	GPIO_InitTypeDef s;
	NVIC_InitTypeDef p;
	EXTI_InitTypeDef q;
	
	uart_init(9600); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&s);
	
	s.GPIO_Pin = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&s);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
	
	q.EXTI_Line = EXTI_Line5;
	q.EXTI_Mode = EXTI_Mode_Interrupt;
	q.EXTI_Trigger = EXTI_Trigger_Falling;
	q.EXTI_LineCmd = ENABLE;
	EXTI_Init(&q);
	
	p.NVIC_IRQChannel = EXTI9_5_IRQn;
	p.NVIC_IRQChannelPreemptionPriority = 2;
	p.NVIC_IRQChannelSubPriority = 1;
	p.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&p);
	
	while(1)
	{
		if(key_flag)
		{
			key_flag = 0;
			delay(1000000);
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0)
			{
				
				on = !on;
				printf("pressed,on = %d\r\n",on);
				if(on)GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				else GPIO_SetBits(GPIOA,GPIO_Pin_8);
				while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0);
			}
		}
	}
}

