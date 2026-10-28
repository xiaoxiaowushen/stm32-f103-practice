#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "stm32f10x.h"
#define IIC_SCL    PCout(12)
#define IIC_SDA    PCout(11)
#define READ_SDA   PCin(11)
#define SDA_IN()  {GPIOC->CRH &= 0XFFFF0FFF; GPIOC->CRH |= 8 << 12;}
#define SDA_OUT() {GPIOC->CRH &= 0XFFFF0FFF; GPIOC->CRH |= 3 << 12;}

void IIC_Init(void)
{
	GPIO_InitTypeDef s;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	s.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	s.GPIO_Mode = GPIO_Mode_Out_PP;
	s.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&s);
	IIC_SCL = 1;
	
	IIC_SDA = 1;
}
void IIC_start(void)
{
	SDA_OUT();//设置为输出模式，由主机控制SDA线
	IIC_SCL = 1;
	IIC_SDA = 1;
	delay_us(4);
	IIC_SDA =0;
	
	delay_us(4);
	IIC_SCL = 0;
}

void IIC_stop(void)
{
	SDA_OUT();
	IIC_SCL = 0;
	IIC_SDA = 0;
	delay_us(4);
	
	IIC_SCL = 1;
	delay_us(4);
	IIC_SDA = 1;
	
	delay_us(4);
}

u8 IIC_wait_ack(void)
{
	u8 t = 0;
	SDA_IN();//这里是从机掌握SDA线
	IIC_SDA = 1;
	delay_us(1);
	IIC_SCL = 1;
	delay_us(1);
	while(READ_SDA)
	{
		t++;
		if(t>250)
		{
			IIC_stop();
			return 1;
		}
	}
	IIC_SDA = 0;
	return 0;
}

void IIC_ack(void)//主机发送应答（ACK）
{
	IIC_SCL = 0;
	SDA_OUT();
	
	IIC_SDA = 0;
	delay_us(2);
	
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}
void IIC_nack(void)//主机发送非应答（NACK）
{
	IIC_SCL = 0;
	delay_us(2);
	
	IIC_SDA = 1;
	delay_us(2);
	
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

void IIC_Send_Byte(u8 txd)
{
	u8 t;
	SDA_OUT();
	IIC_SCL = 0;
	for(t = 0;t<8;t++)
	{
		IIC_SDA = (txd&0x80)>>7;
		txd <<= 1;
		delay_us(2);
		IIC_SCL =1 ;
		delay_us(2);
		IIC_SCL = 0;
		delay_us(2);
	}
}

u8 IIC_Read_Byte(u8 ack)
{
	u8 i;
	u8 receive = 0;
	SDA_IN();
	for(i = 0;i<8;i++)
	{
		IIC_SCL=0;
		delay_us(2);
		
		IIC_SCL = 1;
		receive<<=1;
		if(READ_SDA)receive++;
		delay_us(1);
	}
	
	if(!ack) IIC_nack();
	else IIC_ack();
	//
	return receive;
}

u8 AT24CXX_ReadOneByte(u16 addr)
{
	u8 dat;
	
	IIC_start();
	IIC_Send_Byte(0xA0);//只读
	
	IIC_wait_ack();
	
	IIC_Send_Byte(addr % 256);//
	IIC_wait_ack();
	
	IIC_start();
	IIC_Send_Byte(0xA1);
	IIC_wait_ack();
	
	dat = IIC_Read_Byte(0);
	IIC_stop();
	return dat;
}
u8 AT24CXX_WriteOneByte(u16 addr,u8 dat)//
{
	IIC_start();
	IIC_Send_Byte(0xA0);
	IIC_wait_ack();
	
	IIC_Send_Byte(addr % 256);
	IIC_wait_ack();
	
	IIC_Send_Byte(dat);//
	IIC_wait_ack();
	
	IIC_stop();
	delay_us(5);
	return 0;
}
int main(void)
{
    u8 dat;

    delay_init();
    uart_init(9600);
    IIC_Init();

    dat = AT24CXX_ReadOneByte(20);

    if(dat == 0xFF)                    /* 0xFF = 空白芯片 */
    {
        AT24CXX_WriteOneByte(20, 0x12);
        printf("first run: wrote 0x12\r\n");
    }
    else                                /* 已经有数据 */
    {
        printf("data still there: 0x%02X\r\n", dat);
    }

    while(1);
}
