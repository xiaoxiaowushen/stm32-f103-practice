#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "myiic.h"
#include "24cxx.h"
#include "adc.h"
#include "stm32f10x.h"

/* ============================================================
   历史记录：EEPROM 存取
   地址 0~1   : t_max（高字节 + 低字节）
   地址 2~3   : t_min
   地址 4~203 : 100 条历史温度，每条 2 字节
   地址 204   : 写指针 wp（0~99）
   ============================================================ */

/*
 * 存一条温度进 EEPROM（环形缓冲）
 */
void Savetemp(u32 t)
{
	u8 wp;
	u16 addr;

	wp = AT24CXX_ReadOneByte(204);

	if(wp >= 100)                    /* 空白芯片读到 0xFF，或者数据坏了 */
	{
		wp = 0;
		AT24CXX_WriteOneByte(204, 0);
	}

	addr = 4 + wp * 2;               /* 第 wp 条记录的起始地址 */

	AT24CXX_WriteOneByte(addr,     t >> 8);      /* 高字节 */
	AT24CXX_WriteOneByte(addr + 1, t & 0xFF);    /* 低字节 */

	wp++;                            /* 写指针后移 */
	if(wp >= 100) wp = 0;            /* 环形：满了绕回开头 */

	AT24CXX_WriteOneByte(204, wp);
}

/*
 * 读第 index 条（0~99）
 */
u32 ReadTemp(u8 index)
{
	u16 addr;
	u32 temp;

	if(index > 99) return 0;

	addr = 4 + index * 2;
	temp = (AT24CXX_ReadOneByte(addr) << 8) | AT24CXX_ReadOneByte(addr + 1);

	return temp;
}

/* ============================================================
   全局状态
   ============================================================ */
u8 playback   = 0;      /* 0 = 实时显示，1 = 回放模式 */
u8 play_index = 0;      /* 回放时显示第几条 */

/* ============================================================
   GPIO 初始化
   PA8 推挽输出（LED0），PC5 上拉输入（KEY0）
   ============================================================ */
void led_init(void)
{
	GPIO_InitTypeDef s;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	s.GPIO_Pin   = GPIO_Pin_8;
	s.GPIO_Mode  = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &s);

	s.GPIO_Pin  = GPIO_Pin_5;
	s.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &s);

	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

/*
 * 重画屏幕：清屏 + 画 MAX/MIN 标签
 * 切换模式时调用，避免上一屏的残留
 */
void redraw_screen(void)
{
	LCD_Clear(WHITE);

	POINT_COLOR = RED;
	LCD_ShowString(60, 170, 200, 16, 16, "MAX:");
	LCD_ShowString(60, 190, 200, 16, 16, "MIN:");

	POINT_COLOR = BLUE;
}

/* ============================================================
   按键处理
   短按（< 1 秒）：实时 -> 回放 / 回放 -> 翻一页
   长按（>= 1 秒）：退出回放
   ============================================================ */
void handle_key(void)
{
	u16 hold = 0;

	delay_ms(10);                                    /* 消抖 */

	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) != 0) return;

	/* 测按住时长：每 50ms 记一次 */
	while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0)
	{
		delay_ms(50);
		hold++;
	}

	if(hold >= 20)                                   /* 按住 >= 1 秒：长按退出 */
	{
		if(playback == 1)
		{
			playback   = 0;
			play_index = 0;
			redraw_screen();
		}
	}
	else if(playback == 0)                           /* 短按 + 实时：进入回放 */
	{
		playback   = 1;
		play_index = 0;
		redraw_screen();
	}
	else                                             /* 短按 + 回放：翻一页 */
	{
		play_index++;

		if(play_index >= 100 || ReadTemp(play_index) == 0)
		{
			playback   = 0;                          /* 翻完了一圈，自动退出 */
			play_index = 0;
			redraw_screen();
		}
	}
}

/* ============================================================
   主函数
   ============================================================ */
