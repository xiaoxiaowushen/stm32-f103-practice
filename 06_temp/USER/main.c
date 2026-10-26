#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "stm32f10x.h"

void LED_Init(void)
{
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
}

void Adc_Init(void)
{
	ADC_InitTypeDef s;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);
	
	s.ADC_Mode = ADC_Mode_Independent;
	s.ADC_ScanConvMode = DISABLE;
	s.ADC_ContinuousConvMode = DISABLE;
	s.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	s.ADC_DataAlign = ADC_DataAlign_Right;
	s.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1,&s);
	
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_Cmd(ADC1,ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

u16 Get_Adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 sum = 0;
	u8 t;
	for(t=0;t<times;t++)
	{
		sum += Get_Adc(ch);
		delay_ms(5);
	}
	return sum/times;
}
int main(void)
{
		static u8 blink = 0;
    u16 adcx;
		u32 t100; 
	  u32 mv;
	
		LED_Init();
    delay_init();
	//调用延时函数
    uart_init(9600);
	//设置端口接收比特
    LCD_Init();
	//调用lcd屏函数
	
    Adc_Init();
	
	//内部传感器ADC初始化函数
    while(1)
    {
      adcx = Get_Adc_Average(16, 10);
			
			//读取 ADC 通道（这里是片内温度传感器通道），连续采 10 次 ADC 值，求平均值，结果赋值给 adcx
			mv = adcx * 3300/4096;
			t100 = (1430 - mv) * 1000 / 43 + 2500;
			//转换成毫伏mv
			//(x, y, 数字, 显示位数, 字号, 填充0标志)
			LCD_ShowxNum(60, 60, adcx, 4, 16, 0);//显示adc值
			LCD_ShowxNum(60,90,mv/1000,1,16,0);//显示电压1.393，这里显示的是1
			LCD_ShowxNum(76,90,mv%1000,3,16,0x80);//这里显示的是393
			
			
			if(t100>3400)
			{
				BACK_COLOR = RED;
				POINT_COLOR = WHITE;
				
				blink =! blink;
				if(blink)GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				else GPIO_SetBits(GPIOA,GPIO_Pin_8);
			}
			else
			{
				BACK_COLOR = WHITE;
				POINT_COLOR = BLUE;
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
				blink = 0;
			}
			LCD_ShowxNum(60,120,t100/100,2,16,0);
			LCD_ShowxNum(84,120,t100%100,2,16,0x80);
			delay_ms(250);
    }
}
