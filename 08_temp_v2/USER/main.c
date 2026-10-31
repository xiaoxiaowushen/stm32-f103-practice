#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "myiic.h"
#include "24cxx.h"
#include "adc.h"
#include "stm32f10x.h"
#include "history.h"
#include "playback.h"

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
