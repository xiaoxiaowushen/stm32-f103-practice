#include "playback.h"
#include "history.h"        
#include "lcd.h"             
#include "delay.h"           
#include "stm32f10x.h" 

u8 playback   = 0;
u8 play_index = 0;

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
