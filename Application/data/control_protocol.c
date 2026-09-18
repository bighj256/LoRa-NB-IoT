#include "control_protocol.h"
#include <stdio.h>
#include <string.h>

static const struct {
    control_device_t device;
    const char *json_name;
} device_names[] = {
    { CONTROL_DEVICE_FAN,   "\"fan\"" },
    { CONTROL_DEVICE_PUMP,  "\"pump\"" },
    { CONTROL_DEVICE_LIGHT, "\"light\"" },
    { CONTROL_DEVICE_VALVE, "\"valve\"" },
    { CONTROL_DEVICE_RELAY, "\"relay\"" }
};

static void skip_whitespace(const char **cursor, const char *end)
{
    while (*cursor < end && (**cursor == ' ' || **cursor == '\t' ||
                            **cursor == '\r' || **cursor == '\n'))
        (*cursor)++;
}

/* 仅在完整匹配时前移，所有读取均限制在输入长度内。 */
static int consume(const char **cursor, const char *end, const char *text)
{
    size_t len = strlen(text);
    if ((size_t)(end - *cursor) < len || memcmp(*cursor, text, len) != 0)
        return 0;
    *cursor += len;
    return 1;
}

int control_protocol_parse(const char *json, size_t len, control_cmd_t *out_cmd)
{
    const char *cursor;
    const char *end;
    control_cmd_t command = { CONTROL_DEVICE_UNKNOWN, CONTROL_STATE_OFF };
    unsigned int seen = 0;
    unsigned int field;
    size_t i;

    if (json == NULL || out_cmd == NULL || len == 0 || len > CONTROL_JSON_MAX_LEN)
        return 0;
    cursor = json;
    end = json + len;
    skip_whitespace(&cursor, end);
    if (!consume(&cursor, end, "{"))
        return 0;

    for (;;)
    {
        skip_whitespace(&cursor, end);
        if (consume(&cursor, end, "\"device\""))
            field = 1U;
        else if (consume(&cursor, end, "\"state\""))
            field = 2U;
        else
            return 0;
        if (seen & field)
            return 0;
        seen |= field;
        skip_whitespace(&cursor, end);
        if (!consume(&cursor, end, ":"))
            return 0;
        skip_whitespace(&cursor, end);

        if (field == 1U)
        {
            for (i = 0; i < sizeof(device_names) / sizeof(device_names[0]); i++)
            {
                if (consume(&cursor, end, device_names[i].json_name))
                {
                    command.device = device_names[i].device;
                    break;
                }
            }
            if (command.device == CONTROL_DEVICE_UNKNOWN)
                return 0;
        }
        else
        {
            if (consume(&cursor, end, "0"))
                command.state = CONTROL_STATE_OFF;
            else if (consume(&cursor, end, "1"))
                command.state = CONTROL_STATE_ON;
            else
                return 0;
        }

        skip_whitespace(&cursor, end);
        if (consume(&cursor, end, "}"))
            break;
        if (!consume(&cursor, end, ","))
            return 0;
    }

    skip_whitespace(&cursor, end);
    if (seen != 3U || cursor != end)
        return 0;
    *out_cmd = command;
    return 1;
}

int control_protocol_pack(const control_cmd_t *cmd, char *buffer, size_t capacity)
{
    char json[CONTROL_JSON_MAX_LEN + 1U];
    size_t i;
    int len;

    if (cmd == NULL || buffer == NULL || capacity == 0 ||
        (cmd->state != CONTROL_STATE_OFF && cmd->state != CONTROL_STATE_ON))
        return -1;
    for (i = 0; i < sizeof(device_names) / sizeof(device_names[0]); i++)
    {
        if (device_names[i].device == cmd->device)
        {
            len = snprintf(json, sizeof(json), "{\"device\":%s,\"state\":%u}",
                           device_names[i].json_name, (unsigned int)cmd->state);
            if (len < 0 || (size_t)len >= sizeof(json) || (size_t)len >= capacity)
                return -1;
            memcpy(buffer, json, (size_t)len + 1U);
            return len;
        }
    }
    return -1;
}
