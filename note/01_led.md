# include "stm32f10x.h"

# void delay(u32 count)

`u32`：**unsigned int**，无符号 32 位整型
  - 
  - 范围：`0 ~ 4294967295`
  - 只能存**非负数**，延时次数肯定不会是负数，所以延时函数喜欢用 u32
  `int`：**有符号 32 位整型**
- 范围：`-2147483648 ~ 2147483647`
- 可以存负数
- 在stm32标准库里面
typedef unsigned int u32; // 类型别名，给unsigned int起外号叫u32

{
	u32 i;
	for(i=0;i<count;i++);
}

# int main(void)
{
	GPIO_InitTypeDef s;
    //定义一个结构体变量 `s`，变量类型是 `GPIO_InitTypeDef`,
    `GPIO_InitTypeDef` 是标准库提前写好的**结构体模板**，里面打包了配置 GPIO 需要的全部参数：引脚 Pin、模式 Mode、速度 Speed。


	u8 on = 0;
	//创建一个叫`on`的 8 位变量，初始值是 0。
    //`u8`：`typedef unsigned char u8;`，**无符号 8 位整型** 
    取值范围：`0 ~ 255`，只能存非负数，占 1 字节。

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);
	//调用这个函数，告诉 RCC 模块：**把挂在 APB2 总线上的 GPIOA 和 GPIOC 这两个外设的时钟打开**。STM32 上电默认所有外设时钟关闭（省电），**不开时钟，后面 GPIO 所有配置全部无效**。


	s.GPIO_Pin = GPIO_Pin_8;
    //给结构体成员 GPIO_Pin 赋值，指定要配置第 8 号引脚，仅保存参数，还未写入寄存器。

	s.GPIO_Mode = GPIO_Mode_Out_PP;
    //`s.GPIO_Mode`：结构体里的成员，用来设置**GPIO 引脚工作模式**
    //`GPIO_Mode_Out_PP`：宏全称：**Output Push Pull，推挽输出**
    //推挽输出：引脚**既能输出高电平，也能输出强低电平**，驱动能力强
    拓展：- `GPIO_Mode_Out_OD`：开漏输出，需要上拉电阻，一般不用来点灯
 - `GPIO_Mode_IPU`：上拉输入，读按键
 - `GPIO_Mode_IPD`：下拉输入
 - `GPIO_Mode_AIN`：模拟输入，ADC 采集

	s.GPIO_Speed = GPIO_Speed_50MHz;
	//`s.GPIO_Speed = GPIO_Speed_50MHz;`：设置 GPIO 引脚最大输出速度为 50MHz，代表引脚电平最快翻转上限，参数暂存结构体。

	GPIO_Init(GPIOA,&s);
- `GPIO_Init`：库函数，**GPIO 初始化函数**
- 第 1 个参数 `GPIOA`：指定要操作的外设端口，这里就是 GPIOA
- 第 2 个参数 `&s`：取结构体变量`s`的**地址**（把刚才填好的那一堆配置的地址传给函数）
**把结构体 s 里面存好的 PA8 引脚配置，交给 GPIO_Init 函数，这个函数自动去写芯片内部对应的寄存器，完成硬件配置**。

	GPIO_SetBits(GPIOA,GPIO_Pin_8);
//作用：把 PA8 设置成低电平（0V）
- `Set` = 设置 → **置 1，高电平**
- `Reset` = 清零 → **置 0，低电平**
//### 第 1 个参数：`GPIOA`
指定**哪个 GPIO 端口**，告诉函数：我要操作 A 组的引脚。
可选：`GPIOA` / `GPIOB` / `GPIOC`……

### 第 2 个参数：`GPIO_Pin_8`
指定**这个端口里的哪一根引脚**，就是 A 端口的第 8 号引脚 PA8。

## 拓展：一次控制多个引脚

多个引脚用 `|` 按位或写在一起

```
GPIO_SetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_9); 
// PA8 和 PA9 同时拉高
```

	s.GPIO_Pin = GPIO_Pin_5;
//**往结构体变量 s 里面的 GPIO_Pin 成员，存入`GPIO_Pin_5`这个标记**。


	s.GPIO_Mode = GPIO_Mode_IPU;
