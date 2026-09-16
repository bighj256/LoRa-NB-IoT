#ifndef __TRANSMISSION_TYPES_H
#define __TRANSMISSION_TYPES_H
#include <stdint.h>
/* ─── 发送状态码 ─── */
#define TRANS_OK        0 /* 发送成功 */
#define TRANS_ERROR     1 /* 发送失败（硬件/通信异常） */
#define TRANS_BUSY      2 /* 模块正忙，稍后重试 */
#define TRANS_NO_DATA   3 /* 无有效传感器数据可发送 */
#define TRANS_TRUNCATED 4 /* 数据过长，已截断发送 */

/* ─── 接收状态码 ─── */
#define TRANS_RECV_OK          0 /* 接收成功，数据有效 */
#define TRANS_RECV_NO_DATA     1 /* 无新数据到达 */
#define TRANS_RECV_PARSE_ERROR 2 /* 接收到数据但 JSON 解析失败 */

/* ── 通信状态结构体 ── */
typedef struct {
    uint8_t lora_ready;      /* LoRa 模块空闲可发送 (1: 就绪, 0: 忙) */
    uint8_t nbiot_connected; /* NB-IoT 是否已连接服务器 (1: 已连接, 0: 未连接) */
    int8_t nbiot_rssi;       /* NB-IoT 信号强度 (dBm)，或直接存储 CSQ 索引值 */
} comm_status_t;

#endif // !__TRANSMISSION_TYPES_H
