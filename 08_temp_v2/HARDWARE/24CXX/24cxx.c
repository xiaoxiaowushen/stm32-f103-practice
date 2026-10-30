#include "24cxx.h" 
#include "delay.h"

u8 AT24CXX_ReadOneByte(u16 addr)
//读取at24cxx里的地址的值，0~255
{
	u8 dat;
	
	IIC_Start();//发送开始的信号给从机
	IIC_Send_Byte(0xA0);//告诉芯片我设备地址 + 写方向（我要设置地址了）
	
	IIC_Wait_Ack();//等待从机回应
	
	IIC_Send_Byte(addr % 256);//任何数字对 256 取余，得到的结果范围一定是：0 ~ 255，刚好就是一个字节，为了防止输入大于255的值，数字小于 256 的时候，%256 等于它本身
	IIC_Wait_Ack();
	
	IIC_Start();//重新发一次
	IIC_Send_Byte(0xA1);//告诉芯片设备地址 + 读方向（我要读数据了）
	IIC_Wait_Ack();//等待从机回应
	
	dat = IIC_Read_Byte(0);//将值存入dat
	IIC_Stop();//告诉从机不再接受数据
	return dat;//返回dat值给函数
}
void AT24CXX_WriteOneByte(u16 addr,u8 dat)//
{
	IIC_Start();//发送信号给从机告诉准备开始
	IIC_Send_Byte(0xA0);//只读
	IIC_Wait_Ack();//等待回复ack，也就是第九位
	
	IIC_Send_Byte(addr % 256);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(dat); // 把要写的数据发出去
	IIC_Wait_Ack();
	
	IIC_Stop();
	delay_ms(5);
	
}











