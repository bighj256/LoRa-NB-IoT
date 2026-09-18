#ifndef CONTROL_PROTOCOL_H
#define CONTROL_PROTOCOL_H

#include <stddef.h>
#include "control_data.h"

#define CONTROL_JSON_MAX_LEN 63U

/* 格式：{"device":"fan","state":1}，一次控制一个设备。
 * 设备名：fan / pump / light / valve / relay；state 仅接受数字 0 或 1。
 * 两个字段必填，顺序任意；允许 JSON 空白，拒绝未知字段、重复字段及尾随内容。
 * 字段名和设备名使用上述字面值，不支持 Unicode 或反斜杠转义写法。
 * len 为正文长度，不含 '\0'，最多 CONTROL_JSON_MAX_LEN 字节。
 * 输入无需以 '\0' 结尾；成功返回 1，失败返回 0 且不修改 out_cmd。
 * 本模块只解析指令，不操作硬件，也不判断设备是否已安装。
 */
int control_protocol_parse(const char *json, size_t len, control_cmd_t *out_cmd);

/* 编码为紧凑 JSON。capacity 包括结尾 '\0' 的空间。
 * 成功返回正文长度；参数非法或容量不足返回 -1，失败时不修改 buffer。
 */
int control_protocol_pack(const control_cmd_t *cmd, char *buffer, size_t capacity);

#endif
