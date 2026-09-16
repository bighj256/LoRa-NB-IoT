#ifndef __sensor_protocol_H
#define __sensor_protocol_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../Transmission/transmission_types.h"
#include "sensor_data.h"

int pack_json(const sensor_data_t *data, char *buf, uint16_t size);
int unpack_json(const char *json, sensor_data_t *out_data);

#endif
