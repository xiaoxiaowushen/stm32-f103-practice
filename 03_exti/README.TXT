03_exti —— 外部中断按键

功能：按下 KEY0 翻转 LED0，用外部中断实现，主循环不再轮询按键。

硬件：
    LED0 -> PA8  推挽输出（低电平点亮）
    KEY0 -> PC5  上拉输入（按下读到 0）

配置 7 步：
    1. 开时钟  GPIOA | GPIOC | AFIO
    2. 配 PA8 输出、PC5 输入
    3. NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2)
    4. GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5)
    5. EXTI_Init：Line5 / Interrupt 模式 / 下降沿
    6. NVIC_Init：EXTI9_5_IRQn，抢占 2，子 1
    7. 写 EXTI9_5_IRQHandler：置标志 + 清标志位

关键点：
    - 中断函数名固定（EXTI9_5_IRQHandler），写错不报错但不响应
    - 必须 EXTI_ClearITPendingBit(EXTI_Line5)，否则反复进中断
    - 中断里只置标志，delay_ms 不能在中断里调用（SysTick 全局共享）
    - 标志变量要加 volatile

踩过的坑：
    - 没开 AFIO 时钟           -> 中断完全不触发
    - 没写中断服务函数          -> 跳到 Default_Handler 死循环，程序卡死
    - key_flag 没清零           -> 主循环空转
    - 工程里有两份 led.c        -> L6200E 重复定义
    - 声明写在可执行语句之后     -> #268 错误
			
			
			
			
			
			
			