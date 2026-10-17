#include "stm32f10x.h"

void delay(u32 count)
{
	u32 i;
	for(i=0;i<count;i++);
}
int main(void)
{
	GPIO_InitTypeDef s;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD,ENABLE);
	
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&s);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	
	s.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD,&s);
	GPIO_SetBits(GPIOD,GPIO_Pin_2);
	
	while(1)
	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	  GPIO_SetBits(GPIOA,GPIO_Pin_8);
		delay(30000000);
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	  GPIO_SetBits(GPIOD,GPIO_Pin_2);
		delay(30000000);
	}
}
