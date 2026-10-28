#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "stm32f10x.h"

void led_init(void)
{
	GPIO_InitTypeDef s;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&s);
	s.GPIO_Pin = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&s);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_5);
	
}

void adc_init(void)
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
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	
	
}

u16 Get_adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}
u16 Get_adc_average(u8 ch,u8 times)
{
	u32 sum = 0;
	u8 t ;
	for(t = 0;t<times;t++)
	{
		sum += Get_adc(ch);
		delay_ms(5);
	}
	return sum/times;
}

int main(void)
{
	static u8 blink = 8;
	u16 adcx;
	u32 t100;
	u32 mv;
	
	led_init();
	delay_init();
	uart_init(9600);
	LCD_Init();
	adc_init();
	
	while(1)
	{
		adcx = Get_adc_average(16,10);
		//ADC_Channel_16 是内部温度传感器通道
		mv = adcx* 3300/4096;
		//把 ADC 数字值，转换成温度传感器输出电压（单位：mV）
		t100 = (1430 - mv)* 1000/43 + 2500;
		//\(V_{temp}=1430 - 4.3\times(T-25)\)
		LCD_ShowxNum(60,60,adcx,4,16,0);
		
		LCD_ShowxNum(60,90,mv/1000,1,16,0);
		
		LCD_ShowxNum(76,90,mv%1000,3,16,0x80);
		
		if(t100>3500)
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
