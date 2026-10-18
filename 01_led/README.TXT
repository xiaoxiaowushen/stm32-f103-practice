# stm32-practice

STM32F103 学习练习（正点原子 MiniSTM32 开发板）

## 已完成

| 目录 | 内容 | 用到的外设 |
|---|---|---|
| `01_led`  | 跑马灯：常亮 / 闪烁 / 双灯交替     | GPIO 输出 |
| `02_uart` | 按键 + 串口，按下打印当前状态       | GPIO 输入 + USART |

## 我写的部分

- `USER/main.c`  —— 主逻辑
- `HARDWARE/`    —— 外设驱动

## 现成框架（未修改）

- `CORE/`             启动文件、CMSIS
- `SYSTEM/`           delay / sys / usart
- `STM32F10x_FWLib/`  ST 标准外设库 V3.5

## 环境

- Keil MDK 5 + ARM Compiler 5
- 优化等级 **Level 2**（Level 1 会超出 32KB 免费版限制）

## 硬件备注

- LED0 → PA8，LED1 → PD2（**低电平点亮**）
- KEY0 → PC5，上拉输入，**按下读到 0**
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
