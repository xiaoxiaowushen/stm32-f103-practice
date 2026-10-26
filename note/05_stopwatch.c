#include "stm32f10x.h"
#include "usart.h"
#include "delay.h" 

u32 seconds = 0;            
u8  running = 0;             
volatile u8 key_flag = 0;
//`volatile` 的作用：**告诉编译器，这个变量的值随时可能被中断改掉，不要优化掉它！**

void delay(u32 count)
{
	volatile u32 i ;
	for(i=0;i<count;i++);
}
void TIM3_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    //定时器里面有个计数器 CNT，它就是一个数字，一直在变。
    //向上计数模式：**从 0 开始往上加**。
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM3,
	TIM_IT_Update,
	ENABLE);

//     开启 **TIM3 的更新中断**

// 三个参数：

// 1. `TIM3`：选哪个定时器，这里是定时器 3
// 2. `TIM_IT_Update`：中断类型 ——**更新中断**
// 什么叫更新？就是刚才说的：CNT 数到 ARR，完事一轮，这一瞬间就产生 “更新事件”。
// 3. `ENABLE`：打开这个中断；改成 `DISABLE` 就是关掉

//向上计数，从 0 往上加，加到 ARR → 归零。
//加到 ARR 的这一刻，就会产生 Update 更新事件。

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
//告诉 NVIC：我现在要配置的这一路中断，是 **TIM3 定时器中断**。
// - `NVIC_IRQChannel`：中断通道（哪一个中断）
// - `TIM3_IRQn`：TIM3 对应的中断编号，固定写死，不能随便改

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//抢占优先级：能不能**打断别人正在运行的中断**
// - 数字**越小，优先级越高**
// - 0 就是最高的抢占优先级

	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//只有**抢占（主）优先级一样**的时候，才会去比子优先级；
//子优先级数字越小，优先响应。
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//把 TIM3 这一路中断通道的总开关打开
	NVIC_Init(&NVIC_InitStructure);
//提交配置表	
	TIM_Cmd(TIM3,ENABLE);
//启动 TIM3 定时器，让它开始数数
}
void TIM3_IRQHandler(void)
//TIM3 的中断服务函数,一旦 TIM3 中断触发，CPU 马上放下手里普通代码，跳进来执行这个函数里面的内容
{     
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!= RESET)
//  `TIM_GetITStatus`：读取这个中断标志位的状态

// - `SET`：标志位 = 1，中断来了
// - `RESET`：标志位 = 0，没有这个中断

// `!= RESET` → **不等于 0，也就是标志置 1，是更新中断触发了**
	{
		GPIOA ->ODR ^= (1<<8);
// **把 GPIOA 的第 8 号引脚电平翻转，高变低、低变高**

// 拆开一点点讲：

// 1. `GPIOA->ODR`：ODR 是输出寄存器，这个寄存器里每一位对应一个引脚的输出电平
// bit0 → PA0，bit1 → PA1 …… **bit8 → PA8**
// 2. `1 << 8`：数字 1 往左移动 8 位 → 二进制 `00000001 00000000`，只有第 8 位是 1，其余都是 0
// 3. `^=` 是**异或赋值**
// 异或规则：
//    - 0 ^ 1 = 1
//    - 1 ^ 1 = 0
//    也就是：**和 1 异或，这一位翻转；和 0 异或，不变**
		if(running) seconds++; 
//如果 `running` 不等于 0（打开计时），就让 `seconds` 这个秒数 +1；
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
//清楚TIM3的更新中断标志位
	}
}
void EXTI9_5_IRQHandler(void)
//这是 **EXTI5~EXTI9 共用的中断服务函数**，你这里用的是 EXTI5（PA5 按键）
{
	key_flag = 1;
//按键按下触发外部中断了
	EXTI_ClearITPendingBit(EXTI_Line5);
//清除 EXTI5 这条外部中断线的中断标志位
}

int main(void)
{
	u16 tick = 0;
    //定义一个无符号16位整型变量tick
	NVIC_InitTypeDef n;
    //定义一个中断优先级配置结构体变量n
	GPIO_InitTypeDef s;
    //定义一个GPIO初始化结构体变量s
	EXTI_InitTypeDef e;
    //定义一个外部中断初始化结构体变量e
	
	delay_init(); 
	uart_init(9600);
    //初始化延时函数和串口函数，波特率 9600
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //设置中断分组，决定抢占优先级、响应优先级各占两位	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);
    //开启 GPIOA、GPIOC、AFIO 的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
    //开启 TIM3 的时钟

	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&s);
	
	s.GPIO_Pin = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&s);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
	//把 GPIOC 的第 5 引脚，映射到外部中断线 EXTI Line5
	
	e.EXTI_Line = EXTI_Line5;
    //选择哪一条外部中断线，这里是 EXTI5
	e.EXTI_Mode = EXTI_Mode_Interrupt;
    //选择中断模式，另一种是事件模式,
	e.EXTI_Trigger = EXTI_Trigger_Falling;
    //选择下降沿触发中断，按键按下时，PA5 由高电平变低电平，就会触发中断
	e.EXTI_LineCmd = ENABLE;	
    //打开 EXTI5 这条外部中断线
	EXTI_Init(&e);
	
	n.NVIC_IRQChannel = EXTI9_5_IRQn;
    //告诉 NVIC：我现在要配置的这一路中断，是 EXTI5~EXTI9 共用中断。
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 3;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
	//置中断分组，决定抢占优先级、响应优先级各占两位

	TIM3_Init(9999,7199);
    //初始化 TIM3 定时器，ARR=9999，PSC=7199
	
	while(1)
    //主循环
	{
		if(key_flag)
        //如果按键按下，触发了外部中断
		{
			key_flag = 0;
			//清除按键标志位
			delay_ms(10);
			//延时 10ms，消抖
			if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0)
            //读取如果按键仍然按下（PA5 仍然是低电平）
			{
				running = !running;
                //翻转计时状态，按一次开始计时，再按一次暂停计时 
			}
		}
		tick++;//每次循环 tick +1
		if(tick >= 10)//每 10 次循环，打印一次秒数
		{
			tick = 0;//清零 tick
			printf("time: %d s\r\n", seconds);//打印秒数
		}
		delay_ms(100);//延时 100ms  
	}       
}

// 你的 PC5 是**上拉输入**

// - 没按：PC5 = 1（高）
// - 按下：PC5 = 0（低）
//  是 **1→0 瞬间触发中断**，不是按下变成 1。

// 然后把整个动作给你顺一遍：
// 初始：`running=0`，秒数不涨

// 1. **第一次按下 PC5**
// 1 变 0 → 触发外部中断 → `key_flag=1`
// 回到 while 循环，检测到 `key_flag==1`
// 延时消抖，确认 PC5 还是 0`running = !running;` → running 变成 **1**
//  计时开关打开
// 之后 TIM3 每 1 秒进定时器中断：`if(running) seconds++;` → 秒数持续往上加，LED 一秒翻转一次
// 2. **第二次按下 PC5**
// 又一次 1→0，再次触发中断 → `key_flag=1`
// 进主循环处理，`running = !running;` → running 变回 **0**
//  计时关闭
// 定时器照样每秒进中断、LED 照样闪，但是 `seconds++` 这一句不执行，**数字卡住不动，不再增加**

// > 
// > 不管 running 是 0 还是 1，只要 tick 走到条件，串口都会打印当前 seconds 的值：
// > running=1 的时候打印持续上涨的数字；running=0 的时候打印固定不变的数字。