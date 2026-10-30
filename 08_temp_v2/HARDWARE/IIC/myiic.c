#include "myiic.h"
#include "delay.h"
void IIC_Init(void)//初始化iic
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
	IIC_SCL = 1;//拉高scl
	IIC_SDA = 1;
	delay_us(4);
	IIC_SDA =0;//更改sda
	
	delay_us(4);
	IIC_SCL = 0;//钳住总线，准备逐位传数据
}

void IIC_Stop(void)
{
	SDA_OUT();//输出模式
	IIC_SCL = 0;//更改scl，开辟窗口
	IIC_SDA = 0;//更改sda
	delay_us(4);
	
	IIC_SCL = 1;//拉高scl
	delay_us(4);
	IIC_SDA = 1;//改变sda，发送，告诉从机停止接受数据
	
	delay_us(4);
}

u8 IIC_Wait_Ack(void)
{
	u8 t = 0;
	SDA_IN();//这里是从机掌握SDA线
	IIC_SDA = 1;//sda为1
	delay_us(1);
	IIC_SCL = 1;//scl拉高
	delay_us(1);
	while(READ_SDA)//读取sda的值
	{
		t++;//进入循环时t自增
		if(t>250)//如果大于250次，说明程序错误返回1报错
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;//收完 ACK，拉低时钟，结束这一拍
	return 0;
}

void IIC_Ack(void)//主机发送应答（ACK）
{
	IIC_SCL = 0;//拉低scl 开辟窗口
	SDA_OUT();//输出
	
	IIC_SDA = 0;//更改sda
	delay_us(2);
	
	IIC_SCL = 1;//拉高scl
	delay_us(2);
	IIC_SCL = 0;//拉低，告诉从机收到ack
}
void IIC_Nack(void)//主机发送非应答（NACK）
{
	IIC_SCL = 0;//拉低
	SDA_OUT();
	delay_us(2);
	
	IIC_SDA = 1;//更改sda 为1时候非应答，为0时应答
	delay_us(2);
	
	IIC_SCL = 1;//拉高
	delay_us(2);
	IIC_SCL = 0;//拉低，告诉从机没收到
}

void IIC_Send_Byte(u8 txd)
{
	u8 t;
	SDA_OUT();//输出
	IIC_SCL = 0;//开辟窗口
	for(t = 0;t<8;t++)//循环遍历
	{
		IIC_SDA = (txd&0x80)>>7;//往右移动七位其余为0
		txd <<= 1;//txd往左移动一位，用来给新数据留位置
		delay_us(2);
		IIC_SCL =1 ;//拉高scl发送
		delay_us(2);
		IIC_SCL = 0;//拉低等待
		delay_us(2);
	}
}

u8 IIC_Read_Byte(u8 ack)
{
	u8 i;
	u8 receive = 0;
	SDA_IN();//接收
	for(i = 0;i<8;i++)//遍历
	{
		IIC_SCL=0;//开辟窗口
		delay_us(2);
		
		IIC_SCL = 1;//拉高
		receive<<=1;//往左移动一位
		if(READ_SDA)receive++;//如果为1在里面添加1，为零就默认，不影响，前面会左移一位
		delay_us(1);
	}
	//- `IIC_Wait_Ack() == 0` → **收到 ACK（从机应答正常）**
	//- `IIC_Wait_Ack() == 1` → **没收到 ACK（失败、无应答）**
	if(!ack) IIC_Nack();//参数 ack=0：读完最后一个字节，发 NACK
	else IIC_Ack();//参数 ack=1：还要继续读，发 ACK
	return receive;//返回值给read_byte
}

