#ifndef __ACQUISITION_H
#define __ACQUISITION_H

#include "delay.h"

#include "buzzer.h"
#include "oled.h"
#include "led.h"

#include "ph4052.h"
#include "bh1750.h"
#include "sht30.h"
#include "sh393.h"
#include "jw01.h"
#include "../data/sensor_data.h"



/* 错误码定义 */
#define ACQ_OK       0                  /* 数据采集成功 */
#define ACQ_ERROR    1                  /* 数据采集失败 */
#define ACQ_NO_DATA  2                  /* 没有读取到数据 */

/* 函数声明 */
void acquisition_init(void);
uint8_t acquisition_poll(void);
uint8_t acquisition_read(sensor_data_t *data);

#endif //  __ACQUISITION_H
