#include "adc.h"
#include "delay.h"

void adc_init(void)
{
	ADC_InitTypeDef s;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);   /* 1. 开 ADC1 时钟 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                      /* 2. 72MHz / 6 = 12MHz */
	ADC_DeInit(ADC1);                                      /* 3. 复位 */

	s.ADC_Mode               = ADC_Mode_Independent;       /* 4. 填 6 个字段 */
	s.ADC_ScanConvMode       = DISABLE;
	s.ADC_ContinuousConvMode = DISABLE;
	s.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	s.ADC_DataAlign          = ADC_DataAlign_Right;
	s.ADC_NbrOfChannel       = 1;
	ADC_Init(ADC1, &s);                                    /* 5. 提交 */

	ADC_TempSensorVrefintCmd(ENABLE);                      /* 6. 开内部温度传感器 */
	ADC_Cmd(ADC1, ENABLE);                                 /* 7. 使能 ADC1 */

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

/*
 * 读一个通道一次
 * 三段式：选通道 -> 启动转换 -> 等结果 -> 取值
 */
u16 Get_adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}

/*
 * 读 times 次取平均（ADC 读数会抖动，平均后稳定）
 */
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
