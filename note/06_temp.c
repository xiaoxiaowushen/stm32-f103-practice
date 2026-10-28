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
// > PCLK = **Peripheral Clock，外设总线时钟**

// STM32F1 有两条 APB 外设总线，对应两个 PCLK：

// 1. **PCLK1**：APB1 低速总线时钟，上限 36MHz
// 挂载：USART2/3、I2C、SPI2、TIM2~7
// 2. **PCLK2**：APB2 高速总线时钟，上限 72MHz
// 挂载：GPIO、USART1、SPI1、**ADC1/ADC2**
//意思是配置这个时钟通道 来源选 PCLK2，然后再 6 分频
//PCLK2 = 72MHz
//ADCCLK = 72MHz / 6 = 12MHz

	ADC_DeInit(ADC1);
//把 ADC1 所有相关寄存器恢复成上电默认值，清空之前的配置。
	
	s.ADC_Mode = ADC_Mode_Independent;
//> **独立模式：ADC1 和 ADC2 互相独立，互不影响，各自单独工作。**

	s.ADC_ScanConvMode = DISABLE;
//扫描转换（Scanconvmode）模式 = 能不能自动依次扫描多个通道。

// - ENABLE（开启扫描）：你配置好一组通道序列，ADC 转换完第 1 个通道，**自动接着转换下一个**，循环 / 顺序跑完通道列表，适合多通道连续采集。
// - DISABLE（关闭扫描）：**只转换你当前选中的那 1 个通道**。转换完成一次就停，不会自动切下一个通道。

	s.ADC_ContinuousConvMode = DISABLE;
//连续转换（Continuousconv）模式 = 能不能自动重复采样

	s.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//这个成员用来选择 **ADC 规则组转换的触发源**
// > 不使用外部硬件触发，采用软件触发
// > External 外部，Trig 触发，Conv 转换
// > 意思：**用什么信号来启动 ADC 的转换**
	s.ADC_DataAlign = ADC_DataAlign_Right;
//数据对齐方式 = ADC 采集到的 12 位数据，放在 16 位寄存器的低 12 位，高 4 位清零。
	s.ADC_NbrOfChannel = 1;
//`NbrOfChannel` = Number Of Channel，**通道数量**
//含义：**规则转换序列里，一共有 1 个通道要转换**
	ADC_Init(ADC1,&s);
//提交ADC初始化结构体，完成 ADC1 的初始化
	
	ADC_TempSensorVrefintCmd(ENABLE);
//> 打开片内温度传感器和 Vrefint 通道。Vrefint 是芯片内部的 1.2V 稳压参考电压。
	ADC_Cmd(ADC1,ENABLE);

	
	ADC_ResetCalibration(ADC1);
//**复位 ADC 校准**
//ADC 内部自带校准电路，用来抵消内部的偏移误差，让采样电压更准。
	while(ADC_GetResetCalibrationStatus(ADC1));

//`ADC_GetResetCalibrationStatus(ADC1)`：读取【复位校准状态标志】

// - 返回 **1**：复位校准**正在进行中，还没做完**
// - 返回 **0**：复位校准**已经完成**

// `while(条件)` 的规则：**条件为真，就一直循环卡住；条件为假，跳出循环往下走**

// 所以这句大白话：

// > 
// > **只要复位校准还没做完（返回 1），我就原地等，不往下执行；等它做完变成 0 了，才退出 while，跑下一句 `ADC_StartCalibration`。**

	ADC_StartCalibration(ADC1);
//启动 ADC 自校准
	while(ADC_GetCalibrationStatus(ADC1));
//读取校准状态标志，等待校准完成

//复位校准 → 等复位完成 → 启动校准 → 等校准完成。

}

u16 Get_Adc(u8 ch)//定义自己的 ADC 采集函数，名字叫 Get_Adc，参数是要采集的通道号 ch
{
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
//配置 ADC1 规则组通道，设置通道号、采样时间
// 参数说明：

// 1. ADCx：ADC1
// 2. ADC_Channel：ch，就是传入的通道，比如 `ADC_Channel_0`
// 3. Rank：通道在转换序列里的位置。你是单通道，固定填 `1`
// 4. ADC_SampleTime：采样保持时间，`ADC_SampleTime_239Cycles5` = 239.5 个 ADCCLK 周期
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
//给 ADC 发一个软件命令，启动一次规则组转换
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
//查询 ADC 的某个标志位的状态，返回 SET (1) 或者 RESET (0)
// 1. `ADC_GetFlagStatus`：去查 EOC
// 2. `!`：取反
//    - 没完成（返回 0） → !0=1 → while 条件成立，继续循环等待
//    - 完成（返回 1） → !1=0 → 退出循环
//    整体：**轮询等待 ADC 转换结束**
	return ADC_GetConversionValue(ADC1);
//读取 ADC1 规则转换的数据寄存器里的 12 位采样结果，然后把这个数值 return 出去

//流程：设置通道 -> 启动转换 -> 等待转换完成 -> 读取结果

}
u16 Get_Adc_Average(u8 ch,u8 times)//定义自己的 ADC 采集平均值函数，名字叫 Get_Adc_Average，参数是要采集的通道号 ch 和采样次数 times
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
