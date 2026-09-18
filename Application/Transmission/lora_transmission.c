#include "lora_transmission.h"

static uint8_t lora_startup(uint32_t baudrate);

uint8_t transmission_lora_init(void)
{
    uint8_t ret = lora_startup(115200);
    oled_clear(&oled); // 如果不需要在初始化时清屏，可删除
    return ret == LORA_EOK ? TRANS_OK : TRANS_ERROR;
}

    uint8_t transmission_lora_send(const sensor_data_t *data)
{
    char json_buf[256];
    int len;
    uint8_t ret_lora;

    if (!data->data_valid)
        return TRANS_NO_DATA;

    len = pack_json(data, json_buf, sizeof(json_buf));
    if (len == -1)
        return TRANS_ERROR;

    /* ----- LoRa 发送 ----- */
    if (lora_free() == LORA_EBUSY)
    {
        ret_lora = TRANS_BUSY;
    }
    else
    {
        uint8_t lora_ret = lora_uart_printf("%s\r\n", json_buf);
        switch (lora_ret)
        {
        case LORA_PRINTF_OK:
            ret_lora = TRANS_OK;
            break;
        case LORA_PRINTF_ERR_TRUNCATED:
            ret_lora = TRANS_TRUNCATED;
            break;
        default:
            ret_lora = TRANS_ERROR;
            break;
        }
    }

    /* 根据 LoRa 发送结果返回 */
    return ret_lora;
}

uint8_t transmission_lora_receive(sensor_data_t *out_data)
{
    uint8_t *frame;
    /* ----- LoRa 接收（原有代码） ----- */
    frame = lora_uart_rx_get_frame();
    if (frame != NULL)
    {

        if(!unpack_json((char *)frame, out_data))
        {
            lora_uart_rx_restart();
            return TRANS_RECV_PARSE_ERROR;
        }
        else
        {
            lora_uart_rx_restart();
            return TRANS_RECV_OK;
        }
    }
    return TRANS_RECV_NO_DATA;
}

uint8_t transmission_lora_is_ready(void)
{
    return (lora_free() == LORA_EOK) ? 1 : 0;
}

uint8_t transmission_lora_send_control(const control_cmd_t *cmd)
{
    char json[CONTROL_JSON_MAX_LEN + 1U];
    uint8_t status;
    if (control_protocol_pack(cmd, json, sizeof(json)) < 0)
        return TRANS_ERROR;
    if (lora_free() == LORA_EBUSY)
        return TRANS_BUSY;
    status = lora_uart_printf("%s\r\n", json);
    if (status == LORA_PRINTF_OK)
        return TRANS_OK;
    return status == LORA_PRINTF_ERR_TRUNCATED ? TRANS_TRUNCATED : TRANS_ERROR;
}

uint8_t transmission_lora_receive_control(control_cmd_t *out_cmd)
{
    uint8_t *frame;
    size_t len;
    int parsed;
    if (out_cmd == NULL)
        return TRANS_RECV_PARSE_ERROR;
    frame = lora_uart_rx_get_frame();
    if (frame == NULL)
        return TRANS_RECV_NO_DATA;
    len = lora_uart_rx_get_frame_len();
    /* 传输分隔符不计入协议正文的长度上限。 */
    while (len > 0 && (frame[len - 1] == '\r' || frame[len - 1] == '\n'))
        len--;
    parsed = control_protocol_parse((const char *)frame, len, out_cmd);
    lora_uart_rx_restart();
    return parsed ? TRANS_RECV_OK : TRANS_RECV_PARSE_ERROR;
}

static uint8_t lora_startup(uint32_t baudrate)
{
    uint8_t ret;

    /* 显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);      /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT); /* 透明画刷 */
    oled_set_cursor(&oled, 0, 25);
    oled_draw_string(&oled, "*******************");
    oled_set_cursor(&oled, 0, 40);
    oled_draw_string(&oled, "<<<<   LoRa   >>>>");
    oled_set_cursor(&oled, 0, 55);
    oled_draw_string(&oled, "*******************");
    oled_send_buffer(&oled); // 立即显示
    delay_ms(500);           // 保留必要延时

    oled_clear(&oled);
    /* 1. 硬件初始化 */
    ret = lora_init(baudrate);
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 25);
        oled_draw_string(&oled, "LoRa err!");
        oled_send_buffer(&oled);
        led0_toggle(); // 错误指示

        return ret;
    }

    /* 2. 进入配置模式 */
    lora_enter_config();

    /* 3. 逐一配置参数，累加错误码 */
    ret = lora_addr_config(DEMO_ADDR);
    ret += lora_wlrate_channel_config(DEMO_WLRATE, DEMO_CHANNEL);
    ret += lora_tpower_config(DEMO_TPOWER);
    ret += lora_workmode_config(DEMO_WORKMODE);
    ret += lora_tmode_config(DEMO_TMODE);
    ret += lora_wltime_config(DEMO_WLTIME);
    ret += lora_uart_config(DEMO_UARTRATE, DEMO_UARTPARI);

    /* 4. 退出配置模式 */
    lora_exit_config();

    /* 5. 检查配置是否全部成功 */
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 25);
        oled_draw_string(&oled, "LoRa err!");
        oled_send_buffer(&oled);
        return ret;
    }
    else
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "LoRa ok!");
        oled_send_buffer(&oled);
        delay_ms(500); // 让用户看到成功信息
    }

    return LORA_EOK;
}
