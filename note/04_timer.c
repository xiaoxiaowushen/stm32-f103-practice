#include "stm32f10x.h"
#include "usart.h"

void delay(u32 count)
{
	u32 i ;
	for(i=0;i<count;i++);
}

void TIM3_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//开启挂载在 APB1 总线上的 TIM3 外设时钟,APB1：低速总线 → TIM2、TIM3、TIM4、TIM5 都挂在这里,APB2：高速总线 → TIM1、TIM8，还有 GPIO、USART1、ADC 等在这
	
	TIM_TimeBaseStructure.TIM_Period = arr;//计数器数到这个数，就触发更新（中断），然后清零从头再数
	
	TIM_TimeBaseStructure.TIM_Prescaler = psc;//定时器进来的原始时钟（TIM3 是 72MHz），先经过这个预分频器：每来 psc+1 个原始时钟脉冲，CNT 计数器才 +1
	//- **PSC(psc)**：控制【计数器多久加一下】
  //- **ARR(Period)**：控制【加到多少清零、触发中断】
	TIM_TimeBaseStructure.TIM_CounterMode =	TIM_CounterMode_Up;//配置计数模式为向上计数模式
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//提交这个配置表
	
	
	TIM_ITConfig(//F10X标准库函数 开启 / 关闭定时器的中断使能
	TIM3,//定时器
	TIM_IT_Update,//中断类型
	ENABLE);//开启
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//指定现在要配置的中断通道是 TIM3 的中断,
	//`NVIC_IRQChannel`：中断通道（中断编号）,`
	//TIM3_IRQn`：是宏，代表定时器 3 对应的中断通道号，F103 标准库写死的
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	 = 3;//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//把上面配置的那个TIM3_IRQn的通道打开
	NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	TIM_Cmd(TIM3,ENABLE);
	//启动 TIM3 的计数器，让 CNT 开始往上数数
	
}
void TIM3_IRQHandler(void)//TIM3中断处理函数
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)//TIM_GetITStatus库函数，作用是读取中断标志位的状态返回RESET OR SET
		//- `SET`：标志位为 1，中断发生了
		//- `RESET`：标志位为 0，没有发生这个中断
	{
		//	LED1=!LED1;
		GPIOA->ODR ^= (1 << 8);//反转灯的亮灭
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//清楚那个标志中立位，就是返回的1或0，目的是为了不让程序一直返回一个值然后程序进入死循环
		
	}
}

int main(void)
{
	GPIO_InitTypeDef s;
	
	uart_init(9600);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&s);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	TIM3_Init(4999,7199);
	
	while(1)
	{
		printf("main alive\n");
		delay(10000000);
	}
	
}

// ## 定时器 ARR 速查（PSC 固定 7199）

// | 周期 | ARR |
// |---|---|
// | 10 ms | 99 |
// | 50 ms | 499 |
// | 100 ms | 999 |
// | 200 ms | 1999 |
// | 500 ms | 4999 |
// | 1 s | 9999 |
// | 2 s | 19999 |

// 公式：ARR = 周期(ms) × 10 − 1
// 超过 6.5 秒要改 PSC（改成 71999，ARR = 秒数 × 1000 − 1）