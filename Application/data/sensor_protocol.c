#include "sensor_protocol.h"

int pack_json(const sensor_data_t *data, char *buf, uint16_t size)
{
    if (!data->data_valid)
    {
        return -1;
    }

    int len = snprintf(buf, size,
                       "{\"temp\":%.1f,\"air_humi\":%.1f,\"soil_humi\":%.1f,"
                       "\"light\":%.1f,\"ph\":%.1f,\"co2\":%hu,\"time\":%lu}",
                       data->air_temp,
                       data->air_humi,
                       data->soil_humi,
                       data->light,
                       data->ph,
                       data->co2,
                       data->timestamp);
    if(len < 0 || len >= size)
        return -1;

    return len;
}
int unpack_json(const char *json, sensor_data_t *out_data)
{
    int fields;
    sensor_data_t sensor_temp;
    memset(&sensor_temp, 0, sizeof(sensor_temp));
    fields = sscanf(json, "{\"temp\":%f,\"air_humi\":%f,\"soil_humi\":%f,"
                          "\"light\":%f,\"ph\":%f,\"co2\":%hu,\"time\":%lu}",
                    &sensor_temp.air_temp, &sensor_temp.air_humi, &sensor_temp.soil_humi,
                    &sensor_temp.light, &sensor_temp.ph, &sensor_temp.co2, &sensor_temp.timestamp);
    if(fields == 7)
    {
        sensor_temp.data_valid = 1;
        *out_data = sensor_temp;
        return 1;
    }
    else
    {
        return 0;
    }
}
