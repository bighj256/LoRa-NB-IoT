#include "nbiot_transmission.h"

static uint8_t nbiot_startup(uint32_t baudrate);

uint8_t transmission_nbiot_init(void)
{
    uint8_t ret = nbiot_startup(115200);
    oled_clear(&oled);
    return ret == NBIOT_EOK ? TRANS_OK : TRANS_ERROR;
}
uint8_t transmission_nbiot_send(const sensor_data_t *data)
{
    char json_buf[256];
    int len;
    uint8_t ret_nbiot;

    // 增加静态变量，每次发送累加，避免 QoS=1 队列塞满导致的超时
    static uint16_t s_mqtt_msgid = 1;

    if (!data->data_valid)
        return TRANS_NO_DATA;

    // 1. 生成原始 JSON 字符串
    len = pack_json(data, json_buf, sizeof(json_buf));
    if (len == -1)
        return TRANS_ERROR;

    // 2. 直接使用原始 JSON 字符串发送
    uint8_t nb_ret = nbiot_mqtt_publish(0, s_mqtt_msgid, 1, 0, "farm/sensor/collect", json_buf);

    // 4. 发送完毕后更新 msgid，限定在 1 ~ 65535 范围内
    s_mqtt_msgid++;
    if (s_mqtt_msgid == 0)
    {
        s_mqtt_msgid = 1;
    }

    /* 根据 NB-IoT 发送结果映射错误码 */
    switch (nb_ret)
    {
    case NBIOT_EOK:
        ret_nbiot = TRANS_OK;
        break;
    case NBIOT_TIMEOUT:
        ret_nbiot = TRANS_BUSY;
        break;
    default:
        ret_nbiot = TRANS_ERROR;
        break;
    }

    return ret_nbiot;
}

uint8_t transmission_nbiot_receive(sensor_data_t *out_data)
{
    /* ----- NB‑IoT MQTT 接收 ----- */
    nbiot_mqtt_recv_t mqtt_msg;
    if (nbiot_mqtt_recv(&mqtt_msg) == NBIOT_EOK)
    {
        // 解析 MQTT payload 中的 JSON（前提是 payload 是字符串格式）
        if(!unpack_json(mqtt_msg.payload, out_data))
        {
            return TRANS_RECV_PARSE_ERROR;
        }
        else
        {
            return TRANS_RECV_OK;
        }
    }

    return TRANS_RECV_NO_DATA;
}

nbiot_status_t transmission_nbiot_get_status(void)
{
    nbiot_status_t status;
    static int8_t cached_rssi = -1;
    static uint32_t last_csq_time = 0;
    static uint8_t first_call = 1; // 新增：首次运行标志
    uint32_t now = get_ms();

    status.connected = g_nb_mqtt_connected;
    status.csq = cached_rssi;

    // 逻辑：如果是首次运行，或者时间差 >= 10000ms，就触发
    if (first_call || (now - last_csq_time >= 10000))
    {
        first_call = 0; // 触发后立刻清除标志

        nbiot_csq_t csq;
        if (nbiot_get_csq(&csq) == NBIOT_EOK)
        {
            cached_rssi = csq.rssi;
        }
        else
        {
            cached_rssi = -1;
        }
        last_csq_time = now;
        status.csq = cached_rssi;
    }
    return status;
}

static uint8_t nbiot_startup(uint32_t baudrate)
{
    uint8_t ret;
    nbiot_csq_t csq;

    /* 显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);      /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT); /* 透明画刷 */
    oled_set_cursor(&oled, 0, 25);
    oled_draw_string(&oled, "*******************");
    oled_set_cursor(&oled, 0, 40);
    oled_draw_string(&oled, "<<<   NB-IoT   >>>");
    oled_set_cursor(&oled, 0, 55);
    oled_draw_string(&oled, "*******************");
    oled_send_buffer(&oled); // 立即显示
    delay_ms(500);           // 保留必要延时

    // 1. 模块硬件初始化 + AT 测试 + 关闭回显
    ret = nbiot_init(baudrate);
    if (ret != NBIOT_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "NB err!");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_clear(&oled); // 清屏，准备显示进度
    oled_set_cursor(&oled, 30, 40);
    oled_draw_string(&oled, "NB OK!");
    oled_send_buffer(&oled); // 立即刷新

    // 2. 附着网络 (等待网络注册)
    ret = nbiot_attach_network();
    if (ret != NBIOT_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "NB err!");
        oled_send_buffer(&oled);
        return ret;
    }
    delay_ms(3000); // 等待网络附着完成（必须）
    oled_clear(&oled);
    oled_set_cursor(&oled, 0, 12);
    oled_draw_string(&oled, "Attached");
    oled_send_buffer(&oled);

    // 3. 查询信号质量
    nbiot_get_csq(&csq);
    oled_set_cursor(&oled, 0, 24);
    oled_printf(&oled, "CSQ:%d,%d", csq.rssi, csq.ber);
    oled_send_buffer(&oled);

    // 4. 打开 MQTT 连接（socket）
    ret = nbiot_mqtt_open(0, "122.51.36.76", 1883);
    if (ret != NBIOT_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 10, 20);
        oled_draw_string(&oled, "MQTT open fail");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_set_cursor(&oled, 0, 36);
    oled_draw_string(&oled, "MQTT open OK");
    oled_send_buffer(&oled);

    // 5. MQTT 连接
    ret = nbiot_mqtt_connect(0, "myClient123", "user1", "pass123");
    if (ret != NBIOT_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 10, 20);
        oled_draw_string(&oled, "MQTT conn fail");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_set_cursor(&oled, 0, 48);
    oled_draw_string(&oled, "MQTT conn OK");
    oled_send_buffer(&oled);

    // 6. 订阅主题
    nbiot_mqtt_subscribe(0, 1, "farm/sensor/collect", 1);
    oled_set_cursor(&oled, 0, 60);
    oled_draw_string(&oled, "Subscribed");
    oled_send_buffer(&oled);

    // 可选：等待 2 秒后清屏，准备显示传感器数据
    delay_ms(2000);
    oled_clear(&oled);
    oled_send_buffer(&oled);

    return NBIOT_EOK;
}