int main(void)
{
	static u8 blink = 0;
	u16 adcx;
	u32 t100;
	u32 mv;
	u32 t_max;
	u32 t_min;
	u32 last_max;
	u32 last_min;
	u32 save_tick;
	u32 rec;

	led_init();
	delay_init();
	uart_init(9600);
	LCD_Init();
	adc_init();
	IIC_Init();

	/* ===== 上电从 EEPROM 读历史最值 ===== */
	t_max = (AT24CXX_ReadOneByte(0) << 8) | AT24CXX_ReadOneByte(1);
	t_min = (AT24CXX_ReadOneByte(2) << 8) | AT24CXX_ReadOneByte(3);

	if(t_max == 0xFFFF || t_max < 2000 || t_max > 5000) t_max = 0;
	if(t_min == 0xFFFF || t_min < 2000 || t_min > 5000) t_min = 99999;

	last_max = t_max;
	last_min = t_min;

	redraw_screen();

	while(1)
	{
		/* ========== ① 按键 ========== */
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0)
		{
			handle_key();
		}

		/* ========== ② 实时模式 ========== */
		if(playback == 0)
		{
			adcx = Get_adc_average(16, 10);
			mv   = adcx * 3300 / 4096;
			t100 = (1430 - mv) * 1000 / 43 + 2500;

			/* 更新最值 */
			if(t100 > t_max) t_max = t100;
			if(t100 < t_min) t_min = t100;

			/* 值变了才写回 EEPROM */
			if(t_max != last_max)
			{
				last_max = t_max;
				AT24CXX_WriteOneByte(0, t_max >> 8);
				AT24CXX_WriteOneByte(1, t_max & 0xFF);
			}
			if(t_min != last_min)
			{
				last_min = t_min;
				AT24CXX_WriteOneByte(2, t_min >> 8);
				AT24CXX_WriteOneByte(3, t_min & 0xFF);
			}

			/* ===== 显示 ADC 值 ===== */
			LCD_ShowxNum(60, 60, adcx, 4, 16, 0);

			/* ===== 显示电压 ===== */
			LCD_ShowxNum(60, 90, mv/1000, 1, 16, 0);
			LCD_ShowxNum(76, 90, mv%1000, 3, 16, 0x80);

			/* ===== 每 10 秒存一条历史 ===== */
			save_tick++;
			if(save_tick >= 40)
			{
				save_tick = 0;
				Savetemp(t100);
			}

			/* ===== 超阈值报警 ===== */
			if(t100 > 3500)
			{
				BACK_COLOR  = RED;
				POINT_COLOR = WHITE;

				blink = !blink;
				if(blink) GPIO_ResetBits(GPIOA, GPIO_Pin_8);
				else      GPIO_SetBits(GPIOA, GPIO_Pin_8);
			}
			else
			{
				BACK_COLOR  = WHITE;
				POINT_COLOR = BLUE;

				GPIO_SetBits(GPIOA, GPIO_Pin_8);
				blink = 0;
			}

			/* ===== 显示温度 ===== */
			LCD_ShowxNum(60, 120, t100/100, 2, 16, 0);
			LCD_ShowxNum(84, 120, t100%100, 2, 16, 0x80);

			/* ===== 显示最值 ===== */
			LCD_ShowxNum(100, 170, t_max/100, 2, 16, 0);
			LCD_ShowxNum(124, 170, t_max%100, 2, 16, 0x80);

			if(t_min == 99999)
			{
				LCD_ShowString(100, 190, 200, 16, 16, "--.--");
			}
			else
			{
				LCD_ShowxNum(100, 190, t_min/100, 2, 16, 0);
				LCD_ShowxNum(124, 190, t_min%100, 2, 16, 0x80);
			}

			delay_ms(250);              /* 实时模式：250ms 刷新 */
		}
		/* ========== ③ 回放模式 ========== */
		else
		{
			rec = ReadTemp(play_index);

			LCD_ShowString(60, 60, 200, 16, 16, "PLAY:");
			LCD_ShowxNum(100, 60, play_index, 3, 16, 0);

			LCD_ShowxNum(60, 120, rec/100, 2, 16, 0);
			LCD_ShowxNum(84, 120, rec%100, 2, 16, 0x80);

			delay_ms(50);               /* 回放模式：50ms，按键响应快 */
		}
	}
}
