#include "stm32f10x.h"
#include "usart.h"

void delay(u32 count)
{
	u32 i;
	for(i=0;i<count;i++);
}
volatile u8 key_flag = 0;         
// //- `u8`：类型，就是 `unsigned char`，1 字节无符号字符，取值 0~255，这里用来做标记：0 = 没按键，1 = 有按键触发
// - key_flag：变量名，按键标志位
// - =0：初始化，程序一开始标志是 0（无按键事件）
// volatile 中文叫易变关键字
//告诉编译器：这个变量的值，会被【中断函数】偷偷修改！不要做优化，每次读取都必须去内存真实读，不要缓存到寄存器里。

void EXTI9_5_IRQHandler(void)
{
	key_flag = 1;
//- 进入中断，代表 PC5 检测到**下降沿**（按键按下）
// - 仅仅把标志位 `key_flag` 置 1，**不做延时、不读按键、不串口打印、不翻转 LED**


	EXTI_ClearITPendingBit(EXTI_Line5);
// ITPendingBit：中断待处理标志。
// 当下降沿来了，硬件自动把这个位置 1，告诉内核：有中断事件来了。

}
// - **IRQHandler** = Interrupt Request Handler，**中断服务函数**
// - `EXTI9_5`：外部中断线 5~9 共用这一个中断入口（EXTI5、EXTI6、EXTI7、EXTI8、EXTI9 都进这个函数）

int main(void)
{
	
	u8 on =0;
	GPIO_InitTypeDef s;
// - 类型：`GPIO_InitTypeDef` → GPIO 初始化结构体
// - 作用：存放 GPIO 的配置信息：引脚号、模式、速度。
// - Mode, Speed：GPIO 的模式和速度，都是枚举类型，枚举类型就是一组常量的集合。

	NVIC_InitTypeDef p;
// - 类型：`NVIC_InitTypeDef` → 嵌套向量中断控制器（NVIC）初始化结构体
// - 作用：**配置中断优先级、开启 / 关闭这个中断通道**
//

	EXTI_InitTypeDef q;

//标准库预先定义好的**外部中断初始化结构体类型**

	
	uart_init(9600); 
//调用usart.h 里面写好的串口初始化函数，配置串口波特率 9600。

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);
//开启 APB2 总线上：GPIOA、GPIOC、AFIO 这三个外设的时钟。


	s.GPIO_Pin = GPIO_Pin_8;
//选择配置PA8 引脚，也就是你的 LED 灯所在引脚
	s.GPIO_Mode = GPIO_Mode_Out_PP;
//推挽输出，可以输出高电平、低电平，用来控制 LED
	s.GPIO_Speed = GPIO_Speed_50MHz;
//设置 GPIO 引脚电平最快翻转速度上限，不是让引脚一直 50MHz 高速跳，是设置硬件驱动能力。
//推挽输出时，引脚电平从高变低 / 低变高，这个切换动作不是瞬间完成的，有快慢。


	GPIO_Init(GPIOA,&s);
//把结构体`s`里面存好的这一套参数，写入 GPIOA 的硬件寄存器，完成 PA8 引脚的硬件配置
	
	s.GPIO_Pin = GPIO_Pin_5;
//选择5 号引脚 PC5

	s.GPIO_Mode = GPIO_Mode_IPU;
//配置 PC5 为上拉输入模式。内部接上拉电阻，按键松开默认高电平，按下变为低电平，适合按键检测。

	GPIO_Init(GPIOC,&s);
//把 pc5 的配置写入 GPIOC 寄存器

	GPIO_SetBits(GPIOA,GPIO_Pin_8);
//将八号端口设置为高电平，灯灭，PA8 连接的 LED 灯是低电平点亮，高电平熄灭。
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//设置 NVIC 中断优先级分组为组 2,把中断优先级的 4 个 bit，切分成【抢占 2 位，子优先级 2 位】
//NVIC = **嵌套向量中断控制器**，是 STM32 内核里专门管理所有中断的模块。
//当多个中断同时到来 / 中断执行中途又来了新中断，NVIC 决定：**要不要打断当前代码、先执行哪一个中断**。
//STM32F1 内核里，每个中断有一个 4bit 的优先级寄存器，总共就 4 个位，用来放优先级信息。分组就是决定：这 4 位，多少位分给抢占，多少位子优先级。



	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
//把 PC5 这个引脚，连接映射到 EXTI5 外部中断线上。
//`GPIO_PortSourceGPIOC`：选择端口为 **GPIOC**
//`GPIO_PinSource5`：选择引脚编号 **5**
//`GPIO_EXTILineConfig`：硬件选择【哪个引脚接到 EXTI5】

	q.EXTI_Line = EXTI_Line5;
//选择 EXTI 外部中断的第 5 号线（EXTI5）
	q.EXTI_Mode = EXTI_Mode_Interrupt;