//往结构体`s`的草稿里写配置：**把选中引脚设置成上拉输入**。
上拉输入：**按键松开 → PC5 默认高电平（返回值 1）**
pc5就是key0
1. 手松开按键：PC5 = 高电平 (1) → `==0` 不成立，不进入这个 while 等待
2. 手按下按键：PC5 = 低电平 (0) → 进入 while 循环，卡在原地，一直读引脚
3. **直到手松开按键，PC5 变回高电平 (1)** → 条件不成立，跳出 while，继续运行

	GPIO_Init(GPIOC,&s);
//把结构体`s`里面存好的配置，提交给库函数，**对 GPIOC 端口里被选中的引脚（PC5）完成硬件寄存器修改**。

	while(1)
	{
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) ==0 )
//`GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)`
函数作用：**读取 PC5 引脚当前的电平**
1. 
   - 返回 `1`：引脚高电平
   - 返回 `0`：引脚低电平
   参数：GPIOC（C 端口），GPIO_Pin_5（5 号引脚 PC5）
   读取 PC5 引脚电平，如果读到低电平（0），就执行 if 里面的代码。

		{
			delay(100000);
//CPU 在这里原地空循环 100000 次，**卡住一小段时间，不往下执行后面代码**。
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) ==0)
//延时消抖后再次读取 PC5 电平判断：过滤按键机械抖动，确认低电平是真实按键按下，不是瞬时毛刺。

		{
			on =! on;
- `!`：C 语言**逻辑非运算符**
- `!0` → 结果为 1
- `!非0` → 结果为 0
- `=`：赋值，把 `!on` 的结果重新存回变量 on
**把 on 的值取反，再存回 on。**

- 如果 `on == 0`，执行后 `on = 1`
- 如果 `on == 1`，执行后 `on = 0`

			if(on) GPIO_ResetBits(GPIOA, GPIO_Pin_8);
1. `if(on)`
C 语言规则：**非 0 就代表条件为真，0 代表假**

- `on == 1` → 条件成立，执行后面语句
- `on == 0` → 条件不成立，跳过

			else   GPIO_SetBits(GPIOA, GPIO_Pin_8);  
//否则，拉高 PA8，LED 熄灭。

			while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0);
//空循环，持续检测 PC5；按键保持低电平（按住）就一直等待，直到按键松开才退出循环，防止长按连续触发。
		}	
	}
 }
}
重新写的顺序：时钟 → LED 初始化 → 按键初始化 → while 主循环

---

第一次重写：
void delay(u32 count)
{
	u32 i;
	for(i=0;i<count;i++);
}
int main(void)
{
	u8 on = 0;
	GPIO_InitTypeDef s;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);//困惑的地方：为什么选择GPIOA和GPIOC，而不选择GPIOB Or GPIOD或者其他呢
	//解答：
	//因为你的开发板硬件，LED 接 PA8，KEY 按键接 PC5，硬件引脚固定，所以就启用这两个端口时钟；不是必须选 A/C，LED 按键如果接 B 口，那就开 GPIOB 时钟。
	s.GPIO_Pin = GPIO_Pin_8;
	s.GPIO_Mode = GPIO_Mode_Out_PP;//设置为推挽输出模式，疑惑：推挽输出是干什么的
	//解答：引脚既能主动输出高电平，也能主动拉低到地，驱动能力强，适合驱动 LED。如果是开漏输出，单独没法输出高电平。
	
	s.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&s);
	
	s.GPIO_Pin = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	//上拉输入：芯片内部，在 PC5 引脚和**3.3V 电源**之间接了一个内置电阻。
//- 按键**没按下**：内部电阻把 PC5 引脚拉到 3.3V → 电平 = 高 (1)
//- 按键**按下**：按键把 PC5 直接连到 GND（地线 0V），内部电阻被短路 → 电平 = 低
	//所以代码写 `==0` 判断按下，刚好匹配。
	
	GPIO_Init(GPIOC,&s);
	//`GPIO_SetBits` → **输出高电平（1）**
  //`GPIO_ResetBits` → **输出低电平（0）**
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	while(1)
	{
		if (GPIO_ReadInputDataBit (GPIOC,GPIO_Pin_5)==0)//对于读取判断电平语法运用还不熟练
		{
			delay(1000000);
			if (GPIO_ReadInputDataBit (GPIOC,GPIO_Pin_5)==0)
			{
					on = !on;
				if(on)GPIO_ResetBits(GPIOA,GPIO_Pin_8);//（拉低 PA8，灯亮）
				else GPIO_SetBits(GPIOA,GPIO_Pin_8);//（拉高 PA8，灯熄）
				while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0);//持续读取 PC5，只要电平等于 0（按键还按着）就循环卡住。
			}
		}
	}
}

