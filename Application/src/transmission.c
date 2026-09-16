#include "transmission.h"
#include "../Transmission/lora_transmission.h"
#include "../Transmission/nbiot_transmission.h"

/* ========== 模块内部状态 ========== */
comm_status_t get_comm_status(void)
{
    comm_status_t comm_status;
    nbiot_status_t nbiot_status = transmission_nbiot_get_status();

    comm_status.lora_ready = transmission_lora_is_ready();
    comm_status.nbiot_connected = nbiot_status.connected;
    comm_status.nbiot_rssi = nbiot_status.csq;

    return comm_status;
}
