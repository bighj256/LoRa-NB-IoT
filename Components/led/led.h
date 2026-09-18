#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
#include "delay.h"

void led_init(void);                        /* LED初始化函数 */
void led_set(uint8_t on);                   /* PC13 低电平点亮，非阻塞设置 */
void led0_toggle(void);                     /* LED0翻转函数（带死循环） */

#endif 
