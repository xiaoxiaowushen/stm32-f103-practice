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
	GPIO_Init(GPIOC, &s);
	IIC_SCL = 1;
	
	IIC_SDA = 1;
	//表示空闲状态
}
void IIC_Start(void)
{
	SDA_OUT();
	IIC_SCL = 1;
	IIC_SDA = 1;
	delay_us(4);

	IIC_SDA = 0;
	delay_us(4); 
	IIC_SCL = 0;
	delay_us(4);

	
}
//- Start：SCL=1 不变，SDA：1 → 0 （下降沿，开始）
void IIC_Stop(void)
{
	SDA_OUT();//设置为SDA线输出状态,主机掌控sda
	IIC_SCL = 0;//拉低电平
	IIC_SDA = 0;//
	delay_us(4);
	
	IIC_SCL = 1;
	delay_us(4); 
	IIC_SDA = 1;
	
	delay_us(4);
	//- Stop： SCL=1 不变，SDA：0 → 1 （上升沿，结束）
}

u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;//设置一个变量在这里做计数器
	SDA_IN();//设置SAD模式为输入，由从机控制
	IIC_SDA = 1;//SDA写在这里是为了释放 
	delay_us(1);
	IIC_SCL = 1;//然后拉高，开辟新的窗口，为了让从机答复
	delay_us(1);
	while(READ_SDA)//读取SDA的值,读到1说明从机没有给出ack应答
	{
		ucErrTime++;//进入循环先记时+1
		if(ucErrTime>250)//如果超过250次计数，说明有问题
		{
			IIC_Stop();
			return 1;//返回1错误
		}
	}
	IIC_SCL = 0;//收到 ACK，跳出循环，正常
	return 0;
}

void IIC_Ack(void)//主机输出 SDA，主机发给从机 ACK（主机读取从机字节的时候用）
{
	IIC_SCL = 0;//拉低时钟，SCL低时候才能修改SDA
	SDA_OUT();//修改为输出模式，现在是主机掌握SDA线
	IIC_SDA = 0;
	delay_us(2);
	IIC_SCL = 1;//拉高SCL，告诉从机收到这个字节了
	delay_us(2);
	IIC_SCL = 0;//继续发数据
	
}   

void IIC_Nack(void)
{
	IIC_SCL = 0;//拉低修改SCL
	SDA_OUT();
	IIC_SDA = 1;//修改为SDA
	delay_us(2);
	IIC_SCL=1;//SCL拉高，开辟窗口
	delay_us(2);
	IIC_SCL=0;//结束，不再接受信号
	
}
void IIC_Send_Byte(u8 txd)
{
	u8 t;
	SDA_OUT();
	IIC_SCL = 0;//拉低时钟进行数据传输
	for(t = 0;t < 8;t++)//发送8个比特的数据
	{
		IIC_SDA=(txd&0x80)>>7;
		//只保留 txd 的最高位，其余清零 这里我想是先取首位 往右边移动七位变成 0000 0001付给IIC_SDA
		txd <<= 1;//txd 往左移动一位 因为那个0000 0001取的是首位 所以要把他往前面移动 然后变成0000 0010 新的最右位继续写入
		delay_us(2);   
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
	}
}

u8 IIC_Read_Byte(unsigned char ack)
{
	
	unsigned char i;
	unsigned char receive = 0;
	SDA_IN();//SDA设置为输入，由从机发送
	for(i=0;i<8;i++ )
	{
    IIC_SCL=0;//拉低修改 
        delay_us(2);
		IIC_SCL=1;//拉高开辟窗口
        receive<<=1;//已经收到的bit全部左移一格，右边空出最低位，准备放新读到的bit
        if(READ_SDA)receive++;//如果读到SDA=1，空位填1；读到SDA=0，就不加，空位保持0
		delay_us(1); 
    }					 
    if (!ack)//如果ack = 0
        IIC_Nack();//执行nack函数，说明结束
    else
        IIC_Ack(); //继续接收
    return receive;
}
/* 函数声明放在 main 前面 */
u8 AT24CXX_ReadOneByte(u16 addr);

u8 AT24CXX_ReadOneByte(u16 addr)//读取 AT24C02 里面指定地址 addr 上的 1 个字节数据
{
    u8 dat;

    IIC_Start();
    IIC_Send_Byte(0xA0);//- `0xA0`：写方向指令
												//- `0xA1`：读方向指令
	
    IIC_Wait_Ack();

    IIC_Send_Byte(addr % 256);      //只是告诉芯片：我要访问addr这个位置，定位
    IIC_Wait_Ack();

    IIC_Start();                     //重复start
    IIC_Send_Byte(0xA1);            //告诉芯片我要读取数据了
    IIC_Wait_Ack();

    dat = IIC_Read_Byte(0);         //读取芯片发过来的字节，读到的数据放进dat
    IIC_Stop();                      //停止

    return dat;
}

void AT24CXX_WriteOneByte(u16 addr, u8 dat)
{
    IIC_Start();
    IIC_Send_Byte(0xA0);            
    IIC_Wait_Ack();

    IIC_Send_Byte(addr % 256);      
    IIC_Wait_Ack();

    IIC_Send_Byte(dat);         
    IIC_Wait_Ack();

    IIC_Stop();
}
int main(void)
{
    u8 dat;

    delay_init();
    uart_init(9600);
    IIC_Init();

    printf("write 0x55 to addr 10\r\n");
    AT24CXX_WriteOneByte(10, 0x55);
    delay_ms(5);

    dat = AT24CXX_ReadOneByte(10);
    printf("addr 10 = 0x%02X\r\n", dat);

    while(1);
}
