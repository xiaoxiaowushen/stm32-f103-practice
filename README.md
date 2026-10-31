# stm32-practice

STM32F103 学习练习（正点原子 MiniSTM32 开发板）

按学习顺序排列，每个目录对应一个外设或一个小项目。

## 目录

| 目录 | 内容 | 用到的外设 |
|---|---|---|
| `01_led` | 跑马灯：常亮 / 闪烁 / 双灯交替 | GPIO 输出 |
| `02_uart` | 按键 + 串口，按下打印状态 | GPIO 输入 + USART |
| `03_exti` | 外部中断按键 | EXTI + NVIC |
| `04_timer` | 定时器中断，500ms 翻转 LED | TIM |
| `05_stopwatch` | 秒表：按键启停，串口显示秒数 | TIM + EXTI + USART |
| `06_temp` | 温度计：ADC 采集 + LCD 显示 | ADC + LCD |
| `07_i2c` | I2C 驱动 + 24C02 读写验证 | I2C |
| `08_temp_v2` | **温度记录仪**（见下） | ADC + LCD + I2C + GPIO |

## 温度记录仪（08_temp_v2）

**功能**

- ADC 采集芯片内部温度传感器，整数算法换算（不用浮点）
- LCD 实时显示 ADC 值 / 电压 / 温度 / 最高温 / 最低温
- 超阈值报警：LED 闪烁 + 显示区域变色
- 100 条历史温度循环存储到 24C02 EEPROM，掉电不丢失
- 按键交互：短按翻页回放，长按退出

**自己写的部分**

- `HARDWARE/IIC/` —— I2C 软件模拟驱动（从零写）
- `HARDWARE/24CXX/` —— 24C02 读写函数（含环形缓冲）
- `HARDWARE/ADC/` —— ADC 初始化与温度换算
- `USER/main.c` —— 业务逻辑

## 开发环境

- Keil MDK 5 + ARM Compiler 5
- 优化等级 Level 2（Level 1 会超出 32KB 免费版限制）

## 说明

`CORE/`、`SYSTEM/`、`STM32F10x_FWLib/` 是正点原子的工程模板和 ST 标准外设库，未修改。
