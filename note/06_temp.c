#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "tsensor.h"

int main(void)
{
    u16 adcx;
		u32 t100; 
	  u32 mv;
    delay_init();
	//调用延时函数
    uart_init(9600);
	//设置端口接收比特
    LCD_Init();
	//调用lcd屏函数
    T_Adc_Init();
	//内部传感器ADC初始化函数
    while(1)
    {
      adcx = T_Get_Adc_Average(ADC_CH_TEMP, 10);
			//读取 ADC 通道（这里是片内温度传感器通道），连续采 10 次 ADC 值，求平均值，结果赋值给 adcx
      LCD_ShowxNum(60, 60, adcx, 4, 16, 0);//显示adc值
			mv = adcx * 3300/4096;
			//转换成毫伏mv
			//(x, y, 数字, 显示位数, 字号, 填充0标志)
			LCD_ShowxNum(60,90,mv/1000,1,16,0);//显示电压1.393，这里显示的是1
			LCD_ShowxNum(76,90,mv%1000,3,16,0x80);//这里显示的是393
      delay_ms(250);	
			
			t100 = (1430 - mv)* 1000/43 + 2500;
			///* mv=1388 → 3476 */
			LCD_ShowxNum(60,120,t100/100,2,16,10);
			//除出来整数部分34
			LCD_ShowxNum(84,120,t100%100,2,16,0x80);
			//得出小数部分
			//这一块显示温度
    }
}


------------------------------------------------------------------------------------

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "stm32f10x.h"

void Adc_Init(void)
//定义自己的 ADC 初始化函数，名字叫 Adc_Init
{
	ADC_InitTypeDef s;
	//定义一个 ADC 初始化结构体变量 s
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
    //开启 ADC1 的时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    //设置 ADC 时钟为 PCLK2 的 1/6，PCLK2=72Mhz，ADC 时钟=12Mhz
	ADC_DeInit(ADC1);
    //把 ADC1 恢复成上电默认状态，清空之前配置，防止残留配置捣乱
	
	s.ADC_Mode = ADC_Mode_Independent;
    //ADC 模式：独立模式。只有 ADC1 单独工作，不跟 ADC2 联动。
	s.ADC_ScanConvMode = DISABLE;
    //扫描转换模式：关闭。扫描是一口气连续采集多个通道；我们只采 1 个通道，关掉。
	s.ADC_ContinuousConvMode = DISABLE;
    //连续转换模式：关闭。连续模式会自动反复采样；我们需要需要的时候手动触发一次。
	s.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    //外部触发源：不用外部触发。不用定时器、IO 触发 ADC，只用软件触发。
	s.ADC_DataAlign = ADC_DataAlign_Right;
    //数据对齐方式：右对齐。ADC 采集到的 12 位数据，放在 16 位寄存器的低 12 位，高 4 位清零。
	s.ADC_NbrOfChannel = 1;
    //设置要采集的通道数：1 个。我们只采 1 个通道。
	ADC_Init(ADC1,&s);
    //把上面配置好的结构体 s，传给 ADC_Init 函数，完成 ADC1 的初始化。
	
	ADC_TempSensorVrefintCmd(ENABLE);
    //打开片内温度传感器和 Vrefint 通道。Vrefint 是芯片内部的 1.2V 稳压参考电压。
	ADC_Cmd(ADC1,ENABLE);
    //打开 ADC1
	
	ADC_ResetCalibration(ADC1);
    //复位校准寄存器，清空之前的校准结果
	while(ADC_GetResetCalibrationStatus(ADC1));
    //等待复位校准寄存器完成
	ADC_StartCalibration(ADC1);
    //开始 ADC1 的校准
	while(ADC_GetCalibrationStatus(ADC1));
    //等待 ADC1 校准完成
}

u16 Get_Adc(u8 ch)
//定义自己的 ADC 采集函数，名字叫 Get_Adc，参数是要采集的通道号 ch
{
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
    //配置 ADC1 的规则通道，采集通道 ch，采样时间 239.5 个 ADC 时钟周期
    // 参数1：ADC1          选择哪个ADC外设
    // 参数2：ch            通道号（你这里传16，温度传感器通道）
    // 参数3：1             这个通道在转换序列里的次序：第1个转换
    // 参数4：采样时间
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
    //软件触发 ADC1 开始转换
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
    //等待 ADC1 转换结束标志位 EOC 置位
	return ADC_GetConversionValue(ADC1);
    //返回 ADC1 的转换结果
}
u16 Get_Adc_Average(u8 ch,u8 times)
//定义自己的 ADC 采集平均值函数，名字叫 Get_Adc_Average，参数是要采集的通道号 ch 和采样次数 times
{
	u32 sum = 0;
    //定义一个无符号32位整型变量 sum，用来存储采样值的总和
	u8 t;
    //定义一个无符号8位整型变量 t，用来做循环计数
	for(t=0;t<times;t++)//循环采样 times 次
	{
		sum += Get_Adc(ch);//调用 Get_Adc 函数，采集一次 ADC 值，并累加到 sum 里
		delay_ms(5);//  延时 5ms，等待 ADC 稳定
	}
	return sum/times;//返回平均值
}
int main(void)
{
    u16 adcx;//定义一个无符号16位整型变量 adcx，用来存储 ADC 采样值
		u32 t100; //定义一个无符号32位整型变量 t100，用来存储温度值
	  u32 mv;//定义一个无符号32位整型变量 mv，用来存储电压值
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
      adcx = Get_Adc_Average(16, 10);//
	//- 第 1 个参数 `16`：ADC 通道 16（STM32F1 内部温度传感器专用通道）
    //- 第 2 个参数 `10`：一共采集 10 次
	//读取 ADC 通道（这里是片内温度传感器通道），连续采 10 次 ADC 值，求平均值，结果赋值给 adcx
      LCD_ShowxNum(60, 60, adcx, 4, 16, 0);//显示adc值
			mv = adcx * 3300/4096;
			//转换成毫伏mv
			//(x, y, 数字, 显示位数, 字号, 填充0标志)
			LCD_ShowxNum(60,90,mv/1000,1,16,0);//显示电压1.393，这里显示的是1
			LCD_ShowxNum(76,90,mv%1000,3,16,0x80);//这里显示的是393
      delay_ms(250);	
			
			t100 = (1430 - mv)* 1000/43 + 2500;
			LCD_ShowxNum(60,120,t100/100,2,16,10);
			LCD_ShowxNum(84,120,t100%100,2,16,0x80);
    }
}


----------------------------------------------------------------------

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
