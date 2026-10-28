#include "24cxx.h" 
#include "delay.h"

u8 AT24CXX_ReadOneByte(u16 addr)
{
	u8 dat;
	
	IIC_Start();
	IIC_Send_Byte(0xA0);//只读
	
	IIC_Wait_Ack();
	
	IIC_Send_Byte(addr % 256);//
	IIC_Wait_Ack();
	
	IIC_Start();
	IIC_Send_Byte(0xA1);
	IIC_Wait_Ack();
	
	dat = IIC_Read_Byte(0);
	IIC_Stop();
	return dat;
}
void AT24CXX_WriteOneByte(u16 addr,u8 dat)//
{
	IIC_Start();
	IIC_Send_Byte(0xA0);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(addr % 256);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(dat);//
	IIC_Wait_Ack();
	
	IIC_Stop();
	delay_us(5);
	
}