//选择 EXTI5 线的工作模式为**中断模式**，也可以选择事件模式（Event Mode），但我们这里不需要。
//**`EXTI_Mode_Event` 事件模式**
// 检测到边沿信号，**不通知 CPU**。只是产生一个事件脉冲，用来直接触发 DMA、定时器等外设，CPU 不会进中断。做外设联动才会用，按键场景不用。

	q.EXTI_Trigger = EXTI_Trigger_Falling;
//选择 EXTI5 线的触发方式为**下降沿触发**，也就是按键按下时触发中断。
//### 什么是下降沿？

// **高电平 → 跳变到低电平**
// 你的 PC5 是上拉输入：

// - 按键松开：PC5 = 高电平
// - 按下按键：PC5 拉到 GND，变成低电平
//  按下瞬间，发生「高→低」，就是下降沿，触发 EXTI 中断。
	q.EXTI_LineCmd = ENABLE;
//使能 EXTI5 这条外部中断线，把这条中断线打开，允许它检测边沿、产生中断
//`EXTI_LineCmd`：外部中断线的使能开关

	EXTI_Init(&q);
//读取结构体 q 里面全部 EXTI 配置，写入 EXTI 硬件寄存器，EXTI5 的配置正式生效
	
	p.NVIC_IRQChannel = EXTI9_5_IRQn;
//选择要配置的中断通道是 EXTI9_5，也就是 EXTI5~EXTI9 共用的中断通道
//- `NVIC_IRQChannel`：结构体成员，用来指定**要配置哪一路中断通道**
// - `EXTI9_5_IRQn`：中断通道名。
//理解流程：EXTI5 产生中断信号 → 信号送到 NVIC 里的`EXTI9_5_IRQn`通道 → NVIC 根据优先级判断是否响应中断 → 跳转到`EXTI9_5_IRQHandler`


	p.NVIC_IRQChannelPreemptionPriority = 2;
//`NVIC_IRQChannelPreemptionPriority`：结构体成员，代表**抢占优先级**
//设置这个 EXTI9_5 中断的抢占优先级 = 2
// > 规则：抢占优先级高（数字小）的中断，可以打断正在运行的低抢占优先级中断，实现中断嵌套。

// 举例子：

// - 如果现在正在执行这个按键中断（抢占 = 2）
// - 此时来了一个抢占优先级 = 1 的中断 → **可以打断按键中断，优先执行它**
// - 如果新来的中断抢占优先级 = 3 → 优先级更低，不能打断，排队等待
	p.NVIC_IRQChannelSubPriority = 1;
//设置这个 EXTI9_5 中断的子优先级 = 1
//> 规则：子优先级高（数字小）的中断，在**同一抢占优先级**下，先执行子优先级高的中断。
//- 如果当前正在运行主循环：直接进入中断函数
// - 如果正在跑别的中断：对比抢占优先级，判断**能不能打断（嵌套）**
// - 多个中断同一时间到来：抢占相同就对比子优先级，安排排队顺序


	p.NVIC_IRQChannelCmd = ENABLE;
//打开 EXTI9_5 这一路中断通道的总开关，允许 NVIC 接收这个中断的请求
	NVIC_Init(&p);
//读取 p 盒子里全部 NVIC 配置，写入 NVIC 硬件寄存器，**NVIC 的整套配置正式生效**。
	
	while(1)
// `while(表达式)`，表达式非 0 就一直循环。
// `1`永远不为 0，所以**无限循环，不会自己退出**。
	{

		if(key_flag)
// 判断变量 key_flag 是否非 0；如果 `key_flag == 1`，条件成立，执行大括号里面代码。
		{

			key_flag = 0;
// 把按键标志位清零，表示按键事件已经处理完成
			delay(1000000);
//`delay(1000000);` 是软件延时函数，就是让 CPU 空等一段时间，啥也不干。数字越大，等待时间越长。
// 机械按键内部是金属弹片。**按下 / 松开瞬间，金属触点会快速抖几下**，不是一次性稳定接通。
// 比如你按一下按键，电平会这样：

// > 
// > 高 →低→高→低→高→低，稳定低
// > 这一段快速来回跳变，就叫**抖动**，一般持续**5~20ms**。

//如果不加处理，**你只按一次按键，会产生很多个下降沿，触发多次中断**，LED 会乱翻转、串口重复打印，这就是抖动带来的 bug。

// 检测到按键触发之后，**等一小段时间（10~20ms），等金属弹片抖动结束，电平稳定了，再去判断按键状态**。
// 确认此时还是按下状态，才判定为一次有效的按键。

			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0)
			{
				
				on = !on;
				printf("pressed,on = %d\r\n",on);
				if(on)GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				else GPIO_SetBits(GPIOA,GPIO_Pin_8);
				while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0);
			}
		}
	}
}
