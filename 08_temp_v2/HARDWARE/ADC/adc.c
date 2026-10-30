#include "adc.h"
#include "delay.h"

void adc_init(void)
{
	ADC_InitTypeDef s;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                      
	ADC_DeInit(ADC1);                                      

	s.ADC_Mode               = ADC_Mode_Independent;       
	s.ADC_ScanConvMode       = DISABLE;
	s.ADC_ContinuousConvMode = DISABLE;
	s.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	s.ADC_DataAlign          = ADC_DataAlign_Right;
	s.ADC_NbrOfChannel       = 1;
	ADC_Init(ADC1, &s);                                    

	ADC_TempSensorVrefintCmd(ENABLE);                      
	ADC_Cmd(ADC1, ENABLE);                                 

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

u16 Get_adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}


u16 Get_adc_average(u8 ch, u8 times)
{
	u32 sum = 0;
	u8 t;

	for(t = 0; t < times; t++)
	{
		sum += Get_adc(ch);
		delay_ms(5);
	}
	return sum / times;
}
