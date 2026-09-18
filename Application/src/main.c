/**
 * @file    main.c
 * @brief   主程序入口 - LoRa/NB-IoT 农业监测系统
 * @date    2026-07-13
 */
#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "buzzer.h"
#include "display.h"
#include "acquisition.h"
#include "transmission.h"
#include "control.h"
#include "../Transmission/lora_transmission.h"
#include "../Transmission/nbiot_transmission.h"
//#include "../../Examples/nbiot_demo.h"
// #include "sht30_demo.h"
// #include "lora_demo.h"


/* ──────── 设备选择 ──────── */
/* 节点构建定义 DEVICE_SENDER；网关构建定义 DEVICE_RECEIVER（默认）。 */
#if !defined(DEVICE_SENDER) && !defined(DEVICE_RECEIVER)
#define DEVICE_RECEIVER
#endif
#if defined(DEVICE_SENDER) && defined(DEVICE_RECEIVER)
#error "Select only one device role"
#endif

/****************************************************************
 * 共用初始化函数（避免重复代码）
 ****************************************************************/
static void system_common_init(void)
{
    /* 系统初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    sys_stm32_clock_init(9);        /* 72MHz */
    systick_init();
    
    /* 外设初始化 */
    oled_init();
    led_init();
    buzzer_init();
    key_init();
}

/****************************************************************
 * 发送端 main 函数
 ****************************************************************/
#ifdef DEVICE_SENDER
int main(void)
{
    system_common_init();

    // /* 运行 SHT30 测试 demo */
    //sht30_demo_run();

    /* ─── 初始化硬件与通信 ─── */
    transmission_lora_init();      /* 发送端仅初始化 LoRa 模块 */
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    acquisition_init();            /* 初始化传感器采集外设 */
    
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    /* ─── 运行状态变量 ─── */
    sensor_data_t local_data = {0};
    comm_status_t comm_status = {0};
    
    /* 调度器时间锚点 (单位: 毫秒) */
    uint32_t last_collect_ms = 0;
    uint32_t last_send_ms = 0;
    const uint32_t COLLECT_INTERVAL_MS = 1000;  /* 采集周期: 1秒 */
    const uint32_t SEND_INTERVAL_MS = 5000;     /* 发送周期: 5秒 */

    while (1)
    {
        /* 1. 获取系统运行时间 (用于非阻塞定时调度) */
        uint32_t sys_uptime_ms = get_ms();
        uint8_t key_val = key_scan_noblock(sys_uptime_ms);

        /* 优先处理下行；仅在节点操作 PC13，网关 PC13 已用于 NB-IoT 供电。 */
        if (control_node_poll() == TRANS_ERROR)
            display_error("Control error");

        /* 2. 定时采集传感器数据 */
        if (sys_uptime_ms - last_collect_ms >= COLLECT_INTERVAL_MS) 
        {
            last_collect_ms = sys_uptime_ms;
            if (acquisition_poll() == ACQ_OK) 
            {
                acquisition_read(&local_data);
                local_data.data_valid = 1;
                /* * 架构注意：发送端不具备联网能力，无需关心真实时间戳。
                 * 这里的 local_data.timestamp 保持默认的 0 即可。
                 * 真实时间会在数据抵达接收端网关时，由网关向基站索取并统一打标。
                 */
            } 
            else 
            {
                local_data.data_valid = 0;
            }
        }

        /* 3. 定时通过 LoRa 发射数据 */
        if (sys_uptime_ms - last_send_ms >= SEND_INTERVAL_MS) 
        {
            last_send_ms = sys_uptime_ms;
            
            if (local_data.data_valid) 
            {
                uint8_t lora_send_status = transmission_lora_send(&local_data); 
                
                if (lora_send_status == TRANS_OK) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
                } 
                else if (lora_send_status == TRANS_TRUNCATED) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_ALARM, 200);
                }
            }
        }

        /* 4. 更新 OLED UI 显示 */
        // comm_status = get_comm_status();
        display_sensor_data(&local_data, &comm_status, key_val);

        /* 休眠等待硬件中断唤醒，极大降低传感器节点功耗 */
        __WFI();
    }
}
#endif /* DEVICE_SENDER */


/****************************************************************
 * 接收端 main 函数
 ****************************************************************/
#ifdef DEVICE_RECEIVER
int main(void)
{
    system_common_init();
    
    //lora_demo_run();
    //nbiot_demo();

    /* ─── 初始化通信模块 ─── */
    transmission_lora_init();      /* 恢复：接收端必须初始化 LoRa 才能监听节点数据 */
    transmission_nbiot_init();     /* 初始化 NB-IoT 并建立 MQTT 连接 */
    
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    /* ─── 运行状态变量 ─── */
    sensor_data_t display_data = {0};
    comm_status_t comm_status = {0};

    while (1)
    {
        /* 1. 获取系统运行时间 (毫秒)，用于按键消抖等硬件级非阻塞调度 */
        uint32_t sys_uptime_ms = get_ms();
        uint8_t key_val = key_scan_noblock(sys_uptime_ms);

        /* 2. 先接收控制指令，再执行可能等待 AT 应答的状态查询。 */
        if (control_gateway_poll() == TRANS_ERROR)
            display_error("Control error");

        /* 获取底层通信状态 */
        comm_status = get_comm_status();

        /* 3. 轮询接收 LoRa 节点数据 */
        uint8_t lora_recv_status = transmission_lora_receive(&display_data);

        if (lora_recv_status == TRANS_RECV_OK) 
        {
            // 收到新数据，刷新屏幕并鸣响蜂鸣器
            display_sensor_data(&display_data, &comm_status, key_val);
            buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
            
            /* 获取真实世界时间 (Unix时间戳)，打上时标用于云端数据库入库 */
            uint32_t unix_timestamp = 0;
            if (nbiot_get_timestamp(&unix_timestamp) == NBIOT_EOK) {
                display_data.timestamp = unix_timestamp;  
            }

            /* 将带时间戳的完整数据通过 NB-IoT 转发至云端 */
            uint8_t nbiot_send_status = transmission_nbiot_send(&display_data);
            if (nbiot_send_status != TRANS_OK) 
            {
                display_error("NB Send Fail");
            }
        } 
        else if (lora_recv_status == TRANS_RECV_PARSE_ERROR) 
        {
            display_error("Parse Error");
        } 
        else 
        {
            /* 4. 无新数据时，仅刷新 UI 的通信信号状态和按键响应 */
            display_sensor_data(&display_data, &comm_status, key_val);
        }

        /* 处理查询/上传期间缓存的指令，或重试 LoRa 忙时未发出的指令。 */
        if (control_gateway_poll() == TRANS_ERROR)
            display_error("Control error");

        /* 休眠等待中断，降低系统整体功耗 */
        __WFI();
    }
}
#endif /* DEVICE_RECEIVER */
