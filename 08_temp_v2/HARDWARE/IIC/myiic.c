#include "myiic.h"
#include "delay.h"
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
void IIC_Start(void)
{
	SDA_OUT();//设置为输出模式，由主机控制SDA线
	IIC_SCL = 1;
	IIC_SDA = 1;
	delay_us(4);
	IIC_SDA =0;
	
	delay_us(4);
	IIC_SCL = 0;
}

void IIC_Stop(void)
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

u8 IIC_Wait_Ack(void)
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
			IIC_Stop();
			return 1;
		}
	}
	IIC_SDA = 0;
	return 0;
}

void IIC_Ack(void)//主机发送应答（ACK）
{
	IIC_SCL = 0;
	SDA_OUT();
	
	IIC_SDA = 0;
	delay_us(2);
	
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}
void IIC_Nack(void)//主机发送非应答（NACK）
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
	
	if(!ack) IIC_Nack();
	else IIC_Ack();
	//
	return receive;
}

