#ifndef _JSON_DATA_H
#define _JSON_DATA_H

#include "gd32f4xx.h"                   // Device header

// 设备ID字符串长度（不含结束符）
#define JSON_DEV_ID_LEN			16

// 命令字符串长度（不含结束符）
#define JSON_CMD_NAME_LEN		8

// 继电器数量（按实际硬件修改）
#define JSON_DO_NUM				4

// 单次上报最大数据点数
#define JSON_POINT_MAX			8

// CAN单帧最大字节数
#define JSON_POINT_DATA_MAX		8

// 透传单帧最大字节数
#define JSON_SEND_DATA_MAX		32

// 采集值字符串长度（不含结束符）
#define JSON_VAL_LEN			16

// 报文类型
#define JSON_TYPE_REPORT		"report"	// 终端数据上报
#define JSON_TYPE_CMD			"cmd"		// 服务器命令下发
#define JSON_TYPE_ACK			"ack"		// 终端应答

// 通道编号
#define JSON_CH_CAN				0			// CAN
#define JSON_CH_RS485			1			// RS485
#define JSON_CH_RS232			2			// RS232
#define JSON_CH_DO				3			// 继电器
#define JSON_CH_INVALID			0xFF		// 无效通道

// 命令字
#define JSON_CMD_DO				"do"		// 继电器控制：index(1起)+value(0/1)
#define JSON_CMD_SEND			"send"		// 通道数据下发（透传）：ch+data(hex字符串)
#define JSON_CMD_REPORT			"report"	// 立即上报一次数据
#define JSON_CMD_REBOOT			"reboot"	// 终端复位

// 结果码
#define JSON_RESULT_OK			0
#define JSON_RESULT_ERR			1

/* 工业数据采集终端联网JSON数据格式：
 *
 * 1.数据上报（终端->服务器）：继电器状态 + 各通道采集数据点
 * {"type":"report","devid":"DT-0001","time":1725868800,"do":[1,0,0,1],"data":[
 *  {"ch":"rs485","addr":1,"val":"25.6"},
 *  {"ch":"can","id":291,"data":"0102AABBCCDDEEFF"},
 *  {"ch":"rs232","val":"OK"}]}
 *
 * 2.命令下发（服务器->终端）：
 * 继电器控制（index从1开始，value:0断开/1吸合）：
 * {"type":"cmd","devid":"DT-0001","cmd":"do","index":1,"value":1}
 * 通道数据下发（data为hex字符串，2字符=1字节）：
 * {"type":"cmd","devid":"DT-0001","cmd":"send","ch":"rs485","data":"010300000002"}
 * 立即上报：
 * {"type":"cmd","devid":"DT-0001","cmd":"report"}
 * 终端复位：
 * {"type":"cmd","devid":"DT-0001","cmd":"reboot"}
 *
 * 3.应答（终端->服务器）：result:0成功/1失败
 * {"type":"ack","devid":"DT-0001","cmd":"do","result":0}
 *
 * 字段说明：
 * type   - 报文类型：report/cmd/ack
 * devid  - 终端唯一ID
 * time   - Unix时间戳（秒）
 * do     - 继电器状态数组：0断开，1吸合
 * data   - 数据点数组，每点含：
 *          ch   - 通道名：can/rs485/rs232
 *          addr - RS485设备地址（仅rs485）
 *          val  - 采集值字符串（rs485/rs232）
 *          id   - CAN帧ID十进制（仅can）
 *          data - CAN数据hex字符串（仅can）
 * index  - 继电器序号，从1开始
 * value  - 继电器目标状态：0/1
 * ch     - 下发通道：can/rs485/rs232
 * result - 应答结果：0成功，1失败
 */

// 采集数据点
typedef struct {
	uint8_t ch;                     // 通道：JSON_CH_CAN/RS485/RS232
	uint8_t addr;                   // RS485设备地址（其他通道无效）
	uint16_t can_id;                // CAN帧ID（CAN通道用）
	uint8_t data_len;               // CAN数据字节数
	uint8_t data[JSON_POINT_DATA_MAX];  // CAN数据
	char val[JSON_VAL_LEN + 1];     // 采集值字符串（rs485/rs232用）
} json_point_t;

// 数据上报
typedef struct {
	char devid[JSON_DEV_ID_LEN + 1];    // 终端ID
	uint32_t time;                      // 时间戳（秒）
	uint8_t do_status[JSON_DO_NUM];     // 继电器状态：0断开，1吸合
	json_point_t points[JSON_POINT_MAX];    // 数据点数组
	uint8_t point_num;                  // 数据点个数
} json_report_t;

// 命令下发
typedef struct {
	char devid[JSON_DEV_ID_LEN + 1];    // 终端ID
	char cmd[JSON_CMD_NAME_LEN + 1];    // 命令字
	uint8_t index;                      // 继电器序号（do命令，从1开始）
	uint8_t value;                      // 继电器目标状态（do命令）
	uint8_t ch;                         // 通道（send命令，JSON_CH_x，无效为JSON_CH_INVALID）
	uint8_t data_len;                   // 数据字节数（send命令，hex解码后）
	uint8_t data[JSON_SEND_DATA_MAX];   // 数据（send命令）
} json_cmd_t;

// 命令应答
typedef struct {
	char devid[JSON_DEV_ID_LEN + 1];    // 终端ID
	char cmd[JSON_CMD_NAME_LEN + 1];    // 命令字
	int result;                         // 结果码：0成功，1失败
} json_ack_t;

int json_build_report(char *buf, int buf_size, const json_report_t *rep);	// 生成上报报文（建议buf>=512）
int json_build_ack(char *buf, int buf_size, const json_ack_t *ack);	// 生成应答报文
int json_parse_cmd(const char *str, json_cmd_t *cmd);	// 解析服务器下发的命令，成功返回0，失败返回-1

#endif
