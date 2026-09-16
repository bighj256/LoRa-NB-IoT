#ifndef __sensor_data_H
#define __sensor_data_H

#include <stdint.h>

/* 传感器数据结构体 */
typedef struct {
    float air_temp;  /* 空气温度 (°C) */
    float air_humi;  /* 空气湿度 (%RH) */
    float soil_humi; /* 土壤湿度 (0.0 ~ 100.0%) */
    float light;     /* 光照强度 (lux) */
    float ph;        /* 土壤/水体 pH 值 (0.00 ~ 14.00) */
    uint16_t co2;    /* 二氧化碳浓度 (ppm) */

    uint32_t timestamp; /* 时间戳 */
    uint8_t data_valid; /* 数据有效性标志 */
} sensor_data_t;

#endif
