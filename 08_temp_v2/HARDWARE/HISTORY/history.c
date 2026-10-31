#include "history.h"
#include "24cxx.h" 

void SaveTemp(u32 t)
{
    u8 wp;
    u16 addr;

    wp = AT24CXX_ReadOneByte(204);

    if(wp>=100)
    {
        wp = 0;
        AT24CXX_WriteOneByte(204,0);
    }
    addr = 4 + wp *2;
    
    AT24CXX_WriteOneByte(addr,t>>8);
    AT24CXX_WriteOneByte(addr+1,t&0xFF);

    wp++;
    if(wp>=100)wp=0;
    AT24CXX_WriteOneByte(204,wp);
}
u32 ReadTemp(u8 index)
{
    u16 addr;
    u32 temp;

    if(index > 99) return 0;

    addr = 4 + index * 2;
    temp = (AT24CXX_ReadOneByte(addr) << 8) | AT24CXX_ReadOneByte(addr + 1);

    return temp;
}
