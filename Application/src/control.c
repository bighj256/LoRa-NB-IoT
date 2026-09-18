#include "control.h"
#include "lora_transmission.h"
#include "nbiot_transmission.h"
#include "led.h"

uint8_t control_gateway_poll(void)
{
    static control_cmd_t pending;
    static uint8_t has_pending;
    control_cmd_t incoming;
    uint8_t status;
    uint8_t result = TRANS_OK;
    uint8_t i;

    /* 每次最多处理四条；单风扇演示只保留最新目标状态。 */
    for (i = 0; i < 4; i++)
    {
        status = transmission_nbiot_receive_control(&incoming);
        if (status == TRANS_RECV_NO_DATA)
            break;
        if (status != TRANS_RECV_OK || incoming.device != CONTROL_DEVICE_FAN)
        {
            result = TRANS_ERROR;
            continue;
        }
        pending = incoming;
        has_pending = 1;
    }

    if (has_pending)
    {
        status = transmission_lora_send_control(&pending);
        if (status == TRANS_BUSY)
            return result == TRANS_ERROR ? TRANS_ERROR : TRANS_BUSY;
        has_pending = 0;
        if (status != TRANS_OK)
            return TRANS_ERROR;
    }
    return result;
}

uint8_t control_node_poll(void)
{
    control_cmd_t command;
    uint8_t status = transmission_lora_receive_control(&command);
    if (status == TRANS_RECV_NO_DATA)
        return TRANS_OK;
    if (status != TRANS_RECV_OK || command.device != CONTROL_DEVICE_FAN)
        return TRANS_ERROR;

    /* 仅演示风扇开关；PC13 为低电平点亮的板载 LED。 */
    led_set(command.state == CONTROL_STATE_ON);
    return TRANS_OK;
}
