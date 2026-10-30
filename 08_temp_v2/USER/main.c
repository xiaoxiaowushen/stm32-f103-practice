#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "myiic.h"
#include "24cxx.h"
#include "adc.h"
#include "stm32f10x.h"

void led_init(void)
{
	GPIO_InitTypeDef s;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	s.GPIO_Pin   = GPIO_Pin_8;
	s.GPIO_Mode  = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &s);

	s.GPIO_Pin  = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &s);

	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

int main(void)
{
	static u8 blink = 0;
	u16 adcx;
	u32 t100;
	u32 mv;
	u32 t_max;
	u32 t_min;
	u32 last_max;        
	u32 last_min;
	
	led_init();
	delay_init();
	uart_init(9600);
	LCD_Init();
	adc_init();
	IIC_Init();

	
	t_max = (AT24CXX_ReadOneByte(0) << 8) | AT24CXX_ReadOneByte(1);
	t_min = (AT24CXX_ReadOneByte(2) << 8) | AT24CXX_ReadOneByte(3);

	if(t_max == 0xFFFF || t_max < 2000 || t_max > 5000) t_max = 0;
	if(t_min == 0xFFFF || t_min < 2000 || t_min > 5000) t_min = 99999;
	
	last_max = t_max;
	last_min = t_min;

	LCD_ShowString(60, 170, 200, 16, 16, "MAX:");
	//显示函数
	LCD_ShowString(60, 190, 200, 16, 16, "MIN:");

	while(1)
	{
		adcx = Get_adc_average(16, 10);
		mv   = adcx * 3300 / 4096;
		//毫伏计算公式
		t100 = (1430 - mv) * 1000 / 43 + 2500;
		//计算转换公式


		if(t100 > t_max) t_max = t100;
		//如果t100>t_max 那么最大值就是t100
		if(t100 < t_min) t_min = t100;
		//如果t100<t_min 最小值就是t100
		//这里就是为了让最大最小值实时变化

		if(t_max != last_max)
	{
		last_max = t_max;
		AT24CXX_WriteOneByte(0, t_max >> 8);
		AT24CXX_WriteOneByte(1, t_max & 0xFF);
	}
	if(t_min != last_min)
	{
		last_min = t_min;
		AT24CXX_WriteOneByte(2, t_min >> 8);
		AT24CXX_WriteOneByte(3, t_min & 0xFF);
	}
	
		LCD_ShowxNum(60, 60, adcx, 4, 16, 0);
		//显示adcx的值

		LCD_ShowxNum(60, 90, mv/1000, 1, 16, 0);
		LCD_ShowxNum(76, 90, mv%1000, 3, 16, 0x80);


		if(t100 > 3500)//温度阈值35°
		{
			BACK_COLOR  = RED;
			POINT_COLOR = WHITE;

			blink = !blink;
			if(blink) GPIO_ResetBits(GPIOA, GPIO_Pin_8);
			else      GPIO_SetBits(GPIOA, GPIO_Pin_8);
		}
		else
		{
			BACK_COLOR  = WHITE;
			POINT_COLOR = BLUE;

			GPIO_SetBits(GPIOA, GPIO_Pin_8);
			blink = 0;
		}

		LCD_ShowxNum(60, 120, t100/100, 2, 16, 0);
		LCD_ShowxNum(84, 120, t100%100, 2, 16, 0x80);

		LCD_ShowxNum(100, 170, t_max/100, 2, 16, 0);
		LCD_ShowxNum(124, 170, t_max%100, 2, 16, 0x80);

		if(t_min == 99999)
		{
			LCD_ShowString(100, 190, 200, 16, 16, "--.--");
		}
		else
		{
			LCD_ShowxNum(100, 190, t_min/100, 2, 16, 0);
			LCD_ShowxNum(124, 190, t_min%100, 2, 16, 0x80);
		}

		delay_ms(250);
	}
}
