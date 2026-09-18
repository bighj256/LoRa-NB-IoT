#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include "../Transmission/transmission_types.h"

/* 网关接收 MQTT fan 指令并转发；LoRa 忙时保留最新目标状态等待下次轮询。
 * 返回 TRANS_OK / TRANS_BUSY / TRANS_ERROR，不代表远端已执行。
 */
uint8_t control_gateway_poll(void);

/* 仅供传感器节点调用：将 LoRa fan 指令映射到 PC13 LED。
 * 返回 TRANS_OK（包含暂无指令）或 TRANS_ERROR。
 */
uint8_t control_node_poll(void);

#endif
