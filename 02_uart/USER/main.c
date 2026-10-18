#include "stm32f10x.h"
#include "usart.h"              

void delay(u32 count)
{
    u32 i;
    for(i = 0; i < count; i++);
}

int main(void)
{
    uart_init(9600);            

    while(1)
    {
        printf("hello\r\n");    
        delay(3000000);
    }
}
