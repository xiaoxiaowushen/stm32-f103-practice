#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#include "sys.h"

extern  u8 playback;
extern u8 play_index;
//extern的用法
// 在 .c 文件里定义（分配内存）：      u8 playback = 0;
// 在 .h 文件里声明（告诉编译器有）：   extern u8 playback;

// 其他 .c 文件 include 这个 .h
// → 编译器知道"有这个变量，在别处定义"
// → 链接器负责把它们连起来

void handle_key(void);
void redraw_screen(void);

#endif