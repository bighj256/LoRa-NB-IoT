#ifndef CONTROL_DATA_H
#define CONTROL_DATA_H

/* 单节点、每类设备各一个，直接通过类型定位设备。 */
typedef enum {
    CONTROL_DEVICE_UNKNOWN = 0,
    CONTROL_DEVICE_FAN,
    CONTROL_DEVICE_PUMP,
    CONTROL_DEVICE_LIGHT,
    CONTROL_DEVICE_VALVE,
    CONTROL_DEVICE_RELAY
} control_device_t;

typedef enum {
    CONTROL_STATE_OFF = 0,
    CONTROL_STATE_ON = 1
} control_state_t;

typedef struct {
    control_device_t device;
    control_state_t state;
} control_cmd_t;

#endif
