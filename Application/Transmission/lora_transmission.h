#ifndef __LORA_TRANSMISSION_H
#define __LORA_TRANSMISSION_H

#include "buzzer.h"
#include "delay.h"
#include "lora.h"
#include "oled.h"
#include "led.h"
#include "transmission_types.h"
#include "../data/sensor_data.h"
#include "../data/sensor_protocol.h"
#include "../data/control_protocol.h"

/* LoRa模块配置参数定义 */
#define DEMO_ADDR     2                       /* 设备地址 */
#define DEMO_WLRATE   LORA_WLRATE_19K2        /* 空中速率 */
#define DEMO_CHANNEL  20                      /* 信道 */
#define DEMO_TPOWER   LORA_TPOWER_20DBM       /* 发射功率 */
#define DEMO_WORKMODE LORA_WORKMODE_NORMAL    /* 工作模式 */
#define DEMO_TMODE    LORA_TMODE_TT           /* 发射模式 */
#define DEMO_WLTIME   LORA_WLTIME_1S          /* 休眠时间 */
#define DEMO_UARTRATE LORA_UARTRATE_115200BPS /* UART通讯波特率 */
#define DEMO_UARTPARI LORA_UARTPARI_NONE

uint8_t transmission_lora_init(void);
uint8_t transmission_lora_send(const sensor_data_t *data); // 仅 LoRa
uint8_t transmission_lora_receive(sensor_data_t *out_data);
uint8_t transmission_lora_send_control(const control_cmd_t *cmd);
/* 节点使用此入口；网关仍调用 transmission_lora_receive 接收传感器数据。 */
uint8_t transmission_lora_receive_control(control_cmd_t *out_cmd);
uint8_t transmission_lora_is_ready(void); // 检查 LoRa 模块是否空闲可发送

#endif
