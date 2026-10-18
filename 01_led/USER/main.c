#include "stm32f10x.h"

void delay(u32 count)
{
	u32 i;
	for(i=0;i<count;i++);
}
int main(void)
{
	u8 on =0;
	GPIO_InitTypeDef s;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&s);
	
	s.GPIO_Pin = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&s);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	while(1)
	{
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0)
		{
			delay(1000000);
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0)
			{
				on = !on;
				if(on)GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				else GPIO_SetBits(GPIOA,GPIO_Pin_8);
				while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0);
			}
		}
	}
}

