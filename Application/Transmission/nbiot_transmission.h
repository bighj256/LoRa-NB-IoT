#ifndef __NBIOT_TRANSMISSION_H
#define __NBIOT_TRANSMISSION_H

#include "buzzer.h"
#include "delay.h"
#include "nbiot.h"
#include "oled.h"
#include "led.h"
#include "transmission_types.h"
#include "../data/sensor_data.h"
#include "../data/sensor_protocol.h"
#include "../data/control_protocol.h"

#define CONTROL_MQTT_TOPIC "farm/control/set"

typedef struct {
    uint8_t connected;
    int8_t csq;
} nbiot_status_t;

uint8_t transmission_nbiot_init(void);
uint8_t transmission_nbiot_send(const sensor_data_t *data); // 仅 NB-IoT
uint8_t transmission_nbiot_receive(sensor_data_t *out_data);
/* 与传感器接收共用 MQTT 队列，同一主循环只调用一种接收入口。 */
uint8_t transmission_nbiot_receive_control(control_cmd_t *out_cmd);
nbiot_status_t transmission_nbiot_get_status(void);

#endif
